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
inline uint32_t HASH_TABLE_BUCKET_TYPE::BucketIdx2BitIdx(uint32_t bucket_idx, uint32_t *bitidx) const {
  auto idx = bucket_idx / 8;
  uint32_t bit_idx = 0x01;
  bit_idx <<= (bucket_idx % 8);
  *bitidx = bit_idx;
  return idx;  // not
}

template <typename KeyType, typename ValueType, typename KeyComparator>
size_t HASH_TABLE_BUCKET_TYPE::BitCount(uint32_t byte_char) const {
  size_t cnt = 0;
  while (byte_char != 0) {
    cnt++;
    byte_char &= byte_char - 1;
  }
  return cnt;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::GetValue(KeyType key, KeyComparator cmp, std::vector<ValueType> *result) {
  for (size_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (IsReadable(i) && !cmp(key, array_[i].first)) {
      result->push_back(array_[i].second);
    }
  }
  return !result->empty();
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::ExsitKv(KeyType key, KeyComparator cmp, ValueType value) {
  std::vector<ValueType> values;
  if (GetValue(key, cmp, &values)) {
    for (auto val : values) {
      if (val == value) {
        return true;
      }
    }
  }
  return false;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Insert(KeyType key, ValueType value, KeyComparator cmp) {
  bool is_exsit = false;
  int empty_idx = -1;
  for (size_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    auto readable = IsReadable(i);
    if (!readable && empty_idx == -1) {
      empty_idx = static_cast<int>(i);
    }
    if (readable && cmp(key, array_[i].first) == 0 && value == array_[i].second) {
      is_exsit = true;
    }
  }
  if (is_exsit || empty_idx == -1) {
    return false;
  }
  array_[empty_idx] = std::make_pair(key, value);
  SetOccupied(empty_idx);
  SetReadable(empty_idx);

  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::Remove(KeyType key, ValueType value, KeyComparator cmp) {
  bool result = false;
  for (size_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (IsReadable(i) && !cmp(key, array_[i].first) && value == array_[i].second) {
      RemoveAt(i);
      result = true;
      break;
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
  uint32_t bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx, &bitidx);
  readable_[idx] &= ~(bitidx);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsOccupied(uint32_t bucket_idx) const {
  uint32_t bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx, &bitidx);
  return (occupied_[idx] & bitidx) != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetOccupied(uint32_t bucket_idx) {
  uint32_t bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx, &bitidx);
  occupied_[idx] |= bitidx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsReadable(uint32_t bucket_idx) const {
  uint32_t bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx, &bitidx);
  return (readable_[idx] & bitidx) != 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::SetReadable(uint32_t bucket_idx) {
  uint32_t bitidx;
  auto idx = BucketIdx2BitIdx(bucket_idx, &bitidx);
  readable_[idx] |= bitidx;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsFull() {
  auto is_zero = [](char byte_char) -> bool { return byte_char == 0x11; };
  auto len = sizeof(readable_) / sizeof(char);
  return std::all_of(readable_, readable_ + len, is_zero);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
uint32_t HASH_TABLE_BUCKET_TYPE::NumReadable() {
  uint32_t cnt = 0;
  for (size_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (IsReadable(i)) {
      cnt += 1;
    }
  }
  return cnt;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_BUCKET_TYPE::IsEmpty() {
  uint8_t mask = 255;
  for (size_t i = 0; i < sizeof(readable_) / sizeof(readable_[0]); i++) {
    if ((readable_[i] & mask) > 0) {
      return false;
    }
  }
  return true;
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

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BUCKET_TYPE::GetAllPairs(std::vector<std::pair<KeyType, ValueType>> *result) const {
  for (size_t i = 0; i < BUCKET_ARRAY_SIZE; i++) {
    if (IsReadable(i)) {
      result->push_back(array_[i]);
    }
  }
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
