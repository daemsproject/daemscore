// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "primitives/transaction.h"
#include "core_io.h"
#include "init.h"
#include "keystore.h"
#include "main.h"
#include "fai/contentutil.h"
#include "fai/sqlitewrapper.h"
#include "net.h"
#include "rpcserver.h"
#include "fai/content.h"
#include "script/script.h"
#include "script/sign.h"
#include "script/standard.h"
#include "uint256.h"
#include <string>
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif
#include "util.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace boost;
using namespace boost::assign;
using namespace json_spirit;
using namespace std;

void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out, bool fIncludeHex)
{
    txnouttype type;
    vector<CTxDestination> addresses;
    unsigned int wRequired;

    out.push_back(Pair("asm", scriptPubKey.ToString()));
    if (fIncludeHex)
        out.push_back(Pair("hex", HexStr(scriptPubKey.begin(), scriptPubKey.end())));

    if (!ExtractDestinations(scriptPubKey, type, addresses, wRequired))
    {
        out.push_back(Pair("type", GetTxnOutputType(type)));
        return;
    }
    
    out.push_back(Pair("type", GetTxnOutputType(type)));

//    Array a;
//    BOOST_FOREACH(const CTxDestination& addr, addresses)
//        a.push_back(CBitcoinAddress(addr).ToString());
//    out.push_back(Pair("addresses", a));
    
    if (type == TX_MULTISIG)
    {        
        try{
            string str1=CBitcoinAddress(scriptPubKey).ToString();
            out.push_back(Pair("address",str1 ));
            //LogPrintf("ScriptPubKeyToJSON multisig:%s %s\n",scriptPubKey.ToString(),str1);
            Array a;
            a.push_back(str1);
            out.push_back(Pair("multisig",decodemultisigaddress(a,false)));
        }
        catch(const Object& e){
            LogPrintf("ScriptPubKeyToJSON:get multisig error %s \n",write_string(Value(e),false));
        }
    }else
    {
        string str=CBitcoinAddress(addresses[0]).ToString();
        out.push_back(Pair("address",str ));
    }
    //else
    //    out.push_back(Pair("reqSigs", (uint64_t) wRequired));
    //out.push_back(Pair("sigOpCount", (uint64_t) scriptPubKey.GetSigOpCount(true)));
}

std::string GetContentCode(const std::string& content)
{
    CContent ctt(content);
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    return ctt.ToHumanString(nMaxCC);
}

std::string GetBinaryContent(const std::string& content)
{
    return IsStringPrint(content)? std::string(content.begin(), content.end()): "";
}

