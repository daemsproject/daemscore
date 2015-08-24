// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "base58.h"
#include "clientversion.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "netbase.h"
#include "rpcserver.h"
#include "fai/contentutil.h"
#include "fai/shop.h"
#include "timedata.h"
#include "util.h"
#include "fai/domain.h"
#include "fai/filepackage.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#include "walletdb.h"
#endif

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include <bits/basic_string.h>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader_template.h"
#include "fai/content.h"
#include "utilstrencodings.h"
#include "fai/settings.h"

using namespace boost;
using namespace boost::assign;
using namespace json_spirit;
using namespace std;

/**
 * @note Do not add or change anything in the information returned by this
 * method. `getinfo` exists for backwards-compatibility only. It combines
 * information from wildly different sources in the program, which is a mess,
 * and is thus planned to be deprecated eventually.
 *
 * Based on the source of the information, new information should be added to:
 * - `getblockchaininfo`,
 * - `getnetworkinfo` or
 * - `getwalletinfo`
 *
 * Or alternatively, create a specific query method for the information.
 **/
Value getinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getinfo\n"
            "Returns an object containing various state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,           (numeric) the server version\n"
            "  \"protocolversion\": xxxxx,   (numeric) the protocol version\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total faicoin balance of the wallet\n"
            "  \"blocks\": xxxxxx,           (numeric) the current number of blocks processed in the server\n"
            "  \"timeoffset\": xxxxx,        (numeric) the time offset\n"
            "  \"connections\": xxxxx,       (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",     (string, optional) the proxy used by the server\n"
            "  \"difficulty\": xxxxxx,       (numeric) the current difficulty\n"
            "  \"testnet\": true|false,      (boolean) if the server is using testnet or not\n"
            "  \"keypoololdest\": xxxxxx,    (numeric) the timestamp (seconds since GMT epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,        (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "  \"paytxfee\": x.xxxx,         (numeric) the transaction fee set in ltc/kb\n"
            "  \"relayfee\": x.xxxx,         (numeric) minimum relay fee for non-free transactions in ltc/kb\n"
            "  \"errors\": \"...\"           (string) any error messages\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getinfo", "")
            + HelpExampleRpc("getinfo", "")
            );

    proxyType proxy;
    GetProxy(NET_IPV4, proxy);

    Object obj;
    obj.push_back(Pair("version", CLIENT_VERSION));
    obj.push_back(Pair("protocolversion", PROTOCOL_VERSION));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
        obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
        obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetBalance())));
    }
#endif
    obj.push_back(Pair("blocks",        (int)chainActive.Height()));
    obj.push_back(Pair("timeoffset",    GetTimeOffset()));
    obj.push_back(Pair("connections",   (int)vNodes.size()));
    obj.push_back(Pair("proxy",         (proxy.IsValid() ? proxy.ToStringIPPort() : string())));
    obj.push_back(Pair("difficulty",    (double)GetDifficulty()));
    obj.push_back(Pair("testnet",       Params().TestnetToBeDeprecatedFieldRPC()));
#ifdef ENABLE_WALLET
    if (pwalletMain) {
    //    obj.push_back(Pair("keypoololdest", pwalletMain->GetOldestKeyPoolTime()));
    //    obj.push_back(Pair("keypoolsize",   (int)pwalletMain->GetKeyPoolSize()));
    }
    //if (pwalletMain && pwalletMain->IsCrypted())
    //    obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    obj.push_back(Pair("paytxfeePerKB",      ValueFromAmount(payTxFee.GetFeePerK())));
#endif
    obj.push_back(Pair("relayfeePerKB",      ValueFromAmount(::minRelayTxFee.GetFeePerK())));
    obj.push_back(Pair("errors",        GetWarnings("statusbar")));
    return obj;
}

#ifdef ENABLE_WALLET
class DescribeAddressVisitor : public boost::static_visitor<Object>
{
private:
    isminetype mine;

public:
    DescribeAddressVisitor(isminetype mineIn) : mine(mineIn) {}

    Object operator()(const CNoDestination &dest) const { return Object(); }

    Object operator()(const CPubKey &keyID) const {
        Object obj;
        //CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
        if (mine == ISMINE_SPENDABLE) {
            //pwalletMain->GetPubKey(keyID, vchPubKey);
            obj.push_back(Pair("pubkey", HexStr(keyID)));
            obj.push_back(Pair("iscompressed", keyID.IsCompressed()));
        }
        return obj;
    }
    Object operator()(const CScript &script) const {
        Object obj;
        //CPubKey vchPubKey;
        obj.push_back(Pair("isscript", false));
//        if (mine == ISMINE_SPENDABLE) {
//            //pwalletMain->GetPubKey(keyID, vchPubKey);
//            obj.push_back(Pair("pubkey", HexStr(keyID)));
//            obj.push_back(Pair("iscompressed", keyID.IsCompressed()));
//        }
        return obj;
    }

