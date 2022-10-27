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

bool LockManager::LockShared(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  auto curr_state = txn->GetState();
  if (curr_state != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    return false;
  }
  txn->GetSharedLockSet()->emplace(rid);
  txn_id_t self_txn_id = txn->GetTransactionId();
  bool can_grant = true;
  auto start_it = lock_table_[rid].request_queue_.begin();
  for (auto reit = start_it;
       reit != lock_table_[rid].request_queue_.end(); reit++) {
    if (!reit->granted_) {
      can_grant = false;
      break;
    }
  }
  can_grant &= !(start_it->txn_id_ != self_txn_id && start_it->lock_mode_ == LockMode::EXCLUSIVE
                 && start_it->txn_id_ != INVALID_TXN_ID);

  LockRequest request(self_txn_id, LockMode::SHARED);
  lock_table_[rid].request_queue_.push_front(request);
  auto request_it = lock_table_[rid].request_queue_.begin();

  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    can_grant = true;
    for (auto reit = start_it; reit != lock_table_[rid].request_queue_.end(); reit++) {
      if (!reit->granted_) {
        can_grant = false;
        break;
      }
    }
    can_grant &= !(start_it->txn_id_ != self_txn_id && start_it->lock_mode_ == LockMode::EXCLUSIVE
                   && start_it->txn_id_ != INVALID_TXN_ID);
  }

  lock_table_[rid].upgrading_ = self_txn_id;
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;
}

bool LockManager::LockExclusive(Transaction *txn, const RID &rid) {
  std::unique_lock<std::mutex> guard(latch_);
  auto curr_state = txn->GetState();
  if (curr_state != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    return false;
  }
  txn->GetExclusiveLockSet()->emplace(rid);
  txn_id_t self_txn_id = txn->GetTransactionId();
  bool can_grant = true;
  auto start_it = lock_table_[rid].request_queue_.begin();
  for (auto reit = start_it;
       reit != lock_table_[rid].request_queue_.end(); reit++) {
    if (!reit->granted_) {
      can_grant = false;
      break;
    }
  }
  can_grant &= (start_it->txn_id_ == self_txn_id || start_it->txn_id_ == INVALID_TXN_ID);

  LockRequest request(self_txn_id, LockMode::EXCLUSIVE);
  lock_table_[rid].request_queue_.push_front(request);
  auto request_it = lock_table_[rid].request_queue_.begin();

  while (!can_grant) {
    lock_table_[rid].cv_.wait(guard);
    can_grant = true;
    for (auto reit = start_it; reit != lock_table_[rid].request_queue_.end(); reit++) {
      if (!reit->granted_) {
        can_grant = false;
        break;
      }
    }
    can_grant &= (start_it->txn_id_ == self_txn_id || start_it->txn_id_ == INVALID_TXN_ID);
  }

  lock_table_[rid].upgrading_ = self_txn_id;
  request_it->granted_ = true;
  lock_table_[rid].cv_.notify_all();

  return true;

}

bool LockManager::LockUpgrade(Transaction *txn, const RID &rid) {
  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->emplace(rid);
  return true;
}

bool LockManager::Unlock(Transaction *txn, const RID &rid) {

  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);

  if (lock_table_[rid].upgrading_ == txn->GetTransactionId()) {
    lock_table_[rid].upgrading_ = INVALID_TXN_ID;
  }

  txn->SetState(TransactionState::SHRINKING);
  return true;
}

}  // namespace bustub
