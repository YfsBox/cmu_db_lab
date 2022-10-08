//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"
#include <../include/common/logger.h>
#include "common/macros.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : BufferPoolManagerInstance(pool_size, 1, 0, disk_manager, log_manager) {}

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, uint32_t num_instances, uint32_t instance_index,
                                                     DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size),
      num_instances_(num_instances),
      instance_index_(instance_index),
      next_page_id_(instance_index),
      disk_manager_(disk_manager),
      log_manager_(log_manager) {
  BUSTUB_ASSERT(num_instances > 0, "If BPI is not part of a pool, then the pool size should just be 1");
  BUSTUB_ASSERT(
      instance_index < num_instances,
      "BPI index cannot be greater than the number of BPIs in the pool. In non-parallel case, index should just be 1.");
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) { //free_list中保存的是page在数组中的索引
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete replacer_;
}

bool BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) { //这里的page_id和数组索引是一样的吗
  // Make sure you call DiskManager::WritePage!
  //frame_id_t frame_id = page_table_
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    LOG_WARN("not find page_id %d in page_table",page_id);
    return false;
  }
  //能够找到
  disk_manager_->WritePage(page_id,pages_[it->second].GetData());
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  // You can do it!
  for (auto pageit: page_table_) {
    FlushPage(pageit.first);
  }
}

Page *BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) {
  // 0.   Make sure you call AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  //
  bool is_allpin = true;
  for (size_t i = 0; i < pool_size_; i ++) {
    if (pages_[i].GetPinCount() == 0) {
      is_allpin = false;
    }
  }
  if (is_allpin) {
    LOG_WARN("all are pinned in pool");
    return nullptr;
  }
  frame_id_t rframe_id;
  if (!findFreePage(&rframe_id)) { //找不到freepage
    //LOG_WARN("return nullptr");
    return nullptr;
  }
  if (pages_[rframe_id].IsDirty()) {
    disk_manager_->WritePage(pages_[rframe_id].GetPageId(),pages_[rframe_id].GetData());
  }
  page_table_.erase(pages_[rframe_id].GetPageId());
  *page_id = AllocatePage();
  resetPage(rframe_id);
  pages_[rframe_id].page_id_ = *page_id;
  pages_[rframe_id].pin_count_ = 1;
  //add to table
  page_table_[*page_id] = rframe_id;
  return pages_ + rframe_id;
}
//frame_id作为pages数组的索引
page_id_t BufferPoolManagerInstance::frame2page(const frame_id_t fid) const {
  return pages_[fid].GetPageId();
}

void BufferPoolManagerInstance::resetPage(const frame_id_t frame_id) {
  LOG_DEBUG("the frame_id is %d",frame_id);
  pages_[frame_id].page_id_ = INVALID_PAGE_ID;
  pages_[frame_id].pin_count_ = 0;
  pages_[frame_id].is_dirty_ = false;
  pages_[frame_id].ResetMemory();
}

bool BufferPoolManagerInstance::findFreePage(frame_id_t *frame_id) {
  //LOG_DEBUG("...");
  if (free_list_.empty()) {
    if(auto ok = replacer_->Victim(frame_id);!ok) {
      LOG_WARN("not find victim page from replacer");
      return false;
    }
  } else {
    *frame_id = free_list_.front();//首先应该获取,但是是否应该删除呢?
    free_list_.pop_front();
  }
  return true;
}

//返回其在pages中的地址吗
Page *BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  frame_id_t r_fid;
  auto pageit = page_table_.find(page_id);
  if (pageit != page_table_.end()) { //如果这一页存在
    frame_id_t f_id = pageit->second;
    pages_[f_id].pin_count_ ++;
    if (pages_[f_id].pin_count_ == 1) {
      replacer_->Pin(f_id);
    }
    return pages_ + f_id;
  } else {
    if (!findFreePage(&r_fid)) { //这种情况就是找不到的
      return nullptr;
    }
  }
  //到了这里已经找到一个可用的R了
  page_id_t rpgid = frame2page(r_fid);
  if (pages_[r_fid].IsDirty()) {
    disk_manager_->WritePage(rpgid,pages_[r_fid].GetData());
  } //写入磁盘
  page_table_.erase(rpgid);
  page_table_[page_id] = r_fid;
  //插入一个page
  pages_[r_fid].page_id_ = page_id;
  pages_[r_fid].pin_count_ = 1;
  replacer_->Pin(r_fid);
  disk_manager_->ReadPage(page_id,pages_[r_fid].GetData());
  return pages_ + r_fid;
}

bool BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) {
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  DeallocatePage(page_id);
  auto page_it = page_table_.find(page_id);
  if (page_it == page_table_.end()) { //not exist
    return true;
  } else {
    if (pages_[page_it->second].GetPinCount() > 0) {
      return false;
    } else { //可以删除的情况
      frame_id_t frame_id = page_it->second;
      page_table_.erase(page_it);//移除这一项
      resetPage(frame_id);
      free_list_.push_back(frame_id);
    }
  }
  return false;
}

bool BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  auto page_it = page_table_.find(page_id);
  if (page_it == page_table_.end()) {
    LOG_WARN("can't find from pagetable");
    return false;
  }
  frame_id_t frame_id = page_it->second;
  if (pages_[frame_id].pin_count_ == 0) {
    return false;
  }
  pages_[frame_id].pin_count_ --;
  pages_[frame_id].is_dirty_ = is_dirty;
  if (pages_[frame_id].pin_count_ == 0) {
    replacer_->Unpin(frame_id);
  }
  return true;
}

page_id_t BufferPoolManagerInstance::AllocatePage() {
  const page_id_t next_page_id = next_page_id_;
  next_page_id_ += num_instances_;
  ValidatePageId(next_page_id);
  return next_page_id;
}

void BufferPoolManagerInstance::ValidatePageId(const page_id_t page_id) const {
  assert(page_id % num_instances_ == instance_index_);  // allocated pages mod back to this BPI
}

}  // namespace bustub
