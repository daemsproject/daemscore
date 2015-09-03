// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcclient.h"

#include "rpcprotocol.h"
#include "util.h"
#include "ui_interface.h"

#include <set>
#include <stdint.h>

using namespace std;
using namespace json_spirit;

class CRPCConvertParam
{
public:
    std::string methodName;            //! method whose params want conversion
    int paramIdx;                      //! 0-based idx of param to convert
};

static const CRPCConvertParam vRPCConvertParams[] =
{
    { "stop", 0 },
    { "setmocktime", 0 },
    { "getaddednodeinfo", 0 },
    { "setgenerate", 0 },
    { "setgenerate", 1 },
    { "setgenerate", 3 },
    { "getnetworkhashps", 0 },
    { "getnetworkhashps", 1 },
    { "sendtoaddress", 1 },
    { "settxfee", 0 },
    { "getreceivedbyaddress", 1 },
    { "getreceivedbyaccount", 1 },
//    { "listreceivedbyaddress", 0 },
//    { "listreceivedbyaddress", 1 },
//    { "listreceivedbyaddress", 2 },
//    { "listreceivedbyaccount", 0 },
//    { "listreceivedbyaccount", 1 },
//    { "listreceivedbyaccount", 2 },
    { "getbalance", 0 },
 //   { "getbalance", 2 },
    { "getblockhash", 0 },
    { "getblockbyheight", 0 },
    { "move", 2 },
    { "move", 3 },
    { "sendfrom", 2 },
    { "sendfrom", 3 },
    { "listtransactions", 1 },
    { "listtransactions", 2 },
    { "listtransactions", 3 },
    { "listaccounts", 0 },
    { "listaccounts", 1 },
    { "walletpassphrase", 1 },
    { "getblocktemplate", 0 },
    { "listsinceblock", 1 },
    { "listsinceblock", 2 },
    { "sendmany", 1 },
    { "sendmany", 2 },
    { "addmultisigaddress", 0 },
    { "addmultisigaddress", 1 },
    { "createmultisig", 0 },
    { "createmultisig", 1 },
    { "listunspent", 0 },
    { "listunspent", 1 },
    { "listunspent", 2 },
    { "listunspent2", 0 },
    { "listunspent2", 1 },
    { "listunspent2", 2 },
    { "listunspent2", 3 },
    { "getblock", 1 },
    { "gettransaction", 1 },
    { "getrawtransaction", 1 },
    { "createrawtransaction", 0 },
    { "createrawtransaction", 1 },
    { "signrawtransaction", 1 },
    { "signrawtransaction", 2 },
    { "sendrawtransaction", 1 },
    { "gettxout", 1 },
    { "gettxout", 2 },
//    { "lockunspent", 0 },
//    { "lockunspent", 1 },
    { "importprivkey", 2 },
    { "importaddress", 2 },
    { "verifychain", 0 },
    { "verifychain", 1 },
    { "keypoolrefill", 0 },
    { "getrawmempool", 0 },
    { "estimatefee", 0 },
    { "estimatepriority", 0 },
    { "prioritisetransaction", 1 },
    { "prioritisetransaction", 2 },
    
    { "getlink", 1 },
    { "getcontents", 0 },
    { "getcontentsbyaddresses", 0 },
    { "getpromotedcontents", 0 },
    { "getpromotedcontents", 1 },
    { "getpromotedcontents", 2 },
    { "getpromotedcontents", 3 },
    { "getsalesrecord", 0 },
    { "getsalesrecord", 1 },
    { "getsalesrecord", 2 },
    { "getpurchaserecord", 0 },
    { "getpurchaserecord", 1 },
    { "getpurchaserecord", 2 },
    { "getfirstncc", 1 },
    { "getcontentbylink", 1 },
    { "getcontentbystring", 1 },
    { "encodecontentunit", 2 },
    { "setfollow", 0 },
    { "setunfollow", 0 },
    { "poolmine", 0 },
    { "poolmine", 2 },
    { "poolmine", 3 },
    { "poolmine", 4 },
    { "poolmine", 5 },
    { "mhash", 1 },
     { "getblockbyheight", 1 },
    { "getfeerate", 0 },
     { "getextkey", 2 },
    { "getextpubkey", 2 },
    { "encodevarint", 0 },
    { "getdomaininfo", 0 },
    { "getdomainsbyowner", 0 },
    { "getdomainsbyforward", 0 },
    { "getdomainsbytags", 0 },
    { "getdomainsbytags", 1 },
    { "getdomainsbytags", 2 },
    { "getdomainsbyalias", 0 },
    { "getdomainsexpiring", 0 },
    { "getidbystep", 0 }, 
};

class CRPCConvertTable
{
private:
    std::set<std::pair<std::string, int> > members;

public:
    CRPCConvertTable();

    bool convert(const std::string& method, int idx) {
        return (members.count(std::make_pair(method, idx)) > 0);
    }
};

CRPCConvertTable::CRPCConvertTable()
{
    const unsigned int n_elem =
        (sizeof(vRPCConvertParams) / sizeof(vRPCConvertParams[0]));

    for (unsigned int i = 0; i < n_elem; i++) {
        members.insert(std::make_pair(vRPCConvertParams[i].methodName,
                                      vRPCConvertParams[i].paramIdx));
    }
}

static CRPCConvertTable rpcCvtTable;

/** Convert strings to command-specific RPC representation */
Array RPCConvertValues(const std::string &strMethod, const std::vector<std::string> &strParams)
{
    Array params;

    for (unsigned int idx = 0; idx < strParams.size(); idx++) {
        const std::string& strVal = strParams[idx];

        // insert string value directly
        if (!rpcCvtTable.convert(strMethod, idx)) {
            params.push_back(strVal);
        }

        // parse string as JSON, insert bool/number/object/etc. value
        else {
            Value jVal;
            if (!read_string(strVal, jVal))
                throw runtime_error(string("Error parsing JSON:")+strVal);
            params.push_back(jVal);
        }
    }

    return params;
}

