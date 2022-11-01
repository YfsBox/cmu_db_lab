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
  Transaction *txn = exec_ctx_->GetTransaction();
  try {
    if (plan_->IsRawInsert()) {
      uint32_t value_size = plan_->RawValues().size();
      while (curr_cursor_ < value_size) {
        auto values = plan_->RawValuesAt(curr_cursor_);
        Tuple new_tuple(values, &(info_->schema_));
        info_->table_->InsertTuple(new_tuple, &tmp_rid, AbstractExecutor::exec_ctx_->GetTransaction());
        if (txn->IsSharedLocked(tmp_rid)) {
          exec_ctx_->GetLockManager()->LockUpgrade(txn, tmp_rid);
        } else {
          exec_ctx_->GetLockManager()->LockExclusive(txn, tmp_rid);
        }
        auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
        for (auto &index : indexes) {
          IndexWriteRecord new_record(tmp_rid, info_->oid_, WType::INSERT, new_tuple, info_->oid_,
                                      exec_ctx_->GetCatalog());
          txn->AppendTableWriteRecord(new_record);
          index->index_->InsertEntry(
              new_tuple.KeyFromTuple(info_->schema_, index->key_schema_, index->index_->GetKeyAttrs()), tmp_rid,
              AbstractExecutor::exec_ctx_->GetTransaction());
        }
        curr_cursor_++;
        if (txn->GetIsolationLevel() != IsolationLevel::REPEATABLE_READ) {
          exec_ctx_->GetLockManager()->Unlock(txn, tmp_rid);
        }
      }
    } else {
      while (child_executor_->Next(&tmp_next, &tmp_rid)) {
        info_->table_->InsertTuple(tmp_next, &tmp_rid, exec_ctx_->GetTransaction());
        if (txn->IsSharedLocked(tmp_rid)) {
          exec_ctx_->GetLockManager()->LockUpgrade(txn, tmp_rid);
        } else {
          exec_ctx_->GetLockManager()->LockExclusive(txn, tmp_rid);
        }
        auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(info_->name_);
        for (auto &index : indexes) {
          IndexWriteRecord new_record(tmp_rid, info_->oid_, WType::INSERT, tmp_next, info_->oid_,
                                      exec_ctx_->GetCatalog());
          txn->AppendTableWriteRecord(new_record);
          index->index_->InsertEntry(
              tmp_next.KeyFromTuple(info_->schema_, index->key_schema_, index->index_->GetKeyAttrs()), tmp_rid,
              AbstractExecutor::exec_ctx_->GetTransaction());
        }
        if (txn->GetIsolationLevel() != IsolationLevel::REPEATABLE_READ) {
          exec_ctx_->GetLockManager()->Unlock(txn, tmp_rid);
        }
      }
    }
  } catch (TransactionAbortException &e) {
    return false;
  }
  return false;
}

}  // namespace bustub
