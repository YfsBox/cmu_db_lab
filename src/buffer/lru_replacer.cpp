//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"
#include <../include/common/logger.h>

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages):capacity_(num_pages) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  std::lock_guard<std::mutex> gaurd(latch_);
  if (frames_.size() == 0) {  // 没有，弹出警告
    // LOG_WARN("not frame to victim,lruplacer is empty");
    return false;
  }
  auto endit = frames_.end();
  endit--;
  *frame_id = *endit;
  auto it = itmap_.find(*frame_id);
  frames_.erase(it->second);
  itmap_.erase(it);

  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> gaurd(latch_);
  auto it = itmap_.find(frame_id);
  if (it == itmap_.end()) {
    // LOG_WARN("not have this frame_id %d in lruplacer", frame_id);
    return;
  }
  frames_.erase(it->second);
  itmap_.erase(it);
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> gaurd(latch_);
  auto it = itmap_.find(frame_id);
  if (it != itmap_.end()) {
    // LOG_WARN("the frame %d has exsit in lruplacer", frame_id);
    return;
  }
  if (frames_.size() >= capacity_) {
    // LOG_WARN("the capacity is full in lrureplacer");
    itmap_.erase(frames_.back());
    frames_.pop_back();
  }
  frames_.push_front(frame_id);
  itmap_[frame_id] = frames_.begin();
}

size_t LRUReplacer::Size() {
  std::lock_guard<std::mutex> gaurd(latch_);
  return frames_.size();
}

}  // namespace bustub