void TxToJSON(const CTransaction& tx, const uint256 hashBlock, Object& entry,const int nContentLenLimit,std::map<int,CScript>* pmapPrevoutScriptPubKey,int nTx)
{
   // LogPrintf("TxToJSON1\n");    
    //LogPrintf("TxToJSON2\n");
    uint256 txid=tx.GetHash();
    entry.push_back(Pair("txid", txid.GetHex()));
    //LogPrintf("TxToJSON3\n");
    entry.push_back(Pair("version", tx.nVersion));
    tx.IsCoinBase()?
        entry.push_back(Pair("iscoinbase", true)): entry.push_back(Pair("iscoinbase", false));

    if (hashBlock!=0)
    { 
        entry.push_back(Pair("ntx", nTx>=0?nTx:GetNTx(txid)));    
    }
    //LogPrintf("TxToJSON4\n");
    Array vin;
    Array arrAddresses;
    for(int i=0;i<(int)tx.vin.size();i++) {
        const CTxIn& txin=tx.vin[i];
        Object in;
        in.push_back(Pair("txid", txin.prevout.hash.GetHex()));
        in.push_back(Pair("vout", (int64_t) txin.prevout.n));
        in.push_back(Pair("value", ValueFromAmount(txin.prevout.nValue)));            
        in.push_back(Pair("satoshi", txin.prevout.nValue));
        if (!tx.IsCoinBase()){ 
            CTransaction prevTx;
            uint256 tmphash;
            //LogPrintf("rpcrawtransaction txtojson: prevout hash:%s \n",txin.prevout.hash.GetHex());
            CScript prevoutScriptPubKey;
            if(pmapPrevoutScriptPubKey!=NULL&&(*pmapPrevoutScriptPubKey)[i].size()!=0)
            {
                
                prevoutScriptPubKey=(*pmapPrevoutScriptPubKey)[i];
            }
            else
            {
               // LogPrintf("TxToJSON tx:%s,get prevout tx:%s\n",tx.GetHash().GetHex(),txin.prevout.hash.GetHex());
                if (!GetTransaction(txin.prevout.hash, prevTx, tmphash, true))
                {
                   LogPrintf("TxToJSON null vin prevout\n");
                    continue;
                }
                prevoutScriptPubKey=prevTx.vout[txin.prevout.n].scriptPubKey;
                if(pmapPrevoutScriptPubKey!=NULL)
                    (*pmapPrevoutScriptPubKey)[i]=prevoutScriptPubKey;
            }
            Object a;
            ScriptPubKeyToJSON(prevoutScriptPubKey, a, true);
            in.push_back(Pair("scriptPubKey", a));
            
            string add;
            ScriptPubKeyToString(prevoutScriptPubKey,add);            
            arrAddresses.push_back(Value(add)); 
        }
            Object o;
            o.push_back(Pair("asm", txin.scriptSig.ToString()));
            o.push_back(Pair("hex", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
            in.push_back(Pair("scriptSig", o));
            //LogPrintf("TxToJSON6\n");
        
        vin.push_back(in);
    }
    entry.push_back(Pair("vin", vin));
    Array vout;
    for (unsigned int i = 0; i < tx.vout.size(); i++) {
        const CTxOut& txout = tx.vout[i];
        Object out;
        out.push_back(Pair("value", ValueFromAmount(txout.nValue)));
        out.push_back(Pair("satoshi", txout.nValue));
        out.push_back(Pair("n", (int64_t)i));
        if((int)txout.strContent.size()>nContentLenLimit)
            out.push_back(Pair("contentlen", (int)txout.strContent.size()));
        else
        {
            out.push_back(Pair("content", HexStr(txout.strContent.begin(), txout.strContent.end())));
            out.push_back(Pair("contentText", GetBinaryContent(txout.strContent)));
        }
        out.push_back(Pair("locktime", (int64_t)txout.nLockTime));
        Object o;
        ScriptPubKeyToJSON(txout.scriptPubKey, o, true);
        out.push_back(Pair("scriptPubKey", o));
        string addr;
        ScriptPubKeyToString(txout.scriptPubKey,addr);
        Value add=Value(addr);
        if (find(arrAddresses.begin(),arrAddresses.end(),add)==arrAddresses.end()){
            arrAddresses.push_back(add);
        }
        vout.push_back(out);
        //LogPrintf("TxToJSON7\n");
    }
    entry.push_back(Pair("vout", vout));
    entry.push_back(Pair("ids",arrAddresses));
    if (hashBlock != 0) {
        entry.push_back(Pair("blockhash", hashBlock.GetHex()));
        BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && (*mi).second) {
            CBlockIndex* pindex = (*mi).second;
            if (chainActive.Contains(pindex)) {
                entry.push_back(Pair("blockheight", pindex->nHeight));
                entry.push_back(Pair("confirmations", 1 + chainActive.Height() - pindex->nHeight));
                entry.push_back(Pair("time", pindex->GetBlockTime()));
                entry.push_back(Pair("blocktime", pindex->GetBlockTime()));
            }
            else
                entry.push_back(Pair("confirmations", 0));
        }
    }
    //LogPrintf("TxToJSON8\n");
}

Value getrawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getrawtransaction \"txid\" ( verbose )\n"
            "\nNOTE: By default this function only works sometimes. This is when the tx is in the mempool\n"
            "or there is an unspent output in the utxo for this transaction. To make it always work,\n"
            "you need to maintain a transaction index, using the -txindex command line option.\n"
            "\nReturn the raw transaction data.\n"
            "\nIf verbose=0, returns a string that is serialized, hex-encoded data for 'txid'.\n"
            "If verbose is non-zero, returns an Object with information about 'txid'.\n"

            "\nArguments:\n"
            "1. \"txid\"      (string, required) The transaction id\n"
            "2. verbose       (numeric, optional, default=0) If 0, return a string, other return a json object\n"

            "\nResult (if verbose is not set or set to 0):\n"
            "\"data\"      (string) The serialized, hex-encoded data for 'txid'\n"

            "\nResult (if verbose > 0):\n"
            "{\n"
            "  \"hex\" : \"data\",       (string) The serialized, hex-encoded data for 'txid'\n"
            "  \"txid\" : \"id\",        (string) The transaction id (same as provided)\n"
            "  \"version\" : n,          (numeric) The version\n"
            "  \"locktime\" : ttt,       (numeric) The lock time\n"
            "  \"vin\" : [               (array of json objects)\n"
            "     {\n"
            "       \"txid\": \"id\",    (string) The transaction id\n"
            "       \"vout\": n,         (numeric) \n"
            "       \"scriptSig\": {     (json object) The script\n"
            "         \"asm\": \"asm\",  (string) asm\n"
            "         \"hex\": \"hex\"   (string) hex\n"
            "       },\n"
            "       \"sequence\": n      (numeric) The script sequence number\n"
            "     }\n"
            "     ,...\n"
            "  ],\n"
            "  \"vout\" : [              (array of json objects)\n"
            "     {\n"
            "       \"value\" : x.xxx,            (numeric) The value in ltc\n"
            "       \"n\" : n,                    (numeric) index\n"
            "       \"scriptPubKey\" : {          (json object)\n"
            "         \"asm\" : \"asm\",          (string) the asm\n"
            "         \"hex\" : \"hex\",          (string) the hex\n"
            "         \"reqSigs\" : n,            (numeric) The required sigs\n"
            "         \"type\" : \"pubkey\",  (string) The type, eg 'pubkey'\n"
            "         \"addresses\" : [           (json array of string)\n"
            "           \"faicoinaddress\"        (string) faicoin address\n"
            "           ,...\n"
            "         ]\n"
            "       }\n"
            "     }\n"
            "     ,...\n"
            "  ],\n"
            "  \"blockhash\" : \"hash\",   (string) the block hash\n"
            "  \"confirmations\" : n,      (numeric) The confirmations\n"
            "  \"time\" : ttt,             (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT)\n"
            "  \"blocktime\" : ttt         (numeric) The block time in seconds since epoch (Jan 1 1970 GMT)\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("getrawtransaction", "\"mytxid\"")
            + HelpExampleCli("getrawtransaction", "\"mytxid\" 1")
            + HelpExampleRpc("getrawtransaction", "\"mytxid\", 1")
        );

    uint256 hash = ParseHashV(params[0], "parameter 1");

    bool fVerbose = false;
    if (params.size() > 1)
        fVerbose = (params[1].get_int() != 0);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock, true))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");

    string strHex = EncodeHexTx(tx);

    if (!fVerbose)
        return strHex;

    Object result;
    result.push_back(Pair("hex", strHex));
    TxToJSON(tx, hashBlock, result);
    return result;
}

#ifdef ENABLE_WALLET

