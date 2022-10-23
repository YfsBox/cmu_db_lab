//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), info_(nullptr) ,plan_(plan), child_executor_(std::move(child_executor))  {}

void DeleteExecutor::Init() {
  info_ = AbstractExecutor::exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  if (child_executor_ != nullptr) {
    child_executor_->Init();
  }
}

bool DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  RID tmp_rid;
  auto indexes = AbstractExecutor::exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
  while (child_executor_->Next(&tmp_tup,&tmp_rid)) {
    info_->table_->MarkDelete(tmp_rid,AbstractExecutor::exec_ctx_->GetTransaction());
    for (auto index : indexes) {
      index->index_->DeleteEntry(tmp_tup, tmp_rid ,AbstractExecutor::exec_ctx_->GetTransaction());
    }
  }
  return false;
}

}  // namespace bustub
