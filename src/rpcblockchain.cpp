// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"
#include "main.h"
#include "rpcserver.h"
#include "sync.h"
#include "util.h"
#include "ccc/content.h"
#include "ccc/link.h"

#include <stdint.h>

#include "json/json_spirit_value.h"
#include "base58.h"
#include "utilstrencodings.h"

using namespace json_spirit;
using namespace std;

static const int DEFAULT_MAX_CONTENTS = 100;
static const int DEFAULT_MAX_CONTENTSBYTES = 10000000;

static const int CONTENT_FORMAT_SIZE = 0;
static const int CONTENT_FORMAT_STR_HEX = 1;
static const int CONTENT_FORMAT_STR_BIN = 2;
static const int CONTENT_FORMAT_STR_B64 = 3;
static const int CONTENT_FORMAT_JSON_HEX = 4;
static const int CONTENT_FORMAT_JSON_BIN = 5;
static const int CONTENT_FORMAT_JSON_B64 = 6;
static const int CONTENT_FORMAT_HUMAN_STR = 7;

enum cttflag{
    CONTENT_FALSE = 0X00,
    CONTENT_SHOW_LINK = 0x01,
    CONTENT_SHOW_POSTER = 0x02,
    CONTENT_SHOW_VALUE = 0x04,
    CONTENT_SHOW_ADDR = 0x08,
    CONTENT_SIMPLE_FORMAT = 0x20 // clink:size
};

extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, Object& entry);
void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out, bool fIncludeHex);

double GetDifficulty(const CBlockIndex* blockindex)
{
    // Floating point number that is a multiple of the minimum difficulty,
    // minimum difficulty = 1.0.
    if (blockindex == NULL)
    {
        if (chainActive.Tip() == NULL)
            return 1.0;
        else
            blockindex = chainActive.Tip();
    }

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x000fffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 30)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 30)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}


Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool txDetails = false)
{
    Object result;
    result.push_back(Pair("hash", block.GetHash().GetHex()));
    int confirmations = -1;
    // Only report confirmations if the block is on the main chain
    if (chainActive.Contains(blockindex))
        confirmations = chainActive.Height() - blockindex->nHeight + 1;
    result.push_back(Pair("confirmations", confirmations));
    result.push_back(Pair("size", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION)));
    result.push_back(Pair("height", blockindex->nHeight));
    result.push_back(Pair("version", block.nVersion));
    result.push_back(Pair("merkleroot", block.hashMerkleRoot.GetHex()));
    Array txs;
    BOOST_FOREACH(const CTransaction&tx, block.vtx)
    {
        if(txDetails)
        {
            Object objTx;
            TxToJSON(tx, uint256(0), objTx);
            txs.push_back(objTx);
        }
        else
            txs.push_back(tx.GetHash().GetHex());
    }
    result.push_back(Pair("tx", txs));
    result.push_back(Pair("time", block.GetBlockTime()));
    result.push_back(Pair("nonce", (uint64_t)block.nNonce));
    result.push_back(Pair("bits", strprintf("%08x", block.nBits)));
    result.push_back(Pair("difficulty", GetDifficulty(blockindex)));
    result.push_back(Pair("chainwork", blockindex->nChainWork.GetHex()));

    if (blockindex->pprev)
        result.push_back(Pair("previousblockhash", blockindex->pprev->GetBlockHash().GetHex()));
    CBlockIndex *pnext = chainActive.Next(blockindex);
    if (pnext)
        result.push_back(Pair("nextblockhash", pnext->GetBlockHash().GetHex()));
    return result;
}


Value getblockcount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockcount\n"
            "\nReturns the number of blocks in the longest block chain.\n"
            "\nResult:\n"
            "n    (numeric) The current block count\n"
            "\nExamples:\n"
            + HelpExampleCli("getblockcount", "")
            + HelpExampleRpc("getblockcount", "")
        );

    return chainActive.Height();
}

Value getbestblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getbestblockhash\n"
            "\nReturns the hash of the best (tip) block in the longest block chain.\n"
            "\nResult\n"
            "\"hex\"      (string) the block hash hex encoded\n"
            "\nExamples\n"
            + HelpExampleCli("getbestblockhash", "")
            + HelpExampleRpc("getbestblockhash", "")
        );

    return chainActive.Tip()->GetBlockHash().GetHex();
}

Value getdifficulty(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getdifficulty\n"
            "\nReturns the proof-of-work difficulty as a multiple of the minimum difficulty.\n"
            "\nResult:\n"
            "n.nnn       (numeric) the proof-of-work difficulty as a multiple of the minimum difficulty.\n"
            "\nExamples:\n"
            + HelpExampleCli("getdifficulty", "")
            + HelpExampleRpc("getdifficulty", "")
        );

    return GetDifficulty();
}


