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
static const int DEFAULT_MAX_BLKCOUNT = 10;

static const int CONTENT_FORMAT_SIZE = 0;
static const int CONTENT_FORMAT_STR_HEX = 1;
static const int CONTENT_FORMAT_STR_BIN = 2;
static const int CONTENT_FORMAT_STR_B64 = 3;
static const int CONTENT_FORMAT_JSON_HEX = 4;
static const int CONTENT_FORMAT_JSON_BIN = 5;
static const int CONTENT_FORMAT_JSON_B64 = 6;
static const int CONTENT_FORMAT_JSON_HEX_SUM = 7;
static const int CONTENT_FORMAT_JSON_BIN_SUM = 8;
static const int CONTENT_FORMAT_JSON_B64_SUM = 9;
static const int CONTENT_FORMAT_HUMAN_STR = 10;

enum cttflag
{
    CONTENT_FALSE = 0X00,
    CONTENT_SHOW_LINK = 0x01,
    CONTENT_SHOW_POSTER = 0x02,
    CONTENT_SHOW_VALUE = 0x04,
    CONTENT_SHOW_ADDR = 0x08,
    CONTENT_SIMPLE_FORMAT = 0x20 // clink:size
};

void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out, bool fIncludeHex);
bool _getBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex);

CContent _create_text_content(std::string str)
{
    CContent ctt;
    if (str.size() == 0)
        return ctt;
    Array textArray;
    Object textObj;
    textObj.push_back(Pair("cc_name", "CC_TEXT"));
    textObj.push_back(Pair("content", str));
    textArray.push_back(textObj);
    Array cttArray;
    Object cttObj1;
    Object cttObj2;
    cttObj1.push_back(Pair("cc_name", "CC_TEXT_P"));
    cttObj1.push_back(Pair("content", textArray));
    cttObj2.push_back(Pair("cc_name", "CC_TEXT_ENCODING_UTF8"));
    cttObj2.push_back(Pair("content", ""));
    cttArray.push_back(cttObj1);
    cttArray.push_back(cttObj2);
    ctt.SetJson(cttArray);
    return ctt;
}

CContent _create_file_content(std::string str)
{
    Array fileArray;
    Object fileObj1;
    Object fileObj2;
    fileObj1.push_back(Pair("cc_name", "CC_FILE_NAME"));
    fileObj1.push_back(Pair("content", basename(str.c_str())));
    fileObj2.push_back(Pair("cc_name", "CC_FILE_CONTENT"));
    std::string fileStr;
    FileToString(str, fileStr);
    fileObj2.push_back(Pair("content", fileStr));
    fileArray.push_back(fileObj1);
    fileArray.push_back(fileObj2);

    Array cttArray;
    Object cttObj;
    cttObj.push_back(Pair("cc_name", "CC_FILE_P"));
    cttObj.push_back(Pair("content", fileArray));
    cttArray.push_back(cttObj);
    CContent ctt(cttArray);
    return ctt;
}

CContent _create_content(const Array& params)
{
    std::string str = params[0].get_str();
    CContent ctt = FileExists(str) ? _create_file_content(str) : _create_text_content(str);
    return ctt;
}

Value createcontent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2) {
        string msg = "createcontent \"conntent string\""
                + HelpExampleCli("decodecontent", "This is a test") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("decodecontent", "This is a test");
        throw runtime_error(msg);
    }
    CContent ctt = _create_content(params);
    Object result;
    result.push_back(Pair("hex", HexStr(ctt)));
    result.push_back(Pair("human_string", ctt.ToHumanString()));

    return result;
}

Object _decode_content(const Array& params)
{
    Object error;
    std::string str = params[0].get_str();
    if (!IsHex(str))
        return error;
    vector<unsigned char> str2(ParseHex(str));
    CContent ctt(str2);
    Object jsonObj;
    jsonObj.push_back(Pair("json", ctt.ToJson()));
    jsonObj.push_back(Pair("string", ctt.ToHumanString()));
    return jsonObj;
}

Value decodecontent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2) {
        string msg = "decodecontent \"content string\""
                + HelpExampleCli("decodecontent", "0513040e5468697320697320612074657374890200") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("decodecontent", "0513040e5468697320697320612074657374890200");
        throw runtime_error(msg);
    }

    Object result = _decode_content(params);
    return result;
}

Value getcontent(const Array& params, bool fHelp) // TO DO
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
    if (nTx <= (int) block.vtx.size()) {
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
        case CONTENT_FORMAT_JSON_BIN_SUM:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_BIN_SUM)));
            break;
        case CONTENT_FORMAT_JSON_HEX_SUM:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_HEX_SUM)));
            break;
        case CONTENT_FORMAT_JSON_B64_SUM:r.push_back(Pair("content", ctt.ToJson(STR_FORMAT_B64_SUM)));
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
        txnouttype type;
        vector<CTxDestination> raddresses;
        int nRequired;
        if (!ExtractDestinations(prevTx.vout[in.prevout.n].scriptPubKey, type, raddresses, nRequired))
            continue;

        BOOST_FOREACH(const CTxDestination& raddr, raddresses)
        {
            CBitcoinAddress addr(raddr);
            posters.push_back(addr);
        }
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

bool _parse_getcontents_params(const Array& params, int& fbh, int& maxc, int& maxb, int& blkc, Array& withcc, Array& withoutcc, Array& firstcc, int& fContentFormat, unsigned char& cflag, int& mincsize, Array& addrs, bool& fAsc)
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
    const Value& blkc_v = find_value(param, "blkc");
    try {
        blkc = blkc_v.get_int();
    } catch (std::exception& e) {
        blkc = DEFAULT_MAX_BLKCOUNT;
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
    if (withcc.size() == 0 && withoutcc.size() == 0 && firstcc.size() == 0)
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
    if (!r)
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
    int blkc;
    Array withcc;
    Array withoutcc;
    Array firstcc;
    Array gAddrs;
    int cformat;
    unsigned char cflag;
    int minsz;
    bool fAsc;
    if (!_parse_getcontents_params(params, fbh, maxc, maxb, blkc, withcc, withoutcc, firstcc, cformat, cflag, minsz, gAddrs, fAsc))
        throw runtime_error("Error parsing parameters");
    Array r;
    int c = 0;
    int b = 0;
    if (gAddrs.size() == 0) {
        int nHeight = fAsc ? fbh : std::min(chainActive.Height(), fbh + blkc);
        int totalM = chainActive.Height() - fbh + 1;
        int total = totalM > blkc ? blkc : totalM;
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