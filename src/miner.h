// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include <stdint.h>

class CBlock;
class CBlockHeader;
class CBlockIndex;
class CReserveKey;
class CScript;
class CWallet;
class CPubKey;
class CKeyID;

struct CBlockTemplate;

/** Run the miner threads */
void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads,bool fExtendID=false);
/** Generate a new block, without valid proof-of-work */
CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn,const int nHeightIn=0);
CBlockTemplate* CreateNewBlockWithKey(CKeyID& miningID);
/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
/** Check mined block */
bool CheckWork(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey);
void UpdateTime(CBlockHeader* block, const CBlockIndex* pindexPrev);
uint64_t PoolMiner(bool fGenerate,CBlockHeader block,uint64_t nNonceBegin,uint64_t nNonceEnd,int nThreads,uint32_t nbit=0);
extern double dHashesPerSec;
extern int64_t nHPSTimerStart;

#endif // BITCOIN_MINER_H