    Object operator()(const CScriptID &scriptID) const {
        Object obj;
        obj.push_back(Pair("isscript", true));
        if (mine != ISMINE_NO) {
            CScript subscript;
            pwalletMain->GetCScript(scriptID, subscript);
            std::vector<CTxDestination> addresses;
            txnouttype whichType;
            unsigned int wRequired;
            ExtractDestinations(subscript, whichType, addresses, wRequired);
            obj.push_back(Pair("script", GetTxnOutputType(whichType)));
            obj.push_back(Pair("hex", HexStr(subscript.begin(), subscript.end())));
            Array a;
            BOOST_FOREACH(const CTxDestination& addr, addresses)
            a.push_back(CBitcoinAddress(addr).ToString());
            obj.push_back(Pair("addresses", a));
            if (whichType == TX_MULTISIG)
                obj.push_back(Pair("sigsweightrequired", (uint64_t)wRequired));
        }
        return obj;
    }
};
#endif

Value validateaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "validateaddress \"faicoinaddress\"\n"
            "\nReturn information about the given faicoin address.\n"
            "\nArguments:\n"
            "1. \"faicoinaddress\"     (string, required) The faicoin address to validate\n"
            "\nResult:\n"
            "{\n"
            "  \"isvalid\" : true|false,         (boolean) If the address is valid or not. If not, this is the only property returned.\n"
            "  \"address\" : \"faicoinaddress\", (string) The faicoin address validated\n"
            "  \"ismine\" : true|false,          (boolean) If the address is yours or not\n"
            "  \"isscript\" : true|false,        (boolean) If the key is a script\n"
            "  \"pubkey\" : \"publickeyhex\",    (string) The hex value of the raw public key\n"
            "  \"iscompressed\" : true|false,    (boolean) If the address is compressed\n"
            "  \"account\" : \"account\"         (string) The account associated with the address, \"\" is the default account\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("validateaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"")
            + HelpExampleRpc("validateaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"")
            );

    CBitcoinAddress address(params[0].get_str());
    bool isValid = address.IsValid();

    Object ret;
    ret.push_back(Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        string currentAddress = address.ToString();
        ret.push_back(Pair("address", currentAddress));
#ifdef ENABLE_WALLET
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, dest) : ISMINE_NO;
        ret.push_back(Pair("ismine", (mine & ISMINE_SPENDABLE) ? true : false));
        if (mine != ISMINE_NO) {
            ret.push_back(Pair("iswatchonly", (mine & ISMINE_WATCH_ONLY) ? true: false));
            Object detail = boost::apply_visitor(DescribeAddressVisitor(mine), dest);
            ret.insert(ret.end(), detail.begin(), detail.end());
        }
        if (pwalletMain && pwalletMain->mapAddressBook.count(dest))
            ret.push_back(Pair("account", pwalletMain->mapAddressBook[dest].name));
#endif
    }
    return ret;
}

/**
 * Used by addmultisigaddress / createmultisig:
 */
CScript _createmultisig_redeemScript(const Array& params)
{
    unsigned int nRequired = (unsigned int)params[0].get_int();
    if (nRequired < 1)
        throw runtime_error("a multisignature address must require at least weight one to redeem");
    Object sendTo = params[1].get_obj();
    vector<CTxDestination> setDest;
    vector<unsigned int> setWeight;
    BOOST_FOREACH(const Pair& s, sendTo) {
        CBitcoinAddress address(s.name_);
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Faicoin address: ")+s.name_);

        //        if (setDest. .count(address))
        //            throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+s.name_);
        setDest.push_back(address.Get());
        setWeight.push_back((unsigned int)s.value_.get_int());

    }
    CScript result = GetScriptForMultisigByWeight(nRequired, setDest, setWeight);

    if (result.size() > MAX_SCRIPT_ELEMENT_SIZE)
        throw runtime_error(
            strprintf("redeemScript exceeds size limit: %d > %d", result.size(), MAX_SCRIPT_ELEMENT_SIZE));

    return result;
}

