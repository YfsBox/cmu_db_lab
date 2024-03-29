//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_header_page.cpp
//
// Identification: src/storage/page/hash_table_header_page.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/hash_table_directory_page.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "common/logger.h"

namespace bustub {

static uint32_t GetUpbitmask(uint32_t n) {
  if (n == 0) {
    return 0;
  }
  uint32_t mask = 0x1;
  return mask << (n - 1);
}

static uint32_t Upbit(uint32_t str, uint32_t n) { return str ^ GetUpbitmask(n); }

page_id_t HashTableDirectoryPage::GetPageId() const { return page_id_; }

void HashTableDirectoryPage::SetPageId(bustub::page_id_t page_id) { page_id_ = page_id; }

lsn_t HashTableDirectoryPage::GetLSN() const { return lsn_; }

void HashTableDirectoryPage::SetLSN(lsn_t lsn) { lsn_ = lsn; }

uint32_t HashTableDirectoryPage::GetGlobalDepth() { return global_depth_; }

uint32_t HashTableDirectoryPage::GetGlobalDepthMask() { return GetMaskByLen(global_depth_); }

void HashTableDirectoryPage::IncrGlobalDepth() { global_depth_++; }

void HashTableDirectoryPage::DecrGlobalDepth() { global_depth_--; }

page_id_t HashTableDirectoryPage::GetBucketPageId(uint32_t bucket_idx) { return bucket_page_ids_[bucket_idx]; }

void HashTableDirectoryPage::SetBucketPageId(uint32_t bucket_idx, page_id_t bucket_page_id) {
  bucket_page_ids_[bucket_idx] = bucket_page_id;
}

uint32_t HashTableDirectoryPage::Size() { return 1 << global_depth_; }

bool HashTableDirectoryPage::CanShrink() {
  if (global_depth_ == 0) {
    return false;
  }
  auto curr_size = Size();
  for (size_t i = 0; i < curr_size; i++) {
    if (local_depths_[i] == global_depth_) {
      return false;
    }
  }
  return true;
}

uint32_t HashTableDirectoryPage::GetLocalDepth(uint32_t bucket_idx) { return local_depths_[bucket_idx]; }

void HashTableDirectoryPage::SetLocalDepth(uint32_t bucket_idx, uint8_t local_depth) {
  local_depths_[bucket_idx] = local_depth;
}

void HashTableDirectoryPage::IncrLocalDepth(uint32_t bucket_idx) { local_depths_[bucket_idx]++; }

void HashTableDirectoryPage::DecrLocalDepth(uint32_t bucket_idx) {
  if (local_depths_[bucket_idx] == 0) {
    return;
  }
  local_depths_[bucket_idx]--;
}

uint32_t HashTableDirectoryPage::GetLocalHighBit(uint32_t bucket_idx) { return 0; }

uint32_t HashTableDirectoryPage::GetLocalDepthMask(uint32_t bucket_idx) {
  return GetMaskByLen(GetLocalDepth(bucket_idx));
}

uint32_t HashTableDirectoryPage::GetMaskByLen(uint32_t len) const { return ((1 << len) - 1); }

/**
 * VerifyIntegrity - Use this for debugging but **DO NOT CHANGE**
 *
 * If you want to make changes to this, make a new function and extend it.
 *
 * Verify the following invariants:
 * (1) All LD <= GD.
 * (2) Each bucket has precisely 2^(GD - LD) pointers pointing to it.
 * (3) The LD is the same at each index with the same bucket_page_id
 */
void HashTableDirectoryPage::VerifyIntegrity() {
  //  build maps of {bucket_page_id : pointer_count} and {bucket_page_id : local_depth}
  std::unordered_map<page_id_t, uint32_t> page_id_to_count = std::unordered_map<page_id_t, uint32_t>();
  std::unordered_map<page_id_t, uint32_t> page_id_to_ld = std::unordered_map<page_id_t, uint32_t>();

  //  verify for each bucket_page_id, pointer
  for (uint32_t curr_idx = 0; curr_idx < Size(); curr_idx++) {
    page_id_t curr_page_id = bucket_page_ids_[curr_idx];
    uint32_t curr_ld = local_depths_[curr_idx];
    assert(curr_ld <= global_depth_);

    ++page_id_to_count[curr_page_id];

    if (page_id_to_ld.count(curr_page_id) > 0 && curr_ld != page_id_to_ld[curr_page_id]) {
      uint32_t old_ld = page_id_to_ld[curr_page_id];
      LOG_WARN("Verify Integrity: curr_local_depth: %u, old_local_depth %u, for page_id: %u", curr_ld, old_ld,
               curr_page_id);
      PrintDirectory();
      assert(curr_ld == page_id_to_ld[curr_page_id]);
    } else {
      page_id_to_ld[curr_page_id] = curr_ld;
    }
  }

  auto pit = page_id_to_count.begin();

  while (pit != page_id_to_count.end()) {
    page_id_t curr_page_id = pit->first;
    uint32_t curr_count = pit->second;
    uint32_t curr_ld = page_id_to_ld[curr_page_id];
    uint32_t required_count = 0x1 << (global_depth_ - curr_ld);

    if (curr_count != required_count) {
      LOG_WARN("Verify Integrity: curr_count: %u, required_count %u, for page_id: %u", curr_count, required_count,
               curr_page_id);
      PrintDirectory();
      assert(curr_count == required_count);
    }
    pit++;
  }
}

uint32_t HashTableDirectoryPage::Expand(page_id_t page_id) {
  size_t old_gdepth = global_depth_;
  uint32_t need_split = -1;
  auto new_len = static_cast<uint32_t>(pow(2, static_cast<double>(old_gdepth) + 1));
  auto old_mask = GetGlobalDepthMask();

  IncrGlobalDepth();
  for (uint32_t i = static_cast<uint32_t>(pow(2, static_cast<double>(old_gdepth))); i < new_len; i++) {
    auto mask_i = old_mask & i;
    auto local_depth = GetLocalDepth(mask_i);
    SetLocalDepth(i, local_depth);

    if (mask_i == static_cast<size_t>(page_id)) {
      need_split = i;
    }
    page_id_t pg_id = GetBucketPageId(mask_i);
    SetBucketPageId(i, pg_id);
  }
  return need_split;
}

uint32_t HashTableDirectoryPage::GetBrother(uint32_t bucket_idx) const {
  if (local_depths_[bucket_idx] == 0) {
    return 0;
  }
  return Upbit(bucket_idx, local_depths_[bucket_idx]);
}

void HashTableDirectoryPage::PrintDirectory() {
  LOG_DEBUG("======== DIRECTORY (global_depth_: %u) ========", global_depth_);
  LOG_DEBUG("| bucket_idx | page_id | local_depth |");
  for (uint32_t idx = 0; idx < static_cast<uint32_t>(0x1 << global_depth_); idx++) {
    LOG_DEBUG("|      %u     |     %u     |     %u     |", idx, bucket_page_ids_[idx], local_depths_[idx]);
  }
  LOG_DEBUG("================ END DIRECTORY ================");
}

}  // namespace bustub
