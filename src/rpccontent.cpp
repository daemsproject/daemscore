// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"
#include "main.h"
#include "rpcserver.h"
#include "timedata.h"
#include "sync.h"
#include "util.h"
#include "fai/content.h"
#include "fai/shop.h"
#include "fai/link.h"
#include "fai/filepackage.h"

#include "fai/domain.h"
#include "txdb.h"
#include <stdint.h>
#include "fai/contentutil.h"
#include "json/json_spirit_value.h"
#include "base58.h"
#include "utilstrencodings.h"

using namespace json_spirit;
using namespace std;

static const int DEFAULT_MAX_CONTENTS = 100;
static const int DEFAULT_MAX_PROMOTEDCONTENTS = 100;
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
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    result.push_back(Pair("hex", HexStr(ctt)));
    result.push_back(Pair("human_string", ctt.ToHumanString(nMaxCC)));

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
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    result.push_back(Pair("human_string", ctt.ToHumanString(nMaxCC)));

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
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    result.push_back(Pair("json", ctt.ToJson(nMaxCC,STR_FORMAT_HEX, false)));
    result.push_back(Pair("string", ctt.ToHumanString(nMaxCC)));
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
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    if(params.size()>1)
        nMaxCC=params[1].get_int();
    Object jsonObj;    
    jsonObj.push_back(Pair("json", ctt.ToJson(nMaxCC)));
    jsonObj.push_back(Pair("string", ctt.ToHumanString(nMaxCC)));
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

Object _output_content(const CContent& cttIn, const int& cformat, const unsigned char& cttf, const CLink& clinkIn, const CBitcoinAddress& posterAddress, const CDomain& domain, const CAmount nValue, const CScript& scriptPubKey,int nMaxCCIn=STANDARD_CONTENT_MAX_CC)
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
        Object poster;
        poster.push_back(Pair("id", posterAddress.ToString()));
        if (!domain.IsEmpty())
            poster.push_back(Pair("domain", domain.ToJson()));
        r.push_back(Pair("poster", poster));
        }
    if (cttf & CONTENT_SHOW_VALUE)
        r.push_back(Pair("satoshi", nValue));
    if (cttf & CONTENT_SHOW_ADDR)
    {
        if (scriptPubKey.size() == 0)
            r.push_back(Pair("addr", ""));
        else
        {            
            std::string addr;
            ScriptPubKeyToString(scriptPubKey, addr);
            r.push_back(Pair("addr", addr));
        }
    }
    int nMaxCC=nMaxCCIn;
    switch (cformat)
    {
        case CONTENT_FORMAT_STR_HEX:r.push_back(Pair("content", HexStr(ctt)));
            break;
        case CONTENT_FORMAT_STR_BIN:r.push_back(Pair("content", ctt));
            break;
        case CONTENT_FORMAT_STR_B64:r.push_back(Pair("content", EncodeBase64(ctt)));
            break;
        case CONTENT_FORMAT_JSON_HEX:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_HEX)));
            break;
        case CONTENT_FORMAT_JSON_BIN:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_BIN)));
            break;
        case CONTENT_FORMAT_JSON_B64:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_B64)));
            break;
        case CONTENT_FORMAT_JSON_BIN_SUM:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_BIN_SUM)));
            break;
        case CONTENT_FORMAT_JSON_HEX_SUM:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_HEX_SUM)));
            break;
        case CONTENT_FORMAT_JSON_B64_SUM:r.push_back(Pair("content", ctt.ToJson(nMaxCC,STR_FORMAT_B64_SUM)));
            break;
        case CONTENT_FORMAT_HUMAN_STR:r.push_back(Pair("content", ctt.ToHumanString(nMaxCC)));
            break;
        case CONTENT_FORMAT_SIZE:
        default:
            r.push_back(Pair("content", (int) ctt.size()));
    }
    return r;
}

