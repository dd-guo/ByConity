/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Transaction/CnchProxyTransaction.h>
#include <Common/Exception.h>
#include <Common/PODArray.h>
#include "TransactionCommon.h"
#include <CloudServices/CnchServerClient.h>
#include <Transaction/TransactionCommon.h>
#include <Transaction/TxnTimestamp.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int LOGICAL_ERROR;
    extern const int CNCH_TRANSACTION_COMMIT_TIMEOUT;
    extern const int CNCH_TRANSACTION_COMMIT_ERROR;
    extern const int CNCH_TRANSACTION_ABORT_ERROR;
}

CnchProxyTransaction::CnchProxyTransaction(const ContextPtr & context_, CnchServerClientPtr client, const TxnTimestamp & primary_txn_id, bool read_only)
    : Base(context_), remote_client(std::move(client))
{
    setReadOnly(read_only);

    /// Create remote transaction on target server
    /// @note This is a blocking call
    const auto & [txn_id, start_time] = remote_client->createTransaction(primary_txn_id, read_only);

    TransactionRecord record;
    record.setReadOnly(read_only)
        .setID(txn_id)
        .setInitiator(txnInitiatorToString(CnchTransactionInitiator::Txn))
        .setStatus(CnchTransactionStatus::Running)
        .setType(CnchTransactionType::Implicit)
        .setPrimaryID(primary_txn_id);
    txn_record = std::move(record);
}

void CnchProxyTransaction::precommit()
{
    throw Exception("Proxy transaction does not support precommit", ErrorCodes::LOGICAL_ERROR);
}

TxnTimestamp CnchProxyTransaction::commit()
{
    throw Exception("Proxy transaction does not support commit", ErrorCodes::LOGICAL_ERROR);
}

TxnTimestamp CnchProxyTransaction::commitV2()
{
    throw Exception("Proxy transaction does not support commitV2", ErrorCodes::LOGICAL_ERROR);
}

TxnTimestamp CnchProxyTransaction::abort()
{
    /// With proxy transaction, abort is identical to rollback
    return rollback();
}

TxnTimestamp CnchProxyTransaction::rollback()
{
    if (isReadOnly())
    {
        throw Exception("Invalid rollback operation for read only transaction",
            ErrorCodes::LOGICAL_ERROR);
    }
    else
    {
        /// Call rpc to rollback transaction on target server
        syncTransactionStatus();
        if (getStatus() != CnchTransactionStatus::Aborted)
        {
            setStatus(CnchTransactionStatus::Aborted);
            return remote_client->rollbackTransaction(txn_record.txnID());
        }
        return {};
    }
}

void CnchProxyTransaction::clean(TxnCleanTask &)
{
    /// Call rpc to force finish the transaction
    remote_client->finishTransaction(txn_record.txnID());
}

void CnchProxyTransaction::removeIntermediateData()
{
    if (isReadOnly())
    {
        throw Exception("Invalid removeIntermediateData operation for read only transaction",
            ErrorCodes::LOGICAL_ERROR);
    }
    else
    {
        /// call RPC to clean intermediate data
        remote_client->removeIntermediateData(txn_record.txnID());
    }
    
}

void CnchProxyTransaction::syncTransactionStatus(bool throw_on_missmatch)
{
    /// Call when remote query is done and when precommit the explicit transaction
    auto expected_status = remote_client->getTransactionStatus(txn_record.txnID().toUInt64());
    if (throw_on_missmatch && expected_status != txn_record.status())
    {
        throw Exception("Transaction " + txn_record.txnID().toString() + " status is not consistent with remote server, expected " + String(txnStatusToString(expected_status)) + ", got " + String(txnStatusToString(txn_record.status())), ErrorCodes::LOGICAL_ERROR);
    }
    setStatus(expected_status);
}

void CnchProxyTransaction::setTransactionStatus(CnchTransactionStatus status)
{
    setStatus(status);
}

}
