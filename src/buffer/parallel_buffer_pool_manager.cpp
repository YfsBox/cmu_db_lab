//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// parallel_buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/parallel_buffer_pool_manager.h"
#include "buffer/buffer_pool_manager_instance.h"
#include "../include/common/logger.h"

namespace bustub {

ParallelBufferPoolManager::ParallelBufferPoolManager(size_t num_instances, size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager):num_instances_(num_instances),
pool_size_(pool_size), disk_manager_(disk_manager),
log_manager_(log_manager) {
  // Allocate and create individual BufferPoolManagerInstances
  instances_.resize(num_instances_);
  for (size_t i = 0; i < num_instances_; i ++) {
    instances_[i] = new BufferPoolManagerInstance(pool_size_, num_instances_, i, disk_manager_, log_manager_);
  }
  next_instance_ = 0;
}

// Update constructor to destruct all BufferPoolManagerInstances and deallocate any associated memory
ParallelBufferPoolManager::~ParallelBufferPoolManager() {
  for (auto pool : instances_) {
    delete pool;
  }
}

size_t ParallelBufferPoolManager::GetPoolSize() {
  // Get size of all BufferPoolManagerInstances
  return num_instances_ * pool_size_;
}

BufferPoolManager *ParallelBufferPoolManager::GetBufferPoolManager(page_id_t page_id) {
  // Get BufferPoolManager responsible for handling given page id. You can use this method in your other methods.
  auto ins_id = page_id % num_instances_;
  LOG_DEBUG("get buffer manager %lu", ins_id);
  return instances_[ins_id];
}

Page *ParallelBufferPoolManager::FetchPgImp(page_id_t page_id) {
  // Fetch page for page_id from responsible BufferPoolManagerInstance
  return GetBufferPoolManager(page_id)->FetchPage(page_id);
}

bool ParallelBufferPoolManager::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  // Unpin page_id from responsible BufferPoolManagerInstance
  return GetBufferPoolManager(page_id)->UnpinPage(page_id, is_dirty);
}

bool ParallelBufferPoolManager::FlushPgImp(page_id_t page_id) {
  // Flush page_id from responsible BufferPoolManagerInstance
  return GetBufferPoolManager(page_id)->FlushPage(page_id);
}

Page *ParallelBufferPoolManager::NewPgImp(page_id_t *page_id) {
  // create new page. We will request page allocation in a round robin manner from the underlying
  // BufferPoolManagerInstances
  // 1.   From a starting index of the BPMIs, call NewPageImpl until either 1) success and return 2) looped around to
  // starting index and return nullptr
  // 2.   Bump the starting index (mod number of instances) to start search at a different BPMI each time this function
  // is called
  std::lock_guard<std::mutex> guard(latch_);
  auto start = next_instance_;
  Page *page = nullptr;
  while (true) {
    // LOG_DEBUG("the next_instance is %lu,the start is %lu",next_instance_,start);
    page = instances_[next_instance_]->NewPage(page_id);
    next_instance_ = (next_instance_ + 1) % num_instances_;
    if (page != nullptr) {
      // LOG_DEBUG("find new page");
      break;
    }
    // LOG_DEBUG("the next_instance is %lu,the start is %lu",next_instance_,start);
    if (next_instance_ == start) {
      break;
    }
  }
  return page;
}

bool ParallelBufferPoolManager::DeletePgImp(page_id_t page_id) {
  // Delete page_id from responsible BufferPoolManagerInstance
  return GetBufferPoolManager(page_id)->DeletePage(page_id);
}

void ParallelBufferPoolManager::FlushAllPgsImp() {
  // flush all pages from all BufferPoolManagerInstances
  for (auto instance : instances_) {
    instance->FlushAllPages();  // 内部有锁的保护
  }
}

}  // namespace bustub
