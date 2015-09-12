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
#include "timedata.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#include "walletdb.h"
//#include "script/interpreter.h"
#endif

#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "fai/link.h"
#include "fai/content.h"

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
Value getmaturetime(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getmaturetime\n"
            "Returns the time and blocks to maturity for the input locktime.\n"
            "\nResult:\n"
            "{\n"
            "  \"time\": xxxxx,           (numeric) the time to maturity in seconds\n"
            "  \"blocks\": xxxxx,   (numeric) the blocks to maturity\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getmaturetime", "10000")
            + HelpExampleRpc("getmaturetime", "10000")
            );
    int64_t time = 0;
    int64_t blocks = 0;

    if (params.size() > 0)
    {

        int64_t nLockTime = (uint32_t) params[0].get_int64();
        if (nLockTime != 0)
        {
            if (nLockTime < LOCKTIME_THRESHOLD)
            {
                blocks = max(0, (int) ((int) nLockTime + 1 - (int) chainActive.Height()));
                time = blocks * Params().TargetSpacing();
            } else
            {
                time = (int) max((int64_t) 0, nLockTime - GetAdjustedTime());
                blocks = (int) (time / Params().TargetSpacing());
            }
        }
    }
    Object obj;
    obj.push_back(Pair("time", time));
    obj.push_back(Pair("blocks", blocks));
    return obj;
}

Value encodebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("encodebase32");

    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    return EncodeBase32(raw);
}

Value encodebase32check(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("encodebase32check");

    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    return EncodeBase32Check(raw);
}

Value decodebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("decodebase32");
    std::string b32 = params[0].get_str();
    std::vector<unsigned char> raw;
    if (DecodeBase32(b32, raw))
        return HexStr(raw.begin(), raw.end());
    else
        throw JSONRPCError(RPC_MISC_ERROR, "Decoding failed");
}

Value decodebase32check(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("decodebase32check");
    std::string b32 = params[0].get_str();
    std::vector<unsigned char> raw;
    if (DecodeBase32Check(b32, raw))
        return HexStr(raw.begin(), raw.end());
    else
        throw JSONRPCError(RPC_MISC_ERROR, "Decoding failed");
}

Value standardizebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("standardizebase32");
    std::string b32 = params[0].get_str();
    std::vector<unsigned char> raw;
    if (DecodeBase32(b32, raw))
        return EncodeBase32(raw);
    else
        throw JSONRPCError(RPC_MISC_ERROR, "Decoding failed");
}

Value comparebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() < 2 )
        throw runtime_error("comparebase32");
    std::string s1 = params[0].get_str();
    std::string s2 = params[1].get_str();
    return CompareBase32(s1,s2);
}
Value isvalidpubkeyaddress(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("isvalidpubkeyaddress");
    std::string b32 = params[0].get_str();
    std::vector<unsigned char> raw;
    CPubKey pub;
    if (!CBitcoinAddress(b32).GetKey(pub))
        return Value(false);
    return Value(true);
}
Value getextkey(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() !=3)
        throw runtime_error("getextkey");
    //std::vector<unsigned char> vbasepriv = ParseHexV(params[0],"basepriv");
    
    //basekey.Set(vbasepriv.begin(),vbasepriv.end(),true);
    //std::vector<unsigned char> vsteppriv = ParseHexV(params[1],"steppriv");    
    //stepkey.Set(vsteppriv.begin(),vsteppriv.end(),true);    /
    string strAccount = params[0].get_str();    
    CBitcoinSecret add;
    if (!add.SetString(strAccount))
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid basekey");   
    CKey basekey=add.GetKey();
    strAccount = params[1].get_str();    
     if (!add.SetString(strAccount))
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid stepkey");   
    CKey stepkey=add.GetKey();
    uint64_t nStep = params[2].get_int64();
    if (nStep==0)
        return Value(params[0]);
    CKey extkey;
    basekey.AddSteps(stepkey,Hash(&nStep,&nStep+1),extkey);    
    return Value(CBitcoinSecret(extkey).ToString());
}
Value getextpubkey(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() !=3)
        throw runtime_error("getextpubkey");
    CPubKey basepub = AccountFromValue(params[0]);
    CPubKey steppub = AccountFromValue(params[1]);
    uint64_t nStep = params[2].get_int64();
    CPubKey extpub;
    if (nStep==0)
        return Value(params[0]);
    basepub.AddSteps(steppub,Hash(&nStep,&nStep+1),extpub);
    
    return Value(CBitcoinAddress(extpub).ToString());
}
Value decodemultisigaddress(const json_spirit::Array& params, bool fHelp){
    if (fHelp || params.size() <1)
        throw runtime_error("decodemultisigaddress");
    CScript script;
    if(!StringToScriptPubKey(params[0].get_str(),script))
            throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid address");  
    //LogPrintf("decodemultisigaddress script:%s \n",script.ToString());
    txnouttype typeRet;
    vector<vector<unsigned char> > vSolutions;
    if (!Solver(script, typeRet, vSolutions))
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "address is not multisig"); 
    //LogPrintf("decodemultisigaddress type:%s \n",GetTxnOutputType(typeRet));
    if (typeRet != TX_MULTISIG)
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "address is not multisig");  
    Object obj;
        obj.push_back(Pair("weight required",CScriptNum(vSolutions.front(),false).getint()));
        int nTotalWeight=0;
        Array arr;
        for (unsigned int i = 1; i < vSolutions.size()-1; i+=2)
        {
            //LogPrintf("vsolutions %i:%s \n",i,HexStr(vSolutions[i].begin(),vSolutions[i].end()));
            CPubKey pubKey(vSolutions[i]);
            if (!pubKey.IsValid())
                //continue;
                throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "has invalid pubkey");  
            Object obj1;
                 obj1.push_back(Pair("id",CBitcoinAddress(pubKey).ToString()));
                 int weight=CScriptNum(vSolutions[i+1],false).getint();
                 nTotalWeight+=weight;
            obj1.push_back(Pair("weight",weight));
            arr.push_back(obj1);
        }
        obj.push_back(Pair("totalweight",nTotalWeight));
        obj.push_back(Pair("id-count",(int)arr.size()));
        obj.push_back(Pair("ids",arr));
    return Value(obj);
}
Value gethash(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() <1)
        throw runtime_error("gethash");
    string format="string";
    string formatout="hex";
    if(params.size()>1)
        format=params[1].get_str();
    if(params.size()>1)
        formatout=params[2].get_str();
    uint256 hash;
    if(format=="hex")
    {
        std::vector<unsigned char> vch = ParseHexV(params[0],"basepriv");
        hash=Hash(vch.begin(),vch.end());
    }    
    else if(format=="base64")
    {
        string str=DecodeBase64(params[0].get_str());
        hash=Hash(str.begin(),str.end());
    }
    else
    {
        string str=params[0].get_str();
        hash=Hash(str.begin(),str.end());
    }
    string result;
   if(formatout=="base64")
   {
       result= EncodeBase64((unsigned char*)hash.begin(),32);
   }
   else
       result= hash.GetHex();
    
    return Value(result);
}

Value getlinktype(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("getlinktype");
    Object result;
    CLinkUni clinkuni;
    clinkuni.SetString(params[0].get_str());
    result.push_back(Pair("linktype", GetCcName(clinkuni.linkType)));
    return result;
}