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

    if (params.size() > 0) {

        int64_t nLockTime = params[0].get_int();
        if (nLockTime != 0) {
            if (nLockTime < LOCKTIME_THRESHOLD) {
                blocks = max(0, (int) ((int) nLockTime + 1 - (int) chainActive.Height()));
                time = blocks * Params().TargetSpacing();
            } else {
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
        throw runtime_error("");

    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    return EncodeBase32(raw);
}

Value encodebase32check(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("");

    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    return EncodeBase32Check(raw);
}

Value decodebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 1)
        throw runtime_error("");
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
        throw runtime_error("");
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
        throw runtime_error("");
    std::string b32 = params[0].get_str();
    std::vector<unsigned char> raw;
    if (DecodeBase32(b32, raw))
        return EncodeBase32(raw);
    else
        throw JSONRPCError(RPC_MISC_ERROR, "Decoding failed");
}

Value comparebase32(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 2 || params.size() < 1)
        throw runtime_error("");
    std::string s1 = params[0].get_str();
    std::string s2 = params[1].get_str();
    return CompareBase32(s1,s2);
}
