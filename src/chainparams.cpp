// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include "core_io.h"
#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (  0, uint256("e76e83122b99d9285700699befa7099e46e3d3414af6e2042d520d1a70ad58ee"))
        ;
static const Checkpoints::CCheckpointData data = {
        &mapCheckpoints,
        1437548320, // * UNIX timestamp of last checkpoint block
        0,  // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        500     // * estimated number of transactions per day after checkpoint
    };

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 546, uint256("b6e5e547066b98c23ae988765921ca884e13362b7fd96cd3ef2071908c56a43b"))
        ;
static const Checkpoints::CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1365458829,
        547,
        576
    };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("ebccedc9065007d980ab022ea8eca59d35b17dbf65b1eeb089b5a0a4f7de89eb"))
        ;
static const Checkpoints::CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

class CMainParams : public CChainParams {
public:
    CMainParams() {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /** 
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xec;
        pchMessageStart[1] = 0xe0;
        pchMessageStart[2] = 0xdd;
        pchMessageStart[3] = 0xde;
        vAlertPubKey = ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9");
        nDefaultPort = 7333;
        bnProofOfWorkLimit = ~uint256(0) >> 8;
        nSubsidyHalvingInterval =  480;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 12 * 60 * 60; // 12 hr
        nTargetSpacing = 3 * 60; // 3 minutes

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         * 
         * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
         *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
         *   vMerkleTree: 4a5e1e
         */
        const string pszTimestamp = "2015-07-22 WSJ: Gold Falls to Five-Year Low as Traders Continue to Exit";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(2);
        txNew.vin[0].scriptSig = CScript() << (int)0;
        txNew.vin[0].prevout.nValue=456000;
        txNew.vout[0].nValue = 0;
        txNew.vout[0].scriptPubKey = CScript() << OP_HASH160 << ParseHex("6fd16238813d981a3366c8f66be34b20e6a1c9fa") << OP_EQUAL;
        txNew.vout[0].strContent=pszTimestamp;
        txNew.vout[0].nLockTime = 499999999;
        txNew.vout[1].nValue = 0;
        txNew.vout[1].scriptPubKey = CScript() << ParseHex("51210202c974218072f0d8bc416c45e9810c59b081bae492dca0b4832ef0e9077b6e9d51210343b1dcfa6be152f72d38dfc827c472139164bbb8ce269307834add763c07033951210399c026a70df6ca2370b572afc43efcbdc1e282e019ae0ab080bc4b33c44609e35121034b9a0940b374e3125a2b494134b37c67802efc5dab0318f7163c0b1879ad897051210394bcb5121b56eafbe1db4dbc600debc0c434cdcd35de92f9b9a75bb582d107da5121036a6b39054042b5b379b309b7ac60e31b6596e74d1a5a3aa1889fc0bbb49049fa5156ae"); 
        const string intro = "Developed by Alex, HTL, John Doe ,Jonas, Fred ,Lee. Based on Satoshi Nakamoto's great work";
        txNew.vout[1].strContent=intro;
        txNew.vout[1].nLockTime=499999999;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 3;
        genesis.nBlockHeight = 0;
        genesis.nTime    = 1437548320;
        genesis.nBits    = 0x1e0fffff;
        genesis.nNonce   = 361424;
//        std::cout << "tx size: " << txNew.GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION) << "\n";        
//        uint256 powHash;
//        powHash = genesis.GetPoWHash();
//        while (powHash > ~uint256(0) >> 20){
//            if (++genesis.nNonce==0) break;
//            powHash = genesis.GetPoWHash();
//        }
//        std::cout << "nonce: " << genesis.nNonce << "\n";
        hashGenesisBlock = genesis.GetHash();
        
//        CTransaction gtx;
//        gtx = genesis.vtx.front();
//        std::cout << "g rtx: \n" << EncodeHexTx(gtx) << "\n";
//        std::cout << "g tx: \n" << gtx.ToString() << "\n";
//        std::cout << "g blk: \n" << genesis.ToString() << "\n";
//        std::cout << "blockheader: \n" << HexStr(BEGIN(genesis.nVersion),END(genesis.nNonce)) << "\n";
//        std::cout << "g hash: \n" << hashGenesisBlock.ToString() << "\n";
//        std::cout << "g powh: \n" << genesis.GetPoWHash().ToString() << "\n";
//        std::cout << "g mroot: \n" << genesis.hashMerkleRoot.ToString() << "\n";
        
        assert(hashGenesisBlock == uint256("ceb597f31f1a574c69dacf592d1c91b2e6de0797d43f65c54d5b03180a2e1d87"));
        assert(genesis.hashMerkleRoot == uint256("08c7280ac0e2f5a77456100108f392b29ffd9c42c4b5285ae122fc958d54f616"));
        
        vSeeds.push_back(CDNSSeedData("dns.f", "dnsseed.f"));

        base32Prefixes[PUBKEY_ADDRESS_2]    = list_of(0x02);    //  (51 bytes)(AE/AF/QE/QF)(7 bytes)
        base32Prefixes[PUBKEY_ADDRESS_3]    = list_of(0x03);    //  (51 bytes)(AG/AH/QG/QH)(7 bytes)
        base32Prefixes[SCRIPT_ADDRESS]      = list_of(0x90);    //  (n bytes)(S)(7 bytes)
        base32Prefixes[SCRIPTHASH_ADDRESS]  = list_of(0x38);    //  (32 bytes)(H)(7 bytes)
        base32Prefixes[SECRET_KEY]          = list_of(0x48);
        base32Prefixes[EXT_PUBLIC_KEY]      = list_of(0x04)(0x88)(0xB2)(0x1E);
        base32Prefixes[EXT_SECRET_KEY]      = list_of(0x04)(0x88)(0xAD)(0xE4);

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        // Cccoin: Mainnet v2 enforced as of block 710k
       // nEnforceV2AfterHeight = 710000;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0xec;
        pchMessageStart[1] = 0xe0;
        pchMessageStart[2] = 0xdd;
        pchMessageStart[3] = 0xde;
        vAlertPubKey = ParseHex("03968502fe538a31c883493c47ed169bc74a68b7051c85afeeb05783163b6c173e");
        nDefaultPort = 17333;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        nTargetSpacing = 2.5 * 60; // 2.5 minutes

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1427227400;
        genesis.nNonce = 607439;
        hashGenesisBlock = genesis.GetHash();
//        std::cout << "t g hash: \n" << hashGenesisBlock.ToString() << "\n";
        assert(hashGenesisBlock == uint256("c784b3dafa3f5bdfb441ec0090df58450d4193082b9f83d6be904bdcccd4cfc3"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("dns.f", "dnsseed.f"));

        base32Prefixes[PUBKEY_ADDRESS_2]    = list_of(111);
        base32Prefixes[PUBKEY_ADDRESS_3]    = list_of(112);
        base32Prefixes[SCRIPT_ADDRESS]      = list_of(196);
        base32Prefixes[SCRIPTHASH_ADDRESS]  = list_of(197);
        base32Prefixes[SECRET_KEY]          = list_of(239);    
        base32Prefixes[EXT_PUBLIC_KEY]      = list_of(0x04)(0x88)(0xB2)(0x1E);
        base32Prefixes[EXT_SECRET_KEY]      = list_of(0x04)(0x88)(0xAD)(0xE4);

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        // Cccoin: Testnet v2 enforced as of block 400k
        nEnforceV2AfterHeight = 400000;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xec;
        pchMessageStart[1] = 0xe0;
        pchMessageStart[2] = 0xdd;
        pchMessageStart[3] = 0xde;
        nSubsidyHalvingInterval = 150;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        nTargetSpacing = 2.5 * 60; // 2.5 minutes
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        genesis.nTime = 1296688602;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 607439;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 19444;
//        std::cout << "rt g hash: \n" << hashGenesisBlock.ToString() << "\n";
        assert(hashGenesisBlock == uint256("1acb364a762cb106a085c141d1f20be0439dd64c79a18547fe48e18ec786756e"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        // Cccoin: v2 enforced using Bitcoin's supermajority rule
        nEnforceV2AfterHeight = -1;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams {
public:
    CUnitTestParams() {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 18445;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Unit test mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;

        // Cccoin: v2 enforced using Bitcoin's supermajority rule
        nEnforceV2AfterHeight = -1;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval)  { nSubsidyHalvingInterval=anSubsidyHalvingInterval; }
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority)  { nEnforceBlockUpgradeMajority=anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority)  { nRejectBlockOutdatedMajority=anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority)  { nToCheckBlockUpgradeMajority=anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks)  { fDefaultConsistencyChecks=afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) {  fAllowMinDifficultyBlocks=afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams *pCurrentParams = 0;

CModifiableParams *ModifiableParams()
{
   assert(pCurrentParams);
   assert(pCurrentParams==&unitTestParams);
   return (CModifiableParams*)&unitTestParams;
}

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        case CBaseChainParams::UNITTEST:
            return unitTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