Value getrawmempool(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getrawmempool ( verbose )\n"
            "\nReturns all transaction ids in memory pool as a json array of string transaction ids.\n"
            "\nArguments:\n"
            "1. verbose           (boolean, optional, default=false) true for a json object, false for array of transaction ids\n"
            "\nResult: (for verbose = false):\n"
            "[                     (json array of string)\n"
            "  \"transactionid\"     (string) The transaction id\n"
            "  ,...\n"
            "]\n"
            "\nResult: (for verbose = true):\n"
            "{                           (json object)\n"
            "  \"transactionid\" : {       (json object)\n"
            "    \"size\" : n,             (numeric) transaction size in bytes\n"
            "    \"fee\" : n,              (numeric) transaction fee in cccoins\n"
            "    \"time\" : n,             (numeric) local time transaction entered pool in seconds since 1 Jan 1970 GMT\n"
            "    \"height\" : n,           (numeric) block height when transaction entered pool\n"
            "    \"startingpriority\" : n, (numeric) priority when transaction entered pool\n"
            "    \"currentpriority\" : n,  (numeric) transaction priority now\n"
            "    \"depends\" : [           (array) unconfirmed transactions used as inputs for this transaction\n"
            "        \"transactionid\",    (string) parent transaction id\n"
            "       ... ]\n"
            "  }, ...\n"
            "]\n"
            "\nExamples\n"
            + HelpExampleCli("getrawmempool", "true")
            + HelpExampleRpc("getrawmempool", "true")
        );

    bool fVerbose = false;
    if (params.size() > 0)
        fVerbose = params[0].get_bool();

    if (fVerbose)
    {
        LOCK(mempool.cs);
        Object o;
        BOOST_FOREACH(const PAIRTYPE(uint256, CTxMemPoolEntry)& entry, mempool.mapTx)
        {
            const uint256& hash = entry.first;
            const CTxMemPoolEntry& e = entry.second;
            Object info;
            info.push_back(Pair("size", (int)e.GetTxSize()));
            info.push_back(Pair("fee", ValueFromAmount(e.GetFee())));
            info.push_back(Pair("time", e.GetTime()));
            info.push_back(Pair("height", (int)e.GetHeight()));
            //info.push_back(Pair("startingpriority", e.GetPriority(e.GetHeight())));
            //info.push_back(Pair("currentpriority", e.GetPriority(chainActive.Height())));
            const CTransaction& tx = e.GetTx();
            set<string> setDepends;
            BOOST_FOREACH(const CTxIn& txin, tx.vin)
            {
                if (mempool.exists(txin.prevout.hash))
                    setDepends.insert(txin.prevout.hash.ToString());
            }
            Array depends(setDepends.begin(), setDepends.end());
            info.push_back(Pair("depends", depends));
            o.push_back(Pair(hash.ToString(), info));
        }
        return o;
    }
    else
    {
        vector<uint256> vtxid;
        mempool.queryHashes(vtxid);

        Array a;
        BOOST_FOREACH(const uint256& hash, vtxid)
            a.push_back(hash.ToString());

        return a;
    }
}

Value getblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getblockhash index\n"
            "\nReturns hash of block in best-block-chain at index provided.\n"
            "\nArguments:\n"
            "1. index         (numeric, required) The block index\n"
            "\nResult:\n"
            "\"hash\"         (string) The block hash\n"
            "\nExamples:\n"
            + HelpExampleCli("getblockhash", "1000")
            + HelpExampleRpc("getblockhash", "1000")
        );

    int nHeight = params[0].get_int();
    if (nHeight < 0 || nHeight > chainActive.Height())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block height out of range");

    CBlockIndex* pblockindex = chainActive[nHeight];
    return pblockindex->GetBlockHash().GetHex();
}

bool _getBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex)
{
    if (nHeight < 0 || nHeight > chainActive.Height())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block height out of range");
    pblockindex = chainActive[nHeight];
    
    if (!ReadBlockFromDisk(blockOut, pblockindex))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");
    return true;
}

Value getblockbyheight(const Array& params, bool fHelp)
{


    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblockbyheight index ( verbose )\n"
            "\nIf verbose is false, returns a string that is serialized, hex-encoded data for block 'hash'.\n"
            "If verbose is true, returns an Object with information about block <hash>.\n"
            "\nArguments:\n"
            "1. index         (numeric, required) The block index\n"
            "2. verbose           (boolean, optional, default=true) true for a json object, false for the hex encoded data\n"
            "\nResult (for verbose = true):\n"
            "{\n"
            "  \"hash\" : \"hash\",     (string) the block hash (same as provided)\n"
            "  \"confirmations\" : n,   (numeric) The number of confirmations, or -1 if the block is not on the main chain\n"
            "  \"size\" : n,            (numeric) The block size\n"
            "  \"height\" : n,          (numeric) The block height or index\n"
            "  \"version\" : n,         (numeric) The block version\n"
            "  \"merkleroot\" : \"xxxx\", (string) The merkle root\n"
            "  \"tx\" : [               (array of string) The transaction ids\n"
            "     \"transactionid\"     (string) The transaction id\n"
            "     ,...\n"
            "  ],\n"
            "  \"time\" : ttt,          (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)\n"
            "  \"nonce\" : n,           (numeric) The nonce\n"
            "  \"bits\" : \"1d00ffff\", (string) The bits\n"
            "  \"difficulty\" : x.xxx,  (numeric) The difficulty\n"
            "  \"previousblockhash\" : \"hash\",  (string) The hash of the previous block\n"
            "  \"nextblockhash\" : \"hash\"       (string) The hash of the next block\n"
            "}\n"
            "\nResult (for verbose=false):\n"
            "\"data\"             (string) A string that is serialized, hex-encoded data for block 'hash'.\n"
            "\nExamples:\n"
            + HelpExampleCli("getblockbyheight", "1000")
            + HelpExampleRpc("getblockbyheight", "1000")
            );

    int nHeight = params[0].get_int();
    CBlockIndex* pblockindex;
    CBlock block;
    if(!_getBlockByHeight(nHeight, block, pblockindex))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
    bool fVerbose = true;
    if (params.size() > 1)
        fVerbose = params[1].get_bool();
    if (!fVerbose) {
        CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
        ssBlock << block;
        std::string strHex = HexStr(ssBlock.begin(), ssBlock.end());
        return strHex;
    }
    return blockToJSON(block, pblockindex);
}

