//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// parallel_buffer_pool_manager_test.cpp
//
// Identification: test/buffer/buffer_pool_manager_test.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/parallel_buffer_pool_manager.h"
#include <cstdio>
#include <random>
#include <string>
#include "buffer/buffer_pool_manager.h"
#include "gtest/gtest.h"

namespace bustub {
// 关于第一个测试的话,首先只处理page0,在page0上进行写并且进行检查
// 然后一直创建的新的page，导致pool变满为止
// NOLINTNEXTLINE
// Check whether pages containing terminal characters can be recovered


}  // namespace bustub
