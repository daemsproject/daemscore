#ifndef FAI_LINK_H
#define FAI_LINK_H

#include <string.h>
#include <string>
#include <vector>
//
#include "serialize.h"
#include "fai/cc.h"
#include "uint256.h"
#include "script/script.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

//
using namespace json_spirit;
using namespace std;
//using std::string;

static const std::string URI_SCHEME_NAME = "fai";
static const std::string URI_HEX_HEADER = "x";
static const std::string URI_B32_HEADER = "#";
static const std::string URI_SEPERATOR = ".";
static const std::string URI_COLON = ":";

enum linkformat
{
    LINK_FORMAT_DEC = 0,
    LINK_FORMAT_HEX = 1,
    LINK_FORMAT_B32 = 2,
    LINK_FORMAT_DEC_NOSCHEMA = 128,
};
class CLink
{
public:
    int nHeight;
    unsigned short nTx;
    unsigned short nVout;

    CLink()
    {
        SetEmpty();
    }

    CLink(const std::string& linkStr)
    {
        SetString(linkStr);
    }

    CLink(const vector<unsigned char>& linkVch)
    {
        SetString(linkVch);
    }

    CLink(const int nHeightIn, const int nTxIn, const int nVoutIn = 0)
    {
        nHeight = nHeightIn;
        nTx = (unsigned short) nTxIn;
        nVout = (unsigned short) nVoutIn;
    }
    string Serialize()const;
    int64_t SerializeInt()const;
    bool Unserialize(string& str);
    bool UnserializeConst(const string& str);
    bool Unserialize(const int64_t nLink);
    bool WriteVarInt(const int nIn, string& str) const;
    bool ReadVarInt(string& str, int& n)const;
    
    ADD_SERIALIZE_METHODS;   

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {        
        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nTx));
        READWRITE(VARINT(nVout));
    }
    
    Value ToJson()const;
    bool ToJsonString(std::string& entry)const;
    std::string ToString(const linkformat linkFormat = LINK_FORMAT_DEC)const;
    void SetEmpty();
    bool IsEmpty() const;
    bool IsValid() const;   // Can not guarantee link content exists in block chain
    bool SetInt(const int nHeightIn, const int nTxIn, const int nVoutIn = 0);
    bool SetString(const std::string linkStr);
    bool SetString(const vector<unsigned char>& linkVch);
    bool SetJson(const Object& obj,string& strError);
    friend bool operator==(const CLink& a, const CLink& b)
    {
        return (a.nHeight == b.nHeight && a.nTx == b.nTx && a.nVout == b.nVout);
    }
    friend bool operator!=(const CLink& a, const CLink& b)
    {
        return !(a == b);
    }
    friend bool operator<(const CLink& a, const CLink& b)
    {
        return (a.nHeight < b.nHeight || (a.nHeight == b.nHeight && a.nTx < b.nTx)|| (a.nHeight == b.nHeight && a.nTx == b.nTx && a.nVout < b.nVout));
    }
};
class CLinkUni
{
public:
    cctype linkType;
    int nHeight;
    unsigned short nTx;
    unsigned short nVout;
    uint256 txid;
    string strLink;
    CScript scriptPubKey;
    string strLinkName;
    string strDomainExtension;
    string strDomain;
    vector<string> vTags;
    CLinkUni()
    {
        SetEmpty();
    }
    
    CLinkUni(const std::string& linkStr)
    {
        SetString(linkStr);
    }
    CLinkUni(const vector<unsigned char>& linkVch)
    {
        SetString(linkVch);
    }
    CLinkUni(const int nHeightIn, const int nTxIn, const int nVoutIn = 0)
    {
        linkType=CC_LINK_TYPE_BLOCKCHAIN;
        nHeight = nHeightIn;
        nTx = (unsigned short) nTxIn;
        nVout = (unsigned short) nVoutIn;
    }
    CLinkUni(const uint256 txidIn, const int nVoutIn = 0)
    {
        linkType=CC_LINK_TYPE_TXIDOUT;  
        txid=txidIn;
        nVout = (unsigned short) nVoutIn;
    }
    CLinkUni(const CScript scriptPubKeyIn)
    {
        linkType=CC_LINK_TYPE_SCRIPTPUBKEY;
        scriptPubKey=scriptPubKeyIn;
    }
    bool SetString(const std::string linkStr);
    bool SetString(const vector<unsigned char>& linkVch);
    bool SetStringNative(const std::string linkStr);
    bool SetStringBlockChain(const std::string linkStr);
    bool SetStringTxidOut(const std::string linkStr);
    bool SetStringScriptPubKey(const std::string linkStr);
    bool UnserializeConst(const string& str);
    //this function only outputs the content of link ,w/o link type information
    std::string ToString(const linkformat linkFormat = LINK_FORMAT_DEC)const;
    std::string ToStringBlockChain(const linkformat linkFormat= LINK_FORMAT_DEC)const;
    std::string ToStringTxidOut()const;    
    std::string ToStringScriptPubKey()const;
    bool SetContent(const string& str);
    string ToContent()const;
    bool SetJson(const Object& obj,string& strError);
    Value ToJson(const linkformat linkFormat= LINK_FORMAT_DEC)const;
    string ToJsonString(const linkformat linkFormat= LINK_FORMAT_DEC)const;
    
    ADD_SERIALIZE_METHODS;   

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {    
        READWRITE(VARINT((int)linkType));
        switch (linkType)
        {
            case CC_LINK_TYPE_BLOCKCHAIN:
                READWRITE(VARINT(nHeight));
                READWRITE(VARINT(nTx));
                READWRITE(VARINT(nVout));
                break;
            case CC_LINK_TYPE_TXIDOUT:
                READWRITE(txid);
                READWRITE(VARINT(nVout));
                break;
            case CC_LINK_TYPE_SCRIPTPUBKEY:
                READWRITE(scriptPubKey);
                break;
//            case CC_LINK_TYPE_DOMAIN:
//                READWRITE(strDomain);
//                READWRITE(strDomainExtension);
//                break;
            default:
                READWRITE(strLink);
        }        
    }
    
    
    void SetEmpty();
    bool IsEmpty() const; 
    
    friend bool operator==(const CLinkUni& a, const CLinkUni& b);
    
    friend bool operator!=(const CLinkUni& a, const CLinkUni& b)
    {
        return !(a == b);
    }    
};
#endif // FAI_LINK_H