Value createmultisig(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 2)
    {
        string msg = "createmultisig nrequired [\"key\",...]\n"
                "\nCreates a multi-signature address with n signature of m keys required.\n"
                "It returns a json object with the address and redeemScript.\n"

                "\nArguments:\n"
                "1. nrequired      (numeric, required) The number of required signatures out of the n keys or addresses.\n"
                "2. \"keys\"       (string, required) A json array of keys which are faicoin addresses or hex-encoded public keys\n"
                "     [\n"
                "       \"key\"    (string) faicoin address or hex-encoded public key\n"
                "       ,...\n"
                "     ]\n"

                "\nResult:\n"
                "{\n"
                "  \"address\":\"multisigaddress\",  (string) The value of the new multisig address.\n"
                "  \"redeemScript\":\"script\"       (string) The string value of the hex-encoded redemption script.\n"
                "}\n"

                "\nExamples:\n"
                "\nCreate a multisig address from 2 addresses\n"
                + HelpExampleCli("createmultisig", "2 \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("createmultisig", "2, \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"")
                ;
        throw runtime_error(msg);
    }

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig_redeemScript(params);
    CScriptID innerID(inner);
    CBitcoinAddress saddress(inner);
    CBitcoinAddress shaddress(innerID);

    Object result;
    result.push_back(Pair("scriptAddress", saddress.ToString()));
    result.push_back(Pair("scriptHashAddress", shaddress.ToString()));
    result.push_back(Pair("redeemScript", HexStr(inner.begin(), inner.end())));

    return result;
}

Value verifymessage(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error(
            "verifymessage \"faicoinaddress\" \"signature\" \"message\"\n"
            "\nVerify a signed message\n"
            "\nArguments:\n"
            "1. \"faicoinaddress\"  (string, required) The faicoin address to use for the signature.\n"
            "2. \"signature\"       (string, required) The signature provided by the signer in base 64 encoding (see signmessage).\n"
            "3. \"message\"         (string, required) The message that was signed.\n"
            "\nResult:\n"
            "true|false   (boolean) If the signature is verified or not.\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("verifymessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", \"signature\", \"my message\"")
            );

    string strAddress  = params[0].get_str();
    string strSign     = params[1].get_str();
    string strMessage  = params[2].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CPubKey keyID;
    if(!addr.GetKey(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey == keyID);
}

Value setmocktime(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "setmocktime timestamp\n"
            "\nSet the local time to given timestamp (-regtest only)\n"
            "\nArguments:\n"
            "1. timestamp  (integer, required) Unix seconds-since-epoch timestamp\n"
            "   Pass 0 to go back to using the system time."
            );

    if (!Params().MineBlocksOnDemand())
        throw runtime_error("setmocktime for regression testing (-regtest mode) only");

    RPCTypeCheck(params, boost::assign::list_of(int_type));
    SetMockTime(params[0].get_int64());

    return Value::null;
}

