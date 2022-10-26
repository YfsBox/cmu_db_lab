//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.cpp
//
// Identification: src/execution/aggregation_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>
#include <vector>

#include "execution/executors/aggregation_executor.h"

namespace bustub {

AggregationExecutor::AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                                         std::unique_ptr<AbstractExecutor> &&child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      child_(std::move(child)),
      aht_(plan_->GetAggregates(), plan_->GetAggregateTypes()),
      aht_iterator_(aht_.Begin()) {}

void AggregationExecutor::Init() {
  if (child_ != nullptr) {
    child_->Init();
  }
  Tuple tmp_tup;
  RID tmp_rid;

  while (child_->Next(&tmp_tup, &tmp_rid)) {
    AggregateKey aggkey = MakeAggregateKey(&tmp_tup);
    AggregateValue aggval = MakeAggregateValue(&tmp_tup);
    aht_.InsertCombine(aggkey, aggval);
  }
  aht_iterator_ = aht_.Begin();
  child_->Init();
}

bool AggregationExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  Value is_true(BOOLEAN, 1);
  auto output_schema = plan_->OutputSchema();
  auto have = plan_->GetHaving();
  while (aht_iterator_ != aht_.End()) {
    bool have_ok = false;
    AggregateKey aggkey = aht_iterator_.Key();
    AggregateValue aggval = aht_iterator_.Val();
    ++aht_iterator_;
    if (have != nullptr) {
      auto evaluate = have->EvaluateAggregate(aggkey.group_bys_, aggval.aggregates_);
      have_ok = (evaluate.GetAs<bool>());
    } else {
      have_ok = true;
    }
    if (have_ok) {
      std::vector<Value> values;
      values.reserve(output_schema->GetLength());
      for (auto &col : output_schema->GetColumns()) {
        values.push_back(col.GetExpr()->EvaluateAggregate(aggkey.group_bys_, aggval.aggregates_));
      }
      Tuple res_tup(values, plan_->OutputSchema());
      // LOG_DEBUG("The restup is %s", res_tup.ToString(plan_->OutputSchema()).c_str());
      *tuple = res_tup;
      *rid = tuple->GetRid();
      return true;
    }
  }
  return false;
}

const AbstractExecutor *AggregationExecutor::GetChildExecutor() const { return child_.get(); }

}  // namespace bustub
