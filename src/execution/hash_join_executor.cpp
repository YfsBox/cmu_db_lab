//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/hash_join_executor.h"
#include "execution/expressions/column_value_expression.h"

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left_child,
                                   std::unique_ptr<AbstractExecutor> &&right_child)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      left_child_(std::move(left_child)),
      right_child_(std::move(right_child)) {}

void HashJoinExecutor::Init() {
  Tuple tmp_tuple;
  RID tmp_rid;
  left_child_->Init();
  const AbstractExpression *left_express = plan_->LeftJoinKeyExpression();
  const ColumnValueExpression *left_colval_express = static_cast<const ColumnValueExpression *>(left_express);
  while (left_child_->Next(&tmp_tuple, &tmp_rid)) {
    Value value = left_colval_express->Evaluate(&tmp_tuple, left_child_->GetOutputSchema());
    HashJoinKey joinkey{value};
    hashjoin_table_.Insert(joinkey, tmp_tuple);
  }
  right_child_->Init();
}

bool HashJoinExecutor::Next(Tuple *tuple, RID *rid) {
  Tuple tmp_tup;
  RID tmp_rid;
  const AbstractExpression *right_express = plan_->RightJoinKeyExpression();
  const ColumnValueExpression *right_colval_express = static_cast<const ColumnValueExpression *>(right_express);

  HashJoinKey curr_joinkey = curr_left_tuple_.first;
  if (curr_joinkey.value_.IsNull() || curr_left_tuple_.second == hashjoin_table_.GetSize(curr_joinkey)) {
    while (right_child_->Next(&tmp_tup, &tmp_rid)) {
      Value value = right_colval_express->Evaluate(&tmp_tup, right_child_->GetOutputSchema());
      HashJoinKey joinkey{value};
      curr_right_tuple_ = tmp_tup;
      bool have = (hashjoin_table_.GetSize(joinkey) != -1);
      if (have) {
        curr_left_tuple_.first = joinkey;
        Tuple left_tuple = hashjoin_table_.GetTuple(joinkey, 0);
        GetJoinTuple(left_tuple, tuple, tmp_tup);
        *rid = tuple->GetRid();
        curr_left_tuple_.second = 1;
        return true;
      }
    }
  } else {
    Tuple left_tuple = hashjoin_table_.GetTuple(curr_left_tuple_.first, curr_left_tuple_.second);
    GetJoinTuple(left_tuple, tuple, curr_right_tuple_);
    *rid = tuple->GetRid();
    curr_left_tuple_.second += 1;
    return true;
  }
  return false;
}

bool HashJoinExecutor::GetJoinTuple(const Tuple &left_tup, Tuple *joined_tup, const Tuple &right_tup) const {
  std::vector<Value> values;
  auto output_schema = plan_->OutputSchema();
  values.reserve(output_schema->GetLength());
  for (auto &col : output_schema->GetColumns()) {
    values.push_back(col.GetExpr()->EvaluateJoin(&left_tup, left_child_->GetOutputSchema(), &right_tup,
                                                 right_child_->GetOutputSchema()));
  }
  Tuple res_tup(std::move(values), output_schema);
  *joined_tup = res_tup;
  return true;
}

}  // namespace bustub