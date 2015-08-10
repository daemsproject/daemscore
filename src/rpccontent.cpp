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
#include "ccc/filepackage.h"

#include "ccc/domain.h"
#include "txdb.h"
#include <stdint.h>
#include "ccc/contentutil.h"
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

static const int INPUT_CONTENT_FORMAT_HEX = 0;
static const int INPUT_CONTENT_FORMAT_BIN = 1;
static const int INPUT_CONTENT_FORMAT_B64 = 2;

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
bool GetBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex);

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

//CContent _create_file_content(std::string str)
//{
//    Array fileArray;
//    Object fileObj1;
//    Object fileObj2;
//    fileObj1.push_back(Pair("cc_name", "CC_FILE_NAME"));
//    fileObj1.push_back(Pair("content", basename(str.c_str())));
//    fileObj2.push_back(Pair("cc_name", "CC_FILE"));
//    std::string fileStr;
//    FileToString(str, fileStr);
//    fileObj2.push_back(Pair("content", fileStr));
//    fileArray.push_back(fileObj1);
//    fileArray.push_back(fileObj2);
//
//    Array cttArray;
//    Object cttObj;
//    cttObj.push_back(Pair("cc_name", "CC_FILE_P"));
//    cttObj.push_back(Pair("content", fileArray));
//    cttArray.push_back(cttObj);
//    CContent ctt(cttArray);
//    return ctt;
//}

CContent _create_content(const Array& params)
{
    std::string str = params[0].get_str();
    CContent ctt = _create_text_content(str); //FileExists(str) ? _create_file_content(str) :
    return ctt;
}

Value createcontent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
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

Value encodecontentunit(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
    {
        string msg = "";
        throw runtime_error(msg);
    }
    int fInputFormat = INPUT_CONTENT_FORMAT_HEX;
    if (params.size() == 3)
        fInputFormat = params[2].get_int();
    std::string cttStr;
    switch (fInputFormat)
    {
        case INPUT_CONTENT_FORMAT_HEX:
        {
            std::vector<unsigned char> raw = params.size() > 1 ? ParseHexV(params[1], "parameter 2") : std::vector<unsigned char>();
            cttStr.assign(raw.begin(), raw.end());
            break;
        }
        case INPUT_CONTENT_FORMAT_B64:
        {
            std::string b64 = params.size() > 1 ? params[1].get_str() : "";
            cttStr = DecodeBase64(b64);
            break;
        }
        case INPUT_CONTENT_FORMAT_BIN:
        {
            cttStr = params.size() > 1 ? params[1].get_str() : "";
            break;
        }
        default:
            throw JSONRPCError(RPC_CONTENT_ERROR, "Input format not supported");
    }
    CContent ctt;
    std::string ccname = params[0].get_str();
    if (!ctt.SetUnit(ccname, cttStr))
        throw JSONRPCError(RPC_CONTENT_ERROR, "Error creating content unit");
    Object result;
    result.push_back(Pair("hex", HexStr(ctt)));
    result.push_back(Pair("human_string", ctt.ToHumanString()));

    return result;
}

Value decodecontentunit(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        string msg = "";
        throw runtime_error(msg);
    }
    int fInputFormat = INPUT_CONTENT_FORMAT_HEX;
    std::string cttStr;
    if (params.size() == 2)
        fInputFormat = params[1].get_int();
    switch (fInputFormat)
    {
        case INPUT_CONTENT_FORMAT_HEX:
        {
            std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
            cttStr.assign(raw.begin(), raw.end());
            break;
        }
        case INPUT_CONTENT_FORMAT_B64:
        {
            std::string b64 = params[1].get_str();
            cttStr = DecodeBase64(b64);
            break;
        }
        case INPUT_CONTENT_FORMAT_BIN:
        default:
            throw JSONRPCError(RPC_CONTENT_ERROR, "Input format not supported");
    }
    CContent ctt(cttStr);
    Object result;
    result.push_back(Pair("json", ctt.ToJson(STR_FORMAT_HEX, false)));
    result.push_back(Pair("string", ctt.ToHumanString()));
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
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        string msg = "decodecontent \"content string\""
                + HelpExampleCli("decodecontent", "0513040e5468697320697320612074657374890200") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("decodecontent", "0513040e5468697320697320612074657374890200");
        throw runtime_error(msg);
    }

    Object result = _decode_content(params);
    return result;
}

