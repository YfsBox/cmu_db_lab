//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), curr_cursor_(0), child_executor_(std::move(child_executor)), plan_(plan) {
  // info_ = exec_ctx->GetCatalog()->GetTable(plan->TableOid());
  info_ = AbstractExecutor::exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
}

void InsertExecutor::Init() {
  if (child_executor_ != nullptr) {
    child_executor_->Init();
  }
  curr_cursor_ = 0;
}

bool InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  Tuple tmp_next;
  RID tmp_rid;
  if (plan_->IsRawInsert()) {
    uint32_t value_size = plan_->RawValues().size();
    while (curr_cursor_ < value_size) {
      auto values = plan_->RawValuesAt(curr_cursor_);
      Tuple new_tuple(values, &(info_->schema_));
      info_->table_->InsertTuple(new_tuple, &tmp_rid, AbstractExecutor::exec_ctx_->GetTransaction());
      auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
      for (auto &index : indexes) {
        index->index_->InsertEntry(
            new_tuple.KeyFromTuple(info_->schema_, index->key_schema_, index->index_->GetKeyAttrs()), tmp_rid,
            AbstractExecutor::exec_ctx_->GetTransaction());
      }
      curr_cursor_++;
    }
  } else {
    while (child_executor_->Next(&tmp_next, &tmp_rid)) {
      info_->table_->InsertTuple(tmp_next, &tmp_rid, exec_ctx_->GetTransaction());
      auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
      for (auto &info : indexes) {
        info->index_->InsertEntry(tmp_next, tmp_rid, AbstractExecutor::exec_ctx_->GetTransaction());
      }
    }
  }
  return false;
}

}  // namespace bustub