CPaymentOrder ParseJsonPaymentRequest(const json_spirit::Value paymentRequestJson,const int nType){    
    CPaymentOrder pr;
    pr.fIsValid=false;    
    std::string strError;
    if(paymentRequestJson.type()!=obj_type){        
        strError="input is not object type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    Object obj=paymentRequestJson.get_obj();
    Value valtmp;
    valtmp=find_value(obj, "ids");
    bool fHasIDs=false;    
    if(valtmp.type()==str_type){ 
        LogPrintf("rpcmist processpayment id %s\n",valtmp.get_str());
        CScript scriptPubKey;        
        if(!StringToScriptPubKey(valtmp.get_str(),scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        LogPrintf("rpcmist processpayment script %s\n",scriptPubKey.ToString());
        pr.vFrom.push_back(scriptPubKey);
        fHasIDs=true;
    }else if (valtmp.type()==array_type){
        Array adds=valtmp.get_array();
        for(unsigned int i=0;i<adds.size();i++){
            if(adds[i].type()!=str_type){
                strError="id is not string type";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            LogPrintf("rpcmist processpayment id %s\n",adds[i].get_str());
            CScript scriptPubKey;
            if(!StringToScriptPubKey(adds[i].get_str(),scriptPubKey)){
                strError="id is not valid fromat";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            LogPrintf("rpcmist processpayment script %s\n",scriptPubKey.ToString());
            pr.vFrom.push_back(scriptPubKey);
            fHasIDs=true;
        }
    }
    if(!fHasIDs){
        strError="no input ids";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    valtmp=find_value(obj, "vout");
    if(valtmp.type()!=array_type){  
        strError="vout is not array type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    Array arr=valtmp.get_array();
    if(arr.size()==0){  
        strError="vout is empty";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    for(unsigned int i=0;i<arr.size();i++){
        
        if(arr[i].type()!=obj_type){
            strError="out is not object type";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        
        Object objvout=arr[i].get_obj();
        valtmp=find_value(objvout, "id");
        if(valtmp.type()!=str_type){
            strError="out id is not string type";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }        
        
        CScript scriptPubKey;
        if(!StringToScriptPubKey(valtmp.get_str(),scriptPubKey)){
                strError="out id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        LogPrintf("rpcmist processpayment vout scriptpubkey:%s\n",scriptPubKey.ToString());
        valtmp=find_value(objvout, "amount");
        CAmount amount;
        try{
            amount=AmountFromValue(valtmp);
        }
        catch (Object& objError)
        {
        strError="out amount is not valid fromat";        
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        
        CContent ctt=CContent("");
        valtmp=find_value(objvout, "content");
        if(valtmp.type()==array_type){               
            
            if(!ctt.SetJson(valtmp.get_array()))
                throw JSONRPCError(RPC_INVALID_PARAMETER, "wrong content format");
            //Object objContent=valtmp.get_obj();
//            valtmp=find_value(objContent, "type");
//            if (valtmp.type()!=str_type){
//                strError="content type is not valid format";
//                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
//            }
//            if(valtmp.get_str()=="file"){
//                valtmp=find_value(objContent, "value");
//                if (valtmp.type()!=str_type){
//                    strError="content value is not valid format";
//                    throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
//                }    
//                std::string str = valtmp.get_str();
//                if(!FileExists(str)){
//                    strError="content file not found";
//                    throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
//                }
//                ctt = _create_file_content(str);
//            }
        }
        else if (valtmp.type()==str_type){
            std::string str = valtmp.get_str();
            ctt = _create_text_content(str);
        }
        valtmp=find_value(objvout, "locktime");
        uint32_t nLockTime=0;
        switch (valtmp.type()){
            case int_type:
                nLockTime=(unsigned int)valtmp.get_uint64();
                break;
            case null_type:
                break;
            default:
                strError="lock time is not int type or null";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }  
        pr.vout.push_back(CTxOut(amount,scriptPubKey,ctt,nLockTime));
    }
    //nSigType=129;
      
    valtmp=find_value(obj, "feerate");
    switch (valtmp.type()){
        case real_type:
        {
             pr.dFeeRate=valtmp.get_real();
            break;
        }
        case int_type:
            pr.dFeeRate=valtmp.get_int();
            break;
        case str_type:
        {
            pr.dFeeRate=atof(valtmp.get_str().c_str());
            break;
        }
        case null_type:
        {
            break;
        }
        default:
        {
            strError="fee rate format error";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
    }      
    if (pr.dFeeRate<1000)
        {
            strError="fee rate is lower than limit";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }        
    pr.nRequestType=nType;
    pr.fIsValid=true;
    return pr;
    //valtmp=find_value(obj, "vins");
}
CPaymentOrder MessageRequestToPaymentRequest(const std::string idLocal,const std::string idForeign,const CContent msg, const double feeRate )
{
    CPaymentOrder pr;
    pr.fIsValid=false;    
    std::string strError;
    CScript scriptPubKey;        
    if(!StringToScriptPubKey(idLocal,scriptPubKey)){
            strError="id is not valid fromat";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    LogPrintf("rpcmist MessageRequestToPaymentRequest script %s\n",scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey); 
    if(!StringToScriptPubKey(idForeign,scriptPubKey)){
            strError="out id is not valid format";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    LogPrintf("rpcmist MessageRequestToPaymentRequest vout scriptpubkey:%s\n",scriptPubKey.ToString());        
    CAmount amount=0;
    pr.vout.push_back(CTxOut(amount,scriptPubKey,msg));   
    pr.dFeeRate=feeRate;
    pr.fIsValid=true;
    return pr;
    //valtmp=find_value(obj, "vins");
}

CPaymentOrder GetPublisherPaymentRequest(const std::string idLocal, const std::string idTarget, const std::vector<CContent>& ctts, const double feeRate, const CAmount deposit, const uint32_t nLockTime ) // locktime is only used for the content vout, change vout will not be locked
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    std::string strError;
    CScript scriptPubKey;
    if (!StringToScriptPubKey(idLocal, scriptPubKey)) {
        strError = "id is not valid format";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    LogPrintf("rpcmist MessageRequestToPaymentRequest script %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);

    if (idTarget == "")
        scriptPubKey = CScript();
    else {
        CBitcoinAddress address = CBitcoinAddress(idTarget);
        if (address.IsValid())
            scriptPubKey = GetScriptForDestination(address.Get());
        else {
            strError = "out id is not valid format";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
    }
    LogPrintf("rpcmist MessageRequestToPaymentRequest vout scriptpubkey:%s\n", scriptPubKey.ToString());
    CAmount amount = deposit > 0 ? deposit : 0;
    for (std::vector<CContent>::const_iterator it = ctts.begin(); it != ctts.end(); it++)
        pr.vout.push_back(CTxOut(amount, scriptPubKey, *it, nLockTime));
    pr.fIsValid = true;
    pr.dFeeRate = feeRate;
    pr.nRequestType = PR_PUBLISH;
    return pr;
}

//Value createsimplepr(const json_spirit::Array& params, bool fHelp)
//{
//    if (fHelp || params.size() < 1 || params.size() > 3)
//        throw runtime_error("Wrong number of parameters");
//    Object r;
//    std::string frIds = params[0].get_str();
//    std::string toIds = params[1].get_str();
//    std::vector<unsigned char> raw  = ParseHexV(params[2], "parameter 3");
//    CContent ctt(raw);
//    CPaymentOrder pr = GetPublisherPaymentRequest(frIds, toIds, ctt);
//    r.push_back(Pair("paymentRequest", EncodeHexTx(pr)));
//    return r;
//}
CPaymentOrder GetRegisterDomainPaymentRequest(const string id, const std::string domain, const uint32_t nLockTime,const double dFeeRate)
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    std::string strError;   
     CScript scriptPubKey;        
        if(!StringToScriptPubKey(id,scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
    
    LogPrintf("rpcmist GetRegisterDomainPaymentRequest script %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);    
    CContent ctt;
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_DOMAIN,domain));
    vcc.push_back(make_pair(CC_DOMAIN_REG,""));
    ctt.EncodeP(CC_DOMAIN_P,vcc);
    CAmount amount = GetDomainGroup(domain)*COIN;
    if(domain.find("/")!=domain.npos||amount==0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid domain name");
    pr.vout.push_back(CTxOut(amount, scriptPubKey, ctt,nLockTime));
    if(GetBlocksToMaturity(nLockTime)<480)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid nLocktime");    
    pr.nRequestType=PR_DOMAIN_REGISTER;
    pr.info["domain"]=domain;
    pr.fIsValid = true;
    pr.dFeeRate=dFeeRate;
    return pr;
}
CPaymentOrder GetPublishProductPaymentRequest(const Array arr)
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    std::string strError; 
    if ( arr.size() <2)
    {
        strError="parameters count is not 2";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    if(arr[0].type()!=str_type)
    {
        strError="parameter 0 is not string type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }  
    
    CScript scriptPubKey;        
    if(!StringToScriptPubKey(arr[0].get_str(),scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    
    LogPrintf("rpcmisc GetPublishProductPaymentRequest from ID %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);    
    
    if(arr[1].type()!=array_type)
    {
        strError="product data is not array type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    Array arrProducts=arr[1].get_array();
    if(arrProducts.size()==0)
    {
        strError="product data is empty";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    if(arr.size()>2)
    {
        if(arr[2].type()==int_type)
        pr.dFeeRate=(double)arr[2].get_int();
        if(arr[2].type()==real_type)
        pr.dFeeRate=arr[2].get_real();
    }
    for(unsigned int i=0;i<arrProducts.size();i++)
    {
        Object obj=arrProducts[i].get_obj();
        std::vector<std::pair<int,string> > vcc;
        Value tmp = find_value(obj, "id");
        if (tmp.type() != str_type)
        {            
            strError="invalid product id";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }     
        LogPrintf("rpcmisc GetPublishProductPaymentRequest product id %s\n", tmp.get_str());
        vcc.push_back(make_pair(CC_PRODUCT_ID,tmp.get_str()));
        
        tmp = find_value(obj, "name");
        if (tmp.type() != str_type)
        {            
            strError="invalid product name";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }     
        vcc.push_back(make_pair(CC_NAME,tmp.get_str()));
        LogPrintf("rpcmisc GetPublishProductPaymentRequest product name %s\n", tmp.get_str());
        tmp = find_value(obj, "price");
        CAmount price;
        try{
            price=AmountFromValue(tmp);
        }
        catch (Object& objError)
        {
            strError="price is not valid fromat";        
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        CContent cPrice;
        cPrice.WriteVarInt(price);
        vcc.push_back(make_pair(CC_PRODUCT_PRICE,cPrice));
        LogPrintf("rpcmisc GetPublishProductPaymentRequest product price %i\n",price);
        tmp = find_value(obj, "shipmentfee");
        if (tmp.type() != null_type)
        {            
           CAmount shipmentfee;
            try{
                shipmentfee=AmountFromValue(tmp);
            }
            catch (Object& objError)
            {
                strError="shipment fee is not valid fromat";        
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            CContent cshipmentfee;
            cshipmentfee.WriteVarInt(shipmentfee);
            vcc.push_back(make_pair(CC_PRODUCT_SHIPMENTFEE,cshipmentfee));
            LogPrintf("rpcmisc GetPublishProductPaymentRequest product shipment fee %i\n",shipmentfee);
        }            
        tmp = find_value(obj, "recipient");
        if (tmp.type() != null_type)
        {  
            if(tmp.type()!=str_type)
            {
                strError="recipient is not string type";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            CScript recipient;        
            if(!StringToScriptPubKey(tmp.get_str(),recipient)){
                strError="scriptPubKey is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            vcc.push_back(make_pair(CC_PRODUCT_PAYTO,string(recipient.begin(),recipient.end())));
            LogPrintf("rpcmisc GetPublishProductPaymentRequest recipient %s\n", recipient.ToString());
        } 
        tmp = find_value(obj, "icon");
        if (tmp.type() != null_type)
        {                        
            CLink link;
            if(tmp.type() != str_type||!link.SetString(tmp.get_str()))
            {
                strError="invalid product icon";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            vcc.push_back(make_pair(CC_PRODUCT_ICON,link.Serialize()));     
            LogPrintf("rpcmisc GetPublishProductPaymentRequest product icon %s\n", tmp.get_str());
        }     
        tmp = find_value(obj, "intro");
        if (tmp.type() != null_type)
        {            
            if(tmp.type() != str_type)
            {
                strError="intro is not str type";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            vcc.push_back(make_pair(CC_PRODUCT_INTRO,tmp.get_str()));
            LogPrintf("rpcmisc GetPublishProductPaymentRequest product intro %s\n", tmp.get_str());
        }     
        tmp = find_value(obj, "expiretime");
        uint32_t nLockTime=0;
        if (tmp.type() != null_type)
        {            
            if(tmp.type() != int_type)
            {
                strError="expiretime is not int type";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            nLockTime=tmp.get_int64();
            if(nLockTime!=0&&GetBlocksToMaturity(nLockTime)<480)
                throw JSONRPCError(RPC_INVALID_PARAMETER, "invalid expiretime");
            LogPrintf("rpcmisc GetPublishProductPaymentRequest locktime %i\n", nLockTime);
        }
        tmp = find_value(obj, "tags");        
        int nTags=0;
        if (tmp.type() != null_type)
        {            
            if(tmp.type() != array_type)
            {
                strError="tags is not array type";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
            }
            Array arrTags=tmp.get_array();
            for(unsigned int j=0;j<arrTags.size();j++)
            {
                if(arrTags[j].type()!=str_type)
                {
                    strError="tag is not str type";
                    throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
                }
                if(arrTags[j].get_str()!="")
                {
                    vcc.push_back(make_pair(CC_TAG,arrTags[j].get_str()));
                    if(nLockTime>0)
                        nTags++;
                    LogPrintf("rpcmisc GetPublishProductPaymentRequest product tag %s\n", arrTags[j].get_str());
                }
            }
        }
        
        CContent ctt;
        ctt.EncodeP(CC_PRODUCT_P,vcc);    
        if(nLockTime>0)
            pr.vout.push_back(CTxOut(nTags*COIN, nTags==0?CScript():scriptPubKey, ctt,nLockTime));
        else
            pr.vout.push_back(CTxOut(0, CScript(), ctt,0));
    }
    pr.nRequestType=PR_PUBLISH;    
    //pr.info["domain"]=arr[1].get_str();
    pr.fIsValid = true;
    return pr;
}
CPaymentOrder GetUpdateDomainPaymentRequest(const Array arr)
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    bool fValid=true;
    std::string strError;   
     CScript scriptPubKey;        
        if(!StringToScriptPubKey(arr[0].get_str(),scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
    
    LogPrintf("rpcmisc GetUpdateDomainPaymentRequest script %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);    
    
    if(arr.size()>3)
    {
        if (arr[3].type() == int_type)              
        {

            pr.dFeeRate=(double)arr[3].get_int();
        } else if(arr[3].type() == real_type)
        {
             pr.dFeeRate=arr[3].get_real();
        }
    }
    //std::vector<std::pair<int,string> > vcInfo;
    //std::vector<std::pair<int,string> > vcForward;
    Object obj=arr[2].get_obj();
    CContent cForward;
    CContent cInfo;   
    CContent cTransfer;   
    CLinkUni link; 
    Value tmp = find_value(obj, "forward");
    if (tmp.type() == str_type) {
        link.SetString(tmp.get_str());
        cForward.EncodeUnit(link.linkType,"");
        switch ((int)link.linkType)
        {
            case CC_LINK_TYPE_SCRIPTPUBKEY:
            {
                if(link.scriptPubKey.size()>64)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "forward longer than 64 bytes");
                cForward.EncodeUnit(CC_LINK,string(scriptPubKey.begin(),scriptPubKey.end()));
                tmp = find_value(obj, "forwardsig");
                if (tmp.type() != str_type) 
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "forward sig not provided for scriptpubkey");
                bool fInvalid = false;
                vector<unsigned char> vchSig = DecodeBase64(tmp.get_str().c_str(), &fInvalid);
                if (fInvalid)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "forward sig is not base64 encoded");
                string strSig;
                strSig.assign(vchSig.begin(),vchSig.end());
                cForward.EncodeUnit(CC_SIGNATURE,strSig);
                break;
            }
            case CC_LINK_TYPE_BLOCKCHAIN:
            {
                 CLink link1(link.nHeight, link.nTx, link.nVout);
                 cForward.EncodeUnit(CC_LINK,link1.Serialize());
            }
                 break;
            case CC_LINK_TYPE_TXIDOUT:
            {
                string str(link.txid.begin(), link.txid.end());
                EncodeVarInt(link.nVout, str);
                cForward.EncodeUnit(CC_LINK,str);
                //LogPrintf("GetUpdateDomainPaymentRequest link:%s \n",link.Serialize());
            }
                break;
            default:
                if(link.strLink.size()>64)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "forward longer than 64 bytes");
                cForward.EncodeUnit(CC_LINK,link.strLink);
        }
    }
    tmp = find_value(obj, "transfer");
    if (tmp.type() == str_type) {    
        CScript scriptPubKey2;         
        if(StringToScriptPubKey(tmp.get_str(),scriptPubKey2))        
            cTransfer=string(scriptPubKey2.begin(),scriptPubKey2.end());
    }
    tmp = find_value(obj, "alias");
    if (tmp.type() == str_type&&tmp.get_str().size()<=64)     
            cInfo.EncodeUnit(CC_DOMAIN_INFO_ALIAS,tmp.get_str());
            //vcInfo.push_back(make_pair(CC_DOMAIN_INFO_ALIAS,str)); 
    tmp = find_value(obj, "intro");
    if (tmp.type() == str_type&&tmp.get_str().size()<=128)         
        cInfo.EncodeUnit(CC_DOMAIN_INFO_INTRO,tmp.get_str());
            //vcInfo.push_back(make_pair(CC_DOMAIN_INFO_INTRO,str));  
    tmp = find_value(obj, "icon");
    CLink link2;
    if (tmp.type() == str_type&&link2.SetString(tmp.get_str()))              
    {
            LogPrintf("domain info request link nheight %i,ntx %i,nvout %i\n",link.nHeight,link.nTx,link.nVout);
            string str=link2.Serialize();
            LogPrintf("domain info request link %s\n",HexStr(str.begin(),str.end()));
            //vcInfo.push_back(make_pair(CC_DOMAIN_INFO_ICON,strlink));
            cInfo.EncodeUnit(CC_DOMAIN_INFO_ICON,link2.Serialize());
    }  
    tmp = find_value(obj, "tags");
    if (tmp.type() == array_type)              
    {
        Array arrTags=tmp.get_array();
        CContent cTag;
        for(unsigned int i=0;i<arrTags.size();i++)  
        {
            if(arrTags[i].get_str().size()<=32)
                cInfo.EncodeUnit(CC_TAG,arrTags[i].get_str());
            LogPrintf("domain info request tag %s\n",arrTags[i].get_str());
        }
    } 
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_DOMAIN,arr[1].get_str()));
    if(cForward.size()>0)
        vcc.push_back(make_pair(CC_DOMAIN_FORWARD_P,cForward));
    if(cInfo.size()>0)
        vcc.push_back(make_pair(CC_DOMAIN_INFO_P,cInfo));
    if(cTransfer.size()>0)
        vcc.push_back(make_pair(CC_DOMAIN_TRANSFER,cTransfer));
    if(vcc.size()==1)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "no update info provided");
    CContent ctt;
    ctt.EncodeP(CC_DOMAIN_P,vcc);    
    pr.vout.push_back(CTxOut(0, CScript(), ctt));
    pr.nRequestType=PR_DOMAIN_UPDATE;
    pr.info["domain"]=arr[1].get_str();
    pr.fIsValid = fValid;
    return pr;
}
CPaymentOrder GetBuyProductPaymentRequest(const Array arr)
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    std::string strError; 
    if ( arr.size() <2)
    {
        strError="parameters count is not 2";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    if(arr[0].type()!=str_type)
    {
        strError="parameter 0 is not string type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }  
    
    CScript scriptPubKey;        
    if(!StringToScriptPubKey(arr[0].get_str(),scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    
    LogPrintf("rpcmisc GetPublishProductPaymentRequest from ID %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);    
    
    if(arr[1].type()!=array_type)
    {
        strError="product data is not array type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    Array arrProducts=arr[1].get_array();
    if(arr.size()>2)
    {
        if(arr[2].type()==int_type)
        pr.dFeeRate=(double)arr[2].get_int();
        if(arr[2].type()==real_type)
        pr.dFeeRate=arr[2].get_real();
    }
    if(arrProducts.size()==0)
    {
        strError="product data is empty";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    //CAmount nTotal;
    //std::vector<std::pair<int,string> > vcc;
    for(unsigned int i=0;i<arrProducts.size();i++)
    {
        CPayment payment;
        if(!payment.SetJson(arrProducts[i].get_obj(),strError))
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        if(!payment.IsValid())
        {
            strError="product wrong json format";
            throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        }
        //CContent content=payment.ToContent();
        pr.vout.push_back(CTxOut(payment.GetTotalValue(), payment.recipient, payment.ToContent(),0));
        //vcc.push_back(make_pair(CC_PAYMENT_ITEM_P,content));
    }   
        //CContent ctt;
        //ctt.EncodeP(CC_PAYMENT_P,vcc);    
        
       // pr.vout.push_back(CTxOut(0, CScript(), ctt,0));
    
    pr.nRequestType=PR_SHOP_BUY;    
    //pr.info["domain"]=arr[1].get_str();
    pr.fIsValid = true;
    return pr;
}

CPaymentOrder GetPublishPackagetPaymentRequest(const Array arr)
{
    CPaymentOrder pr;
    pr.fIsValid = false;
    std::string strError; 
    if ( arr.size() <2)
    {
        strError="parameters count is not 2";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    if(arr[0].type()!=str_type)
    {
        strError="parameter 0 is not string type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }  
    
    CScript scriptPubKey;        
    if(!StringToScriptPubKey(arr[0].get_str(),scriptPubKey)){
                strError="id is not valid format";
                throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    
    LogPrintf("rpcmisc GetPublishPackagetPaymentRequest from ID %s\n", scriptPubKey.ToString());
    pr.vFrom.push_back(scriptPubKey);    
    CAmount lockValue=0;
    pr.dFeeRate=1000;
    if(arr.size()>2)
    {
        if(arr[2].type()==int_type)
            pr.dFeeRate=arr[2].get_int();
        if(arr[2].type()==real_type)
            pr.dFeeRate=arr[2].get_real();
    }
    LogPrintf("rpcmisc GetPublishPackagetPaymentRequest feerate %f\n", pr.dFeeRate);
    if(arr.size()>3)
    {
        lockValue=AmountFromValue(arr[3]);
    }
    uint32_t nLockTime=0;
    if(arr.size()>4)
    {
        nLockTime=(uint32_t)arr[4].get_int64();
    }
    Value valfp;
    if(arr[1].type()==str_type)
    {
        LogPrintf("publish package:%s \n",arr[1].get_str());
        json_spirit::read_string(arr[1].get_str(),valfp);
    }
    else
        valfp=arr[1];
    if(valfp.type()!=obj_type)    
    {
        LogPrintf("rpcmisc GetPublishPackagetPaymentRequest pakcage data is not obj type %i\n", arr[1].type()); 
        strError="pakcage data is not obj type";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    CFilePackage fp;
    if(!fp.SetJson(valfp))
    {
        strError="pakcage data is not valid";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
    }
    if(!fp.IsValid())
    {
        strError="pakcage has invalid link";
        throw JSONRPCError(RPC_INVALID_PARAMETER, strError);
        
    }
        //CContent content=payment.ToContent();
        pr.vout.push_back(CTxOut(lockValue, scriptPubKey, fp.ToContent(),nLockTime));
        //vcc.push_back(make_pair(CC_PAYMENT_ITEM_P,content));
       
        //CContent ctt;
        //ctt.EncodeP(CC_PAYMENT_P,vcc);    
        
       // pr.vout.push_back(CTxOut(0, CScript(), ctt,0));
    
    pr.nRequestType=PR_PUBLISH;    
    //pr.info["domain"]=arr[1].get_str();
    pr.fIsValid = true;
    return pr;
}
//extern CPaymentOrder GetRenewPaymentRequest(const Array arr)
//{
//    CPaymentOrder pr;
//    return pr;
//}
//extern CPaymentOrder GetTransferPaymentRequest(const Array arr)
//{
//    CPaymentOrder pr;
//    return pr;
//}
Value getsettings(const Array& params, bool fHelp)
{    
    return settings.ToJson();
}
Value updatesettings(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 3)
        throw runtime_error("updatesetting, 3params");
     RPCTypeCheck(params, boost::assign::list_of(str_type));    
    return settings.ChangeSetting(params[0].get_str(),params[1].get_str(),params[2].get_str());        
}