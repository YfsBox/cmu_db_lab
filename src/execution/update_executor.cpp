//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(std::move(child_executor)) {
  table_info_ = AbstractExecutor::exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
}

void UpdateExecutor::Init() {
  if (child_executor_ != nullptr) {
    child_executor_->Init();
  }
}

bool UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  RID tmp_rid;
  Transaction *transaction = AbstractExecutor::exec_ctx_->GetTransaction();
  auto indexes = AbstractExecutor::exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
  while (child_executor_->Next(&tmp_tup, &tmp_rid)) {
    Tuple updated_tup = GenerateUpdatedTuple(tmp_tup);
    assert(table_info_->table_->UpdateTuple(updated_tup, tmp_rid, transaction));
    for (auto index : indexes) {
      index->index_->DeleteEntry(tmp_tup, tmp_rid, transaction);
      index->index_->InsertEntry(updated_tup, tmp_rid, transaction);
    }
  }
  return false;
}

Tuple UpdateExecutor::GenerateUpdatedTuple(const Tuple &src_tuple) {
  const auto &update_attrs = plan_->GetUpdateAttr();
  Schema schema = table_info_->schema_;
  uint32_t col_count = schema.GetColumnCount();
  std::vector<Value> values;
  for (uint32_t idx = 0; idx < col_count; idx++) {
    if (update_attrs.find(idx) == update_attrs.cend()) {
      values.emplace_back(src_tuple.GetValue(&schema, idx));
    } else {
      const UpdateInfo info = update_attrs.at(idx);
      Value val = src_tuple.GetValue(&schema, idx);
      switch (info.type_) {
        case UpdateType::Add:
          values.emplace_back(val.Add(ValueFactory::GetIntegerValue(info.update_val_)));
          break;
        case UpdateType::Set:
          values.emplace_back(ValueFactory::GetIntegerValue(info.update_val_));
          break;
      }
    }
  }
  return Tuple{values, &schema};
}

}  // namespace bustub
