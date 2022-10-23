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
    : AbstractExecutor(exec_ctx), plan_(plan), child_(std::move(child)),
      aht_(plan_->GetAggregates(), plan_->GetAggregateTypes()),
      aht_iterator_(aht_.Begin()) {}

bool AggregationExecutor::FindKeyFromAggHash(const AggregateKey &aggkey,AggregateValue *aggval) {
  bool have_key = false;
  aht_iterator_ = aht_.Begin();
  while (aht_iterator_ != aht_.End()) {
    if (aht_iterator_.Key() == aggkey) {
      have_key = true;
      *aggval = aht_iterator_.Val();
      break;
    }
    ++aht_iterator_;
  }
  return have_key;
}


void AggregationExecutor::Init() {
  if (child_ != nullptr) {
    child_->Init();
  }
  Tuple tmp_tup;
  RID tmp_rid;

  while (child_->Next(&tmp_tup,&tmp_rid)) {
    AggregateKey aggkey = MakeAggregateKey(&tmp_tup);
    AggregateValue aggval = MakeAggregateValue(&tmp_tup);
    aht_.InsertCombine(aggkey, aggval);
  }
  child_->Init();
}

bool AggregationExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  RID tmp_rid;
  Value is_true(BOOLEAN, 1);
  auto have = plan_->GetHaving();
  while (aht_iterator_ != aht_.End()) {
    bool have_ok = false;
    AggregateKey aggkey = aht_iterator_.Key();
    AggregateValue aggval = aht_iterator_.Val();
    ++aht_iterator_;
    //LOG_DEBUG("loop");
    if (have != nullptr) {
      auto evaluate = have->EvaluateAggregate(aggkey.group_bys_, aggval.aggregates_);
      have_ok = (evaluate.CompareEquals(is_true) == CmpBool::CmpTrue);
    } else {
      have_ok = true;
    }
    if (have_ok) {
      std::vector<Value> values;
      values.reserve(aggkey.group_bys_.size() + aggval.aggregates_.size());
      for (auto key : aggkey.group_bys_) {
        values.push_back(key);
      }
      for (auto val : aggval.aggregates_) {
        values.push_back(val);
      }
      Tuple res_tup(values,plan_->OutputSchema());
      LOG_DEBUG("The restup is %s", res_tup.ToString(plan_->OutputSchema()).c_str());
      *tuple = res_tup;
      *rid = tuple->GetRid();
      return true;
    }
  }
  return false;
}

const AbstractExecutor *AggregationExecutor::GetChildExecutor() const { return child_.get(); }

}  // namespace bustub