Value listunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "listunspent ( minconf maxconf  [\"address\",...] )\n"
            "\nReturns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.\n"
            "Optionally filter to only include txouts paid to specified addresses.\n"
            "Results are an array of Objects, each of which has:\n"
            "{txid, vout, scriptPubKey, amount, confirmations}\n"
            "\nArguments:\n"
            "1. minconf          (numeric, optional, default=1) The minimum confirmations to filter\n"
            "2. maxconf          (numeric, optional, default=9999999) The maximum confirmations to filter\n"
            "3. \"addresses\"    (string) A json array of faicoin addresses to filter\n"
            "    [\n"
            "      \"address\"   (string) faicoin address\n"
            "      ,...\n"
            "    ]\n"
            "\nResult\n"
            "[                   (array of json object)\n"
            "  {\n"
            "    \"txid\" : \"txid\",        (string) the transaction id \n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"address\" : \"address\",  (string) the faicoin address\n"
            "    \"account\" : \"account\",  (string) The associated account, or \"\" for the default account\n"
            "    \"scriptPubKey\" : \"key\", (string) the script key\n"
            "    \"amount\" : x.xxx,         (numeric) the transaction amount in ltc\n"
            "    \"confirmations\" : n       (numeric) The number of confirmations\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples\n"
            + HelpExampleCli("listunspent", "")
            + HelpExampleCli("listunspent", "6 9999999 \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"")
            + HelpExampleRpc("listunspent", "6, 9999999 \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"")
        );

    RPCTypeCheck(params, list_of(int_type)(int_type)(array_type));

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    int nMaxDepth = 9999999;
    if (params.size() > 1)
        nMaxDepth = params[1].get_int();

    set<CBitcoinAddress> setAddress;
    if (params.size() > 2) {
        Array inputs = params[2].get_array();
        BOOST_FOREACH(Value& input, inputs)
        {
            CBitcoinAddress address(input.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Faicoin address: ")+input.get_str());
            if (setAddress.count(address))
                throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+input.get_str());
           setAddress.insert(address);
        }
    }

    Array results;
    vector<COutput> vecOutputs;
    assert(pwalletMain != NULL);
    pwalletMain->AvailableCoins(vecOutputs, false);
    BOOST_FOREACH(const COutput& out, vecOutputs) 
    {
        if (out.nDepth < nMinDepth || out.nDepth > nMaxDepth)
            continue;

        if (setAddress.size()) {
            CTxDestination address;
            if (!ExtractDestination(out.tx->vout[out.i].scriptPubKey, address))
                continue;

            if (!setAddress.count(address))
                continue;
        }

        CAmount nValue = out.tx->vout[out.i].nValue;
        const CScript& pk = out.tx->vout[out.i].scriptPubKey;
        Object entry;
        entry.push_back(Pair("txid", out.tx->GetHash().GetHex()));
        entry.push_back(Pair("vout", out.i));
        CTxDestination address;
        if (ExtractDestination(out.tx->vout[out.i].scriptPubKey, address)) {
            entry.push_back(Pair("address", CBitcoinAddress(address).ToString()));
            if (pwalletMain->mapAddressBook.count(address))
                entry.push_back(Pair("account", pwalletMain->mapAddressBook[address].name));
        }
        entry.push_back(Pair("scriptPubKey", HexStr(pk.begin(), pk.end())));
        if (pk.IsPayToScriptHash()) {
            CTxDestination address;
            if (ExtractDestination(pk, address)) {
                const CScriptID& hash = boost::get<const CScriptID&>(address);
                CScript redeemScript;
                if (pwalletMain->GetCScript(hash, redeemScript))
                    entry.push_back(Pair("redeemScript", HexStr(redeemScript.begin(), redeemScript.end())));
            }
        }
        entry.push_back(Pair("amount", ValueFromAmount(nValue)));
        entry.push_back(Pair("satoshi", nValue));
        entry.push_back(Pair("confirmations", out.nDepth));
        entry.push_back(Pair("spendable", out.fSpendable));
        results.push_back(entry);
    }

    return results;
}
#endif
json_spirit::Value listunspent2(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error("list unspent2,array ids,bool fSpendableonly,nMaxItems,nOffset");        
    
    bool fSpendableonly =false;
    if(params.size()>1)
    {
        fSpendableonly=params[1].get_bool();
    }
    int nMaxItems=1000000;
    int nOffset=0;
    if(params.size()>2)
    {
        nMaxItems=params[2].get_int();
    }
    if(params.size()>3)
    {
        nOffset=params[3].get_int();
    }
    Array arrIDs=params[0].get_array();   
    vector<CScript> vScriptPubKeys;
    for(unsigned int i=0;i<arrIDs.size();i++)
    {
        CScript script;
        //console.log("listunspent2 address:%s \n",arrIDs[i].get_str());
        if(!StringToScriptPubKey(arrIDs[i].get_str(),script))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Faicoin address");
        vScriptPubKeys.push_back(script);
    }
    vector<CCheque>vCheques;
    GetUnspentCheques(vScriptPubKeys,vCheques,fSpendableonly,nMaxItems,nOffset);        
    Array results;
    for(unsigned int i=0;i<vCheques.size();i++)
    {
        const CScript& pk = vCheques[i].scriptPubKey;   
        Object entry;
        entry.push_back(Pair("txid", vCheques[i].txid.GetHex()));
        entry.push_back(Pair("vout", vCheques[i].nOut));
        CTxDestination address;
        if (ExtractDestination(vCheques[i].scriptPubKey, address)) {
            entry.push_back(Pair("address", CBitcoinAddress(address).ToString()));            
        }
        entry.push_back(Pair("scriptPubKey", HexStr(pk.begin(), pk.end())));
        if (pk.IsPayToScriptHash()) {
            CTxDestination address;
            if (ExtractDestination(pk, address)) {
                const CScriptID& hash = boost::get<const CScriptID&>(address);
                CScript redeemScript;
                if (pwalletMain&&pwalletMain->GetCScript(hash, redeemScript))
                    entry.push_back(Pair("redeemScript", HexStr(redeemScript.begin(), redeemScript.end())));
            }
        }
        entry.push_back(Pair("amount", ValueFromAmount(vCheques[i].nValue)));
        entry.push_back(Pair("satoshi", vCheques[i].nValue));
        entry.push_back(Pair("blockheight", vCheques[i].txIndex>>16));
        entry.push_back(Pair("ntx", vCheques[i].txIndex&0xffff));
        entry.push_back(Pair("locktime",(int64_t)vCheques[i].nLockTime));
        results.push_back(entry);
    }
    return results;
}
Value createrawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "createrawtransaction [{\"txid\":\"id\",\"vout\":n,\"satoshi\":satoshi},...] [{\"address\":\"address\",\"satoshi\":amount,\"content\":\"contenthex\"}]\n"
            "\nCreate a transaction spending the given inputs and sending to the given addresses.\n"
            "Returns hex-encoded raw transaction.\n"
            "Note that the transaction's inputs are not signed, and\n"
            "it is not stored in the wallet or transmitted to the network.\n"

            "\nArguments:\n"
            "1. \"transactions\"        (string, required) A json array of json objects\n"
            "     [\n"
            "       {\n"
            "         \"txid\":\"id\",  (string, required) The transaction id\n"
            "         \"vout\":n        (numeric, required) The output number\n"
            "         \"satoshi\":n        (integer, required) The output value in satoshi\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"
            "2. \"addresses\"           (string, required) a json object with addresses as keys and amounts as satoshis\n"
            "    [{\n"
            "      \"address\": \"address\" (string, required) The address to recieve coin\n"
            "      \"satoshi\":n        (integer, required) The output value in satoshi\n"
            "      \"content\": \"content\" (string, optional) content in hex format\n"
            "      \"locktime\": \"locktime\" (integer, optional) locktime\n"
            "      ,...\n"
            "    }\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "\"transaction\"            (string) hex string of the transaction\n"

            "\nExamples\n"
            + HelpExampleCli("createrawtransaction", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0,\\\"satoshi\\\":0}]\" \"[{\\\"address\\\":\\\"myaddress\\\",\\\"satoshi\\\":0.01}]\"")
            + HelpExampleRpc("createrawtransaction", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0,,\\\"satoshi\\\":0}]\", \"[{\\\"address\\\":\\\"myaddress\\\",\\\"satoshi\\\":0.01}]\"")
        );

    RPCTypeCheck(params, list_of(array_type)(array_type));

    Array inputs = params[0].get_array();
    Array sendTo = params[1].get_array(); //get_obj();

    CMutableTransaction rawTx;

    BOOST_FOREACH(const Value& input, inputs)
    {
        const Object& o = input.get_obj();

        uint256 txid = ParseHashO(o, "txid");

        const Value& vout_v = find_value(o, "vout");
        if (vout_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");
        int nOutput = vout_v.get_int();
        if (nOutput < 0||nOutput>65535)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, nvout must be positive");
        const Value& value_v = find_value(o, "satoshi");
        if (value_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter,input missing satoshi key");
        CAmount nValue = value_v.get_int64();
        if (nValue < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, input satoshi must be positive");
        CTxIn in(COutPoint(txid, nOutput, nValue));
        rawTx.vin.push_back(in);
    }

    //set<CBitcoinAddress> setAddress;
    //BOOST_FOREACH(const Pair& s, sendTo) {

    BOOST_FOREACH(const Value& output, sendTo)
    {
            const Object& o = output.get_obj();
            const Value& address_v = find_value(o, "address");
        CScript scriptPubKey = CScript();
        if (address_v.get_str() != "") {
                CBitcoinAddress address(address_v.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Faicoin address: ") + address_v.get_str());
            scriptPubKey = GetScriptForDestination(address.Get());
        }
        const Value& value_v = find_value(o, "satoshi");
        if (value_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, output missing satoshi key");
        CAmount nAmount = value_v.get_int64();
        if (nAmount < 0 || nAmount > MAX_MONEY)
            throw JSONRPCError(RPC_MISC_ERROR, string("output value out of range"));
        const Value& content_v = find_value(o, "content");
        string strContent = "";
        if (!content_v.is_null()) {
            vector<unsigned char> vContent;
            if (content_v.get_str().size() > 0)
                vContent = ParseHexV(content_v, "content");
            for (vector<unsigned char>::iterator iter = vContent.begin(); iter != vContent.end(); ++iter) {
                strContent += *iter;
            }
        }            
        const Value& locktime_v = find_value(o, "locktime");
        uint32_t nLocktime = 0;
        if (!locktime_v.is_null()) {
            int64_t tmpLocktime =  locktime_v.get_int64();
            
            if (tmpLocktime >= 0 && tmpLocktime <= 0xffffffff) 
                nLocktime = (uint32_t) tmpLocktime;
            else
                throw JSONRPCError(RPC_MISC_ERROR, string("Invalid locktime: "));
        }
        CTxOut out(nAmount, scriptPubKey, strContent,nLocktime);
        rawTx.vout.push_back(out);
    }

    return EncodeHexTx(rawTx);
}

Value decoderawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "decoderawtransaction \"hexstring\"\n"
            "\nReturn a JSON object representing the serialized, hex-encoded transaction.\n"

            "\nArguments:\n"
            "1. \"hex\"      (string, required) The transaction hex string\n"

            "\nResult:\n"
            "{\n"
            "  \"txid\" : \"id\",        (string) The transaction id\n"
            "  \"version\" : n,          (numeric) The version\n"
            "  \"locktime\" : ttt,       (numeric) The lock time\n"
            "  \"vin\" : [               (array of json objects)\n"
            "     {\n"
            "       \"txid\": \"id\",    (string) The transaction id\n"
            "       \"vout\": n,         (numeric) The output number\n"
            "       \"scriptSig\": {     (json object) The script\n"
            "         \"asm\": \"asm\",  (string) asm\n"
            "         \"hex\": \"hex\"   (string) hex\n"
            "       },\n"
            "       \"sequence\": n     (numeric) The script sequence number\n"
            "     }\n"
            "     ,...\n"
            "  ],\n"
            "  \"vout\" : [             (array of json objects)\n"
            "     {\n"
            "       \"value\" : x.xxx,            (numeric) The value in ltc\n"
            "       \"n\" : n,                    (numeric) index\n"
            "       \"scriptPubKey\" : {          (json object)\n"
            "         \"asm\" : \"asm\",          (string) the asm\n"
            "         \"hex\" : \"hex\",          (string) the hex\n"
            "         \"reqSigs\" : n,            (numeric) The required sigs\n"
            "         \"type\" : \"pubkey\",  (string) The type, eg 'pubkey'\n"
            "         \"addresses\" : [           (json array of string)\n"
            "           \"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"   (string) faicoin address\n"
            "           ,...\n"
            "         ]\n"
            "       }\n"
            "     }\n"
            "     ,...\n"
            "  ],\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("decoderawtransaction", "\"hexstring\"")
            + HelpExampleRpc("decoderawtransaction", "\"hexstring\"")
        );

    RPCTypeCheck(params, list_of(str_type));

    CTransaction tx;

    if (!DecodeHexTx(tx, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");

    Object result;
    TxToJSON(tx, 0, result,MAX_STANDARD_TX_SIZE);

    return result;
}
Value encoderawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "enocderawtransaction [{txjson}]\n"
            );
    CMutableTransaction rawTx;
    Object objTx=params[0].get_obj();
    rawTx.nVersion=find_value(objTx, "version").get_int();
    Array inputs = find_value(objTx, "vin").get_array();
    Array sendTo = find_value(objTx, "vout").get_array(); 
    BOOST_FOREACH(const Value& input, inputs)
    {
        const Object& o = input.get_obj();
        uint256 txid = ParseHashO(o, "txid");
        const Value& vout_v = find_value(o, "vout");
        if (vout_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");
        int nOutput = vout_v.get_int();
        if (nOutput < 0||nOutput>65535)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, nvout must be positive");
        const Value& value_v = find_value(o, "satoshi");
        if (value_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter,input missing satoshi key");
        CAmount nValue = value_v.get_int64();
        if (nValue < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, input satoshi must be positive");
        CScript scriptSig;
         const Value& script_v = find_value(o, "scriptSig");
        if (script_v.type() == obj_type)
        {
            const Value scripthex_v = find_value(script_v.get_obj(), "hex");
            if(scripthex_v.type()==str_type&&scripthex_v.get_str().size() > 0)
            {
                vector<unsigned char> vScript;            
                vScript = ParseHexV(scripthex_v, "scriptSig");
                scriptSig.assign(vScript.begin(),vScript.end());
            }
        }
        CTxIn in(COutPoint(txid, nOutput, nValue),scriptSig);
        rawTx.vin.push_back(in);
    }
    BOOST_FOREACH(const Value& output, sendTo)
    {
        const Object& o = output.get_obj();
        CScript scriptPubKey = CScript();
        const Value& scriptPubKey_v = find_value(o, "scriptPubKey");
        const Value scripthex_v = find_value(scriptPubKey_v.get_obj(), "hex");
        if(scripthex_v.type()==str_type)
        {
            if(scripthex_v.get_str().size()>0)
            {
            vector<unsigned char> vScript;            
            vScript = ParseHexV(scripthex_v, "scriptPubKey");
            scriptPubKey.assign(vScript.begin(),vScript.end());
            }
        }
        else 
        {
            const Value&  address_v= find_value(scriptPubKey_v.get_obj(), "address");
            if (address_v.get_str() != "")
            {
                CBitcoinAddress address(address_v.get_str());
                if (!address.IsValid())
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Faicoin address: ") + address_v.get_str());
                scriptPubKey = GetScriptForDestination(address.Get());
            }
        }
        const Value& value_v = find_value(o, "satoshi");
        if (value_v.type() != int_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, output missing satoshi key");
        CAmount nAmount = value_v.get_int64();
        if (nAmount < 0 || nAmount > MAX_MONEY)
            throw JSONRPCError(RPC_MISC_ERROR, string("output value out of range"));
        const Value& content_v = find_value(o, "content");
        string strContent = "";
        if (!content_v.is_null()) {
            vector<unsigned char> vContent;
            if (content_v.get_str().size() > 0)
                vContent = ParseHexV(content_v, "content");
            for (vector<unsigned char>::iterator iter = vContent.begin(); iter != vContent.end(); ++iter) {
                strContent += *iter;
            }
        }            
        const Value& locktime_v = find_value(o, "locktime");
        uint32_t nLocktime = 0;
        if (!locktime_v.is_null()) {
            int64_t tmpLocktime =  locktime_v.get_int64();
            
            if (tmpLocktime >= 0 && tmpLocktime <= 0xffffffff) 
                nLocktime = (uint32_t) tmpLocktime;
            else
                throw JSONRPCError(RPC_MISC_ERROR, string("Invalid locktime: "));
        }
        CTxOut out(nAmount, scriptPubKey, strContent,nLocktime);
        rawTx.vout.push_back(out);
    }
    return EncodeHexTx(rawTx);
}
Value decodescript(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "decodescript \"hex\"\n"
            "\nDecode a hex-encoded script.\n"
            "\nArguments:\n"
            "1. \"hex\"     (string) the hex encoded script\n"
            "\nResult:\n"
            "{\n"
            "  \"asm\":\"asm\",   (string) Script public key\n"
            "  \"hex\":\"hex\",   (string) hex encoded public key\n"
            "  \"type\":\"type\", (string) The output type\n"
            "  \"reqSigs\": n,    (numeric) The required signatures\n"
            "  \"addresses\": [   (json array of string)\n"
            "     \"address\"     (string) faicoin address\n"
            "     ,...\n"
            "  ],\n"
            "  \"p2sh\",\"address\" (string) script address\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("decodescript", "\"hexstring\"")
            + HelpExampleRpc("decodescript", "\"hexstring\"")
        );

    RPCTypeCheck(params, list_of(str_type));

    Object r;
    CScript script;
    if (params[0].get_str().size() > 0) {
        vector<unsigned char> scriptData(ParseHexV(params[0], "argument"));
        script = CScript(scriptData.begin(), scriptData.end());
    } else {
        // Empty scripts are valid
    }
    ScriptPubKeyToJSON(script, r, false);

    r.push_back(Pair("p2sh", CBitcoinAddress(CScriptID(script)).ToString()));
    return r;
}

Value signrawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 5)
        throw runtime_error(
            "signrawtransaction \"hexstring\" ( [{\"txid\":\"id\",\"vout\":n,\"scriptPubKey\":\"hex\",\"redeemScript\":\"hex\"},...] [\"privatekey1\",...] sighashtype )\n"
            "\nSign inputs for raw transaction (serialized, hex-encoded).\n"
            "The second optional argument (may be null) is an array of previous transaction outputs that\n"
            "this transaction depends on but may not yet be in the block chain.\n"
            "The third optional argument (may be null) is an array of base32-encoded private\n"
            "keys that, if given, will be the only keys used to sign the transaction.\n"
#ifdef ENABLE_WALLET
            + HelpRequiringPassphrase() + "\n"
#endif

            "\nArguments:\n"
            "1. \"hexstring\"     (string, required) The transaction hex string\n"
            "2. \"prevtxs\"       (string, optional) An json array of previous dependent transaction outputs\n"
            "     [               (json array of json objects, or 'null' if none provided)\n"
            "       {\n"
            "         \"txid\":\"id\",             (string, required) The transaction id\n"
            "         \"vout\":n,                  (numeric, required) The output number\n"
            "         \"scriptPubKey\": \"hex\",   (string, required) script key\n"
            "         \"redeemScript\": \"hex\"    (string, required for P2SH) redeem script\n"
            "       }\n"
            "       ,...\n"
            "    ]\n"
            "3. \"privatekeys\"     (string, optional) A json array of base32-encoded private keys for signing\n"
            "    [                  (json array of strings, or 'null' if none provided)\n"
            "      \"privatekey\"   (string) private key in base32-encoding\n"
            "      ,...\n"
            "    ]\n"
            "4. \"sighashtype\"     (string, optional, default=ALL) The signature hash type. Must be one of\n"
            "       \"ALL\"\n"
            "       \"NONE\"\n"
            "       \"SINGLE\"\n"
            "       \"ALL|ANYONECANPAY\"\n"
            "       \"NONE|ANYONECANPAY\"\n"
            "       \"SINGLE|ANYONECANPAY\"\n"

            "\nResult:\n"
            "{\n"
            "  \"hex\": \"value\",   (string) The raw transaction with signature(s) (hex-encoded string)\n"
            "  \"complete\": n       (numeric) if transaction has a complete set of signature (0 if not)\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("signrawtransaction", "\"myhex\"")
            + HelpExampleRpc("signrawtransaction", "\"myhex\"")
        );

    RPCTypeCheck(params, list_of(str_type)(array_type) (array_type) (str_type), true);
   // int64_t startTime=GetTimeMillis();
    vector<unsigned char> txData(ParseHexV(params[0], "argument 1"));
    CDataStream ssData(txData, SER_NETWORK, PROTOCOL_VERSION);
    vector<CMutableTransaction> txVariants;
    while (!ssData.empty()) {
        try {
            CMutableTransaction tx;

            CDataStream vrecvcopy = ssData;
            ssData >> tx;
            CDataStream vrecvretrieve = ssData;
            vrecvretrieve << tx;
            if (vrecvcopy != vrecvretrieve)
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
            txVariants.push_back(tx);
        } catch (const std::exception &) {
            throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
        }
    }
   
    if (txVariants.empty())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Missing transaction");

    // mergedTx will end up with all the signatures; it
    // starts as a clone of the rawtx:
    CMutableTransaction mergedTx(txVariants[0]);
    bool fComplete = true;
 //LogPrintf("rpc signrawtransacxtion tx prepared %ld \n",GetTimeMillis()-startTime);
    // Fetch previous transactions (inputs):
    CCoinsView viewDummy;
    CCoinsViewCache view(&viewDummy);
    {
        LOCK(mempool.cs);
        CCoinsViewCache &viewChain = *pcoinsTip;
        CCoinsViewMemPool viewMempool(&viewChain, mempool);
        view.SetBackend(viewMempool); // temporarily switch cache backend to db+mempool view

        BOOST_FOREACH(const CTxIn& txin, mergedTx.vin)
        {
            const uint256& prevHash = txin.prevout.hash;
            CCoins coins;
            view.AccessCoins(prevHash); // this is certainly allowed to fail
        }

        view.SetBackend(viewDummy); // switch back to avoid locking mempool for too long
    }
 //LogPrintf("rpc signrawtransacxtion prevouts fetched %ld \n",GetTimeMillis()-startTime);
    bool fGivenKeys = false;
    CBasicKeyStore tempKeystore;
    if (params.size() > 2 && params[2].type() != null_type) {
        
        Array keys = params[2].get_array();
        if(keys.size()>0)
            fGivenKeys = true;
        BOOST_FOREACH(Value k, keys)
        {
            CBitcoinSecret vchSecret;
            bool fGood = vchSecret.SetString(k.get_str());
            if (!fGood)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
            CKey key = vchSecret.GetKey();
            if (!key.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");
            tempKeystore.AddKey(key);
        }
    }
#ifdef ENABLE_WALLET
    else
        EnsureWalletIsUnlocked();
#endif

    // Add previous txouts given in the RPC call:
    if (params.size() > 1 && params[1].type() != null_type) {
        Array prevTxs = params[1].get_array();

        BOOST_FOREACH(Value& p, prevTxs)
        {
            if (p.type() != obj_type)
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "expected object with {\"txid'\",\"vout\",\"scriptPubKey\"}");

            Object prevOut = p.get_obj();

            RPCTypeCheck(prevOut, map_list_of("txid", str_type)("vout", int_type)("scriptPubKey", str_type));

            uint256 txid = ParseHashO(prevOut, "txid");

            int nOut = find_value(prevOut, "vout").get_int();
            if (nOut < 0)
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "vout must be positive");

            vector<unsigned char> pkData(ParseHexO(prevOut, "scriptPubKey"));
            CScript scriptPubKey(pkData.begin(), pkData.end());

            {
                CCoinsModifier coins = view.ModifyCoins(txid);
                if (coins->IsAvailable(nOut) && coins->vout[nOut].scriptPubKey != scriptPubKey) {
                    string err("Previous output scriptPubKey mismatch:\n");
                    err = err + coins->vout[nOut].scriptPubKey.ToString() + "\nvs:\n" +
                        scriptPubKey.ToString();
                    throw JSONRPCError(RPC_DESERIALIZATION_ERROR, err);
                }
                if ((unsigned int) nOut >= coins->vout.size())
                    coins->vout.resize(nOut + 1);
                coins->vout[nOut].scriptPubKey = scriptPubKey;
                coins->vout[nOut].nValue = 0; // we don't know the actual output value
            }

            // if redeemScript given and not using the local wallet (private keys
            // given), add redeemScript to the tempKeystore so it can be signed:
            if (fGivenKeys && scriptPubKey.IsPayToScriptHash()) {
                RPCTypeCheck(prevOut, map_list_of("txid", str_type)("vout", int_type)("scriptPubKey", str_type)("redeemScript", str_type));
                Value v = find_value(prevOut, "redeemScript");
                if (!(v == Value::null)) {
                    vector<unsigned char> rsData(ParseHexV(v, "redeemScript"));
                    CScript redeemScript(rsData.begin(), rsData.end());
                    tempKeystore.AddCScript(redeemScript);
                }
            }
        }
    }

