//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "container/hash/extendible_hash_table.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_TYPE::ExtendibleHashTable(const std::string &name, BufferPoolManager *buffer_pool_manager,
                                     const KeyComparator &comparator, HashFunction<KeyType> hash_fn)
    : buffer_pool_manager_(buffer_pool_manager), comparator_(comparator), hash_fn_(std::move(hash_fn)) {
  //  implement me!
  auto page = buffer_pool_manager_->NewPage(&directory_page_id_);
  if (page != nullptr) {
    HashTableDirectoryPage *dir_page = reinterpret_cast<HashTableDirectoryPage *>(page);
    dir_page->SetPageId(directory_page_id_);
    page_id_t bucket_id;
    Page *bucket = buffer_pool_manager_->NewPage(&bucket_id);
    if (bucket != nullptr) {
      dir_page->SetBucketPageId(0, bucket_id);
      buffer_pool_manager_->UnpinPage(bucket_id, true);
    }
    buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  }
}

/*****************************************************************************
 * HELPERS
 *****************************************************************************/
/**
 * Hash - simple helper to downcast MurmurHash's 64-bit hash to 32-bit
 * for extendible hashing.
 *
 * @param key the key to hash
 * @return the downcasted 32-bit hash
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::Hash(KeyType key) {
  return static_cast<uint32_t>(hash_fn_.GetHash(key));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToDirectoryIndex(KeyType key, HashTableDirectoryPage *dir_page) {
  uint32_t mask = dir_page->GetGlobalDepthMask();
  uint32_t idx = Hash(key) & mask;
  // LOG_DEBUG("mask is 0x%x and idx is 0x%x",mask,idx);
  return idx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToPageId(KeyType key, HashTableDirectoryPage *dir_page) {
  uint32_t idx = KeyToDirectoryIndex(key, dir_page);
  return dir_page->GetBucketPageId(idx);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HashTableDirectoryPage *HASH_TABLE_TYPE::FetchDirectoryPage() {
  return reinterpret_cast<HashTableDirectoryPage *>(buffer_pool_manager_->FetchPage(directory_page_id_));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_BUCKET_TYPE *HASH_TABLE_TYPE::FetchBucketPage(page_id_t bucket_page_id) {
  return reinterpret_cast<HASH_TABLE_BUCKET_TYPE *>(buffer_pool_manager_->FetchPage(bucket_page_id));
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key, std::vector<ValueType> *result) {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();

  auto page_idx = KeyToDirectoryIndex(key, dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);

  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  Page *page = reinterpret_cast<Page *>(bucket_page);
  page->RLatch();
  bool res = bucket_page->GetValue(key, comparator_, result);
  page->RUnlatch();
  buffer_pool_manager_->UnpinPage(page_id, false);
  buffer_pool_manager_->UnpinPage(directory_page_id_, false);
  table_latch_.RUnlock();
  return res;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  bool result = false;
  auto page_idx = KeyToDirectoryIndex(key, dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  Page *page = reinterpret_cast<Page *>(bucket_page);
  page->WLatch();
  uint32_t bucket_size = 1 << dir_page->GetLocalDepth(page_idx);
  if (bucket_page->NumReadable() < bucket_size) {
    result = bucket_page->Insert(key, value, comparator_);
    // std::cout << "Insert " << key << " , " << value << "ok,the result is " << result << "\n";
    page->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, true);
    table_latch_.RUnlock();
  } else {
    page->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, false);
    table_latch_.RUnlock();
    result = SplitInsert(transaction, key, value);
    // std::cout << "SplitInsert " << key << " , " << value << "ok,the result is " << result << "\n";
  }
  return result;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::SplitInsert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  auto page_idx = KeyToDirectoryIndex(key, dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  Page *page_bucket = reinterpret_cast<Page *>(bucket_page);
  page_bucket->WLatch();
  uint32_t bucket_size = 1 << dir_page->GetLocalDepth(page_idx);
  if (bucket_page->NumReadable() < bucket_size) {
    // LOG_DEBUG("Not need SplitInsert");
    page_bucket->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, false);
    table_latch_.WUnlock();
    return Insert(transaction, key, value);
  }
  if (dir_page->GetLocalDepth(page_idx) >= 9) {
    page_bucket->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, false);
    table_latch_.WUnlock();
    return false;
  }
  if (dir_page->GetGlobalDepth() == dir_page->GetLocalDepth(page_idx)) {
    dir_page->Expand(page_idx);
  }
  page_id_t new_page_id;
  uint32_t new_mask;
  uint32_t old_mask;
  auto new_page = buffer_pool_manager_->NewPage(&new_page_id);
  assert(new_page != nullptr);
  new_page->WLatch();
  HASH_TABLE_BUCKET_TYPE *new_bucket = reinterpret_cast<HASH_TABLE_BUCKET_TYPE *>(new_page);
  old_mask = dir_page->GetLocalDepthMask(page_idx);
  dir_page->IncrLocalDepth(page_idx);
  new_mask = dir_page->GetLocalDepthMask(page_idx);

  for (size_t i = 0; i < dir_page->Size(); i++) {
    if (i == page_idx || page_id != dir_page->GetBucketPageId(i)) {
      continue;
    }
    dir_page->IncrLocalDepth(i);
    auto old_page_idx = old_mask & page_idx;
    auto new_page_idx = new_mask & page_idx;
    if ((old_mask & i) == old_page_idx && (new_mask & i) != new_page_idx) {
      dir_page->SetBucketPageId(i, new_page_id);
    }
  }
  ReHash(page_idx, bucket_page, new_bucket, new_mask);
  new_page->WUnlatch();
  buffer_pool_manager_->UnpinPage(new_page_id, true);
  page_bucket->WUnlatch();
  buffer_pool_manager_->UnpinPage(page_id, true);
  buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  table_latch_.WUnlock();
  return Insert(transaction, key, value);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.RLock();
  bool result = false;
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  auto page_idx = KeyToDirectoryIndex(key, dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  Page *page_bucket = reinterpret_cast<Page *>(bucket_page);

  page_bucket->WLatch();
  result = bucket_page->Remove(key, value, comparator_);
  if (!bucket_page->IsEmpty()) {
    page_bucket->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, true);
    table_latch_.RUnlock();
  } else {
    page_bucket->WUnlatch();
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    buffer_pool_manager_->UnpinPage(page_id, true);
    // LOG_DEBUG("Merge pages because %d",page_id);
    table_latch_.RUnlock();
    Merge(transaction, key, value);
  }
  return result;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Merge(Transaction *transaction, const KeyType &key, const ValueType &value) {
  table_latch_.WLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  auto page_idx = KeyToDirectoryIndex(key, dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  auto bro_page_idx = dir_page->GetBrother(page_idx);
  auto bro_page_id = dir_page->GetBucketPageId(bro_page_idx);

  if (dir_page->GetGlobalDepth() == 0 || dir_page->GetLocalDepth(page_idx) == 0) {
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    table_latch_.WUnlock();
    return;
  }
  if (dir_page->GetLocalDepth(bro_page_idx) != dir_page->GetLocalDepth(page_idx)) {
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    table_latch_.WUnlock();
    return;
  }
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  Page *page_bucket = reinterpret_cast<Page *>(bucket_page);
  page_bucket->RLatch();
  if (!bucket_page->IsEmpty()) {
    page_bucket->RUnlatch();
    buffer_pool_manager_->UnpinPage(page_id, false);
    buffer_pool_manager_->UnpinPage(directory_page_id_, false);
    table_latch_.WUnlock();
    return;
  }
  dir_page->SetBucketPageId(page_idx, bro_page_id);
  dir_page->DecrLocalDepth(page_idx);
  dir_page->DecrLocalDepth(bro_page_idx);

  page_bucket->RUnlatch();
  buffer_pool_manager_->UnpinPage(page_id, true);
  buffer_pool_manager_->DeletePage(page_id);

  auto local_depth = dir_page->GetLocalDepth(bro_page_idx);
  for (size_t i = 0; i < dir_page->Size(); i++) {
    page_id_t tmp_page_id = dir_page->GetBucketPageId(i);
    if (tmp_page_id != page_id && tmp_page_id != bro_page_id) {
      continue;
    }
    dir_page->SetBucketPageId(i, bro_page_id);
    dir_page->SetLocalDepth(i, local_depth);
  }
  while (dir_page->CanShrink()) {
    dir_page->DecrGlobalDepth();
  }
  buffer_pool_manager_->UnpinPage(directory_page_id_, true);
  table_latch_.WUnlock();
}

/*****************************************************************************
 * GETGLOBALDEPTH - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_TYPE::GetGlobalDepth() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  uint32_t global_depth = dir_page->GetGlobalDepth();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
  return global_depth;
}

/*****************************************************************************
 * VERIFY INTEGRITY - DO NOT TOUCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::VerifyIntegrity() {
  table_latch_.RLock();
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  dir_page->VerifyIntegrity();
  assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false, nullptr));
  table_latch_.RUnlock();
}
// move bucket1 to bucket2
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::ReHash(uint32_t idx, HASH_TABLE_BUCKET_TYPE *bucket1, HASH_TABLE_BUCKET_TYPE *bucket2,
                             uint32_t mask) {
  std::vector<MappingType> pairs;
  bucket1->GetAllPairs(&pairs);

  for (auto pair : pairs) {
    auto hash = Hash(pair.first);
    if ((hash & mask) != (idx & mask)) {
      bucket1->Remove(pair.first, pair.second, comparator_);
      bucket2->Insert(pair.first, pair.second, comparator_);
    }
  }  // not
}
/*****************************************************************************
 * TEMPLATE DEFINITIONS - DO NOT TOUCH
 *****************************************************************************/
template class ExtendibleHashTable<int, int, IntComparator>;

template class ExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class ExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class ExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class ExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class ExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