Value getblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblock \"hash\" ( verbose )\n"
            "\nIf verbose is false, returns a string that is serialized, hex-encoded data for block 'hash'.\n"
            "If verbose is true, returns an Object with information about block <hash>.\n"
            "\nArguments:\n"
            "1. \"hash\"          (string, required) The block hash\n"
            "2. verbose           (boolean, optional, default=true) true for a json object, false for the hex encoded data\n"
            "\nResult (for verbose = true):\n"
            "{\n"
            "  \"hash\" : \"hash\",     (string) the block hash (same as provided)\n"
            "  \"confirmations\" : n,   (numeric) The number of confirmations, or -1 if the block is not on the main chain\n"
            "  \"size\" : n,            (numeric) The block size\n"
            "  \"height\" : n,          (numeric) The block height or index\n"
            "  \"version\" : n,         (numeric) The block version\n"
            "  \"merkleroot\" : \"xxxx\", (string) The merkle root\n"
            "  \"tx\" : [               (array of string) The transaction ids\n"
            "     \"transactionid\"     (string) The transaction id\n"
            "     ,...\n"
            "  ],\n"
            "  \"time\" : ttt,          (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)\n"
            "  \"nonce\" : n,           (numeric) The nonce\n"
            "  \"bits\" : \"1d00ffff\", (string) The bits\n"
            "  \"difficulty\" : x.xxx,  (numeric) The difficulty\n"
            "  \"previousblockhash\" : \"hash\",  (string) The hash of the previous block\n"
            "  \"nextblockhash\" : \"hash\"       (string) The hash of the next block\n"
            "}\n"
            "\nResult (for verbose=false):\n"
            "\"data\"             (string) A string that is serialized, hex-encoded data for block 'hash'.\n"
            "\nExamples:\n"
            + HelpExampleCli("getblock", "\"0c3b2c31c8aa025e5ae7a87dfe63d1795a061b95e7b00aee61e5384338a26739\"")
            + HelpExampleRpc("getblock", "\"0c3b2c31c8aa025e5ae7a87dfe63d1795a061b95e7b00aee61e5384338a26739\"")
        );

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);

    bool fVerbose = true;
    if (params.size() > 1)
        fVerbose = params[1].get_bool();

    if (mapBlockIndex.count(hash) == 0)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];

    if(!ReadBlockFromDisk(block, pblockindex))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    if (!fVerbose)
    {
        CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
        ssBlock << block;
        std::string strHex = HexStr(ssBlock.begin(), ssBlock.end());
        return strHex;
    }

    return blockToJSON(block, pblockindex);
}

Value gettxoutsetinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "gettxoutsetinfo\n"
            "\nReturns statistics about the unspent transaction output set.\n"
            "Note this call may take some time.\n"
            "\nResult:\n"
            "{\n"
            "  \"height\":n,     (numeric) The current block height (index)\n"
            "  \"bestblock\": \"hex\",   (string) the best block hash hex\n"
            "  \"transactions\": n,      (numeric) The number of transactions\n"
            "  \"txouts\": n,            (numeric) The number of output transactions\n"
            "  \"bytes_serialized\": n,  (numeric) The serialized size\n"
            "  \"hash_serialized\": \"hash\",   (string) The serialized hash\n"
            "  \"total_amount\": x.xxx          (numeric) The total amount\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("gettxoutsetinfo", "")
            + HelpExampleRpc("gettxoutsetinfo", "")
        );

    Object ret;

    CCoinsStats stats;
    FlushStateToDisk();
    if (pcoinsTip->GetStats(stats)) {
        ret.push_back(Pair("height", (int64_t)stats.nHeight));
        ret.push_back(Pair("bestblock", stats.hashBlock.GetHex()));
        ret.push_back(Pair("transactions", (int64_t)stats.nTransactions));
        ret.push_back(Pair("txouts", (int64_t)stats.nTransactionOutputs));
        ret.push_back(Pair("bytes_serialized", (int64_t)stats.nSerializedSize));
        ret.push_back(Pair("hash_serialized", stats.hashSerialized.GetHex()));
        ret.push_back(Pair("total_amount", ValueFromAmount(stats.nTotalAmount)));
    }
    return ret;
}

