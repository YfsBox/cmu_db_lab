//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// distinct_executor.cpp
//
// Identification: src/execution/distinct_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/distinct_executor.h"

namespace bustub {

std::vector<const AbstractExpression *> DistinctExecutor::empty_agg_exprs = {};
std::vector<AggregationType> DistinctExecutor::empty_agg_types = {};

DistinctExecutor::DistinctExecutor(ExecutorContext *exec_ctx, const DistinctPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)),
      aht_(empty_agg_exprs, empty_agg_types),
      aht_iterator_(aht_.Begin()){}

void DistinctExecutor::Init() {

  if (child_executor_ != nullptr) {
    child_executor_ ->Init();
  }
  Tuple tmp_tup;
  RID tmp_rid;
  child_executor_->Next(&tmp_tup, &tmp_rid);
  uint32_t colcnt = child_executor_->GetOutputSchema()->GetColumnCount();
  while (child_executor_->Next(&tmp_tup,&tmp_rid)) {
    std::vector<Value> values;
    // LOG_DEBUG("the output of child is %s",child_executor_->GetOutputSchema()->ToString().c_str());
    values.reserve(child_executor_->GetOutputSchema()->GetColumnCount());
    for (size_t i = 0; i < colcnt; i++) {
      values.push_back(tmp_tup.GetValue(child_executor_->GetOutputSchema(),i));
    }
    AggregateKey aggkey{std::move(values)};
    aht_.InsertCombine(aggkey, {});
  }
  aht_iterator_ = aht_.Begin();
  child_executor_->Init();
}

bool DistinctExecutor::Next(Tuple *tuple, RID *rid) {

  auto output_schema = plan_->OutputSchema();
  while (aht_iterator_ != aht_.End()) {
    std::vector<Value> values;
    values.reserve(output_schema->GetLength());
    for (auto val : aht_iterator_.Key().group_bys_) {
      values.push_back(val);
    }
    Tuple res_tup(values, output_schema);
    *tuple = res_tup;
    *rid = tuple->GetRid();
    ++aht_iterator_;
    return true;
  }
  return false;
}

}  // namespace bustub
