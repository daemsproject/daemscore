// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"
#include "pow/tromp/equi_miner.h"

#include "amount.h"
#include "chainparams.h"
#include "consensus/consensus.h"
#include "consensus/validation.h"
#include "hash.h"
#include "main.h"
#include "metrics.h"
#include "net.h"
#include "pow.h"
#include "primitives/transaction.h"
#include "random.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "base58.h"
#include "utilstrencodings.h"
#include "core_io.h"
#ifdef ENABLE_WALLET
#include "crypto/equihash.h"
#include "wallet/wallet.h"
#include <functional>
#endif

#include "sodium.h"

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <mutex>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// BitcoinMiner
//

//
// Unconfirmed transactions in the memory pool often depend on other
// transactions in the memory pool. When we select transactions from the
// pool, we select by highest priority or fee rate, so we might consider
// transactions that depend on transactions that aren't yet in the block.
// The COrphan class keeps track of these 'temporary orphans' while
// CreateBlock is figuring out which transactions to include.
//

static uint64_t nPoolMiningResult=0;
static bool fPoolMiningFinished=false;
class COrphan
{
public:
    const CTransaction* ptx;
    set<uint256> setDependsOn;
    CFeeRate feeRate;
    double dPriority;

    COrphan(const CTransaction* ptxIn) : ptx(ptxIn), feeRate(0), dPriority(0)
    {
    }
};

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;

// We want to sort transactions by priority and fee rate, so:
typedef boost::tuple<double, CFeeRate, const CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;

