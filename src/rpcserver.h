// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPCSERVER_H
#define BITCOIN_RPCSERVER_H

#include "amount.h"
#include "rpcprotocol.h"
#include "uint256.h"
#include "primitives/transaction.h"
//#include "ccc/content.h"
//#include "ccc/link.h"
#include "base58.h"
#include <list>
#include <map>
#include <stdint.h>
#include <string>

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;
class CBlockIndex;
class CNetAddr;
class CBlock;
class CContent;
class CLink;
class CMessage;

class AcceptedConnection
{
public:
    virtual ~AcceptedConnection() {}

    virtual std::iostream& stream() = 0;
    virtual std::string peer_address_to_string() const = 0;
    virtual void close() = 0;
};

/** Start RPC threads */
void StartRPCThreads();
/**
 * Alternative to StartRPCThreads for the GUI, when no server is
 * used. The RPC thread in this case is only used to handle timeouts.
 * If real RPC threads have already been started this is a no-op.
 */
void StartDummyRPCThread();
/** Stop RPC threads */
void StopRPCThreads();
/** Query whether RPC is running */
bool IsRPCRunning();

/** 
 * Set the RPC warmup status.  When this is done, all RPC calls will error out
 * immediately with RPC_IN_WARMUP.
 */
void SetRPCWarmupStatus(const std::string& newStatus);
/* Mark warmup as done.  RPC calls will be processed from now on.  */
void SetRPCWarmupFinished();

/* returns the current warmup state.  */
bool RPCIsInWarmup(std::string *statusOut);

/**
 * Type-check arguments; throws JSONRPCError if wrong type given. Does not check that
 * the right number of arguments are passed, just that any passed are the correct type.
 * Use like:  RPCTypeCheck(params, boost::assign::list_of(str_type)(int_type)(obj_type));
 */
void RPCTypeCheck(const json_spirit::Array& params,
                  const std::list<json_spirit::Value_type>& typesExpected, bool fAllowNull=false);
/**
 * Check for expected keys/value types in an Object.
 * Use like: RPCTypeCheck(object, boost::assign::map_list_of("name", str_type)("value", int_type));
 */
void RPCTypeCheck(const json_spirit::Object& o,
                  const std::map<std::string, json_spirit::Value_type>& typesExpected, bool fAllowNull=false);

/**
 * Run func nSeconds from now. Uses boost deadline timers.
 * Overrides previous timer <name> (if any).
 */
void RPCRunLater(const std::string& name, boost::function<void(void)> func, int64_t nSeconds);

//! Convert boost::asio address to CNetAddr
extern CNetAddr BoostAsioToCNetAddr(boost::asio::ip::address address);

typedef json_spirit::Value(*rpcfn_type)(const json_spirit::Array& params, bool fHelp);

class CRPCCommand
{
public:
    std::string category;
    std::string name;
    rpcfn_type actor;
    bool okSafeMode;
    bool threadSafe;
    bool reqWallet;
};

/**
 * Bitcoin RPC command dispatcher.
 */
class CRPCTable
{
private:
    std::map<std::string, const CRPCCommand*> mapCommands;
public:
    CRPCTable();
    const CRPCCommand* operator[](std::string name) const;
    std::string help(std::string name) const;

    /**
     * Execute a method.
     * @param method   Method to execute
     * @param params   Array of arguments (JSON objects)
     * @returns Result of the call.
     * @throws an exception (json_spirit::Value) when an error happens.
     */
    json_spirit::Value execute(const std::string &method, const json_spirit::Array &params) const;
};

extern const CRPCTable tableRPC;

/**
 * Utilities: convert hex-encoded Values
 * (throws error if not hex).
 */
extern uint256 ParseHashV(const json_spirit::Value& v, std::string strName);
extern uint256 ParseHashO(const json_spirit::Object& o, std::string strKey);
extern std::vector<unsigned char> ParseHexV(const json_spirit::Value& v, std::string strName);
extern std::vector<unsigned char> ParseHexO(const json_spirit::Object& o, std::string strKey);

extern void InitRPCMining();
extern void ShutdownRPCMining();