bool _get_poster(const CTransaction& tx, string& address, CDomain& domain, int flag = 3) // return false for coinbase, flags 3 get both, 1 get id, 2 get domain
{  
    if (flag < 1 || flag > 3)
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid flag");
    if (flag & 1)
    {
        CTxIn in = tx.vin[0];
        if (in.prevout.hash.EqualTo(0))
            return false;
        CTransaction prevTx;
        uint256 tmphash;
        if (!GetTransaction(in.prevout.hash, prevTx, tmphash, true))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Get prev tx failed");
        ScriptPubKeyToString(prevTx.vout[in.prevout.n].scriptPubKey, address);
    }
    if (flag & 2)
{
        if(address.size() == 0)
            _get_poster(tx,address,domain,1);
        if(address.size() == 0)
        return false;
        CScript scriptPubKey;
        if(!StringToScriptPubKey(address, scriptPubKey))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Invalid address");
        pDomainDBView->GetDomainByForward(scriptPubKey, domain, true);
    }
    return true;

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
    string address;
    CDomain domain;
    _get_poster(tx, address, domain);
    unsigned char cflag = CONTENT_SHOW_LINK | CONTENT_SHOW_POSTER | CONTENT_SHOW_VALUE | CONTENT_SHOW_ADDR;
    Object r = _output_content(content, cformat, cflag, clink, address, domain, tx.vout[clink.nVout].nValue, tx.vout[clink.nVout].scriptPubKey);
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
    CAmount nValue = 0;
    CScript scriptPubKey;
    string address;
    CDomain domain;
    Object r = _output_content(content, cformat, cflag, clink, address, domain, nValue, scriptPubKey);
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

bool _parse_getcontents_params(const Array& params, int& fbh, int& fntx, int&fnout, int& maxc, int& maxb, int& blkc, Array& withcc, Array& withoutcc, Array& firstcc, int& fContentFormat, unsigned char& cflag, int& mincsize, Array& frAddrs, Array& toAddrs, bool& fAsc)
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
        fbh = chainActive.Height();
    }
    const Value& fntx_v = find_value(param, "fntx");
    try
    {
        fntx = fntx_v.get_int();
    } catch (std::exception& e)
    {
        fntx = 0;
    }
    const Value& fnout_v = find_value(param, "fnout");
    try
    {
        fnout = fnout_v.get_int();
    } catch (std::exception& e)
    {
        fnout = 0;
    }
    const Value& flink_v = find_value(param, "flink");
    try
    {
        CLink flink(flink_v.get_str());
        if (flink.IsValid())
        {
            fbh = flink.nHeight;
            fntx = flink.nTx;
            fnout = flink.nVout;
        }
    } catch (std::exception& e)
    {
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
    const Value& frAddrs_v = find_value(param, "frAddrs");
    try
    {
        frAddrs = frAddrs_v.get_array();
    } catch (std::exception& e)
    {
    }
    const Value& toAddrs_v = find_value(param, "toAddrs");
    try
    {
        toAddrs = toAddrs_v.get_array();
    } catch (std::exception& e)
    {
    }
    return true;
}
bool _parse_getpromotedcontents_params(const Array& params, int& maxc,int& nOffset, int& maxb, Array& firstcc,Array& arrTags, int& fContentFormat, unsigned char& cflag, Array& frAddrs)
{
    if (params.size() >1 )
        return false;
    if (params.size() == 0)
    {       
        maxc = DEFAULT_MAX_PROMOTEDCONTENTS;
        maxb = DEFAULT_MAX_CONTENTSBYTES;
        return true;
    }
    Object param = params[0].get_obj();
    const Value& maxc_v = find_value(param, "maxc");
    try
    {
        maxc = maxc_v.get_int();
    } catch (std::exception& e)
    {
        maxc = DEFAULT_MAX_PROMOTEDCONTENTS;
    }
    const Value& nOffset_v = find_value(param, "offset");
    try
    {
        nOffset = nOffset_v.get_int();
    } catch (std::exception& e)
    {
        nOffset = 0;
    }
    const Value& maxb_v = find_value(param, "maxb");
    try
    {
        maxb = maxb_v.get_int();
    } catch (std::exception& e)
    {
        maxb = DEFAULT_MAX_CONTENTSBYTES;
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
    const Value& farrTags_v = find_value(param, "tags"); 
    if(farrTags_v.type()!=null_type)
    {
        if(farrTags_v.type()!=array_type)
            return false;
        arrTags=farrTags_v.get_array();
    }
    const Value& ffirstcc_v = find_value(param, "ccs");   
    if(ffirstcc_v.type()!=null_type)
    {
        if(ffirstcc_v.type()!=array_type)
            return false;
        firstcc = ffirstcc_v.get_array();  
    }
    const Value& ffrAddrs_v = find_value(param, "ids"); 
    if(ffrAddrs_v.type()!=null_type)
    {
        if(ffrAddrs_v.type()!=array_type)
            return false;
        frAddrs = ffrAddrs_v.get_array();
    }
    return true;
}

// true if have one in firstcc array, and one in withcc array, and none in withoutcc array

bool _check_cc(const CContent& ctt, const Array& withcc, const Array& withoutcc, const Array& firstcc,int nMaxCC)
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
        if (cttcopy.HasCc(cc,nMaxCC))
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
        if (cttcopy.HasCc(cc,nMaxCC))
            return false;
    }

    return true;
}

