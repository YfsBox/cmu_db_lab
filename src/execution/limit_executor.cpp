//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// limit_executor.cpp
//
// Identification: src/execution/limit_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/limit_executor.h"

namespace bustub {

LimitExecutor::LimitExecutor(ExecutorContext *exec_ctx, const LimitPlanNode *plan,
                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), limit_cnt_(0), child_executor_(std::move(child_executor)) {}

void LimitExecutor::Init() {
  if (child_executor_ != nullptr) {
    child_executor_->Init();
  }
  limit_cnt_ = 0;
}

bool LimitExecutor::Next(Tuple *tuple, RID *rid) {
  if (limit_cnt_ >= plan_->GetLimit()) {
    return false;
  }
  Tuple tmp_tup;
  RID tmp_rid;
  while (child_executor_->Next(&tmp_tup, &tmp_rid)) {
    *tuple = tmp_tup;
    *rid = tmp_rid;
    limit_cnt_++;
    return true;
  }
  return false;
}

}  // namespace bustub