Value gettxout(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "gettxout \"txid\" n ( includemempool )\n"
            "\nReturns details about an unspent transaction output.\n"
            "\nArguments:\n"
            "1. \"txid\"       (string, required) The transaction id\n"
            "2. n              (numeric, required) vout value\n"
            "3. includemempool  (boolean, optional) Whether to included the mem pool\n"
            "\nResult:\n"
            "{\n"
            "  \"bestblock\" : \"hash\",    (string) the block hash\n"
            "  \"confirmations\" : n,       (numeric) The number of confirmations\n"
            "  \"value\" : x.xxx,           (numeric) The transaction value in ltc\n"
            "  \"scriptPubKey\" : {         (json object)\n"
            "     \"asm\" : \"code\",       (string) \n"
            "     \"hex\" : \"hex\",        (string) \n"
            "     \"reqSigs\" : n,          (numeric) Number of required signatures\n"
            "     \"type\" : \"pubkeyhash\", (string) The type, eg pubkeyhash\n"
            "     \"addresses\" : [          (array of string) array of cccoin addresses\n"
            "        \"cccoinaddress\"     (string) cccoin address\n"
            "        ,...\n"
            "     ]\n"
            "  },\n"
            "  \"version\" : n,            (numeric) The version\n"
            "  \"coinbase\" : true|false   (boolean) Coinbase or not\n"
            "}\n"

            "\nExamples:\n"
            "\nGet unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nView the details\n"
            + HelpExampleCli("gettxout", "\"txid\" 1") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("gettxout", "\"txid\", 1")
        );

    Object ret;

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);
    int n = params[1].get_int();
    bool fMempool = true;
    if (params.size() > 2)
        fMempool = params[2].get_bool();

    CCoins coins;
    if (fMempool) {
        LOCK(mempool.cs);
        CCoinsViewMemPool view(pcoinsTip, mempool);
        if (!view.GetCoins(hash, coins))
            return Value::null;
        mempool.pruneSpent(hash, coins); // TODO: this should be done by the CCoinsViewMemPool
    } else {
        if (!pcoinsTip->GetCoins(hash, coins))
            return Value::null;
    }
    if (n<0 || (unsigned int)n>=coins.vout.size() || coins.vout[n].IsNull())
        return Value::null;

    BlockMap::iterator it = mapBlockIndex.find(pcoinsTip->GetBestBlock());
    CBlockIndex *pindex = it->second;
    ret.push_back(Pair("bestblock", pindex->GetBlockHash().GetHex()));
    if ((unsigned int)coins.nHeight == MEMPOOL_HEIGHT)
        ret.push_back(Pair("confirmations", 0));
    else
        ret.push_back(Pair("confirmations", pindex->nHeight - coins.nHeight + 1));
    ret.push_back(Pair("value", ValueFromAmount(coins.vout[n].nValue)));
    Object o;
    ScriptPubKeyToJSON(coins.vout[n].scriptPubKey, o, true);
    ret.push_back(Pair("scriptPubKey", o));
    ret.push_back(Pair("version", coins.nVersion));
    ret.push_back(Pair("coinbase", coins.fCoinBase));

    return ret;
}

Value verifychain(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "verifychain ( checklevel numblocks )\n"
            "\nVerifies blockchain database.\n"
            "\nArguments:\n"
            "1. checklevel   (numeric, optional, 0-4, default=3) How thorough the block verification is.\n"
            "2. numblocks    (numeric, optional, default=288, 0=all) The number of blocks to check.\n"
            "\nResult:\n"
            "true|false       (boolean) Verified or not\n"
            "\nExamples:\n"
            + HelpExampleCli("verifychain", "")
            + HelpExampleRpc("verifychain", "")
        );

    int nCheckLevel = GetArg("-checklevel", 3);
    int nCheckDepth = GetArg("-checkblocks", 288);
    if (params.size() > 0)
        nCheckLevel = params[0].get_int();
    if (params.size() > 1)
        nCheckDepth = params[1].get_int();

    return CVerifyDB().VerifyDB(pcoinsTip, nCheckLevel, nCheckDepth);
}

Value getblockchaininfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockchaininfo\n"
            "Returns an object containing various state info regarding block chain processing.\n"
            "\nResult:\n"
            "{\n"
            "  \"chain\": \"xxxx\",        (string) current network name as defined in BIP70 (main, test, regtest)\n"
            "  \"blocks\": xxxxxx,         (numeric) the current number of blocks processed in the server\n"
            "  \"headers\": xxxxxx,        (numeric) the current number of headers we have validated\n"
            "  \"bestblockhash\": \"...\", (string) the hash of the currently best block\n"
            "  \"difficulty\": xxxxxx,     (numeric) the current difficulty\n"
            "  \"verificationprogress\": xxxx, (numeric) estimate of verification progress [0..1]\n"
            "  \"chainwork\": \"xxxx\"     (string) total amount of work in active chain, in hexadecimal\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getblockchaininfo", "")
            + HelpExampleRpc("getblockchaininfo", "")
        );

    Object obj;
    obj.push_back(Pair("chain",                 Params().NetworkIDString()));
    obj.push_back(Pair("blocks",                (int)chainActive.Height()));
    obj.push_back(Pair("headers",               pindexBestHeader ? pindexBestHeader->nHeight : -1));
    obj.push_back(Pair("bestblockhash",         chainActive.Tip()->GetBlockHash().GetHex()));
    obj.push_back(Pair("difficulty",            (double)GetDifficulty()));
    obj.push_back(Pair("verificationprogress",  Checkpoints::GuessVerificationProgress(chainActive.Tip())));
    obj.push_back(Pair("chainwork",             chainActive.Tip()->nChainWork.GetHex()));
    return obj;
}

/** Comparison function for sorting the getchaintips heads.  */
struct CompareBlocksByHeight
{
    bool operator()(const CBlockIndex* a, const CBlockIndex* b) const
    {
        /* Make sure that unequal blocks with the same height do not compare
           equal. Use the pointers themselves to make a distinction. */

        if (a->nHeight != b->nHeight)
          return (a->nHeight > b->nHeight);

        return a < b;
    }
};

