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
#include <cmath>

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
    HashTableDirectoryPage *dir_page = reinterpret_cast<HashTableDirectoryPage*>(page);
    dir_page->SetPageId(directory_page_id_);
    page_id_t bucket_id;
    Page *bucket = buffer_pool_manager_->NewPage(&bucket_id);
    if (bucket != nullptr) {
      dir_page->SetBucketPageId(0, bucket_id);
    }
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
  LOG_DEBUG("mask is 0x%x and idx is 0x%x",mask,idx);
  return idx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
inline uint32_t HASH_TABLE_TYPE::KeyToPageId(KeyType key, HashTableDirectoryPage *dir_page) {
  uint32_t idx = KeyToDirectoryIndex(key,dir_page);
  return dir_page->GetBucketPageId(idx);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HashTableDirectoryPage *HASH_TABLE_TYPE::FetchDirectoryPage() {
  return reinterpret_cast<HashTableDirectoryPage*>(buffer_pool_manager_->FetchPage(directory_page_id_));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_BUCKET_TYPE *HASH_TABLE_TYPE::FetchBucketPage(page_id_t bucket_page_id) {
  return reinterpret_cast<HASH_TABLE_BUCKET_TYPE*>(buffer_pool_manager_->FetchPage(bucket_page_id));
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key, std::vector<ValueType> *result) {
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  auto page_id = KeyToPageId(key,dir_page);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  //bucket_page->PrintBucket();
  return bucket_page->GetValue(key,comparator_,result);
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  // 首先定位到应该插入的位置
  LOG_DEBUG("the insert key hash is 0x%x", Hash(key));
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  dir_page->PrintDirectory();
  auto page_idx = KeyToDirectoryIndex(key,dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  // 判断容量是否足够
  uint32_t bucket_size = static_cast<uint32_t>(pow(2,dir_page->GetLocalDepth(page_idx)));
  if (bucket_page->NumReadable() < bucket_size) {  // 直接插入的情况
    return bucket_page->Insert(key,value,comparator_);
  }
  if (dir_page->GetGlobalDepth() > dir_page->GetLocalDepth(page_idx)) {  // 直接进行扩容的情况
    //dir_page->IncrLocalDepth(page_idx);
    for (size_t i = 0; i < dir_page->Size(); i++) {
      if (dir_page->GetBucketPageId(i) == page_id) {
        dir_page->IncrLocalDepth(i);
      }
    }
    return bucket_page->Insert(key,value,comparator_);
  }
  return SplitInsert(transaction,key,value);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::SplitInsert(Transaction *transaction, const KeyType &key, const ValueType &value) {
  // 获取bucket_page和dir_page
  bool result;
  HashTableDirectoryPage *dir_page = FetchDirectoryPage();
  auto page_idx = KeyToDirectoryIndex(key,dir_page);
  auto page_id = dir_page->GetBucketPageId(page_idx);
  HASH_TABLE_BUCKET_TYPE *bucket_page = FetchBucketPage(page_id);
  // 获取旧的gdepth和新的size
  size_t old_gdepth = dir_page->GetGlobalDepth();
  size_t new_len = static_cast<size_t> (pow(2,static_cast<double>(old_gdepth) + 1));
  auto old_mask = dir_page->GetGlobalDepthMask();
  //如果已经有这个kv了
  if (bucket_page->ExsitKv(key,comparator_,value)) {
    return false;
  }
  dir_page->IncrGlobalDepth();
  auto new_mask = dir_page->GetGlobalDepthMask();
  for (size_t i = static_cast<size_t>(pow(2,static_cast<double>(old_gdepth))); i < new_len; i++) {
    auto mask_i = old_mask & i;  // 取旧depth的位数
    auto local_depth = dir_page->GetLocalDepth(mask_i);
    dir_page->SetLocalDepth(i,local_depth);  // 设置localdepth
    if (mask_i == page_idx) {
      // 分配一个newpage
      page_id_t new_page_id;
      if (auto new_ok = buffer_pool_manager_->NewPage(&new_page_id);new_ok) {
        auto new_page = FetchBucketPage(new_page_id);
        dir_page->SetBucketPageId(i,new_page_id);
        dir_page->IncrLocalDepth(page_idx);
        dir_page->IncrLocalDepth(i);
        // 还剩rehash和insert
        ReHash(page_idx,bucket_page,new_page,new_mask);
        if ((Hash(key) & new_mask) == page_idx) {
          result = bucket_page->Insert(key,value,comparator_);
        } else {
          result = new_page->Insert(key,value,comparator_);
        }
      } else {
        return false;
      }
    } else {
      page_id_t pg_id = dir_page->GetBucketPageId(mask_i);
      dir_page->SetBucketPageId(i,pg_id);
    }
  }
  return result;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key, const ValueType &value) {
  return false;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Merge(Transaction *transaction, const KeyType &key, const ValueType &value) {}

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
void HASH_TABLE_TYPE::ReHash(uint32_t idx,HASH_TABLE_BUCKET_TYPE *bucket1, HASH_TABLE_BUCKET_TYPE *bucket2,uint32_t mask) {
  std::vector<MappingType> pairs;
  bucket1->GetAllPairs(&pairs);

  for (auto pair : pairs) {
    auto hash = Hash(pair.first);
    if ((hash & mask) != idx) {
     bucket1->Remove(pair.first,pair.second,comparator_);
     bucket2->Insert(pair.first,pair.second,comparator_);
    }
  }
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

// (1) (0,0) 1100 -> bucket 1
// (2) ()