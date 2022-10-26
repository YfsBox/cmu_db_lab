//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx),
      left_empty_(false),
      plan_(plan),
      left_executor_(std::move(left_executor)),
      right_executor_(std::move(right_executor)) {}

void NestedLoopJoinExecutor::Init() {
  left_executor_->Init();
  RID tmp_rid;
  left_empty_ = !left_executor_->Next(&curr_left_tuple_, &tmp_rid);
  right_executor_->Init();
}

bool NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) {
  if (left_empty_) {
    return false;
  }
  Tuple right_tup;
  RID right_rid;
  auto output_schema = plan_->OutputSchema();
  while (right_executor_->Next(&right_tup, &right_rid)) {
    std::vector<Value> values;
    values.reserve(output_schema->GetLength());
    if (plan_->Predicate() == nullptr || plan_->Predicate()
                                             ->EvaluateJoin(&curr_left_tuple_, left_executor_->GetOutputSchema(),
                                                            &right_tup, right_executor_->GetOutputSchema())
                                             .GetAs<bool>()) {
      for (auto &col : output_schema->GetColumns()) {
        values.push_back(col.GetExpr()->EvaluateJoin(&curr_left_tuple_, left_executor_->GetOutputSchema(), &right_tup,
                                                     right_executor_->GetOutputSchema()));
      }
      Tuple res_tup(std::move(values), plan_->OutputSchema());
      *tuple = res_tup;
      *rid = res_tup.GetRid();
      return true;
    }
  }
  RID left_rid;
  if (left_executor_->Next(&curr_left_tuple_, &left_rid)) {
    right_executor_->Init();
    return Next(tuple, rid);
  }
  return false;
}
}  // namespace bustub