Value getchaintips(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getchaintips\n"
            "Return information about all known tips in the block tree,"
            " including the main chain as well as orphaned branches.\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"height\": xxxx,         (numeric) height of the chain tip\n"
            "    \"hash\": \"xxxx\",         (string) block hash of the tip\n"
            "    \"branchlen\": 0          (numeric) zero for main chain\n"
            "    \"status\": \"active\"      (string) \"active\" for the main chain\n"
            "  },\n"
            "  {\n"
            "    \"height\": xxxx,\n"
            "    \"hash\": \"xxxx\",\n"
            "    \"branchlen\": 1          (numeric) length of branch connecting the tip to the main chain\n"
            "    \"status\": \"xxxx\"        (string) status of the chain (active, valid-fork, valid-headers, headers-only, invalid)\n"
            "  }\n"
            "]\n"
            "Possible values for status:\n"
            "1.  \"invalid\"               This branch contains at least one invalid block\n"
            "2.  \"headers-only\"          Not all blocks for this branch are available, but the headers are valid\n"
            "3.  \"valid-headers\"         All blocks are available for this branch, but they were never fully validated\n"
            "4.  \"valid-fork\"            This branch is not part of the active chain, but is fully validated\n"
            "5.  \"active\"                This is the tip of the active main chain, which is certainly valid\n"
            "\nExamples:\n"
            + HelpExampleCli("getchaintips", "")
            + HelpExampleRpc("getchaintips", "")
        );

    /* Build up a list of chain tips.  We start with the list of all
       known blocks, and successively remove blocks that appear as pprev
       of another block.  */
    std::set<const CBlockIndex*, CompareBlocksByHeight> setTips;
    BOOST_FOREACH(const PAIRTYPE(const uint256, CBlockIndex*)& item, mapBlockIndex)
        setTips.insert(item.second);
    BOOST_FOREACH(const PAIRTYPE(const uint256, CBlockIndex*)& item, mapBlockIndex)
    {
        const CBlockIndex* pprev = item.second->pprev;
        if (pprev)
            setTips.erase(pprev);
    }

    // Always report the currently active tip.
    setTips.insert(chainActive.Tip());

    /* Construct the output array.  */
    Array res;
    BOOST_FOREACH(const CBlockIndex* block, setTips)
    {
        Object obj;
        obj.push_back(Pair("height", block->nHeight));
        obj.push_back(Pair("hash", block->phashBlock->GetHex()));

        const int branchLen = block->nHeight - chainActive.FindFork(block)->nHeight;
        obj.push_back(Pair("branchlen", branchLen));

        string status;
        if (chainActive.Contains(block)) {
            // This block is part of the currently active chain.
            status = "active";
        } else if (block->nStatus & BLOCK_FAILED_MASK) {
            // This block or one of its ancestors is invalid.
            status = "invalid";
        } else if (block->nChainTx == 0) {
            // This block cannot be connected because full block data for it or one of its parents is missing.
            status = "headers-only";
        } else if (block->IsValid(BLOCK_VALID_SCRIPTS)) {
            // This block is fully validated, but no longer part of the active chain. It was probably the active block once, but was reorganized.
            status = "valid-fork";
        } else if (block->IsValid(BLOCK_VALID_TREE)) {
            // The headers for this block are valid, but it has not been validated. It was probably never part of the most-work chain.
            status = "valid-headers";
        } else {
            // No clue.
            status = "unknown";
        }
        obj.push_back(Pair("status", status));

        res.push_back(obj);
    }

    return res;
}

Value getmempoolinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getmempoolinfo\n"
            "\nReturns details on the active state of the TX memory pool.\n"
            "\nResult:\n"
            "{\n"
            "  \"size\": xxxxx                (numeric) Current tx count\n"
            "  \"bytes\": xxxxx               (numeric) Sum of all tx sizes\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmempoolinfo", "")
            + HelpExampleRpc("getmempoolinfo", "")
        );

    Object ret;
    ret.push_back(Pair("size", (int64_t) mempool.size()));
    ret.push_back(Pair("bytes", (int64_t) mempool.GetTotalTxSize()));

    return ret;
}

Value invalidateblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "invalidateblock \"hash\"\n"
            "\nPermanently marks a block as invalid, as if it violated a consensus rule.\n"
            "\nArguments:\n"
            "1. hash   (string, required) the hash of the block to mark as invalid\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("invalidateblock", "\"blockhash\"")
            + HelpExampleRpc("invalidateblock", "\"blockhash\"")
        );

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);
    CValidationState state;

    {
        LOCK(cs_main);
        if (mapBlockIndex.count(hash) == 0)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

        CBlockIndex* pblockindex = mapBlockIndex[hash];
        InvalidateBlock(state, pblockindex);
    }

    if (state.IsValid()) {
        ActivateBestChain(state);
    }

    if (!state.IsValid()) {
        throw JSONRPCError(RPC_DATABASE_ERROR, state.GetRejectReason());
    }

    return Value::null;
}

Value reconsiderblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "reconsiderblock \"hash\"\n"
            "\nRemoves invalidity status of a block and its descendants, reconsider them for activation.\n"
            "This can be used to undo the effects of invalidateblock.\n"
            "\nArguments:\n"
            "1. hash   (string, required) the hash of the block to reconsider\n"
            "\nResult:\n"
            "\nExamples:\n"
            + HelpExampleCli("reconsiderblock", "\"blockhash\"")
            + HelpExampleRpc("reconsiderblock", "\"blockhash\"")
        );

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);
    CValidationState state;

    {
        LOCK(cs_main);
        if (mapBlockIndex.count(hash) == 0)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

        CBlockIndex* pblockindex = mapBlockIndex[hash];
        ReconsiderBlock(state, pblockindex);
    }

    if (state.IsValid()) {
        ActivateBestChain(state);
    }

    if (!state.IsValid()) {
        throw JSONRPCError(RPC_DATABASE_ERROR, state.GetRejectReason());
    }

    return Value::null;
}

