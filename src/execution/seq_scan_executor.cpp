//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan) : AbstractExecutor(exec_ctx), info_(nullptr), iterator_(nullptr), plan_(plan) {}

SeqScanExecutor::~SeqScanExecutor() {}

void SeqScanExecutor::Init() {
  info_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());
  iterator_ = std::make_shared<TableIterator>(TableIterator(info_->table_->Begin(exec_ctx_->GetTransaction())));
}

bool SeqScanExecutor::Next(Tuple *tuple, RID *rid) {

  TableIterator end_it = info_->table_->End();
  TableIterator last_it = *iterator_;
  Value is_true(BOOLEAN,1);
  while (*iterator_ != end_it) {
    Tuple tup = **iterator_;
    Value evalueate;
    (*iterator_)++;
    if (plan_->GetPredicate() != nullptr) {
      evalueate = plan_->GetPredicate()->Evaluate(&tup, &info_->schema_);
    }
    if (plan_->GetPredicate() == nullptr || evalueate.CompareEquals(is_true) == CmpBool::CmpTrue) {
      *rid = tup.GetRid();
      std::vector<Value> values;
      auto output_schema = plan_->OutputSchema();
      values.reserve(output_schema->GetColumnCount());
      for (auto &col : output_schema->GetColumns()) {
        values.push_back(col.GetExpr()->Evaluate(&tup,output_schema));
      }
      *tuple = Tuple(values,output_schema);
      return true;
    }
  }
  *iterator_ = last_it;
  return false;
}

}  // namespace bustub