#ifdef ENABLE_WALLET
    const CBasicKeyStore& keystore = ((fGivenKeys || !pwalletMain) ? tempKeystore : *pwalletMain);
#else
    const CBasicKeyStore& keystore = tempKeystore;
#endif

    int nHashType = SIGHASH_ALL;
    if (params.size() > 3 && params[3].type() != null_type) {
        static map<string, int> mapSigHashValues =
            boost::assign::map_list_of
            (string("ALL"), int(SIGHASH_ALL))
                (string("ALL|ANYONECANPAY"), int(SIGHASH_ALL | SIGHASH_ANYONECANPAY))
            (string("NONE"), int(SIGHASH_NONE))
                (string("NONE|ANYONECANPAY"), int(SIGHASH_NONE | SIGHASH_ANYONECANPAY))
            (string("SINGLE"), int(SIGHASH_SINGLE))
                (string("SINGLE|ANYONECANPAY"), int(SIGHASH_SINGLE | SIGHASH_ANYONECANPAY))
            ;
        string strHashType = params[3].get_str();
        if (mapSigHashValues.count(strHashType))
            nHashType = mapSigHashValues[strHashType];
        else
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid sighash param");
    }
    CScript p2shScript;
    if(params.size()>4){
        string scriptB32;
        scriptB32=params[4].get_str();
        CBitcoinAddress add;
        if(!StringToScriptPubKey(scriptB32,p2shScript))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid script param");
        
    }
    
 //LogPrintf("rpc signrawtransacxtion tx other params prepared %ld \n",GetTimeMillis()-startTime);
    // Sign what we can:
    for (unsigned int i = 0; i < mergedTx.vin.size(); i++) {
        CTxIn& txin = mergedTx.vin[i];
        const CCoins* coins = view.AccessCoins(txin.prevout.hash);
        if (coins == NULL || !coins->IsAvailable(txin.prevout.n)) {
            fComplete = false;
            continue;
        }
        const CScript& prevScriptPubKey = coins->vout[txin.prevout.n].scriptPubKey;
     //    LogPrintf("rpc signrawtransacxtion prevoutscript %ld \n",GetTimeMillis()-startTime);
        int nHashTypeIn=nHashType;
        if(txin.scriptSig.size()==1)//this means sigtype
        {
            nHashTypeIn=txin.scriptSig[0];
            static map<int,int> mapSigHashValues =
            boost::assign::map_list_of
            ( int(SIGHASH_ALL),0)
                (int(SIGHASH_ALL | SIGHASH_ANYONECANPAY),0)
            (int(SIGHASH_NONE),0)
                (int(SIGHASH_NONE | SIGHASH_ANYONECANPAY),0)
            (int(SIGHASH_SINGLE),0)
                (int(SIGHASH_SINGLE | SIGHASH_ANYONECANPAY),0)
            ;
            if (!mapSigHashValues.count(nHashTypeIn))            
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid sighash param");
        }
        bool fHashSingle = ((nHashTypeIn & ~SIGHASH_ANYONECANPAY) == SIGHASH_SINGLE);
        txin.scriptSig.clear();
        // Only sign SIGHASH_SINGLE if there's a corresponding output:
        if (!fHashSingle || (i < mergedTx.vout.size()))
        {
            if(p2shScript.size()>0)
                SignSignature4SH(keystore, prevScriptPubKey, mergedTx, i,p2shScript, nHashTypeIn);
            else
                SignSignature(keystore, prevScriptPubKey, mergedTx, i, nHashTypeIn);
        }

        // ... and merge in other signatures:
     //    LogPrintf("rpc signrawtransacxtion signed %ld \n",GetTimeMillis()-startTime);
        BOOST_FOREACH(const CMutableTransaction& txv, txVariants)
        {
            txin.scriptSig = CombineSignatures(prevScriptPubKey, mergedTx, i, txin.scriptSig, txv.vin[i].scriptSig);
        }
       //  LogPrintf("rpc signrawtransacxtion tx merged %ld \n",GetTimeMillis()-startTime);
        if (!VerifyScript(txin.scriptSig, prevScriptPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, MutableTransactionSignatureChecker(&mergedTx, i)))
            fComplete = false;
    }

    Object result;
    result.push_back(Pair("hex", EncodeHexTx(mergedTx)));
    result.push_back(Pair("complete", fComplete));

    return result;
}