Value getcontent(const Array& params, bool fHelp)  // TO DO
{
    if (fHelp || params.size() != 1)
        throw runtime_error("");
    return Value::null;
}

Value getlink(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 2)
        throw runtime_error("");
    Object result;
    CLink clink;
    if (IsHex(params[0].get_str())) { // input is txid
        uint256 hash = ParseHashV(params[0], "parameter 1");

        int nVout = params.size() == 2 ? params[1].get_int() : 0;
        CTransaction tx;
        uint256 hashBlock = 0;
        if (!GetTransaction(hash, tx, hashBlock, true))
            throw JSONRPCError(RPC_LINK_ERROR, "No information available about transaction");
        if ((int) tx.vout.size() <= nVout)
            throw JSONRPCError(RPC_LINK_ERROR, "nVout larger than total vout count");
        int nTx = 0;
        nTx = GetNTx(tx.GetHash());
        if (nTx < 0)
            throw JSONRPCError(RPC_LINK_ERROR, "Get nTx failed");

        BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && (*mi).second) {
            CBlockIndex* pindex = (*mi).second;
            clink.SetInt(pindex->nHeight, nTx, nVout);

        } else
            throw JSONRPCError(RPC_LINK_ERROR, "Fail to get link");
    } else { // input is link
        clink.SetString(params[0].get_str());
    }
    result.push_back(Pair("link", clink.ToString(LINK_FORMAT_DEC)));
    result.push_back(Pair("linkHex", clink.ToString(LINK_FORMAT_HEX)));
    result.push_back(Pair("linkB32", clink.ToString(LINK_FORMAT_B32)));
    return result;
}

bool _getTxFrBlock(const CBlock& block, const int nTx, CTransaction& txOut)
{
    if (nTx <= (int)block.vtx.size()) {
        txOut = block.vtx[nTx];
        return true;
    } else
        return false;
}

bool _getVoutFrTx(const CTransaction& tx, const int nVout, CTxOut& vout)
{
    if (nVout <= (int) tx.vout.size()) {
        vout = tx.vout[nVout];
        return true;
    } else
        return false;
}

bool _getContentFrTx(const CTransaction& tx, const int nVout, CContent& content)
{
    if (nVout <= (int) tx.vout.size()) {
        CTxOut vout = tx.vout[nVout];
        content.SetString(vout.strContent);
        return true;
    } else
        return false;
}

Object _voutToJson(const CTxOut& txout)
{
    Object out;
    out.push_back(Pair("value", ValueFromAmount(txout.nValue)));
    out.push_back(Pair("content", HexStr(txout.strContent.begin(), txout.strContent.end())));
    out.push_back(Pair("contentText", GetBinaryContent(txout.strContent)));
    Object o;
    ScriptPubKeyToJSON(txout.scriptPubKey, o, true);
    out.push_back(Pair("scriptPubKey", o));
    return out;
}

Object _output_content(const CContent& cttIn, const int& cformat, const unsigned char& cttf, const CLink& clinkIn, const std::vector<CBitcoinAddress>& posters, const CAmount nValue, const CScript& scriptPubKey)
{
    CLink clink = clinkIn;
    CContent ctt = cttIn;
    Object r;
    if (cttf & CONTENT_SIMPLE_FORMAT) {
        r.push_back(Pair(clink.ToString(), ctt.size()));
        return r;
    }
    if (cttf & CONTENT_SHOW_LINK)
        r.push_back(Pair("link", clink.ToString()));

    if (cttf & CONTENT_SHOW_POSTER) {
        Array poster;

        BOOST_FOREACH(const CBitcoinAddress& addr, posters)
        {
            poster.push_back(addr.ToString());
        }
        r.push_back(Pair("poster", poster));
    }
    if (cttf & CONTENT_SHOW_VALUE)
        r.push_back(Pair("satoshi", nValue));
    if (cttf & CONTENT_SHOW_ADDR) {
        if (scriptPubKey.size() == 0)
            r.push_back(Pair("addr", ""));
        else {
            CBitcoinAddress addr(scriptPubKey);
            r.push_back(Pair("addr", addr.ToString()));
        }
    }
    switch (cformat) {
        case CONTENT_FORMAT_STR_HEX:r.push_back(Pair("content", HexStr(ctt)));
            break;
        case CONTENT_FORMAT_STR_BIN:r.push_back(Pair("content", ctt));
            break;
        case CONTENT_FORMAT_STR_B64:r.push_back(Pair("content", EncodeBase64(ctt)));
            break;
        case CONTENT_FORMAT_JSON_HEX:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_HEX)));
            break;
        case CONTENT_FORMAT_JSON_BIN:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_BIN)));
            break;
        case CONTENT_FORMAT_JSON_B64:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_B64)));
            break;
        case CONTENT_FORMAT_HUMAN_STR:r.push_back(Pair("content", ctt.ToHumanString()));
            break;
        case CONTENT_FORMAT_SIZE:
        default:
            r.push_back(Pair("content", ctt.size()));
    }
    return r;
}

