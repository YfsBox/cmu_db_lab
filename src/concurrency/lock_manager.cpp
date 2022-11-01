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
#include <vector>
#include "concurrency/transaction_manager.h"

namespace bustub {

bool LockManager::IsConflictLock(Transaction *txn, const LockRequest &request, const LockOpType &locktype) const {
  txn_id_t self_txn_id = txn->GetTransactionId();
  if (locktype == LockOpType::SHARED_OP) {
    return !(request.txn_id_ != self_txn_id && request.lock_mode_ == LockMode::EXCLUSIVE &&
             request.txn_id_ != INVALID_TXN_ID);
  }
  if (locktype == LockOpType::EXCLUSIVE_OP) {
    return (request.txn_id_ == self_txn_id || request.txn_id_ == INVALID_TXN_ID);
  }
  return false;
}

bool LockManager::CanGrant(Transaction *txn, const RID &rid, const LockOpType &mode,
                           std::list<LockRequest>::iterator reqit) {
  bool can_grant = true;
  txn_id_t self_txn_id = txn->GetTransactionId();
  auto start_it = lock_table_[rid].request_queue_.begin();
  std::vector<txn_id_t> abort_txns;

  for (auto reit = start_it; reit != reqit; ++reit) {
    bool is_conflict = IsConflictLock(txn, *reit, mode);
    can_grant &= is_conflict;
    if (!can_grant) {
      break;
    }
  }
  reqit++;
  for (auto reit = reqit; reit != lock_table_[rid].request_queue_.end(); ++reit) {
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
  for (auto reit = start_it; reit != lock_table_[rid].request_queue_.end();) {
    txn_id_t txn_id = reit->txn_id_;
    if (txn_id > self_txn_id && !IsConflictLock(txn, *reit, mode)) {
      Transaction *tmp_txn = TransactionManager::GetTransaction(txn_id);
      tmp_txn->SetState(TransactionState::ABORTED);
      have_abort = true;
      reit = lock_table_[rid].request_queue_.erase(reit);
    } else {
      ++reit;
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
  // LOG_DEBUG("the txn push lock request to queue");
  if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCKSHARED_ON_READ_UNCOMMITTED);
  }
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
  }
  txn_id_t self_txn_id = txn->GetTransactionId();
  LockRequest request(self_txn_id, LockMode::SHARED);
  lock_table_[rid].request_queue_.push_front(request);

  auto request_it = lock_table_[rid].request_queue_.begin();
  if (txn->IsSharedLocked(rid) || txn->IsExclusiveLocked(rid)) {
    lock_table_[rid].upgrading_ = self_txn_id;
    request_it->granted_ = true;
    lock_table_[rid].cv_.notify_all();
    return true;
  }

  bool can_grant = CanGrant(txn, rid, LockOpType::SHARED_OP, request_it);
  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    if (txn->GetState() == TransactionState::ABORTED) {
      lock_table_[rid].cv_.notify_all();
      throw TransactionAbortException(txn->GetTransactionId(), AbortReason::DEADLOCK);
    }
    can_grant = CanGrant(txn, rid, LockOpType::SHARED_OP, request_it);
  }
  txn->GetSharedLockSet()->emplace(rid);
  lock_table_[rid].upgrading_ = self_txn_id;
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    return false;
  }
  if (txn->IsExclusiveLocked(rid)) {
    return true;
  }
  txn_id_t self_txn_id = txn->GetTransactionId();
  LockRequest request(self_txn_id, LockMode::EXCLUSIVE);
  lock_table_[rid].request_queue_.push_front(request);
  auto request_it = lock_table_[rid].request_queue_.begin();
  bool can_grant = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, request_it);
  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    if (txn->GetState() == TransactionState::ABORTED) {
      lock_table_[rid].cv_.notify_all();
      throw TransactionAbortException(txn->GetTransactionId(), AbortReason::DEADLOCK);
    }
    can_grant = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, request_it);
  }
  lock_table_[rid].upgrading_ = self_txn_id;
  txn->GetExclusiveLockSet()->emplace(rid);
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;
}

bool LockManager::LockUpgrade(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  if (txn->IsExclusiveLocked(rid)) {
    return true;
  }
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    if (txn->GetState() == TransactionState::SHRINKING) {
      throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
    }
    return false;
  }
  auto find_it = lock_table_[rid].request_queue_.begin();
  for (; find_it != lock_table_[rid].request_queue_.end(); find_it++) {
    if (find_it->granted_ && find_it->lock_mode_ == LockMode::SHARED && find_it->txn_id_ == txn->GetTransactionId()) {
      break;
    }
  }
  if (find_it == lock_table_[rid].request_queue_.end()) {
    return false;
  }
  bool can_upgrade = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, find_it);
  while (!can_upgrade) {
    lock_table_[rid].cv_.wait(guard);
    if (txn->GetState() == TransactionState::ABORTED) {
      throw TransactionAbortException(txn->GetTransactionId(), AbortReason::DEADLOCK);
    }
    can_upgrade = CanGrant(txn, rid, LockOpType::EXCLUSIVE_OP, find_it);
  }

  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);
  lock_table_[rid].upgrading_ = txn->GetTransactionId();
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
  for (auto reqit = lock_table_[rid].request_queue_.begin(); reqit != lock_table_[rid].request_queue_.end();) {
    if (reqit->txn_id_ == txn_id && reqit->granted_) {
      reqit = lock_table_[rid].request_queue_.erase(reqit);
    } else {
      reqit++;
    }
  }
  if (txn->GetState() != TransactionState::GROWING) {
    lock_table_[rid].cv_.notify_all();
    return true;
  }
  if (txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ) {
    txn->SetState(TransactionState::SHRINKING);
  }
  lock_table_[rid].cv_.notify_all();
  return true;
}

}  // namespace bustub