Value sendrawtransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "sendrawtransaction \"hexstring\" ( allowhighfees )\n"
            "\nSubmits raw transaction (serialized, hex-encoded) to local node and network.\n"
            "\nAlso see createrawtransaction and signrawtransaction calls.\n"
            "\nArguments:\n"
            "1. \"hexstring\"    (string, required) The hex string of the raw transaction)\n"
            "2. allowhighfees    (boolean, optional, default=false) Allow high fees\n"
            "\nResult:\n"
            "\"hex\"             (string) The transaction hash in hex\n"
            "\nExamples:\n"
            "\nCreate a transaction\n"
            + HelpExampleCli("createrawtransaction", "\"[{\\\"txid\\\" : \\\"mytxid\\\",\\\"vout\\\":0}]\" \"{\\\"myaddress\\\":0.01}\"") +
            "Sign the transaction, and get back the hex\n"
            + HelpExampleCli("signrawtransaction", "\"myhex\"") +
            "\nSend the transaction (signed hex)\n"
            + HelpExampleCli("sendrawtransaction", "\"signedhex\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendrawtransaction", "\"signedhex\"")
        );

    RPCTypeCheck(params, list_of(str_type)(bool_type));

    // parse hex string from parameter
    CTransaction tx;
    if (!DecodeHexTx(tx, params[0].get_str()))
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    uint256 hashTx = tx.GetHash();

    bool fOverrideFees = false;
    if (params.size() > 1)
        fOverrideFees = params[1].get_bool();

    CCoinsViewCache &view = *pcoinsTip;
    const CCoins* existingCoins = view.AccessCoins(hashTx);
    bool fHaveMempool = mempool.exists(hashTx);
    bool fHaveChain = existingCoins && existingCoins->nHeight < 1000000000;
    if (!fHaveMempool && !fHaveChain) {
        // push to local node and sync with wallets
        CValidationState state;
        if (!AcceptToMemoryPool(mempool, state, tx, false, NULL, !fOverrideFees)) {
            if (state.IsInvalid())
                throw JSONRPCError(RPC_TRANSACTION_REJECTED, strprintf("%i: %s", state.GetRejectCode(), state.GetRejectReason()));
            else
                throw JSONRPCError(RPC_TRANSACTION_ERROR, state.GetRejectReason());
        }
    } else if (fHaveChain) {
        throw JSONRPCError(RPC_TRANSACTION_ALREADY_IN_CHAIN, "transaction already in block chain");
    }
    RelayTransaction(tx);
    CWalletTx wtx(pwalletMain,tx);
    wtx.nTimeReceived=GetTime();
    pwalletMain->addUnconfirmedTx(wtx);
    return hashTx.GetHex();
}
CWalletTx CreateRawTransaction(CPaymentOrder pr,bool& fRequestPassword,CWallet*& pwallet){
    //LogPrintf("rpcrawtransaction %s:%s\n",__func__,pr.vFrom[0].ToString());
    CPubKey id;
    CTxDestination address;
    if(!ExtractDestination(pr.vFrom[0],address))
        throw JSONRPCError(RPC_TRANSACTION_ERROR, "wrong idfrom");
    CBitcoinAddress pub;
    pub.Set(address);
    if(!pub.GetKey(id))
        throw JSONRPCError(RPC_TRANSACTION_ERROR, "wrong idfrom");
    
    pwallet=new CWallet(id);
    fRequestPassword=false;
    if (pwallet->IsCrypted())
        fRequestPassword=true;
    CWalletTx wtxNew;
    std::string strFailReason;
    if (!pwallet->CreateTransactionUnsigned(pr,wtxNew,strFailReason))
            throw JSONRPCError(RPC_TRANSACTION_ERROR, strFailReason);
    return wtxNew;
}