std::vector<CBitcoinAddress> _get_posters(CTransaction tx)
{
    std::vector<CBitcoinAddress> posters;

    BOOST_FOREACH(const CTxIn& in, tx.vin)
    {
        if (in.prevout.hash.EqualTo(0))
            continue;
        CTransaction prevTx;
        uint256 tmphash;
        if (!GetTransaction(in.prevout.hash, prevTx, tmphash, true))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Get prev tx failed");
        CBitcoinAddress addr(prevTx.vout[in.prevout.n].scriptPubKey);
        posters.push_back(addr);
    }
    return posters;
}

Value getcontentbylink(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error("Wrong number of parameters");
    CLink clink(params[0].get_str());

    CBlockIndex* pblockindex;
    CBlock block;
    if (!_getBlockByHeight(clink.nHeight, block, pblockindex))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
    CTransaction tx;
    if (!_getTxFrBlock(block, clink.nTx, tx))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get tx failed");
    CContent content;
    if (!_getContentFrTx(tx, clink.nVout, content))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get content failed");
    int cformat = (params.size() == 2) ? params[1].get_int() : CONTENT_FORMAT_STR_HEX;
    std::vector<CBitcoinAddress> posters = _get_posters(tx);
    unsigned char cflag = CONTENT_SHOW_LINK | CONTENT_SHOW_POSTER | CONTENT_SHOW_VALUE | CONTENT_SHOW_ADDR;
    Object r = _output_content(content, cformat, cflag, clink, posters, tx.vout[clink.nVout].nValue, tx.vout[clink.nVout].scriptPubKey);
    return r;
}

bool _parse_getcontents_params(const Array& params, int& fbh, int& maxc, int& maxb, Array& withcc, Array& withoutcc, Array& firstcc, int& fContentFormat, unsigned char& cflag, int& mincsize, Array& addrs, bool& fAsc)
{
    if (params.size() > 3)
        return false;
    if (params.size() == 0) {
        fbh = 0;
        maxc = DEFAULT_MAX_CONTENTS;
        maxb = DEFAULT_MAX_CONTENTSBYTES;
        return true;
    }
    Object param = params[0].get_obj();
    const Value& fbh_v = find_value(param, "fbh");
    try {
        fbh = fbh_v.get_int();
    } catch (std::exception& e) {
        fbh = 0;
    }
    const Value& maxc_v = find_value(param, "maxc");
    try {
        maxc = maxc_v.get_int();
    } catch (std::exception& e) {
        maxc = DEFAULT_MAX_CONTENTS;
    }
    const Value& maxb_v = find_value(param, "maxb");
    try {
        maxb = maxb_v.get_int();
    } catch (std::exception& e) {
        maxb = DEFAULT_MAX_CONTENTSBYTES;
    }
    const Value& fContentFormat_v = find_value(param, "cformat");
    try {
        fContentFormat = fContentFormat_v.get_int();
    } catch (std::exception& e) {
        fContentFormat = CONTENT_FORMAT_JSON_B64;
    }

    // cflag
    cttflag fShowLink;
    const Value& fShowLink_v = find_value(param, "fShowLink");
    try {
        fShowLink = fShowLink_v.get_bool() ? CONTENT_SHOW_LINK : CONTENT_FALSE;
    } catch (std::exception& e) {
        fShowLink = CONTENT_SHOW_LINK;
    }
    cttflag fShowPoster;
    const Value& fShowPoster_v = find_value(param, "fShowPoster");
    try {
        fShowPoster = fShowPoster_v.get_bool() ? CONTENT_SHOW_POSTER : CONTENT_FALSE;
    } catch (std::exception& e) {
        fShowPoster = CONTENT_SHOW_POSTER;
    }
    cttflag fShowValue;
    const Value& fShowValue_v = find_value(param, "fShowValue");
    try {
        fShowValue = fShowValue_v.get_bool() ? CONTENT_SHOW_VALUE : CONTENT_FALSE;
    } catch (std::exception& e) {
        fShowValue = CONTENT_SHOW_VALUE;
    }
    cttflag fShowAddr;
    const Value& fShowAddr_v = find_value(param, "fShowAddr");
    try {
        fShowAddr = fShowAddr_v.get_bool() ? CONTENT_SHOW_ADDR : CONTENT_FALSE;
    } catch (std::exception& e) {
        fShowAddr = CONTENT_SHOW_ADDR;
    }
    cttflag fSimpleFormat;
    const Value& fSimpleFormat_v = find_value(param, "fSimpleFormat");
    try {
        fSimpleFormat = fSimpleFormat_v.get_bool() ? CONTENT_SIMPLE_FORMAT : CONTENT_FALSE;
    } catch (std::exception& e) {
        fSimpleFormat = CONTENT_FALSE;
    }
    cflag = fShowLink | fShowPoster | fShowValue | fShowAddr | fSimpleFormat;

    const Value& mincsize_v = find_value(param, "mincsize");
    try {
        mincsize = mincsize_v.get_int();
    } catch (std::exception& e) {
        mincsize = 1;
    }
    const Value& fAsc_v = find_value(param, "fAsc");
    try {
        fAsc = fAsc_v.get_bool();
    } catch (std::exception& e) {
        fAsc = false;
    }
    const Value& withcc_v = find_value(param, "withcc");
    try {
        withcc = withcc_v.get_array();
    } catch (std::exception& e) {
    }
    const Value& withoutcc_v = find_value(param, "withoutcc");
    try {
        withoutcc = withoutcc_v.get_array();
    } catch (std::exception& e) {
    }
    const Value& firstcc_v = find_value(param, "firstcc");
    try {
        firstcc = firstcc_v.get_array();
    } catch (std::exception& e) {
    }
    if (params.size() > 1)
        addrs = params[1].get_array();
    return true;
}

