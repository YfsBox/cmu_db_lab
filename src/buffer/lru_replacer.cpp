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
  if (Size() == 0) { //没有，弹出警告
    LOG_WARN("not frame to victim,lruplacer is empty");
    return false;
  }
  auto endit = frames_.end();
  endit --;
  *frame_id = *endit;//返回取值
  //移除这一项
  auto it = itmap_.find(*frame_id);
  frames_.erase(it->second);
  itmap_.erase(it);

  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {//从中移除一个
  auto it = itmap_.find(frame_id);
  if (it == itmap_.end()) { //说明找不到
    LOG_WARN("not have this frame_id %d in lruplacer",frame_id);
    return;
  }
  //然后将这一部分移除
  frames_.erase(it->second);//直接根据迭代器进行移除
  itmap_.erase(it); //移除

}

void LRUReplacer::Unpin(frame_id_t frame_id) { //如果减到了0,就加入到容器中,确实在0时调用
  if (Size() >= capacity_) { //是否需要置换呢？
    LOG_WARN("the capacity is full in lrureplacer");
    return;
  }
  auto it = itmap_.find(frame_id);
  if (it != itmap_.end()) { //如果能找到那就什么也不做
    LOG_WARN("the frame %d has exsit in lruplacer",frame_id);
    return;
  }
  //新数据加入到头部
  frames_.push_front(frame_id);
  itmap_[frame_id] = frames_.begin();
}

size_t LRUReplacer::Size() { return frames_.size(); }

}  // namespace bustub
