//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.h
//
// Identification: src/include/execution/executors/hash_join_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>

#include "common/util/hash_util.h"
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/hash_join_plan.h"
#include "storage/table/tuple.h"


namespace bustub {

class HashJoinKey {
 public:
  Value value_;
  bool operator==(const HashJoinKey &key) const {
    auto cmp = value_.CompareEquals(key.value_);
    return cmp == CmpBool::CmpTrue;
  }
};

class HashJoinHashFunc {
 public:
  std::size_t operator()(const bustub::HashJoinKey &hashJoinKey) const {
    size_t curr_hash = 0;
    curr_hash = bustub::HashUtil::CombineHashes(curr_hash, bustub::HashUtil::HashValue(&hashJoinKey.value_));
    return curr_hash;
  }
};

class SimpleHashJoinHashTable {
 public:
  using tuples = std::vector<Tuple>;

  SimpleHashJoinHashTable() = default;
  void Insert(const HashJoinKey &hashkey, const Tuple &tuple) {
    if (buckets_.count(hashkey) == 0) {
      tuples init_tuples;
      init_tuples.push_back(tuple);
      buckets_.insert({hashkey, std::move(init_tuples)});
    } else {
      buckets_[hashkey].push_back(tuple);
    }
  }

  bool GetTuples(const HashJoinKey &hashkey, tuples *tuples) const {
    auto find_it = buckets_.find(hashkey);
    if (find_it == buckets_.end()) {
      return false;
    }
    *tuples = find_it->second;
    return true;
  }

  Tuple GetTuple(const HashJoinKey &hashkey, uint32_t idx) const {
    auto find_it = buckets_.find(hashkey);
    assert(find_it != buckets_.end());
    return find_it->second[idx];
  }

  int32_t GetSize(const HashJoinKey &hashkey) const {
    auto find_it = buckets_.find(hashkey);
    if (find_it == buckets_.end()) {
      return -1;
    }
    return static_cast<int32_t>(find_it->second.size());
  }

 private:
  std::unordered_map<HashJoinKey, tuples, HashJoinHashFunc> buckets_;
};


/**
 * HashJoinExecutor executes a nested-loop JOIN on two tables.
 */
class HashJoinExecutor : public AbstractExecutor {
 public:
  /**
   * Construct a new HashJoinExecutor instance.
   * @param exec_ctx The executor context
   * @param plan The HashJoin join plan to be executed
   * @param left_child The child executor that produces tuples for the left side of join
   * @param right_child The child executor that produces tuples for the right side of join
   */
  HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                   std::unique_ptr<AbstractExecutor> &&left_child, std::unique_ptr<AbstractExecutor> &&right_child);

  /** Initialize the join */
  void Init() override;

  /**
   * Yield the next tuple from the join.
   * @param[out] tuple The next tuple produced by the join
   * @param[out] rid The next tuple RID produced by the join
   * @return `true` if a tuple was produced, `false` if there are no more tuples
   */
  bool Next(Tuple *tuple, RID *rid) override;

  /** @return The output schema for the join */
  const Schema *GetOutputSchema() override { return plan_->OutputSchema(); };

 private:
  /** The NestedLoopJoin plan node to be executed. */
  bool GetJoinTuple(const Tuple &left_tup, Tuple *joined_tup, const Tuple &right_tup) const;

  SimpleHashJoinHashTable hashjoin_table_;
  const HashJoinPlanNode *plan_;  // 其中还有描述了左key和右key
  std::unique_ptr<AbstractExecutor> left_child_;
  std::unique_ptr<AbstractExecutor> right_child_;
  std::pair<HashJoinKey, int32_t> curr_left_tuple_;
  Tuple curr_right_tuple_;
};

}  // namespace bustub