bool _check_cc(const CContent& ctt, const Array& withcc, const Array& withoutcc, const Array& firstcc)
{
    if (withcc.size() == 0 && withoutcc.size() == 0 && firstcc.size() == 0 )
        return true;
    CContent cttcopy = ctt;

    bool r = false;

    BOOST_FOREACH(const Value& ccName_v, firstcc)
    {
        cctype cc = GetCcValue(ccName_v.get_str());
        if (cttcopy.FirstCc(cc)) {
            r = true;
            break;
        }
    }
    if(!r)
        return false;

    BOOST_FOREACH(const Value& ccName_v, withcc)
    {
        cctype cc = GetCcValue(ccName_v.get_str());
        if (!cttcopy.HasCc(cc))
            return false;
    }

    BOOST_FOREACH(const Value& ccName_v, withoutcc)
    {
        cctype cc = GetCcValue(ccName_v.get_str());
        if (cttcopy.HasCc(cc))
            return false;
    }

    return true;
}



Value getcontents(const Array& params, bool fHelp) // withcc and without cc is very costly, DONOT use in standard occasion
{
    if (fHelp)
        throw runtime_error("Help msg");
    int fbh;
    int maxc;
    int maxb;
    Array withcc;
    Array withoutcc;
    Array firstcc;
    Array gAddrs;
    int cformat;
    unsigned char cflag;
    int minsz;
    bool fAsc;
    if (!_parse_getcontents_params(params, fbh, maxc, maxb, withcc, withoutcc, firstcc, cformat, cflag, minsz, gAddrs, fAsc))
        throw runtime_error("Error parsing parameters");
    Array r;
    int c = 0;
    int b = 0;
    if (gAddrs.size() == 0) {
        int nHeight = fAsc ? fbh : chainActive.Height();
        int total = chainActive.Height() - fbh + 1;
        for (int i = 0; i < total; i++) {
            CBlockIndex* pblockindex;
            CBlock block;
            if (!_getBlockByHeight(nHeight, block, pblockindex))
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
            int nTx = 0;

            BOOST_FOREACH(const CTransaction& tx, block.vtx)
            {
                int nVout = 0;
                std::vector<CBitcoinAddress> posters;
                if (cflag & CONTENT_SHOW_POSTER)
                    posters = _get_posters(tx);

                BOOST_FOREACH(const CTxOut& out, tx.vout)
                {
                    if (c >= maxc)
                        return r;
                    if ((int) out.strContent.size() >= minsz) {
                        b += out.strContent.size();
                        if (c > maxc || b > maxb)
                            return r;
                        CContent ctt(out.strContent);
                        if (_check_cc(ctt, withcc, withoutcc, firstcc)) {
                            CLink clink(nHeight, nTx, nVout);
                            Object cttr = _output_content(ctt, cformat, cflag, clink, posters, out.nValue, out.scriptPubKey);
                            r.push_back(cttr);
                            c++;
                        } else {
                            b -= out.strContent.size();
                            continue;
                        }

                    }
                    nVout++;
                }
                nTx++;
            }
            fAsc ? nHeight++ : nHeight--;
        }
    } else {
        std::vector<CScript> vIds;
        BOOST_FOREACH(const Value& addrStr, gAddrs)
        {
            CBitcoinAddress address(addrStr.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
            CScript scriptPubKey = GetScriptForDestination(address.Get());
            vIds.push_back(scriptPubKey);
        }
        std::vector<std::pair<CTransaction, uint256> > vTxs;
        if (!GetTransactions(vIds, vTxs))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Get tx failed");
        for (std::vector<std::pair<CTransaction, uint256> >::iterator it = vTxs.begin(); it != vTxs.end(); ++it) {
            BlockMap::iterator mi = mapBlockIndex.find(it->second);
            if (mi != mapBlockIndex.end() && (*mi).second) {
                std::vector<CBitcoinAddress> posters;
                if (cflag & CONTENT_SHOW_POSTER)
                    posters = _get_posters(it->first);
                for (int i = 0; i < (int) it->first.vout.size(); i++) {
                    if (c >= maxc)
                        return r;
                    CBlockIndex* pindex = (*mi).second;
                    if (pindex->nHeight > fbh) {
                        int nTx = GetNTx(it->first.GetHash()); // Very inefficient
                        CLink clink(pindex->nHeight, nTx, i);
                        CContent ctt;
                        if (_getContentFrTx(it->first, i, ctt)) {
                            if ((int) ctt.size() >= minsz) {
                                b += ctt.size();
                                if (c > maxc || b > maxb)
                                    return r;
                                if (_check_cc(ctt, withcc, withoutcc, firstcc)) {
                                    Object cttr = _output_content(ctt, cformat, cflag, clink, posters, it->first.vout[i].nValue, it->first.vout[i].scriptPubKey);
                                    r.push_back(cttr);
                                    c++;
                                } else
                                    b -= ctt.size();
                            }
                        }
                    }
                }
            }
        }
    }
    return r;
}

std::string _test()
{
    std::string str;
    return str;
}

Value devtest(const Array& params, bool fHelp)
{
    return _test();
} 