Value getcontents(const Array& params, bool fHelp) // withcc and without cc is very costly, DONOT use in standard occasion
{
    if (fHelp)
        throw runtime_error("Help msg");
    int fbh;
    int fntx;
    int fnout;
    int maxc;
    int maxb;
    int blkc;
    int nMaxCC=STANDARD_CONTENT_MAX_CC;
    Array withcc;
    Array withoutcc;
    Array firstcc;
    Array frAddrs;
    Array toAddrs;
    int cformat;
    unsigned char cflag;
    int minsz;
    bool fAsc; // get the block by ascending or descending sequence
    if (!_parse_getcontents_params(params, fbh, fntx, fnout, maxc, maxb, blkc, withcc, withoutcc, firstcc, cformat, cflag, minsz, frAddrs, toAddrs, fAsc))
        throw runtime_error("Error parsing parameters");
    Array r;
    int c = 0;
    int b = 0;
    int nHeight = fbh;
    int totalM = fAsc ? chainActive.Height() - fbh + 1 : std::min(blkc, fbh);
        int total = totalM > blkc ? blkc : totalM;
    if (frAddrs.size() == 0 && toAddrs.size() == 0)
    {
        std::map<std::string,CDomain> posters;
        for (int i = 0; i < total; i++)
        {
            CBlockIndex* pblockindex;
            CBlock block;
            if (!GetBlockByHeight(nHeight, block, pblockindex))
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Get block failed");
            for (int nTx = i == 0 ? fntx : 0; nTx < (int) block.vtx.size(); nTx++)
            {
                const CTransaction& tx = block.vtx.at(nTx);
                string address;
                CDomain domain;
                if (cflag & CONTENT_SHOW_POSTER)
                {
                    _get_poster(tx, address, domain, 1);
                    if (posters.count(address)==0)
                    {
                        _get_poster(tx, address, domain, 2);
                         posters[address] = domain;
                    }else
                    {
                        domain = posters[address];
                    }
                }
                for (int nVout = (i == 0 && nTx == fntx) ? fnout : 0; nVout < (int) tx.vout.size(); nVout++)
                {
                    const CTxOut& out = tx.vout.at(nVout);
                    if (c >= maxc)
                        return r;
                    if ((int) out.strContent.size() >= minsz)
                    {
                        b += out.strContent.size();
                        if (c > maxc || b > maxb)
                            return r;
                        CContent ctt(out.strContent);
                        if (_check_cc(ctt, withcc, withoutcc, firstcc,nMaxCC))
                        {
                            CLink clink(nHeight, nTx, nVout);
                            Object cttr = _output_content(ctt, cformat, cflag, clink, address, domain, out.nValue, out.scriptPubKey,nMaxCC);
                            r.push_back(cttr);
                            c++;
                        } else
                        {
                            b -= out.strContent.size();
                            continue;
                        }

                    }
                }
            }
            fAsc ? nHeight++ : nHeight--;
        }
    } else
    {
        std::map<std::string,CDomain> posters;
        std::vector<CScript> vFrIds;

        BOOST_FOREACH(const Value& addrStr, frAddrs)
        {
            CBitcoinAddress address(addrStr.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
            CScript scriptPubKey = GetScriptForDestination(address.Get());
            vFrIds.push_back(scriptPubKey);
        }
        std::vector<CScript> vToIds;

        BOOST_FOREACH(const Value& addrStr, toAddrs)
        {
            CBitcoinAddress address(addrStr.get_str());
            if (!address.IsValid())
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
            CScript scriptPubKey = GetScriptForDestination(address.Get());
            vToIds.push_back(scriptPubKey);
        }
        std::vector<CTxPosItem> vTxPosFr;
        GetDiskTxPoses(vFrIds, vTxPosFr);
        std::vector<CTxPosItem> vTxPosTo;
        GetDiskTxPoses(vToIds, vTxPosTo);
        unsigned int nBeginDataPos;
        int nBeginFile;
        unsigned int nEndDataPos;
        int nEndFile;
        if(fbh>chainActive.Height())
            fbh=chainActive.Height();
        int nEndHeight;        
        if(fAsc)
        {
            nBeginFile=chainActive[fbh]->nFile;
            nBeginDataPos=chainActive[fbh]->nDataPos;
            nEndHeight=fbh+blkc;
            if(nEndHeight>=chainActive.Height())       
            {
                nEndFile=chainActive.Tip()->nFile+1;
                nEndDataPos=0;
            }
            else
            {
                nEndFile=chainActive[nEndHeight+1]->nFile;
                nEndDataPos=chainActive[nEndHeight+1]->nDataPos;
            }
        }
        else
        {            
            nEndFile=chainActive[fbh]->nFile;
            if(fbh==chainActive.Height())
                 nEndDataPos=chainActive[fbh]->nDataPos+DEFAULT_BLOCK_MAX_SIZE;
            else
                nEndDataPos=chainActive[fbh+1]->nDataPos;
            nEndHeight=fbh-blkc;
            if(nEndHeight<=0)
            {
               nBeginFile=0;
                nBeginDataPos=0; 
            }
            else
            {
                nBeginFile=chainActive[nEndHeight-1]->nFile;
                nBeginDataPos=chainActive[nEndHeight-1]->nDataPos;
            }
        }
        for (std::vector<CTxPosItem>::iterator it = vTxPosFr.begin(); it != vTxPosFr.end(); it++)
            {
            uint256 hashBlock;
            int nHeight;
            if (!(it->nFlags & 1 << TXITEMFLAG_SENDCONTENT))
                continue;
            if ((it->nFile < nBeginFile)||((it->nFile==nBeginFile)&&(it->nPos<nBeginDataPos))
                    ||(it->nFile > nEndFile)||((it->nFile==nEndFile)&&(it->nPos>nEndDataPos)))
                continue;
            pBlockPosDB->GetByPos(it->nFile, it->nPos, hashBlock, nHeight);
            if ((fAsc ? nHeight < fbh : nHeight > fbh) || std::abs(nHeight - fbh) > total)
                continue;
            CTransaction tx;

            

            if (!GetTransaction(*it, tx))
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Get transaction failed");
            int nTx = GetNTx(tx.GetHash());
            if (nHeight == fbh && nTx < fntx)
                continue;
            string address;
            CDomain domain;
                if (cflag & CONTENT_SHOW_POSTER)
            {
                _get_poster(tx, address, domain, 1);
                if (posters.count(address) == 0)
                {
                    _get_poster(tx, address, domain, 2);
                    posters[address] = domain;
                } else
                {
                    domain = posters[address];
                }
            }
            for (int nVout = (nHeight == fbh && nTx == fntx) ? fnout : 0; nVout < (int) tx.vout.size(); nVout++)
                {
                    if (c >= maxc)
                        return r;
                CLink clink(nHeight, nTx, nVout);
                        CContent ctt;
                if (!GetContentFromVout(tx, nVout, ctt))
                    continue;
                if ((int) ctt.size() < minsz)
                    continue;
                b += ctt.size();
                if (b > maxb)
                    return r;
                if (!_check_cc(ctt, withcc, withoutcc, firstcc,nMaxCC))
                {
                    b -= ctt.size();
                    continue;
                }
                    
                Object cttr = _output_content(ctt, cformat, cflag, clink, address, domain, tx.vout[nVout].nValue, tx.vout[nVout].scriptPubKey,nMaxCC);
                r.push_back(cttr);
                c++;
            }

        }
        for (std::vector<CTxPosItem>::iterator it = vTxPosTo.begin(); it != vTxPosTo.end(); it++)
                        {
            
            uint256 hashBlock;
            int nHeight;
             if (!(it->nFlags & 1 << TXITEMFLAG_RECEIVECONTENT))
                continue;
            if ((it->nFile < nBeginFile)||((it->nFile==nBeginFile)&&(it->nPos<nBeginDataPos))
                    ||(it->nFile > nEndFile)||((it->nFile==nEndFile)&&(it->nPos>nEndDataPos)))
                continue;
            pBlockPosDB->GetByPos(it->nFile, it->nPos, hashBlock, nHeight);
            if ((fAsc ? nHeight < fbh : nHeight > fbh) || std::abs(nHeight - fbh) > total)
                continue;
            CTransaction tx;

           

            if (!GetTransaction(*it, tx))
                throw JSONRPCError(RPC_INTERNAL_ERROR, "Get transaction failed");
            int nTx = GetNTx(tx.GetHash());
            if (nHeight == fbh && nTx < fntx)
                continue;
            string address;
            CDomain domain;
            if (cflag & CONTENT_SHOW_POSTER)
            {
                _get_poster(tx, address, domain, 1);
                if (posters.count(address) == 0)
                {
                    _get_poster(tx, address, domain, 2);
                    posters[address] = domain;
                } else
                {
                    domain = posters[address];
                }
            }
            for (int nVout = (nHeight == fbh && nTx == fntx) ? fnout : 0; nVout < (int) tx.vout.size(); nVout++)
                            {
                if (c >= maxc)
                    return r;
                CLink clink(nHeight, nTx, nVout);
                CContent ctt;
                if (!GetContentFromVout(tx, nVout, ctt))
                    continue;
                if ((int) ctt.size() < minsz)
                    continue;
                                b += ctt.size();
                if (b > maxb)
                                    return r;
                if (!_check_cc(ctt, withcc, withoutcc, firstcc,nMaxCC))
                 {
                    b -= ctt.size();
                    continue;
                }
                Object cttr = _output_content(ctt, cformat, cflag, clink, address, domain, tx.vout[nVout].nValue, tx.vout[nVout].scriptPubKey,nMaxCC);
                                    r.push_back(cttr);
                                    c++;
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

json_spirit::Value getpromotedcontents(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error("getpromotedcontents Help msg");    
    int maxb;    
    int nMaxResults;
    int nOffset;
    Array firstcc;
    Array gAddrs;
    Array arrTags;
    int cformat;
    unsigned char cflag;    
    if (!_parse_getpromotedcontents_params(params, nMaxResults,nOffset, maxb, firstcc,arrTags, cformat, cflag, gAddrs))
        throw runtime_error("getpromotedcontents Error parsing parameters");
    vector<CScript> vSenders;
    BOOST_FOREACH(const Value& addrStr, gAddrs)
    {
        CBitcoinAddress address(addrStr.get_str());
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
        CScript scriptPubKey = GetScriptForDestination(address.Get());
        vSenders.push_back(scriptPubKey);
    }    
    vector<int> vCCs;
    BOOST_FOREACH(const Value& cc, firstcc)
        if(GetCcValue(cc.get_str())!=0)
            vCCs.push_back(GetCcValue(cc.get_str()));
    vector<string> vTags;
    BOOST_FOREACH(const Value& tag, arrTags)
        vTags.push_back(tag.get_str());
    vector<CContentDBItem> vContents;
    Array r;    
    int b = 0;
    if(!SearchPromotedContents(vSenders,vCCs,vTags,vContents,nMaxResults,nOffset))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "SearchPromotedContents failed");      
    for (int i = 0; i < (int) vContents.size(); i++)
    {
        if ( b > maxb)
            return r;
        CContent ctt;
        CTxOut out;
        if (GetTxOutFromVoutPos(vContents[i].pos,out))
        {
            string address;
            CDomain domain;
            if (cflag & CONTENT_SHOW_POSTER)
            {
                ScriptPubKeyToString(vContents[i].sender, address);
                pDomainDBView->GetDomainByForward(vContents[i].sender, domain, true);
            }
            b += ctt.size();
            Object cttr = _output_content(CContent(out.strContent), cformat, cflag, vContents[i].link, address, domain, vContents[i].lockValue, out.scriptPubKey);
            r.push_back(cttr);
        }            
    }            
    return r;
}
json_spirit::Value getsalesrecord(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp||params.size()<1)
        throw runtime_error("getpromotedcontents Help msg");    
    
    
    std::vector<CScript> vIds;

    BOOST_FOREACH(const Value& addrStr, params[0].get_array())
    {
        CBitcoinAddress address(addrStr.get_str());
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
        CScript scriptPubKey = GetScriptForDestination(address.Get());
        vIds.push_back(scriptPubKey);
    }
    //int nDirection=0;
    //if (params.size()>1)
    //    nDirection=params[1].get_int();
    int nMax=1000;
    if (params.size()>1)
        nMax=params[1].get_int();
    int nOffset=0;
    if (params.size()>2)
        nOffset=params[2].get_int();
    std::vector<CTxPosItem> vTxPosAll;
    GetDiskTxPoses (vIds,vTxPosAll); 
    Array r;    
    int nCount=0;
    for (int i =(int)vTxPosAll.size()-1; i >=0; i--)  
    {        
        //if(((nDirection==0)&&!(vTxPosAll[i].nFlags&((1<<TXITEMFLAG_RECEIVECONTENT)|(1<<TXITEMFLAG_RECEIVEMONEY))))
        //        ||((nDirection==1)&&!(vTxPosAll[i].nFlags&(1<<TXITEMFLAG_SENDCONTENT))))
        if(!(vTxPosAll[i].nFlags&((1<<TXITEMFLAG_RECEIVECONTENT)|(1<<TXITEMFLAG_RECEIVEMONEY))))
            continue;
        CTransaction txOut;
        if(!GetTransaction(vTxPosAll[i], txOut))
            continue;
        string payer;
        CDomain domain;
        bool fGotPoster=false;
        
        for (int j =0; j <(int)txOut.vout.size(); j++)  
        {
            const CTxOut& out=txOut.vout[j];
            //LogPrintf("getsalesrecord1 \n" );
            if(out.strContent.size()==0||CContent(out.strContent).GetFirstCc()!=CC_PAYMENT_P)
                continue;
            LogPrintf("getsalesrecord2 \n" );
            CPayment payment;
            payment.recipient=out.scriptPubKey;
            if(!payment.SetContent(out.strContent))
                continue;    
            LogPrintf("getsalesrecord3 \n" );
            nCount++;
            if(nCount<nOffset)
                continue;
            if(nCount>nOffset+nMax)
                break;
            LogPrintf("getsalesrecord4 \n" );
            if(!fGotPoster)
            {
                _get_poster(txOut,payer,domain);
                fGotPoster=true;
            }
            
            Object obj;
            Value val=payment.ToJson();
            uint256 hashBlock;
            int nHeight;
            pBlockPosDB->GetByPos(vTxPosAll[i].nFile,vTxPosAll[i].nPos,hashBlock,nHeight);
            obj=val.get_obj();
            obj.push_back(Pair("paidvalue",(double)out.nValue/COIN));
            Object objPayer;
            objPayer.push_back(Pair("id",payer));
            if(!domain.IsEmpty())
                objPayer.push_back(Pair("domain",domain.ToJson()));
            obj.push_back(Pair("payer",objPayer));
             obj.push_back(Pair("link",CLink(nHeight,vTxPosAll[i].nTx,j).ToString()));
              obj.push_back(Pair("blockhash",hashBlock.GetHex()));
              if(nHeight>0)
                obj.push_back(Pair("time",(int64_t)chainActive[nHeight]->nTime));
              else
                  obj.push_back(Pair("time",(int64_t)GetAdjustedTime()));
            obj.push_back(Pair("txid",txOut.GetHash().GetHex()));
            r.push_back(obj);
            LogPrintf("getsalesrecord5\n" );
        }   
        if(nCount>nOffset+nMax)
                break;
    }
    return r;
}
json_spirit::Value getpurchaserecord(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp||params.size()<1)
        throw runtime_error("getpromotedcontents Help msg");    
    
    
    std::vector<CScript> vIds;

    BOOST_FOREACH(const Value& addrStr, params[0].get_array())
    {
        CBitcoinAddress address(addrStr.get_str());
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
        CScript scriptPubKey = GetScriptForDestination(address.Get());
        vIds.push_back(scriptPubKey);
    }
    //int nDirection=0;
    //if (params.size()>1)
    //    nDirection=params[1].get_int();
    int nMax=1000;
    if (params.size()>1)
        nMax=params[1].get_int();
    int nOffset=0;
    if (params.size()>2)
        nOffset=params[2].get_int();
    std::vector<CTxPosItem> vTxPosAll;
    GetDiskTxPoses (vIds,vTxPosAll); 
    Array r;    
    int nCount=0;
    for (int i =(int)vTxPosAll.size()-1; i >=0; i--)  
    {       
        
        if(!(vTxPosAll[i].nFlags&(1<<TXITEMFLAG_SENDCONTENT)))
            continue;
        CTransaction txOut;
        if(!GetTransaction(vTxPosAll[i], txOut))
            continue;
        
        CDomain domain;
        
        
        for (int j =0; j <(int)txOut.vout.size(); j++)  
        {
            const CTxOut& out=txOut.vout[j];
            //LogPrintf("getsalesrecord1 \n" );
            if(out.strContent.size()==0||CContent(out.strContent).GetFirstCc()!=CC_PAYMENT_P)
                continue;
            LogPrintf("getpurchaserecord2 \n" );
            CPayment payment;
            payment.recipient=out.scriptPubKey;
            if(!payment.SetContent(out.strContent))
                continue;    
            LogPrintf("getpurchaserecord2 \n" );
            nCount++;
            if(nCount<nOffset)
                continue;
            if(nCount>nOffset+nMax)
                break;
            LogPrintf("getpurchaserecord4 \n" );
            pDomainDBView->GetDomainByForward(payment.recipient, domain, true);
            Object obj;
            Value val=payment.ToJson();
            uint256 hashBlock;
            int nHeight;
            pBlockPosDB->GetByPos(vTxPosAll[i].nFile,vTxPosAll[i].nPos,hashBlock,nHeight);
            obj=val.get_obj();
            obj.push_back(Pair("paidvalue",(double)out.nValue/COIN));
            
            if(!domain.IsEmpty())
                obj.push_back(Pair("shopdomain",domain.ToJson()));            
             obj.push_back(Pair("link",CLink(nHeight,vTxPosAll[i].nTx,j).ToString()));
              obj.push_back(Pair("blockhash",hashBlock.GetHex()));
              if(nHeight>0)
                obj.push_back(Pair("time",(int64_t)chainActive[nHeight]->nTime));
              else
                  obj.push_back(Pair("time",(int64_t)GetAdjustedTime()));
            obj.push_back(Pair("txid",txOut.GetHash().GetHex()));
            r.push_back(obj);
            LogPrintf("getpurchaserecord5\n" );
        }   
        if(nCount>nOffset+nMax)
                break;
    }
    return r;
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
    int64_t startTime=GetTimeMillis();
    GetDiskTxPoses(vIDsLocal, vTxPos);
    LogPrintf("getmsgs time after getdistxposes:%i,%i,%i \n",startTime,GetTimeMillis(),GetTimeMillis()-startTime);
    if (fIncludeMempool)
        mempool.GetUnconfirmedTransactions(vIDsLocal, vMemTx);
    //LogPrintf("getmessages mempooltxs:%u \n", vMemTx.size());
    //LogPrintf("getmessages blockchaintxs:%u \n", vTxPos.size());
    LogPrintf("getmsgs time after get memepool:%i,%i,%i \n",startTime,GetTimeMillis(),GetTimeMillis()-startTime);
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
    LogPrintf("getmsgs time after get memepool msg:%i,%i,%i \n",startTime,GetTimeMillis(),GetTimeMillis()-startTime);
    vMessages.clear();
    for (int i = vTxPos.size() - 1; i >= 0; i--)
    {
        if (nPos >= nOffset + nCount)
            break;
        if((((vTxPos[i].nFlags&(1<<TXITEMFLAG_SENDCONTENT))==0)&&((vTxPos[i].nFlags&(1<<TXITEMFLAG_RECEIVECONTENT))==0))||
                (nDirectionFilter == OUTPUT_ONLY&&((vTxPos[i].nFlags&(1<<TXITEMFLAG_SENDCONTENT))==0))||
                (nDirectionFilter == INCOMING_ONLY&&((vTxPos[i].nFlags&(1<<TXITEMFLAG_RECEIVECONTENT))==0)))
                continue;
        CTransaction tx;
        uint256 hashBlock;
        int nHeight = -1;
        int nTime = GetTime();
        int nTx = -1;
        LogPrintf("getmessages txpos   file:%i,pos:%u,flags:%i\n",vTxPos[i].nFile,vTxPos[i].nPos,vTxPos[i].nFlags);
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
        CContent ctt(txout.strContent);
        if (txout.strContent.size() >= 39&&ctt.GetFirstCc()==CC_MESSAGE_P)
        {
            std::vector<std::pair<int, string> > vContent;
            if (ctt.Decode(vContent))
            {
                for (unsigned int j = 0; j < vContent.size(); j++)
                {
                   // if (vContent[j].first == CC_MESSAGE_P)
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
                                if (vInnerContent[k].first == CC_ENCRYPT_PARAMS_IV)
                                    hasIV = true;
                                else if (vInnerContent[k].first == CC_ENCRYPT)
                                    hasContent = true;
                            }
                            if (hasIV && hasContent)
                            {
                                CContent cmsg;
                                cmsg.EncodeUnit(CC_MESSAGE_P, vContent[j].second);
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
json_spirit::Value getdomainsbytags(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter ids, expected array");
    Array arrTags = params[0].get_array(); 
    int nMax=100;
    if(params.size()>1)
        nMax=params[1].get_int();
    std::vector<CDomain> vDomain;
    bool fInclude100=true;
    if(params.size()>2)
        fInclude100=params[2].get_bool();
    vector<string>vTags;
    if(arrTags.size()>9)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "too many tags");
    for (unsigned int i = 0; i < arrTags.size(); i++)
    {
        LogPrintf("getdomainsbytags tag:%s \n", arrTags[i].get_str());
        vTags.push_back(arrTags[i].get_str());
    }   
    
        pDomainDBView->GetDomainByTags(vTags, vDomain, fInclude100,nMax,true);    
        LogPrintf("getdomainsbytags domains:%i \n", vDomain.size());
    Array arrDomains;
   for (unsigned int i = 0; i <vDomain.size(); i++) 
   {
       arrDomains.push_back(vDomain[i].ToJson());
   }
    LogPrintf("getdomainsbytags toJson%i \n", arrDomains.size());
    return Value(arrDomains);
}

json_spirit::Value getdomainsbyalias(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");    
    string alias = params[0].get_str(); 
    vector<CDomain> vDomain;
    pDomainDBView->GetDomainsByAlias(alias, vDomain, true);    
    Array arrDomains;
   for (unsigned int i = 0; i < vDomain.size(); i++) 
   {
       arrDomains.push_back(vDomain[i].ToJson());
   }
    LogPrintf("getdomainsbyalias toJson%i \n", arrDomains.size());
    return Value(arrDomains);
}

json_spirit::Value getdomainsexpiring(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != array_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter ids, expected array");
    uint32_t time = params[0].get_int(); //note:this parameter is time from now on
    vector<string> vDomain;
    pDomainDBView->GetDomainNamesToExpire(vDomain, time, true);
    Array arrDomains;
   for (unsigned int i = 0; i < vDomain.size(); i++) 
   {
       arrDomains.push_back(vDomain[i]);
   }
    LogPrintf("getdomainsexpiring toJson%i \n", arrDomains.size());
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
    if (fHelp)
        throw runtime_error("searchproducts Help msg");    
    int maxb;    
    int nMaxResults;
    int nOffset;
    Array firstcc;
    Array gAddrs;
    Array arrTags;
    int cformat;
    unsigned char cflag;    
    if (!_parse_getpromotedcontents_params(params, nMaxResults,nOffset, maxb, firstcc,arrTags, cformat, cflag, gAddrs))
        throw runtime_error("getpromotedcontents Error parsing parameters");
    vector<CScript> vSenders;
    LogPrintf("searchproducts arrtags:%i \n",arrTags.size());
    BOOST_FOREACH(const Value& addrStr, gAddrs)
    {
        CBitcoinAddress address(addrStr.get_str());
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Address is invalid");
        CScript scriptPubKey = GetScriptForDestination(address.Get());
        vSenders.push_back(scriptPubKey);
    }    
    vector<int> vCCs;    
    vCCs.push_back(CC_PRODUCT_P);
    if(arrTags.size()>9)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "too many tags");
    vector<string> vTags;
    BOOST_FOREACH(const Value& tag, arrTags)
        vTags.push_back(tag.get_str());
    vector<CContentDBItem> vContents;
    Array r;    
    int b = 0;
    if(!SearchPromotedContents(vSenders,vCCs,vTags,vContents,nMaxResults,nOffset))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "SearchPromotedContents failed");
    Array arrProducts;
    vector<CProduct> vProduct;
    for (int i = 0; i < (int) vContents.size(); i++)
   {
       if ( b > maxb)
            return r;
       CTxOut out;       
       if (!GetTxOutFromVoutPos(vContents[i].pos,out))
           continue;       
        CContent content(out.strContent);           
       CProduct product;       
        if (product.SetContent(content))
       {           
           LogPrintf("searchproducts secontent done \n");
            product.link = vContents[i].link;
            product.seller = vContents[i].sender;
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
               
                pDomainDBView->GetDomainByForward(product.seller, product.sellerDomain,true);                
            }
            if (product.recipient.size() == 0)
                product.recipient = product.seller;   
            product.nExpireTime=LockTimeToTime(out.nLockTime);
           vProduct.push_back(product);           
       }
           
   }
    for (unsigned int i = 0; i < vProduct.size(); i++) 
        arrProducts.push_back(vProduct[i].ToJson());   
    LogPrintf("searchproducts toJson%i \n", arrProducts.size());
    return Value(arrProducts);
}
json_spirit::Value getproductbylink(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp||params.size()<1)
        throw runtime_error("getproductbylink Help msg"); 
    CLink link;
    //LogPrintf("getproductbylink 1 \n");
    if(!link.SetString(params[0].get_str()))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link");
    CContent content;    
    CBlockIndex* pblockindex;
    CBlock block;
    //LogPrintf("getproductbylink 2 \n");
    if (!GetBlockByHeight(link.nHeight, block, pblockindex))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link,content not found");
    CTransaction tx;
    //LogPrintf("getproductbylink 3 \n");
    if (!GetTxFromBlock(block, link.nTx, tx))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link,content not found"); 
    //LogPrintf("getproductbylink 4 \n");
    if (!GetContentFromVout(tx, link.nVout, content))
       throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link,content not found");
    /////LogPrintf("getproductbylink 5 \n");
    CProduct product;
    if (!product.SetContent(content))
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link,content is not product");
    //LogPrintf("getproductbylink 6 \n");
    product.link = link;
    string seller;
    _get_poster(tx,seller,product.sellerDomain);   
    //LogPrintf("getproductbylink 7 \n");
    StringToScriptPubKey(seller,product.seller);
   // LogPrintf("getproductbylink 8 \n");
    if (product.recipient.size() == 0)
        product.recipient = product.seller; 
    //LogPrintf("getproductbylink 9 \n");
    return product.ToJson();
}
json_spirit::Value getfilepackageurl(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != str_type)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter link, expected str");
    CLink link;
    if (!link.SetString(params[0].get_str()))
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