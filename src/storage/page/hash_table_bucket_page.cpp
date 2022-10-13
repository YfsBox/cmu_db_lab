//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_bucket_page.cpp
//
// Identification: src/storage/page/hash_table_bucket_page.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "storage/page/hash_table_bucket_page.h"
#include "common/logger.h"
#include "common/util/hash_util.h"
#include "storage/index/generic_key.h"
#include "storage/index/hash_comparator.h"
#include "storage/table/tmp_tuple.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_BUCKET_TYPE::BucketIdx2BitIdx(uint32_t bucket_idx,char &bitidx) const {
  auto idx = bucket_idx / 8;
  char bit_idx = 0x01;
  bit_idx <<= (bucket_idx % 8);
  bitidx = bit_idx;
  return idx;
}

template <typename KeyType, typename  ValueType, typename KeyComparator>
size_t HASH_TABLE_BUCKET_TYPE::BitCount(char byte_char) const {
  size_t cnt = 0;
  while (byte_char != 0) {
    cnt++;
    byte_char &= (byte_char - 1);
  }
  return cnt;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::GetValue(KeyType key, KeyComparator cmp, std::vector<ValueType> *result) {
  size_t len = sizeof(array_) / sizeof(MappingType);
  for (size_t i = 0; i < len; i++) {
    if (!cmp(key,array_[i].first) && IsReadable(i)) {
      result->push_back(array_[i].second);
    }
  }
  return !result->empty();
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Insert(KeyType key, ValueType value, KeyComparator cmp) {
  size_t len = sizeof(readable_) / sizeof(char);
  bool is_exsit = false;
  int empty_idx = -1;
  for (size_t i = 0; i < len * 8; i++) {  // 这里越界了
    if (!IsReadable(i) && empty_idx == -1) {
      empty_idx = static_cast<int>(i);
    } else if (cmp(key,array_[i].first) && value == array_[i].second) {
      is_exsit = true;
    }
    LOG_DEBUG("Insert loop %lu",i);
  }
  if (is_exsit || empty_idx == -1) {
    return false;
  }
  array_[empty_idx] = std::make_pair(key,value);
  SetOccupied(empty_idx);
  SetReadable(empty_idx);

  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Remove(KeyType key, ValueType value, KeyComparator cmp) {
  size_t len = sizeof(readable_) / sizeof(char);
  bool result = false;
  for (size_t i = 0; i < len * 8; i++) {
    if (!cmp(key,array_[i].first) && IsReadable(i)) {
      RemoveAt(i);
      result = true;
    }
  }
  return result;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
KeyType HASH_TABLE_BUCKET_TYPE::KeyAt(uint32_t bucket_idx) const {
  return array_[bucket_idx].first;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
ValueType HASH_TABLE_BUCKET_TYPE::ValueAt(uint32_t bucket_idx) const {
  return array_[bucket_idx].second;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::RemoveAt(uint32_t bucket_idx) {
  char bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx,bitidx);
  readable_[idx] &= ~(bitidx);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsOccupied(uint32_t bucket_idx) const {
  char bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx,bitidx);
  return (occupied_[idx] & bitidx) != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetOccupied(uint32_t bucket_idx) {
  char bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx,bitidx);
  occupied_[idx] |= bitidx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsReadable(uint32_t bucket_idx) const {
  char bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx,bitidx);
  return (readable_[idx] & bitidx) != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetReadable(uint32_t bucket_idx) {
  char bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx,bitidx);
  readable_[idx] |= bitidx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsFull() {
  auto is_zero = [](char byte_char) -> bool {
    return byte_char == 0x11;
  };
  auto len = sizeof (readable_) / sizeof(char) ;
  return std::all_of(readable_, readable_ + len, is_zero);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_BUCKET_TYPE::NumReadable() {
  int cnt = 0;
  for (auto readable : readable_) {
    cnt += BitCount(readable);
  }
  return cnt;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsEmpty() {
  auto is_zero = [](char byte_char) -> bool {
    return byte_char == 0;
  };
  auto len = sizeof (readable_) / sizeof(char) ;
  return std::all_of(readable_, readable_ + len, is_zero);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::PrintBucket() {
  uint32_t size = 0;
  uint32_t taken = 0;
  uint32_t free = 0;
  for (size_t bucket_idx = 0; bucket_idx < BUCKET_ARRAY_SIZE; bucket_idx++) {
    if (!IsOccupied(bucket_idx)) {
      break;
    }

    size++;

    if (IsReadable(bucket_idx)) {
      taken++;
    } else {
      free++;
    }
  }

  LOG_INFO("Bucket Capacity: %lu, Size: %u, Taken: %u, Free: %u", BUCKET_ARRAY_SIZE, size, taken, free);
}

// DO NOT REMOVE ANYTHING BELOW THIS LINE
template class HashTableBucketPage<int, int, IntComparator>;

template class HashTableBucketPage<GenericKey<4>, RID, GenericComparator<4>>;
template class HashTableBucketPage<GenericKey<8>, RID, GenericComparator<8>>;
template class HashTableBucketPage<GenericKey<16>, RID, GenericComparator<16>>;
template class HashTableBucketPage<GenericKey<32>, RID, GenericComparator<32>>;
template class HashTableBucketPage<GenericKey<64>, RID, GenericComparator<64>>;

// template class HashTableBucketPage<hash_t, TmpTuple, HashComparator>;

}  // namespace bustub