public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) { }

    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee)
        {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        }
        else
        {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

void UpdateTime(CBlockHeader* pblock, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{
    pblock->nTime = std::max(pindexPrev->GetMedianTimePast()+1, GetAdjustedTime());

    // Updating time can change work required on testnet:
    if (consensusParams.fPowAllowMinDifficultyBlocks)
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock, consensusParams);
}

CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn,const int nHeightIn)
{
    const CChainParams& chainparams = Params();
    // Create new block
    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if(!pblocktemplate.get())
        return NULL;
    CBlock *pblock = &pblocktemplate->block; // pointer for convenience
    CBlockIndex* pindexPrev;
    if(nHeightIn<=0)
    {
        pindexPrev = chainActive.Tip();

    }
    else
        pindexPrev = chainActive[nHeightIn-1];
    int nHeight = pindexPrev->nBlockHeight + 1;
    pblock->nBlockHeight=nHeight;
    UpdateTime(pblock, pindexPrev);
    // -regtest only: allow overriding block.nVersion with
    // -blockversion=N to test forking scenarios
    if (Params().MineBlocksOnDemand())
        pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

    // Create coinbase tx
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vin[0].prevout.n=nHeight;    
    txNew.vin[0].scriptSig=CScript()<<0;
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKeyIn;
    
    txNew.vout[0].nLockTime=nHeight +COINBASE_MATURITY;

    // Add dummy coinbase tx as first transaction
    pblock->vtx.push_back(CTransaction());
    pblocktemplate->vTxFees.push_back(-1); // updated at end
    pblocktemplate->vTxSigOps.push_back(-1); // updated at end

    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    // Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
    nBlockMaxSize = std::max((unsigned int)1000, std::min((unsigned int)(MAX_BLOCK_SIZE-1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);
    //min tx size to judge finish of block.set this a little bit higher so as to make mining faster
    unsigned int nMinTxSize=200;
    // Collect memory pool transactions into the block
    CAmount nFees = 0;
    uint64_t nBlockSize = 1000;
    uint64_t nBlockTx = 0;
    int nBlockSigOps = 100;
    {
        LOCK2(cs_main, mempool.cs);
                if(nHeightIn<=0)
        {
        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        pblock->nTime = GetAdjustedTime();
        const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();
        CCoinsViewCache view(pcoinsTip);

        // Priority order to process transactions
        list<COrphan> vOrphan; // list memory doesn't move
        map<uint256, vector<COrphan*> > mapDependers;
        bool fPrintPriority = GetBoolArg("-printpriority", false);

        // This vector will be sorted into a priority queue:
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (map<uint256, CTxMemPoolEntry>::iterator mi = mempool.mapTx.begin();
             mi != mempool.mapTx.end(); ++mi)
        {
            const CTransaction& tx = mi->second.GetTx();

            int64_t nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST)
                                    ? nMedianTimePast
                                    : pblock->GetBlockTime();

            if (tx.IsCoinBase() || !IsFinalTx(tx, nHeight, nLockTimeCutoff))
                continue;

            COrphan* porphan = NULL;
            double dPriority = 0;
            CAmount nTotalIn = 0;
            bool fMissingInputs = false;
            BOOST_FOREACH(const CTxIn& txin, tx.vin)
            {
                // Read prev transaction
                if (!view.HaveCoins(txin.prevout.hash))
                {
                        //don't take in transactions with prevout in mempool,because txs are queued by fee, not sequence, we can't guarantee it's
                        //previous tx can be included in this block
                        fMissingInputs = true;
                        break;
                    // This should never happen; all transactions in the memory
                    // pool should connect to either transactions in the chain
                    // or other transactions in the memory pool.
                    if (!mempool.mapTx.count(txin.prevout.hash))
                    {
                        LogPrintf("ERROR: mempool transaction missing input\n");
                        if (fDebug) assert("mempool transaction missing input" == 0);
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    // Has to wait for dependencies
                    if (!porphan)
                    {
                        // Use list for automatic deletion
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx[txin.prevout.hash].GetTx().vout[txin.prevout.n].nValue;
                    continue;
                }
                const CCoins* coins = view.AccessCoins(txin.prevout.hash);
                assert(coins);
                     if ((int64_t)coins->vout[txin.prevout.n].nLockTime >= ((int64_t)coins->vout[txin.prevout.n].nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nHeight :  std::min((int64_t)pindexPrev->nTime,std::max((int64_t)pindexPrev->GetMedianTimePast()+1, (int64_t)GetAdjustedTime()))))
                         fMissingInputs = true;
                CAmount nValueIn = coins->vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = nHeight - coins->nHeight;

                dPriority += (double)nValueIn * nConf;
            }
            nTotalIn += tx.GetJoinSplitValueIn();

            if (fMissingInputs) continue;

            // Priority is sum(valuein * age) / modified_txsize
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority = tx.ComputePriority(dPriority, nTxSize);

            uint256 hash = tx.GetHash();
            mempool.ApplyDeltas(hash, dPriority, nTotalIn);

            CFeeRate feeRate(nTotalIn-tx.GetValueOut(), nTxSize);

            if (porphan)
            {
                porphan->dPriority = dPriority;
                porphan->feeRate = feeRate;
            }
            else
                vecPriority.push_back(TxPriority(dPriority, feeRate, &mi->second.GetTx()));
        }

        // Collect transactions into block
        uint64_t nBlockSize = 1000;
        uint64_t nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        while (!vecPriority.empty())
        {
            // Take highest priority transaction off the priority queue:
            double dPriority = vecPriority.front().get<0>();
            CFeeRate feeRate = vecPriority.front().get<1>();
            const CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            // Size limits
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            // Legacy limits on sigOps:
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;

            // Skip free transactions if we're past the minimum block size:
            const uint256& hash = tx.GetHash();
            double dPriorityDelta = 0;
            CAmount nFeeDelta = 0;
            mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
            if (fSortedByFee && (dPriorityDelta <= 0) && (nFeeDelta <= 0) && (feeRate < ::minRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
                continue;

            // Prioritise by fee once past the priority size or we run out of high-priority
            // transactions:
            if (!fSortedByFee &&
                ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority)))
            {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            if (!view.HaveInputs(tx))
                continue;
                CAmount nTxFees = tx.GetFee();
            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;
            // Note that flags: we don't want to set mempool/IsStandard()
            // policy here, but we still have to ensure that the block we
            // create only contains transactions that are valid in new blocks.
            CValidationState state;
            if (!ContextualCheckInputs(tx, tx,state, view, pblock,true, MANDATORY_SCRIPT_VERIFY_FLAGS, true, Params().GetConsensus()))
                continue;

            UpdateCoins(tx, state, view, nHeight);

            // Added
            pblock->vtx.push_back(tx);
            pblocktemplate->vTxFees.push_back(nTxFees);
            pblocktemplate->vTxSigOps.push_back(nTxSigOps);
            nBlockSize += nTxSize;
            ++nBlockTx;
            nBlockSigOps += nTxSigOps;
            nFees += nTxFees;
                if (nBlockSize+nMinTxSize>nBlockMaxSize) 
                    break;
            }

            // Add transactions that depend on this one to the priority queue
            if (mapDependers.count(hash))
            {
                BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
                {
                    if (!porphan->setDependsOn.empty())
                    {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty())
                        {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->feeRate, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
        CBlock prevBlock;
        ReadBlockFromDisk(prevBlock, pindexPrev);
        CAmount prevCoinbaseFee=prevBlock.vtx[0].GetFee();
        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        LogPrintf("CreateNewBlock(): total size %u\n", nBlockSize);

        // Compute final coinbase transaction.
        CAmount coinbaseInput=GetBlockValue(nHeight, nFees)+prevCoinbaseFee;        
        txNew.vin[0].prevout.nValue = coinbaseInput;        
        txNew.vout[0].nValue = 0;
        CAmount coinbaseFee=CFeeRate(DEFAULT_TRANSACTION_FEE).GetFee(txNew.GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION)+10);
        CAmount coinbaseOutput=coinbaseInput-coinbaseFee;
    if(coinbaseOutput<=minRelayTxFee.GetFee(DUST_THRESHOLD))
    {
         if(nHeightIn<=0)
            return NULL;  
            txNew.vout[0].nValue =0;
        }
    else
        txNew.vout[0].nValue =coinbaseOutput;
        pblock->vtx[0] = txNew;
        pblocktemplate->vTxFees[0] = -nFees;

        // Randomise nonce
        arith_uint256 nonce = UintToArith256(GetRandHash());
        // Clear the top and bottom 16 bits (for local use as thread flags and counters)
        nonce <<= 32;
        nonce >>= 16;
        pblock->nNonce = ArithToUint256(nonce);

        // Fill in header
        pblock->hashPrevBlock  = pindexPrev->GetBlockHash();
        pblock->hashReserved   = uint256();
        UpdateTime(pblock, Params().GetConsensus(), pindexPrev);
        pblock->nBits          = GetNextWorkRequired(pindexPrev, pblock, Params().GetConsensus());
        pblock->nSolution.clear();
        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (nHeightIn<=0&&!TestBlockValidity(state, *pblock, pindexPrev, false, false))
        {
            LogPrintf("CreateNewBlock() : TestBlockValidity failed \n" );
            return NULL;
          //  throw std::runtime_error("CreateNewBlock() : TestBlockValidity failed");
    }
    }
    return pblocktemplate.release();
}

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock)
    {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
//    unsigned int nHeight = pindexPrev->nHeight+1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    
    txCoinbase.vout[0].strContent = strprintf("%s",nExtraNonce);
    //txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}
static boost::thread_group* minerThreads = NULL;
#ifdef ENABLE_WALLET
//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//
double dHashesPerSec = 0.0;
int64_t nHPSTimerStart = 0;

CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey)
{
//    CPubKey pubkey;
//    if (!reservekey.GetReservedKey(pubkey))
//        return NULL;
//
    CBitcoinAddress address;
    address.Set(miningID);
    CScript scriptPubKey = GetScriptForDestination(address.Get());
    CBlockTemplate* pBlockTemplate=NULL;
    while(pBlockTemplate==NULL)
        pBlockTemplate=CreateNewBlock(scriptPubKey);
    return pBlockTemplate;
}

bool ProcessBlockFound(CBlock* pblock, CWallet& wallet, CPubKey& miningID,bool fExtendID)
{
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("DamesMiner: generated block is stale");
    }

    // Remove key from key pool
    //reservekey.KeepKey();

    // Track how many getdata requests this block gets
    {
        LOCK(wallet.cs_wallet);
        wallet.mapRequestCount[pblock->GetHash()] = 0;
    }

    // Process this block the same as if we had received it from another node
    CValidationState state;
    if (!ProcessNewBlock(state, NULL, pblock, true, NULL))
        return error("ZcashMiner: ProcessNewBlock, block not accepted");

    TrackMinedBlock(pblock->GetHash());

    return true;
}

void BitcoinMiner(CWallet *pwallet,bool fExtendID)
{
    LogPrintf("DaemsMiner started\n");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("daems-miner");
    const CChainParams& chainparams = Params();

    // Each thread has its own key and counter
    CReserveKey reservekey(pwallet);
    unsigned int nExtraNonce = 0;
    CPubKey miningID;
    if(fExtendID)
        pwallet->GenerateNewKey(miningID);
    else
        miningID=pwallet->GetID();
    unsigned int n = chainparams.EquihashN();
    unsigned int k = chainparams.EquihashK();

    std::string solver = GetArg("-equihashsolver", "default");
    assert(solver == "tromp" || solver == "default");
    LogPrint("pow", "Using Equihash solver \"%s\" with n = %u, k = %u\n", solver, n, k);

    std::mutex m_cs;
    bool cancelSolver = false;
    boost::signals2::connection c = uiInterface.NotifyBlockTip.connect(
        [&m_cs, &cancelSolver](const uint256& hashNewTip) mutable {
            std::lock_guard<std::mutex> lock{m_cs};
            cancelSolver = true;
        }
    );

    try {
        while (true) {
            if (chainparams.MiningRequiresPeers()) {
                // Busy-wait for the network to come online so we don't waste time mining
                // on an obsolete chain. In regtest mode we expect to fly solo.
                do {
                    bool fvNodesEmpty;
                    {
                        LOCK(cs_vNodes);
                        fvNodesEmpty = vNodes.empty();
                    }
                    if (!fvNodesEmpty && !IsInitialBlockDownload())
                        break;
                    MilliSleep(1000);
                } while (true);
            }

            //
            // Create new block
            //
            unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrev = chainActive.Tip();

            unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlockWithKey(reservekey));
            if (!pblocktemplate.get())
            {
                MilliSleep(10);
                continue;
            }
            CBlock *pblock = &pblocktemplate->block;
            //IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);
            pblock->hashMerkleRoot = pblock->BuildMerkleTree();
            //pblock->nBlockHeight=pindexPrev->nBlockHeight+1;
            unsigned int rounds=(unsigned int)int(16*sqrt((double)pblock->nBlockHeight));
            LogPrintf("Running FaicoinMiner with %u transactions in block (%u bytes),%u rounds mhash\n", pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION),rounds);            

            LogPrintf("Running ZcashMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            //
            // Search
            //
            int64_t nStart = GetTime();
            arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);

            while (true) {
                // Hash state
                crypto_generichash_blake2b_state state;
                EhInitialiseState(n, k, state);

                // I = the block header minus nonce and solution.
                CEquihashInput I{*pblock};
                CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                ss << I;

                // H(I||...
                crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

                // H(I||V||...
                crypto_generichash_blake2b_state curr_state;
                curr_state = state;
                crypto_generichash_blake2b_update(&curr_state,
                                                  pblock->nNonce.begin(),
                                                  pblock->nNonce.size());

                // (x_1, x_2, ...) = A(I, V, n, k)
                LogPrint("pow", "Running Equihash solver \"%s\" with nNonce = %s\n",
                         solver, pblock->nNonce.ToString());

                std::function<bool(std::vector<unsigned char>)> validBlock =
                        [&pblock, &hashTarget, &pwallet, &reservekey, &m_cs, &cancelSolver, &chainparams]
                        (std::vector<unsigned char> soln) {
                    // Write the solution to the hash and compute the result.
                    LogPrint("pow", "- Checking solution against target\n");
                    pblock->nSolution = soln;
                    solutionTargetChecks.increment();

                    if (UintToArith256(pblock->GetHash()) > hashTarget) {
                        return false;
                    }

                    // Found a solution
                    SetThreadPriority(THREAD_PRIORITY_NORMAL);
                    LogPrintf("ZcashMiner:\n");
                    LogPrintf("proof-of-work found  \n  hash: %s  \ntarget: %s\n", pblock->GetHash().GetHex(), hashTarget.GetHex());
                    if (ProcessBlockFound(pblock, *pwallet, reservekey)) {
                        // Ignore chain updates caused by us
                        std::lock_guard<std::mutex> lock{m_cs};
                        cancelSolver = false;
                    }
                    SetThreadPriority(THREAD_PRIORITY_LOWEST);

                    // In regression test mode, stop mining after a block is found.
                    if (chainparams.MineBlocksOnDemand()) {
                        // Increment here because throwing skips the call below
                        ehSolverRuns.increment();
                        throw boost::thread_interrupted();
                    }

                    return true;
                };
                std::function<bool(EhSolverCancelCheck)> cancelled = [&m_cs, &cancelSolver](EhSolverCancelCheck pos) {
                    std::lock_guard<std::mutex> lock{m_cs};
                    return cancelSolver;
                };

                // TODO: factor this out into a function with the same API for each solver.
                if (solver == "tromp") {
                    // Create solver and initialize it.
                    equi eq(1);
                    eq.setstate(&curr_state);

                    // Intialization done, start algo driver.
                    eq.digit0(0);
                    eq.xfull = eq.bfull = eq.hfull = 0;
                    eq.showbsizes(0);
                    for (u32 r = 1; r < WK; r++) {
                        (r&1) ? eq.digitodd(r, 0) : eq.digiteven(r, 0);
                        eq.xfull = eq.bfull = eq.hfull = 0;
                        eq.showbsizes(r);
                    }
                    eq.digitK(0);
                    ehSolverRuns.increment();

                    // Convert solution indices to byte array (decompress) and pass it to validBlock method.
                    for (size_t s = 0; s < eq.nsols; s++) {
                        LogPrint("pow", "Checking solution %d\n", s+1);
                        std::vector<eh_index> index_vector(PROOFSIZE);
                        for (size_t i = 0; i < PROOFSIZE; i++) {
                            index_vector[i] = eq.sols[s][i];
                        }
                        std::vector<unsigned char> sol_char = GetMinimalFromIndices(index_vector, DIGITBITS);

                        if (validBlock(sol_char)) {
                            // If we find a POW solution, do not try other solutions
                            // because they become invalid as we created a new block in blockchain.
                            break;
                        }
                    }
                } else {
                    try {
                        // If we find a valid block, we rebuild
                        bool found = EhOptimisedSolve(n, k, curr_state, validBlock, cancelled);
                        ehSolverRuns.increment();
                        if (found) {
                            break;
                        }
                    } catch (EhSolverCancelledException&) {
                        LogPrint("pow", "Equihash solver cancelled\n");
                        std::lock_guard<std::mutex> lock{m_cs};
                        cancelSolver = false;
                    }
                }

                // Check for stop or if block needs to be rebuilt
                boost::this_thread::interruption_point();
                // Regtest mode doesn't require peers
                if (vNodes.empty() && chainparams.MiningRequiresPeers())
                    break;
                if ((UintToArith256(pblock->nNonce) & 0xffff) == 0xffff)
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;

                // Update nNonce and nTime
                pblock->nNonce = ArithToUint256(UintToArith256(pblock->nNonce) + 1);
                UpdateTime(pblock, chainparams.GetConsensus(), pindexPrev);
                if (chainparams.GetConsensus().fPowAllowMinDifficultyBlocks)
                {
                    // Changing pblock->nTime can change work required on testnet:
                    hashTarget.SetCompact(pblock->nBits);
                }
            }
        }
    }
    catch (const boost::thread_interrupted&)
    {
        c.disconnect();
        LogPrintf("ZcashMiner terminated\n");
        throw;
    }
    catch (const std::runtime_error &e)
    {
        c.disconnect();
        LogPrintf("ZcashMiner runtime error: %s\n", e.what());
        return;
    }
    c.disconnect();
}

void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads,bool fExtendID)
{
    static boost::thread_group* minerThreads = NULL;


    if (nThreads < 0) {
        // In regtest threads defaults to 1
        if (Params().DefaultMinerThreads())
            nThreads = Params().DefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }

    if (minerThreads != NULL)
    {
        minerThreads->interrupt_all();
        fPoolMiningFinished=true;
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++)
        minerThreads->create_thread(boost::bind(&BitcoinMiner, pwallet,fExtendID));
}

#endif // ENABLE_WALLET

static CCriticalSection pool_mine_cs1;
static CCriticalSection pool_mine_cs2;
static CCriticalSection cs_minespeed;
void PoolMiningThread(CBlockHeader& block,uint64_t nNonceBegin,uint64_t nNonceEnd,uint32_t nbit)
{
    //LogPrintf("FaicoinpoolMiner started header:nbit%i time%i, height%i\n",block.nBits,block.nTime,block.nBlockHeight);
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("faicoin-poolminer");
    CBlockHeader block1=block;   
    block1.nNonce=nNonceBegin==0?1:nNonceBegin;

    try {
        
            

           
            //unsigned int rounds=(unsigned int)int(16*sqrt((double)block1.nBlockHeight));
            //LogPrintf("Running FaicoinMiner with %u rounds mhash\n", rounds);            
            
            //
            // Search
            //
            int64_t nStart = GetTime();
            //int64_t nTimePrev = nStart;
            uint256 hashTarget = uint256().SetCompact(nbit==0?block1.nBits:nbit);
            //LogPrintf("hashTarget:%s\n", hashTarget.GetHex()); 
            uint256 thash;
            while (true) {
                unsigned int nHashesDone = 0;
                //block1.nNonce += 1;
                while(true)
                {
                    boost::this_thread::interruption_point();
                    thash=block1.GetHash();
                    
                    mixHash(&thash,(unsigned int)block1.nBlockHeight);
                    //LogPrintf("hash:%s\n", thash.GetHex()); 
                    if (thash <= hashTarget)
                    {
                        // Found a solution
                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        //LogPrintf("FaicoinpoolMiner:\n");
                        LogPrintf("proof-of-work found  \n  powhash: %s  \ntarget: %s\n nonce:%lld\n", thash.GetHex(), hashTarget.GetHex(),block1.nNonce);
//                        thash=block1.GetHash();
//                        mixHash(&thash,(unsigned int)block1.nBlockHeight);
//                    LogPrintf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nBlockHeight=%d, nTime=%u, nBits=%08x, nNonce=%u)\n",
//        block1.GetHash().ToString(),
//        block1.nVersion,
//        block1.hashPrevBlock.ToString(),
//        block1.hashMerkleRoot.ToString(),
//        block1.nBlockHeight,
//        block1.nTime, block1.nBits, block1.nNonce
//        );
//                        LogPrintf("recalc powhash:%s \n",thash.GetHex());
                        //TODO feedback 
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);
                        
                        {
                            LOCK(pool_mine_cs1);     
                        nPoolMiningResult=block1.nNonce ;
                            // throw boost::thread_interrupted();
                        }
                           return;
                    }
                    block1.nNonce += 1;
                    nHashesDone += 1;
                    if ((block1.nNonce & 0xFF) == 0)
                        break;
                }

                // Meter hashes/sec
                static int64_t nHashCounter;
                if (nHPSTimerStart == 0)
                {
                    nHPSTimerStart = GetTimeMillis();
                    nHashCounter = 0;
                }
                else
                    nHashCounter += nHashesDone;
                if (GetTimeMillis() - nHPSTimerStart > 4000)
                {
                    
                    {
                        LOCK(cs_minespeed);
                        if (GetTimeMillis() - nHPSTimerStart > 4000)
                        {
                            dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                            static int64_t nLogTime;
                            if (GetTime() - nLogTime > 30 * 60)
                            {
                                nLogTime = GetTime();
                                LogPrintf("hashmeter %6.3f khash/s\n", dHashesPerSec/1000.0);
                            }
                        }
                    }
                }
//                if(GetTime()-nTimePrev>1)
//                {
//                    block.nTime++;
//                    nTimePrev=GetTime();
//                   
//                    
//                }
                // Check for stop or if block needs to be rebuilt
                
                
                if (block1.nNonce > nNonceEnd||(GetTime()-nStart)>60)
                {
                    //static CCriticalSection cs;
                    {
                        LOCK(pool_mine_cs2);
                        fPoolMiningFinished=true;
                        //LogPrintf("FaicoinpoolMiner finished:\n");
                        
                    }
                    return;// throw boost::thread_interrupted();
                }
                
            }
        
    }
    catch (boost::thread_interrupted)
    {
        //LogPrintf("FaicoinMiner terminated\n");
       // if(pwallet!=pwalletMain&&pwallet!=NULL)
         //   delete pwallet;
        throw;
    }
}
uint64_t PoolMiner(bool fGenerate,CBlockHeader block,uint64_t nNonceBegin,uint64_t nNonceEnd,int nThreads,uint32_t nbit)
{
    
    if (nThreads < 0) {
        // In regtest threads defaults to 1
        if (Params().DefaultMinerThreads())
            nThreads = Params().DefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }
     //LogPrintf("PoolMiner nThreads %i\n",nThreads);
    if (minerThreads != NULL)
    {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return 0;

    minerThreads = new boost::thread_group();
    uint64_t step=(uint64_t)((nNonceEnd-nNonceBegin)/nThreads);
    //LogPrintf("PoolMiner step %i\n",step);
    {
        LOCK2(pool_mine_cs1,pool_mine_cs2);
        fPoolMiningFinished=false;
        nPoolMiningResult=0;
    }
    for (int i = 0; i < nThreads; i++)
        minerThreads->create_thread(boost::bind(&PoolMiningThread, block,nNonceBegin+step*i,nNonceBegin+step*(i+1),nbit));
    
    while(true)
    {
        MilliSleep(10);
        
           uint64_t pmr=0;
            {
                LOCK(pool_mine_cs1);
                pmr=nPoolMiningResult;
            }
            if(pmr>0)
            {
                LogPrintf("PoolMiner nPoolMiningResult %lld\n",pmr);
                minerThreads->interrupt_all();
                delete minerThreads;
                minerThreads = NULL;
                return pmr;
            }
        
        //LogPrintf("PoolMiner minerThreads %i\n",minerThreads->size());
           bool fpmf=false;
        {
            LOCK(pool_mine_cs2);
            fpmf=fPoolMiningFinished;
        }
            if(fpmf)
            {

                //LogPrintf("PoolMiner Finished at nonce:%i \n",nNonceEnd);
                minerThreads->interrupt_all();
                delete minerThreads;
                minerThreads = NULL;
                return 0;
            }
          
    }  
    
}
#endif // ENABLE_WALLET