extern int64_t nWalletUnlockTime;
extern CAmount AmountFromValue(const json_spirit::Value& value);
extern json_spirit::Value ValueFromAmount(const CAmount& amount);
extern CPubKey AccountFromValue(const Value& value);
extern double GetDifficulty(const CBlockIndex* blockindex = NULL);
extern std::string HelpRequiringPassphrase();
extern std::string HelpExampleCli(std::string methodname, std::string args);
extern std::string HelpExampleRpc(std::string methodname, std::string args);
extern std::string GetBinaryContent(const std::string& content);

extern void EnsureWalletIsUnlocked();

extern json_spirit::Value getconnectioncount(const json_spirit::Array& params, bool fHelp); // in rpcnet.cpp
extern json_spirit::Value getpeerinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value ping(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value addnode(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getaddednodeinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getnettotals(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value broadcastmessage(const Array& params, bool fHelp);

extern json_spirit::Value dumpprivkey(const json_spirit::Array& params, bool fHelp); // in rpcdump.cpp
extern json_spirit::Value importprivkey(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value importaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value dumpwallet(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value importwallet(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value getgenerate(const json_spirit::Array& params, bool fHelp); // in rpcmining.cpp
extern json_spirit::Value setgenerate(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getnetworkhashps(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value gethashespersec(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getmininginfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value prioritisetransaction(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getblocktemplate(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value submitblock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value estimatefee(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value estimatepriority(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value poolmine(const Array& params, bool fHelp);
extern json_spirit::Value mhash(const Array& params, bool fHelp);

extern json_spirit::Value getnewid(const json_spirit::Array& params, bool fHelp); // in rpcwallet.cpp
extern json_spirit::Value getmainid(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getrawchangeaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value setaccount(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getaccount(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getidlist(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value sendtoaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value signmessage(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value verifymessage(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getreceivedbyaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getreceivedbyaccount(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getbalance(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getunconfirmedbalance(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value movecmd(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value sendfrom(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value sendmany(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value addmultisigaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value createmultisig(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value createcontent(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodecontent(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value listreceivedbyaddress(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value listreceivedbyaccount(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value listtransactions(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value listaddressgroupings(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value listaccounts(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value listsinceblock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value gettransaction(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value backupwallet(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value keypoolrefill(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value walletpassphrase(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value walletpassphrasechange(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value walletlock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value encryptwallet(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value validateaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getwalletinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getblockchaininfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getnetworkinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value setmocktime(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getmaturetime(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getcontacts(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value addcontacts(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getsimplesig(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value publishpackage(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value getrawtransaction(const json_spirit::Array& params, bool fHelp); // in rcprawtransaction.cpp
extern json_spirit::Value listunspent(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value listunspent2(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value lockunspent(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value listlockunspent(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value createrawtransaction(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decoderawtransaction(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodescript(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value signrawtransaction(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value sendrawtransaction(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value getblockcount(const json_spirit::Array& params, bool fHelp); // in rpcblockchain.cpp
extern json_spirit::Value getbestblockhash(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdifficulty(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value settxfee(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getmempoolinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getrawmempool(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getblockhash(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getblockbyheight(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getblock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value gettxoutsetinfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value gettxout(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value verifychain(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getchaintips(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value invalidateblock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value reconsiderblock(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getcontent(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getfeerate(const Array& params, bool fHelp);

// New
extern json_spirit::Value getcontentbylink(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getcontentbystring(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value encodecontentunit(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodecontentunit(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getlinkbytxidout(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getlinktype(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getcontents(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getpromotedcontents(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getsalesrecord(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getfirstncc(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getmessages(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value getbrowserconf(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value getfollowed(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value setfollow(const json_spirit::Array& params, bool fHelp);
//extern json_spirit::Value setunfollow(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getfilepackageurl(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value encodevarint(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodevarint(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value getdomaininfo(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainsbyowner(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainbyforward(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainsbyforward(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainsbytags(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainsbyalias(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getdomainsexpiring(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value searchproducts(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value setconf(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getconf(const json_spirit::Array& params, bool fHelp); 
extern json_spirit::Value readfile(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value writefile(const json_spirit::Array& params, bool fHelp);
extern Value getsettings(const Array& params, bool fHelp);
extern Value updatesettings(const Array& params, bool fHelp);


extern json_spirit::Value encodebase32(const json_spirit::Array& params, bool fHelp);//rpc util
extern json_spirit::Value decodebase32(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value encodebase32check(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodebase32check(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value standardizebase32(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value comparebase32(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value isvalidpubkeyaddress(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getextkey(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value getextpubkey(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value gethash(const json_spirit::Array& params, bool fHelp);

extern json_spirit::Value devtest(const json_spirit::Array& params, bool fHelp); // to be deleted

extern json_spirit::Value createcontent(const json_spirit::Array& params, bool fHelp);
extern json_spirit::Value decodecontent(const json_spirit::Array& params, bool fHelp);

extern CContent _create_text_content(std::string str);
//extern CContent _create_file_content(std::string str);
extern CContent _create_content(const json_spirit::Array& params);
extern json_spirit::Object _decode_content(const json_spirit::Array& params);
extern bool GetTxFromBlock(const CBlock& block, const int nTx, CTransaction& txOut);
extern bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout);
extern bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content);
//extern json_spirit::Object _voutToJson(const CTxOut& txout);
//extern json_spirit::Object _output_content(const CContent& cttIn, const int& cformat, const unsigned char& cttf, const CLink& clinkIn, const std::vector<CBitcoinAddress>& posters, const CAmount nValue, const CScript& scriptPubKey);
//extern json_spirit::Array _get_posters(const CTransaction&tx);
////extern bool _parse_getcontents_params(const json_spirit::Array& params, int& fbh, int& maxc, int& maxb, int& blkc, json_spirit::Array& withcc, json_spirit::Array& withoutcc, json_spirit::Array& firstcc, int& fContentFormat, unsigned char& cflag, int& mincsize, json_spirit::Array& addrs, bool& fAsc);
//extern bool _check_cc(const CContent& ctt, const json_spirit::Array& withcc, const json_spirit::Array& withoutcc, const json_spirit::Array& firstcc);


// in rest.cpp
extern bool HTTPReq_REST(AcceptedConnection *conn,
                  std::string& strURI,
                  std::map<std::string, std::string>& mapHeaders,
                  bool fRun);
class CPaymentOrder;
class CWalletTx;
class CWallet;
extern CPaymentOrder ParseJsonPaymentRequest(const json_spirit::Value paymentRequestJson,int nType=0);
extern CPaymentOrder MessageRequestToPaymentRequest(const std::string idLocal,const  std::string idForeign,const CContent msg);
extern CPaymentOrder GetPublisherPaymentRequest(const std::string idLocal,const  std::string idForeign,const CContent& ctt, const double feeRate = 1000.0, const CAmount deposit = 0, const uint32_t nLockTime = 0);
extern CPaymentOrder GetRegisterDomainPaymentRequest(const std::string id, const std::string domain, const uint32_t nLockTime);
extern CPaymentOrder GetUpdateDomainPaymentRequest(const Array arr);
extern CPaymentOrder GetRenewPaymentRequest(const Array arr);
extern CPaymentOrder GetTransferPaymentRequest(const Array arr);
extern CPaymentOrder GetPublishProductPaymentRequest(const Array arr);
extern CPaymentOrder GetBuyProductPaymentRequest(const Array arr);
extern CPaymentOrder GetBuyProductPaymentRequest(const Array arr);
extern CPaymentOrder GetPublishPackagetPaymentRequest(const Array arr);
extern CWalletTx CreateRawTransaction(CPaymentOrder pr,bool& fRequestPassword,CWallet*& pwallet);
extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry,const int nContentLenLimit=1024,std::map<int,CScript>* pmapPrevoutScriptPubKey=NULL,int nTx=-1);
extern Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool txDetails = false);
extern void GetMessagesFromTx(std::vector<CMessage>& vMessages,const CTransaction& tx,const int nBlockHeight,int nTx,int nTime,const std::vector<CScript>& vIDsLocal,
        const std::vector<CScript>& vIDsForeign,int nDirectionFilter,bool fLinkonly,int nPos,int nOffset,int nCount);
//extern int GetBlocksToMaturity(const unsigned int nLockTime);
//extern int GetLockLasting(uint32_t nLockTime);
//extern uint32_t LockTimeToTime(uint32_t nLockTime);
extern void SortMessages(std::vector<CMessage>& vMsg,std::vector<CScript> vIDsLocal);
extern Value gettxmessages(const json_spirit::Array& params, bool fHelp);

#endif // BITCOIN_RPCSERVER_H
