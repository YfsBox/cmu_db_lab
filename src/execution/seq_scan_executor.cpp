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
  //TableIterator last_it = *iterator_;
  auto output_schema = plan_->OutputSchema();
  while (*iterator_ != end_it) {
    Tuple tup = **iterator_;
    // LOG_DEBUG("the Tuple is %s",tup.ToString(output_schema).c_str());
    (*iterator_)++;
    if (plan_->GetPredicate() == nullptr || plan_->GetPredicate()->Evaluate(&tup, &info_->schema_).GetAs<bool>()) {
      std::vector<Value> values;
      values.reserve(output_schema->GetColumnCount());
      for (size_t i = 0; i < output_schema->GetColumnCount(); i++) {
        values.push_back(output_schema->GetColumn(i).GetExpr()->Evaluate(&tup,&info_->schema_));
      }
      Tuple res_tup(std::move(values), output_schema);
      *tuple = res_tup;
      *rid = tup.GetRid();
      return true;
    }
  }
  //*iterator_ = last_it;
  return false;
}

}  // namespace bustub
