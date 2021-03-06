#pragma once
/*-------------------------------------------------------------------------
 * This source code is confidential proprietary information which is
 * Copyright (c) 2017 by Great Hill Corporation.
 * All Rights Reserved
 *
 * The LICENSE at the root of this repo details your rights (if any)
 *------------------------------------------------------------------------*/
/*
 * This file was generated with makeClass. Edit only those parts of the code inside
 * of 'EXISTING_CODE' tags.
 */
#include "etherlib.h"
#include "transaction.h"

namespace qblocks {

//--------------------------------------------------------------------------
class CBlock;
typedef SFArrayBase<CBlock>         CBlockArray;
typedef SFList<CBlock*>             CBlockList;
typedef SFUniqueList<CBlock*>       CBlockListU;

// EXISTING_CODE
// EXISTING_CODE

//--------------------------------------------------------------------------
class CBlock : public CBaseNode {
public:
    SFGas gasLimit;
    SFGas gasUsed;
    SFHash hash;
    blknum_t blockNumber;
    SFHash parentHash;
    SFAddress miner;
    uint64_t difficulty;
    double price;
    timestamp_t timestamp;
    CTransactionArray transactions;

public:
    CBlock(void);
    CBlock(const CBlock& bl);
    virtual ~CBlock(void);
    CBlock& operator=(const CBlock& bl);

    DECLARE_NODE(CBlock);

    const CBaseNode *getObjectAt(const SFString& fieldName, uint32_t index) const override;

    // EXISTING_CODE
    // EXISTING_CODE
    friend ostream& operator<<(ostream& os, const CBlock& item);

protected:
    void Clear(void);
    void Init(void);
    void Copy(const CBlock& bl);
    bool readBackLevel(SFArchive& archive) override;

    // EXISTING_CODE
    // EXISTING_CODE
};

//--------------------------------------------------------------------------
inline CBlock::CBlock(void) {
    Init();
    // EXISTING_CODE
    // EXISTING_CODE
}

//--------------------------------------------------------------------------
inline CBlock::CBlock(const CBlock& bl) {
    // EXISTING_CODE
    // EXISTING_CODE
    Copy(bl);
}

// EXISTING_CODE
// EXISTING_CODE

//--------------------------------------------------------------------------
inline CBlock::~CBlock(void) {
    Clear();
    // EXISTING_CODE
    // EXISTING_CODE
}

//--------------------------------------------------------------------------
inline void CBlock::Clear(void) {
    // EXISTING_CODE
    // EXISTING_CODE
}

//--------------------------------------------------------------------------
inline void CBlock::Init(void) {
    CBaseNode::Init();

    gasLimit = 0;
    gasUsed = 0;
    hash = "";
    blockNumber = 0;
    parentHash = "";
    miner = "";
    difficulty = 0;
    price = 0.0;
    timestamp = 0;
    transactions.Clear();

    // EXISTING_CODE
    // EXISTING_CODE
}

//--------------------------------------------------------------------------
inline void CBlock::Copy(const CBlock& bl) {
    Clear();
    CBaseNode::Copy(bl);

    gasLimit = bl.gasLimit;
    gasUsed = bl.gasUsed;
    hash = bl.hash;
    blockNumber = bl.blockNumber;
    parentHash = bl.parentHash;
    miner = bl.miner;
    difficulty = bl.difficulty;
    price = bl.price;
    timestamp = bl.timestamp;
    transactions = bl.transactions;

    // EXISTING_CODE
    // EXISTING_CODE
    finishParse();
}

//--------------------------------------------------------------------------
inline CBlock& CBlock::operator=(const CBlock& bl) {
    Copy(bl);
    // EXISTING_CODE
    // EXISTING_CODE
    return *this;
}

//---------------------------------------------------------------------------
IMPLEMENT_ARCHIVE_ARRAY(CBlockArray);
IMPLEMENT_ARCHIVE_ARRAY_C(CBlockArray);
IMPLEMENT_ARCHIVE_LIST(CBlockList);

//---------------------------------------------------------------------------
extern SFArchive& operator<<(SFArchive& archive, const CBlock& blo);
extern SFArchive& operator>>(SFArchive& archive, CBlock& blo);

//---------------------------------------------------------------------------
// EXISTING_CODE
// EXISTING_CODE
}  // namespace qblocks