Value getlinkbytxidout(const Array& params, bool fHelp) // TO DO: Help msg
{
    if (fHelp || params.size() > 2)
        throw runtime_error("");
    Object result;
    CLink clink;
    if (IsHex(params[0].get_str()))
    { // input is txid
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
        if (mi != mapBlockIndex.end() && (*mi).second)
        {
            CBlockIndex* pindex = (*mi).second;
            clink.SetInt(pindex->nHeight, nTx, nVout);

        } else
            throw JSONRPCError(RPC_LINK_ERROR, "Fail to get link");
    } else
    { // input is link
        throw JSONRPCError(RPC_LINK_ERROR, "Fail to get link");
    }
    result.push_back(Pair("link", clink.ToString(LINK_FORMAT_DEC)));
    result.push_back(Pair("linkHex", clink.ToString(LINK_FORMAT_HEX)));
    result.push_back(Pair("linkB32", clink.ToString(LINK_FORMAT_B32)));
    return result;
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

Object _output_content(const CContent& cttIn, const int& cformat, const unsigned char& cttf, const CLink& clinkIn, const Array& posters, const CAmount nValue, const CScript& scriptPubKey)
{
    CLink clink = clinkIn;
    CContent ctt = cttIn;
    Object r;
    if (cttf & CONTENT_SIMPLE_FORMAT)
    {
        r.push_back(Pair(clink.ToString(), (int) ctt.size()));
        return r;
    }
    if (cttf & CONTENT_SHOW_LINK)
        r.push_back(Pair("link", clink.ToString()));

    if (cttf & CONTENT_SHOW_POSTER)
        {
        r.push_back(Pair("poster", posters));
        }
    if (cttf & CONTENT_SHOW_VALUE)
        r.push_back(Pair("satoshi", nValue));
    if (cttf & CONTENT_SHOW_ADDR)
    {
        if (scriptPubKey.size() == 0)
            r.push_back(Pair("addr", ""));
        else
        {
            CBitcoinAddress addr(scriptPubKey);
            r.push_back(Pair("addr", addr.ToString()));
        }
    }
    switch (cformat)
    {
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
            r.push_back(Pair("content", (int) ctt.size()));
    }
    return r;
}

Array _get_posters(const CTransaction& tx)
{
    Array posters;

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
        unsigned int wRequired;
        if (!ExtractDestinations(prevTx.vout[in.prevout.n].scriptPubKey, type, raddresses, wRequired))
            continue;
        CDomain domain;
        if (pDomainDBView->GetDomainByForward(prevTx.vout[in.prevout.n].scriptPubKey, domain, true))
        {
            Object obj;
            obj.push_back(Pair("domain", domain.strDomain));
            posters.push_back(obj);
        }

        BOOST_FOREACH(const CTxDestination & raddr, raddresses)
        {
            CBitcoinAddress addr(raddr);
            Object obj;
            obj.push_back(Pair("id", addr.ToString()));
            posters.push_back(obj);
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
    if (!GetBlockByHeight(clink.nHeight, block, pblockindex))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
    CTransaction tx;
    if (!GetTxFromBlock(block, clink.nTx, tx))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get tx failed");
    CContent content;
    if (!GetContentFromVout(tx, clink.nVout, content))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Get content failed");
    int cformat = (params.size() == 2) ? params[1].get_int() : CONTENT_FORMAT_STR_HEX;
    Array posters = _get_posters(tx);
    unsigned char cflag = CONTENT_SHOW_LINK | CONTENT_SHOW_POSTER | CONTENT_SHOW_VALUE | CONTENT_SHOW_ADDR;
    Object r = _output_content(content, cformat, cflag, clink, posters, tx.vout[clink.nVout].nValue, tx.vout[clink.nVout].scriptPubKey);
    return r;
}

Value getcontentbystring(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error("Wrong number of parameters");
    CContent content;
    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    if (!content.SetString(raw))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Set content failed");
    int cformat = (params.size() == 2) ? params[1].get_int() : CONTENT_FORMAT_STR_HEX;
    unsigned char cflag = CONTENT_SHOW_ADDR;
    CLink clink;
    Array posters;
    CAmount nValue = 0;
    CScript scriptPubKey;

    Object r = _output_content(content, cformat, cflag, clink, posters, nValue, scriptPubKey);
    return r;
}

Value getfirstncc(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error("Wrong number of parameters");
    CContent content;
    std::vector<unsigned char> raw = ParseHexV(params[0], "parameter 1");
    if (!content.SetString(raw))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Set content failed");
    std::vector<cctype> ccv;
    bool countOverN = false;
    int n = params.size() == 2 ? params[1].get_int() : STANDARD_CONTENT_MAX_CC;
    content.FirstNCc(ccv, countOverN, n);
    Object r;
    Array cnarr;

    BOOST_FOREACH(const cctype& cc, ccv)
    {
        cnarr.push_back(GetCcName(cc));
    }
    r.push_back(Pair("cc", cnarr));
    r.push_back(Pair("countOverN", countOverN));
    return r;
}

bool _parse_getcontents_params(const Array& params, int& fbh, int& maxc, int& maxb, int& blkc, Array& withcc, Array& withoutcc, Array& firstcc, int& fContentFormat, unsigned char& cflag, int& mincsize, Array& frAddrs, bool& fAsc)
{
    if (params.size() > 3)
        return false;
    if (params.size() == 0)
    {
        fbh = 0;
        maxc = DEFAULT_MAX_CONTENTS;
        maxb = DEFAULT_MAX_CONTENTSBYTES;
        return true;
    }
    Object param = params[0].get_obj();
    const Value& fbh_v = find_value(param, "fbh");
    try
    {
        fbh = fbh_v.get_int();
    } catch (std::exception& e)
    {
        fbh = 0;
    }
    const Value& maxc_v = find_value(param, "maxc");
    try
    {
        maxc = maxc_v.get_int();
    } catch (std::exception& e)
    {
        maxc = DEFAULT_MAX_CONTENTS;
    }
    const Value& maxb_v = find_value(param, "maxb");
    try
    {
        maxb = maxb_v.get_int();
    } catch (std::exception& e)
    {
        maxb = DEFAULT_MAX_CONTENTSBYTES;
    }
    const Value& blkc_v = find_value(param, "blkc");
    try
    {
        blkc = blkc_v.get_int();
    } catch (std::exception& e)
    {
        blkc = DEFAULT_MAX_BLKCOUNT;
    }
    const Value& fContentFormat_v = find_value(param, "cformat");
    try
    {
        fContentFormat = fContentFormat_v.get_int();
    } catch (std::exception& e)
    {
        fContentFormat = CONTENT_FORMAT_JSON_B64;
    }

    // cflag
    cttflag fShowLink;
    const Value& fShowLink_v = find_value(param, "fShowLink");
    try
    {
        fShowLink = fShowLink_v.get_bool() ? CONTENT_SHOW_LINK : CONTENT_FALSE;
    } catch (std::exception& e)
    {
        fShowLink = CONTENT_SHOW_LINK;
    }
    cttflag fShowPoster;
    const Value& fShowPoster_v = find_value(param, "fShowPoster");
    try
    {
        fShowPoster = fShowPoster_v.get_bool() ? CONTENT_SHOW_POSTER : CONTENT_FALSE;
    } catch (std::exception& e)
    {
        fShowPoster = CONTENT_SHOW_POSTER;
    }
    cttflag fShowValue;
    const Value& fShowValue_v = find_value(param, "fShowValue");
    try
    {
        fShowValue = fShowValue_v.get_bool() ? CONTENT_SHOW_VALUE : CONTENT_FALSE;
    } catch (std::exception& e)
    {
        fShowValue = CONTENT_SHOW_VALUE;
    }
    cttflag fShowAddr;
    const Value& fShowAddr_v = find_value(param, "fShowAddr");
    try
    {
        fShowAddr = fShowAddr_v.get_bool() ? CONTENT_SHOW_ADDR : CONTENT_FALSE;
    } catch (std::exception& e)
    {
        fShowAddr = CONTENT_SHOW_ADDR;
    }
    cttflag fSimpleFormat;
    const Value& fSimpleFormat_v = find_value(param, "fSimpleFormat");
    try
    {
        fSimpleFormat = fSimpleFormat_v.get_bool() ? CONTENT_SIMPLE_FORMAT : CONTENT_FALSE;
    } catch (std::exception& e)
    {
        fSimpleFormat = CONTENT_FALSE;
    }
    cflag = fShowLink | fShowPoster | fShowValue | fShowAddr | fSimpleFormat;

    const Value& mincsize_v = find_value(param, "mincsize");
    try
    {
        mincsize = mincsize_v.get_int();
    } catch (std::exception& e)
    {
        mincsize = 1;
    }
    const Value& fAsc_v = find_value(param, "fAsc");
    try
    {
        fAsc = fAsc_v.get_bool();
    } catch (std::exception& e)
    {
        fAsc = false;
    }
    const Value& withcc_v = find_value(param, "withcc");
    try
    {
        withcc = withcc_v.get_array();
    } catch (std::exception& e)
    {
    }
    const Value& withoutcc_v = find_value(param, "withoutcc");
    try
    {
        withoutcc = withoutcc_v.get_array();
    } catch (std::exception& e)
    {
    }
    const Value& firstcc_v = find_value(param, "firstcc");
    try
    {
        firstcc = firstcc_v.get_array();
    } catch (std::exception& e)
    {
    }
    if (params.size() > 1)
        frAddrs = params[1].get_array();
    return true;
}

// true if have one in firstcc array, and one in withcc array, and none in withoutcc array

bool _check_cc(const CContent& ctt, const Array& withcc, const Array& withoutcc, const Array& firstcc)
{
    if (withcc.size() == 0 && withoutcc.size() == 0 && firstcc.size() == 0)
        return true;
    CContent cttcopy = ctt;

    bool rf = firstcc.size() == 0 ? true : false;

    BOOST_FOREACH(const Value& ccName_v, firstcc)
    {
        cctype cc = GetCcValue(ccName_v.get_str());
        if (cttcopy.FirstCc(cc))
        {
            rf = true;
            break;
        }
    }
    if (!rf)
        return false;

    bool rw = withcc.size() == 0 ? true : false;

    BOOST_FOREACH(const Value& ccName_v, withcc)
    {
        cctype cc = GetCcValue(ccName_v.get_str());
        if (cttcopy.HasCc(cc))
        {
            rw = true;
            break;
        }
    }
    if (!rw)
            return false;

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
    int fbh = chainActive.Height();
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
    bool fAsc; // get the block by ascending or descending sequence
    if (!_parse_getcontents_params(params, fbh, maxc, maxb, blkc, withcc, withoutcc, firstcc, cformat, cflag, minsz, gAddrs, fAsc))
        throw runtime_error("Error parsing parameters");
    Array r;
    int c = 0;
    int b = 0;
    int nHeight = fbh;
    int totalM = fAsc ? chainActive.Height() - fbh + 1 : std::min(blkc, fbh);
        int total = totalM > blkc ? blkc : totalM;
    if (gAddrs.size() == 0)
    {
        for (int i = 0; i < total; i++)
        {
            CBlockIndex* pblockindex;
            CBlock block;
            if (!GetBlockByHeight(nHeight, block, pblockindex))
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
            int nTx = 0;

            BOOST_FOREACH(const CTransaction& tx, block.vtx)
            {
                int nVout = 0;
                Array posters;
                if (cflag & CONTENT_SHOW_POSTER)
                    posters = _get_posters(tx);

                BOOST_FOREACH(const CTxOut& out, tx.vout)
                {
                    if (c >= maxc)
                        return r;
                    if ((int) out.strContent.size() >= minsz)
                    {
                        b += out.strContent.size();
                        if (c > maxc || b > maxb)
                            return r;
                        CContent ctt(out.strContent);
                        if (_check_cc(ctt, withcc, withoutcc, firstcc))
                        {
                            CLink clink(nHeight, nTx, nVout);
                            Object cttr = _output_content(ctt, cformat, cflag, clink, posters, out.nValue, out.scriptPubKey);
                            r.push_back(cttr);
                            c++;
                        } else
                        {
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
    } else
    {
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
        for (std::vector<std::pair<CTransaction, uint256> >::iterator it = vTxs.begin(); it != vTxs.end(); ++it)
        {
            BlockMap::iterator mi = mapBlockIndex.find(it->second);
            if (mi != mapBlockIndex.end() && (*mi).second)
            {
                Array posters;
                if (cflag & CONTENT_SHOW_POSTER)
                    posters = _get_posters(it->first);
                for (int i = 0; i < (int) it->first.vout.size(); i++)
                {
                    if (c >= maxc)
                        return r;
                    CBlockIndex* pindex = (*mi).second;
                    if ((fAsc ? pindex->nHeight >= fbh : pindex->nHeight <= fbh) && std::abs(pindex->nHeight - fbh) <= total)
                    { // make sure the tx is in block range
                        int nTx = GetNTx(it->first.GetHash()); // Very inefficient
                        CLink clink(pindex->nHeight, nTx, i);
                        CContent ctt;
                        if (GetContentFromVout(it->first, i, ctt))
                        {
                            if ((int) ctt.size() >= minsz)
                            {
                                b += ctt.size();
                                if (c > maxc || b > maxb)
                                    return r;
                                if (_check_cc(ctt, withcc, withoutcc, firstcc))
                                {
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

Value getmessages(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected array");
    Array arrIDs = params[0].get_array();
    std::vector<CScript> vIDsLocal;
    std::vector<CScript> vIDsForeign;
    int nDirectionFilter = 0;
    bool fLinkOnly = false;
    bool fIncludeMempool = true;
    int nOffset = 0;
    int nCount = 10000;
    for (unsigned int i = 0; i < arrIDs.size(); i++)
    {
        CScript script;
        if (arrIDs[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected str");
        //LogPrintf("getmessages idlocal:%s \n", arrIDs[i].get_str());
        StringToScriptPubKey(arrIDs[i].get_str(), script);
        vIDsLocal.push_back(script);
    }
    if (params.size() == 2)
    {

        Object options = params[1].get_obj();
        Value tmp;
        tmp = find_value(options, "IDsForeign");
        if (tmp.type() != null_type)
        {

            Array arrIDs2 = tmp.get_array();
            for (unsigned int i = 0; i < arrIDs2.size(); i++)
            {
                CScript script;
                StringToScriptPubKey(arrIDs[i].get_str(), script);
                vIDsForeign.push_back(script);
            }
        }
        tmp = find_value(options, "directionFilter");
        if (tmp.type() != null_type)
        {

            nDirectionFilter = tmp.get_int();
        }
        tmp = find_value(options, "fLinkOnly");
        if (tmp.type() != null_type)
        {

            fLinkOnly = tmp.get_bool();
        }
        tmp = find_value(options, "fIncludeMempool");
        if (tmp.type() != null_type)
        {

            fIncludeMempool = tmp.get_bool();
        }
        tmp = find_value(options, "nOffset");
        if (tmp.type() != null_type)
        {

            nOffset = tmp.get_int();
        }
        if (nOffset < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative offset");
        tmp = find_value(options, "nCount");
        if (tmp.type() != null_type)
        {

            nCount = tmp.get_int();
        }
        if (nCount < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");

    }
    std::vector<CTransaction> vMemTx;
    std::vector<CTxPosItem> vTxPos;
    GetDiskTxPoses(vIDsLocal, vTxPos);
    if (fIncludeMempool)
        mempool.GetUnconfirmedTransactions(vIDsLocal, vMemTx);
    //LogPrintf("getmessages mempooltxs:%u \n", vMemTx.size());
    //LogPrintf("getmessages blockchaintxs:%u \n", vTxPos.size());
    if (vIDsForeign.size() > 0)
    {
        std::vector<CTxPosItem> vTxPosForeign;
        GetDiskTxPoses(vIDsForeign, vTxPosForeign);
        for (std::vector<CTxPosItem>::iterator it = vTxPos.begin(); it != vTxPos.end(); it++) {
            if (find(vTxPosForeign.begin(), vTxPosForeign.end(), *it) == vTxPosForeign.end())
                vTxPos.erase(it);
        }
        if (fIncludeMempool)
        {
            std::vector<CTransaction> vMemTxForeign;
            mempool.GetUnconfirmedTransactions(vIDsForeign, vMemTxForeign);

            for (std::vector<CTransaction>::iterator it = vMemTx.begin(); it != vMemTx.end(); it++)
            {
                if (find(vMemTxForeign.begin(), vMemTxForeign.end(), *it) == vMemTxForeign.end())
                    vMemTx.erase(it);
            }
            //LogPrintf("getmessages filtered mempooltxs:%u \n", vMemTx.size());
        }
    }
    Array arrMsg;
    std::vector<CMessage> vMessages;
    int nPos = 0;
    if (fIncludeMempool)
    {
        int nTime = GetTime();

        BOOST_FOREACH(const CTransaction& tx, vMemTx)
        {
            vMessages.empty();
            GetMessagesFromTx(vMessages, tx, -1, -1, nTime, vIDsLocal, vIDsForeign, nDirectionFilter, fLinkOnly, nPos, nOffset, nCount);
            BOOST_FOREACH(CMessage msg, vMessages)
            arrMsg.push_back(msg.ToJson(fLinkOnly));
        }
    }
    vMessages.clear();
    for (int i = vTxPos.size() - 1; i >= 0; i--)
    {
        if (nPos >= nOffset + nCount)
            break;
        if((((vTxPos[i].nFlags&1<<TXITEMFLAG_SENDCONTENT)==0)&&((vTxPos[i].nFlags&1<<TXITEMFLAG_RECEIVECONTENT)==0))||
                (nDirectionFilter == OUTPUT_ONLY&&((vTxPos[i].nFlags&1<<TXITEMFLAG_SENDCONTENT)==0))||
                (nDirectionFilter == INCOMING_ONLY&&((vTxPos[i].nFlags&1<<TXITEMFLAG_RECEIVECONTENT)==0)))
                continue;
        CTransaction tx;
        uint256 hashBlock;
        int nHeight = -1;
        int nTime = GetTime();
        int nTx = -1;
        //LogPrintf("getmessages txpos   file:%i,pos:%u,txpos:%i\n",vTxPos[i].nFile,vTxPos[i].nPos,vTxPos[i].nTxOffset);
        if (GetTransaction(vTxPos[i], tx)) 
        {
            pBlockPosDB->GetByPos(vTxPos[i].nFile,vTxPos[i].nPos,hashBlock,nHeight);
            BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
            if (mi != mapBlockIndex.end())
            {
                const CBlockIndex* pindex = (*mi).second;
                if (pindex && chainActive.Contains(pindex))
                {
                    nHeight = pindex->nBlockHeight;
                    nTime = pindex->nTime;
                }
            }
            //LogPrintf("getmessages txpos i:%i \n",i);
            //vMessages.empty(); 
            //LogPrintf("getmessages: tx vin size:%u \n",tx.vin.size());
            //LogPrintf("getmessages: tx vout size:%u \n",tx.vout.size());
            //LogPrintf("getmessages: tx hash:%s \n",tx.GetHash().GetHex());
            GetMessagesFromTx(vMessages, tx, nHeight, nTx, nTime, vIDsLocal, vIDsForeign, nDirectionFilter, fLinkOnly, nPos, nOffset, nCount);


        }
    }
    //LogPrintf("getmessages:%i \n",vMessages.size());
    SortMessages(vMessages, vIDsLocal);
    //LogPrintf("getmessages sorted:%i\n",vMessages.size());
    BOOST_FOREACH(CMessage& msg, vMessages)
    arrMsg.push_back(msg.ToJson(fLinkOnly));
    LogPrintf("getmessages:%i \n", arrMsg.size());
    return Value(arrMsg);

}

Value gettxmessages(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter ids, expected array");
    Array arrIDs = params[0].get_array();
    if (params[1].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter txs, expected array");
    Array arrTxs = params[1].get_array();
    std::vector<CScript> vIDsLocal;
    std::vector<CScript> vIDsForeign;
    int nDirectionFilter = 0;
    bool fLinkOnly = false;

    for (unsigned int i = 0; i < arrIDs.size(); i++)
    {
        if (arrIDs[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected str");
        CScript script;
        //LogPrintf("gettxmessages idlocal:%s \n", arrIDs[i].get_str());
        StringToScriptPubKey(arrIDs[i].get_str(), script);
        vIDsLocal.push_back(script);
    }
    Array arrMsg;
    std::vector<CMessage> vMessages;
    for (unsigned int i = 0; i < arrTxs.size(); i++)
    {
        if (arrTxs[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter txid, expected str");

        //LogPrintf("gettxmessages txid:%s \n", arrTxs[i].get_str());
        //vector<unsigned char> vch=;
        uint256 txid;
        txid.SetHex(arrTxs[i].get_str());
        CTransaction tx;
        uint256 hashBlock;
        int nTime = GetTime();
        int nHeight = -1;
        int nTx = -1;
        if (!GetTransaction(txid, tx, hashBlock, true))
            LogPrintf("gettxmessages tx not found \n");
        //LogPrintf("gettxmessages tx:%s \n",tx.ToString());
        BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end())
        {
            const CBlockIndex* pindex = (*mi).second;
            if (pindex && chainActive.Contains(pindex))
            {
                nHeight = pindex->nBlockHeight;
                nTime = pindex->nTime;
            }
        }
        vMessages.empty();
        GetMessagesFromTx(vMessages, tx, nHeight, nTx, nTime, vIDsLocal, vIDsForeign, nDirectionFilter, fLinkOnly, 0, 0, 65536);
        BOOST_FOREACH(CMessage msg, vMessages)
            arrMsg.push_back(msg.ToJson(fLinkOnly));
    }
    LogPrintf("getmessages toJson%i \n", arrMsg.size());
    return Value(arrMsg);

}

void GetMessagesFromTx(std::vector<CMessage>& vMessages, const CTransaction& tx, const int nBlockHeight, int nTx, int nTime, const std::vector<CScript>& vIDsLocal,
        const std::vector<CScript>& vIDsForeign, int nDirectionFilter, bool fLinkonly, int nPos, int nOffset, int nCount)
{
    if (tx.vin.size() == 0)
        return;
    //process vout first
    std::vector<std::pair<int, string> >vRawMsg;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        CTxOut txout = tx.vout[i];
        if (txout.strContent.size() >= 39)
        {
            std::vector<std::pair<int, string> > vContent;
            if (CContent(txout.strContent).Decode(vContent))
            {
                for (unsigned int j = 0; j < vContent.size(); j++)
                {
                    if (vContent[j].first == 0x15)
                    {
                        //LogPrintf("getmessagesFromtx:effective msg found:%s\n",vContent[j].second);
                        std::vector<std::pair<int, string> > vInnerContent;
                        if (CContent(vContent[j].second).Decode(vInnerContent))
                        {

                            bool hasIV = false;
                            bool hasContent = false;
                            for (unsigned int k = 0; k < vInnerContent.size(); k++)
                            {
                                //LogPrintf("getmessagesFromtx:effective msg found:%s\n",vInnerContent[k].second);
                                if (vInnerContent[k].first == 0x140002)
                                    hasIV = true;
                                else if (vInnerContent[k].first == 0x14)
                                    hasContent = true;
                            }
                            if (hasIV && hasContent)
                            {
                                CContent cmsg;
                                cmsg.EncodeUnit(0x15, vContent[j].second);
                                vRawMsg.push_back(make_pair(i, cmsg));
                            }
                        }
                    }
                }
            }
        }

    }
    //LogPrintf("getmessagesFromtx:effective msg:%i\n",vRawMsg.size());
    if (vRawMsg.size() == 0)
        return;
    bool fIncoming = true;
    CTransaction prevTx;
    uint256 tmphash;
    //LogPrintf("getmessagesFromtx: prevout hash:%s \n",tx.vin[0].prevout.hash.GetHex());
    //LogPrintf("getmessagesFromtx1\n");
    if (!GetTransaction(tx.vin[0].prevout.hash, prevTx, tmphash, true))
    {
        LogPrintf("getmessagesFromtx: null vin prevout\n");
        return;
    }
    ////LogPrintf("getmessagesFromtx2\n");
    CScript IDFrom = prevTx.vout[tx.vin[0].prevout.n].scriptPubKey;
    //LogPrintf("getmessagesFromtx,idfrom:%s\n", IDFrom.ToString());
    if (find(vIDsLocal.begin(), vIDsLocal.end(), IDFrom) != vIDsLocal.end())
    {
        fIncoming = false;
        //LogPrintf("getmessagesFromtx:output msg\n");
    } else if (vIDsForeign.size() > 0)
        if (find(vIDsForeign.begin(), vIDsForeign.end(), IDFrom) == vIDsForeign.end())
            return;
    //LogPrintf("getmessagesFromtx: IDFrom:%s \n",tx.vin[0].prevout.hash.GetHex());

    //LogPrintf("getmessagesFromtx5\n");
    if ((nDirectionFilter == OUTPUT_ONLY && fIncoming) || (nDirectionFilter == INCOMING_ONLY && !fIncoming))
        return;
    //LogPrintf("getmessagesFromtx msg:%i\n", vRawMsg.size());
    uint256 hash = tx.GetHash();
    for (unsigned int i = 0; i < vRawMsg.size(); i++)
    {
        CScript scriptPubKey = tx.vout[vRawMsg[i].first].scriptPubKey;
        if (fIncoming)
        {
            if (find(vIDsLocal.begin(), vIDsLocal.end(), scriptPubKey) != vIDsLocal.end())
            {
                if (nPos >= nOffset && nPos < (nOffset + nCount))
                {
                    //LogPrintf("getmessagesFromtx7\n");
                    CMessage msg;
                    msg.txid = hash;
                    msg.nVout = i;
                    msg.IDFrom = IDFrom;
                    msg.IDTo = scriptPubKey;
                    msg.content = CContent(vRawMsg[i].second);
                    msg.nBlockHeight = nBlockHeight;
                    msg.nTx = nTx;
                    msg.nTime = nTime;
                    vMessages.push_back(msg);
                    //LogPrintf("getmessagesFromtx8\n");
                }
                nPos++;
            }
        } else
        {
            if (vIDsForeign.size() > 0)
                if (find(vIDsForeign.begin(), vIDsForeign.end(), scriptPubKey) == vIDsForeign.end())
                    continue;
            if (nPos >= nOffset && nPos < (nOffset + nCount))
            {
                //LogPrintf("getmessagesFromtx9\n");
                CMessage msg;
                msg.txid = hash;
                msg.nVout = i;
                msg.IDFrom = IDFrom;
                msg.IDTo = scriptPubKey;
                msg.content = CContent(vRawMsg[i].second);
                msg.nBlockHeight = nBlockHeight;
                msg.nTx = nTx;
                msg.nTime = nTime;
                vMessages.push_back(msg);
                //LogPrintf("getmessagesFromtx10\n");
            }
            nPos++;
        }
    }
}

void SortMessages(std::vector<CMessage>& vMsg, std::vector<CScript> vIDsLocal)
{
    std::vector<CMessage> vMsgOut;
    if (vMsg.size() < 2)
        return;
    for (std::vector<CScript>::iterator it = vIDsLocal.begin(); it != vIDsLocal.end(); it++)
    {
        std::vector<CMessage> vMsgByIDLocal;
        //round1 sort by idlocal
        for (std::vector<CMessage>::iterator it1 = vMsg.begin(); it1 != vMsg.end(); it1++)
        {
            if (it1->IDFrom == *it || it1->IDTo == *it)
            {
                vMsgByIDLocal.push_back(*it1);
                //LogPrintf("SortMessages found idlocal \n");
            }
        }
        //round2 sort by idforeign
        std::map<CScript, std::vector<CMessage> >mapByIDForeign;

        for (std::vector<CMessage>::iterator it1 = vMsgByIDLocal.begin(); it1 != vMsgByIDLocal.end(); it1++)
        {
            CScript IDForeign = (it1->IDFrom == *it) ? it1->IDTo : it1->IDFrom;
            std::map<CScript, std::vector<CMessage> >::iterator it2 = mapByIDForeign.find(IDForeign);
            if (it2 == mapByIDForeign.end())
            {
                //LogPrintf("SortMessages new idForeign\n");
                std::vector<CMessage> vMsgByIDForeign;
                vMsgByIDForeign.push_back(*it1);
                mapByIDForeign[IDForeign] = vMsgByIDForeign;
            } else
            {
                it2->second.push_back(*it1);
                //LogPrintf("SortMessages old idForeign\n");
            }
        }
        // LogPrintf("SortMessages mapByIDForeign:%i\n",mapByIDForeign.size());
        //round3 sort by time
        for (std::map<CScript, std::vector<CMessage> >::iterator it1 = mapByIDForeign.begin(); it1 != mapByIDForeign.end(); it1++)
        {
            // LogPrintf("SortMessages mapByIDForeign:%s,%i\n",it1->first.ToString(),it1->second.size());
            bool fChanged = true;
            while (it1->second.size() > 1 && fChanged)
            {
                fChanged = false;
                for (std::vector<CMessage>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
                {
                    std::vector<CMessage>::iterator it3 = it2;
                    it3++;
                    if (it3 == it1->second.end())
                        break;
                    if (it3->nTime > it2->nTime)
                    {
                        CMessage tmp = *it2;
                        *it2 = *it3;
                        *it3 = tmp;
                        fChanged = true;
                        //speed up rolling
                        std::vector<CMessage>::iterator it4 = it2;
                        bool fChanged2 = true;
                        while (fChanged2 && (it4 != it1->second.begin()))
                        {
                            fChanged2 = false;
                            std::vector<CMessage>::iterator it5 = it4;
                            it5--;
                            if (it4->nTime > it5->nTime)
                            {
                                CMessage tmp = *it5;
                                *it5 = *it4;
                                *it4 = tmp;
                                fChanged2 = true;
                            }
                            it4--;
                        }
                    }
                }
            }
            //LogPrintf("SortMessages mapByIDForeign:%i\n",it1->second.size());
            vMsgOut.insert(vMsgOut.end(), it1->second.begin(), it1->second.end());
        }
    }
    // LogPrintf("SortMessages :%i\n",vMsgOut.size());
    vMsg = vMsgOut;
}

json_spirit::Value getdomaininfo(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError( RPC_INVALID_PARAMETER, "Invalid parameter arrDomainNames, expected array");
    Array arrDomainNames = params[0].get_array();  
    Array arrDomains;
    for (unsigned int i = 0; i < arrDomainNames.size(); i++)
    {
        if (arrDomainNames[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter DomainName, expected str");        
        CDomain domain;
        pDomainDBView->GetDomainByName(arrDomainNames[i].get_str(), domain);
        arrDomains.push_back(domain.ToJson());
    }    
    LogPrintf("getdomaininfo toJson%i \n", arrDomains.size());
    return Value(arrDomains);
}

json_spirit::Value getdomainsbyowner(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter ids, expected array");
    Array arrIDs = params[0].get_array(); 
    std::vector<CDomain> vDomain;
    for (unsigned int i = 0; i < arrIDs.size(); i++)
    {
        if (arrIDs[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected str");
        CScript script;        
        StringToScriptPubKey(arrIDs[i].get_str(), script);
        pDomainDBView->GetDomainByOwner(script, vDomain, true);
    }
    Array arrDomains;
   for (unsigned int i = 0; i < vDomain.size(); i++) 
   {
       arrDomains.push_back(vDomain[i].ToJson());
   }
    LogPrintf("getdomainsbyowner toJson%i \n", arrDomains.size());
    return Value(arrDomains);
}

json_spirit::Value getdomainsbyforward(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter ids, expected array");
    Array arrIDs = params[0].get_array(); 
    std::vector<CDomain> vDomain;
    for (unsigned int i = 0; i < arrIDs.size(); i++)
    {
        if (arrIDs[i].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected str");
        CScript script;        
        StringToScriptPubKey(arrIDs[i].get_str(), script);
        pDomainDBView->GetDomainByForward(script, vDomain, true);
    }
    Array arrDomains;
   for (unsigned int i = 0; i < vDomain.size(); i++) 
   {
       arrDomains.push_back(vDomain[i].ToJson());
   }
    LogPrintf("getdomainsbyoForward toJson%i \n", arrDomains.size());
    return Value(arrDomains);
}

json_spirit::Value getdomainbyforward(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != str_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected string");
    std::string id = params[0].get_str();
    CDomain domain;
    CScript script;
    StringToScriptPubKey(id, script);
    pDomainDBView->GetDomainByForward(script, domain, true);
    return Value(domain.ToJson());
}

json_spirit::Value searchproducts(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter tags, expected array");
    Array arrTag = params[0].get_array();    
    std::vector<string> vTag;
    for (unsigned int i = 0; i < arrTag.size(); i++)
    {
        if (arrTag[i].type() == str_type)
        {
            string tag = arrTag[i].get_str();
            if (tag != "" && tag.size() <= 32)
                vTag.push_back(tag);
        }
    }
    int nMaxItems = 100;
    if (params.size() > 1 && params[1].type() == int_type)
        nMaxItems = params[1].get_int();
    int nOffset = 0;
    if (params.size() > 2 && params[2].type() == int_type)
        nOffset = params[2].get_int();
    vector<CLink> vLink;
    pTagDBView->Search(vLink, vTag, CC_PRODUCT, nMaxItems, nOffset);
    LogPrintf("search tag result %i \n", vLink.size());
    Array arrProducts;
    vector<CProduct> vProduct;
    for (unsigned int i = 0; i < vLink.size(); i++) 
   {
       CBlockIndex* pblockindex;
        CBlock block;
        if (!GetBlockByHeight(vLink[i].nHeight, block, pblockindex))
            continue;
        CTransaction tx;
        if (!GetTxFromBlock(block, vLink[i].nTx, tx))
            continue;
        CContent content;
        if (!GetContentFromVout(tx, vLink[i].nVout, content))
            continue;    
       CProduct product;
       
        if (product.SetContent(content))
       {           
           LogPrintf("searchproducts secontent done \n");
            product.link = vLink[i];
            product.seller = GetTxInScriptPubKey(tx.vin[0]);
           LogPrintf("searchproducts seller %s \n", product.seller.ToString());
            bool fFound = false;
            for (unsigned int j = 0; j < vProduct.size(); j++)
           {
                if (vProduct[j].link == product.link)
               {
                    fFound = true;
                   break;
               }
                if (vProduct[j].seller == product.seller && vProduct[j].id == product.id)
                {
                    if (vProduct[j].link < product.link)
                        vProduct[j] = product;
                    fFound = true;
                   break;
               }
           }
           if (fFound)
               continue;           
            if (product.seller.size() > 0)
           {
               vector<CDomain> vDomain;
                pDomainDBView->GetDomainByForward(product.seller, vDomain, true);
                 for (unsigned int j = 0; j < vDomain.size(); j++) 
                 {
                    if (vDomain[j].strAlias.size() > 0)
                         product.vSellerDomain.push_back(vDomain[j].strAlias);
                     else
                     product.vSellerDomain.push_back(vDomain[j].strDomain);
                 }
                if (product.recipient.size() == 0)
                    product.recipient = product.seller;
           }
           vProduct.push_back(product);           
       }
           
   }
    for (unsigned int i = 0; i < vProduct.size(); i++) 
        arrProducts.push_back(vProduct[i].ToJson());   
    LogPrintf("searchproducts toJson%i \n", arrProducts.size());
    return Value(arrProducts);
}

json_spirit::Value getfilepackageurl(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != str_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link, expected str");
    CLink link;
    if (!link.UnserializeConst(params[0].get_str()))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link content");
    string url;
    if (!GetFilePackageUrl(link, url))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Linked to invalid file package");
    return Value(url);
}

json_spirit::Value encodevarint(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        string msg = "encodevarint \"int\""
                + HelpExampleCli("encodevarint", "To Do") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("encodevarint", "To Do");
        throw runtime_error(msg);
    }
    uint64_t n = params[0].get_int64();
    std::vector<unsigned char> sv;
    if (EncodeVarInt(sv, n))
        return HexStr(sv.begin(), sv.end());
    else
        return "";

}

json_spirit::Value decodevarint(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        string msg = "decodevarint \"varint hex string\""
                + HelpExampleCli("decodevarint", "To Do") +
                "\nAs a json rpc call\n"
                + HelpExampleRpc("decodevarint", "To Do");
        throw runtime_error(msg);
    }
    std::vector<unsigned char> sv = ParseHexV(params[0], "parameter 1");
    uint64_t n = 0;
    DecodeVarInt(sv, n);
    return n;
}    