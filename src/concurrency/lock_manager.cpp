//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.cpp
//
// Identification: src/concurrency/lock_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/lock_manager.h"
#include "concurrency/transaction_manager.h"

#include <utility>
#include <vector>

namespace bustub {

bool LockManager::IsConflictLock(Transaction *txn, const LockRequest &request, const LockOpType &locktype) const {
  txn_id_t self_txn_id = txn->GetTransactionId();
  if (locktype == LockOpType::SHARED_OP) {
    return !(request.txn_id_ != self_txn_id && request.lock_mode_ == LockMode::EXCLUSIVE
             && request.txn_id_ != INVALID_TXN_ID);
  }
  if (locktype == LockOpType::EXCLUSIVE_OP) {
    return (request.txn_id_ == self_txn_id || request.txn_id_ == INVALID_TXN_ID);
  }
  return false;
}


bool LockManager::CanGrant(Transaction *txn, const RID &rid, const LockOpType &mode, std::list<LockRequest>::iterator lock_it) {
  bool can_grant = true;
  txn_id_t self_txn_id = txn->GetTransactionId();
  auto start_it = lock_table_[rid].request_queue_.begin();
  std::vector<txn_id_t> abort_txns;

  for (auto reit = start_it; reit != lock_table_[rid].request_queue_.end(); ++reit) {
    if (!reit->granted_) {
      can_grant = false;
      break;
    }
    bool is_conflict = IsConflictLock(txn, *reit, mode);
    can_grant &= is_conflict;
    if (!can_grant) {
      break;
    }
  }
  if (can_grant) {
    return true;
  }
  bool have_abort = false;
  for (auto reit = start_it; reit != lock_table_[rid].request_queue_.end(); ++reit) {
    txn_id_t txn_id = reit->txn_id_;
    if (txn_id > self_txn_id && !IsConflictLock(txn, *reit, mode)) {
      // LOG_DEBUG("txn %d push ths %d to abort",self_txn_id , txn_id);
      Transaction *tmp_txn = TransactionManager::GetTransaction(txn_id);
      tmp_txn->SetState(TransactionState::ABORTED);
      have_abort = true;
    }
  }
  if (have_abort) {
    lock_table_[rid].cv_.notify_all();
    return true;
  }
  return false;
}

bool LockManager::LockShared(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    if (txn->GetState() == TransactionState::SHRINKING) {
      throw AbortReason::LOCK_ON_SHRINKING;
    }
    return false;
  }
  txn->GetSharedLockSet()->emplace(rid);
  txn_id_t self_txn_id = txn->GetTransactionId();
  bool can_grant = CanGrant(txn, rid, LockOpType::SHARED_OP, lock_table_[rid].request_queue_.begin());

  LockRequest request(self_txn_id, LockMode::SHARED);
  lock_table_[rid].request_queue_.push_front(request);
  auto request_it = lock_table_[rid].request_queue_.begin();

  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    if (txn->GetState() == TransactionState::ABORTED) {
      throw AbortReason::DEADLOCK;
    }
    can_grant = CanGrant(txn, rid, LockOpType::SHARED_OP, request_it);
  }

  lock_table_[rid].upgrading_ = self_txn_id;
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    if (txn->GetState() == TransactionState::SHRINKING) {
      throw AbortReason::LOCK_ON_SHRINKING;
    }
    return false;
  }
  txn->GetExclusiveLockSet()->emplace(rid);
  txn_id_t self_txn_id = txn->GetTransactionId();
  bool can_grant = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, lock_table_[rid].request_queue_.begin());

  LockRequest request(self_txn_id, LockMode::EXCLUSIVE);
  lock_table_[rid].request_queue_.push_front(request);
  auto request_it = lock_table_[rid].request_queue_.begin();

  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    if (txn->GetState() == TransactionState::ABORTED) {
      return false;
    }
    can_grant = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, request_it);
  }

  lock_table_[rid].upgrading_ = self_txn_id;
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;
}

bool LockManager::LockUpgrade(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);

  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    if (txn->GetState() == TransactionState::SHRINKING) {
      throw AbortReason::LOCK_ON_SHRINKING;
    }
    return false;
  }
  auto find_it = lock_table_[rid].request_queue_.begin();
  for (;find_it != lock_table_[rid].request_queue_.end(); ++find_it) {
    if (find_it->granted_ && find_it->lock_mode_ == LockMode::SHARED && find_it->txn_id_ == txn->GetTransactionId()) {
      break;
    }
  }
  if (find_it == lock_table_[rid].request_queue_.end()) {
    txn->SetState(TransactionState::ABORTED);
    throw AbortReason::DEADLOCK;
  }
  bool can_upgrade = true;
  auto rest_it = find_it;
  rest_it++;
  for (; rest_it != lock_table_[rid].request_queue_.end(); ++rest_it) {
    if (rest_it->granted_ && rest_it->txn_id_ != txn->GetTransactionId()) {
      can_upgrade = false;
      break;
    }
  }
  if (!can_upgrade) {
    txn->SetState(TransactionState::ABORTED);
    throw AbortReason::UPGRADE_CONFLICT;
  }

  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);
  find_it->lock_mode_ = LockMode::EXCLUSIVE;
  return true;
}

bool LockManager::Unlock(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);
  if (lock_table_[rid].upgrading_ == txn->GetTransactionId()) {
    lock_table_[rid].upgrading_ = INVALID_TXN_ID;
  }

  txn_id_t txn_id = txn->GetTransactionId();
  for (auto reqit = lock_table_[rid].request_queue_.begin(); reqit !=
       lock_table_[rid].request_queue_.end();) {
    if (reqit->txn_id_ == txn_id && reqit->granted_) {
      reqit = lock_table_[rid].request_queue_.erase(reqit);  // 在容器中进行移除的注意事项
    } else {
      reqit++;
    }
  }
  if (txn->GetState() == TransactionState::GROWING) {
    txn->SetState(TransactionState::SHRINKING);
  }
  lock_table_[rid].cv_.notify_all();
  return true;
}

}  // namespace bustub
