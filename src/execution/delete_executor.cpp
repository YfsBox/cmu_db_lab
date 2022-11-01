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
    : AbstractExecutor(exec_ctx), info_(nullptr), plan_(plan), child_executor_(std::move(child_executor)) {}

void DeleteExecutor::Init() {
  info_ = AbstractExecutor::exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  if (child_executor_ != nullptr) {
    child_executor_->Init();
  }
}

bool DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  RID tmp_rid;
  Transaction *txn = exec_ctx_->GetTransaction();
  auto indexes = AbstractExecutor::exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
  while (child_executor_->Next(&tmp_tup, &tmp_rid)) {
    try {
      if (txn->IsSharedLocked(tmp_rid)) {
        exec_ctx_->GetLockManager()->LockUpgrade(txn, tmp_tup.GetRid());
      } else {
        exec_ctx_->GetLockManager()->LockExclusive(txn, tmp_tup.GetRid());
      }
      txn->AddIntoDeletedPageSet(tmp_tup.GetRid().GetPageId());
      info_->table_->MarkDelete(tmp_rid, AbstractExecutor::exec_ctx_->GetTransaction());
      for (auto index : indexes) {
        IndexWriteRecord index_record(tmp_rid, info_->oid_, WType::DELETE, tmp_tup, index->index_oid_,
                                      exec_ctx_->GetCatalog());
        txn->AppendTableWriteRecord(index_record);
        index->index_->DeleteEntry(
            tmp_tup.KeyFromTuple(info_->schema_, index->key_schema_, index->index_->GetKeyAttrs()), tmp_rid,
            AbstractExecutor::exec_ctx_->GetTransaction());
      }
      if (txn->GetIsolationLevel() != IsolationLevel::REPEATABLE_READ) {
        exec_ctx_->GetLockManager()->Unlock(txn, tmp_tup.GetRid());
      }
    } catch (TransactionAbortException &e) {
      return false;
    }
  }
  return false;
}

}  // namespace bustub
