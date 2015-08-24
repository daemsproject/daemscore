#ifndef FAI_CONTENT_H
#define FAI_CONTENT_H

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "fai/cc.h"
#include "utiltime.h"
#include "serialize.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "uint256.h"
#include "script/script.h"
#include "fai/link.h"

#include "amount.h"
using namespace json_spirit;
using namespace std;
using std::string;

std::string GetCcName(const cctype cc);
cctype GetCcValue(const std::string ccName);
std::string GetCcHex(const cctype cc);
bool IsCcParent(const cctype& cc);
cctype GetPrimeCC(const cctype cc,unsigned int nDigits=1);
int GetCcLen(const cctype ccIn);
bool FilterCc(const cctype cc, const std::string contentStr, Object& ccUnit);


class CContent : public std::string
{
public:

    CContent()
    {
        SetEmpty();
    }

    CContent(const std::string& cttStr)
    {
        SetString(cttStr);
    }

    CContent(const vector<unsigned char>& cttVch)
    {
        SetString(cttVch);
    }

    CContent(const Array& cttJson)
    {
        SetJson(cttJson);
    }
    Array ToJson(int& nMaxCC,stringformat fFormat = STR_FORMAT_BIN, bool fRecursive = true)const;
    bool ToJsonString(std::string& entry)const;
    bool SetEmpty();
    bool IsEmpty()const;
    bool SetString(const std::string& cttStr);
    bool SetString(const vector<unsigned char>& cttVch);
    bool SetJson(const Array& cttJson);
    bool SetUnit(const cctype& cc, const std::string& cttStr);
    bool SetUnit(const std::string& ccname, const std::string& cttStr);
    std::string ToHumanString(int& nMaxCC)const;
    bool GetCcUnit(const_iterator& pc, cctype& ccRet, std::string& content) const;
    bool ReadVarInt(const_iterator& pc, uint64_t& n)const;
    bool ReadCompactSize(const_iterator& pc, uint64_t& n)const;
    bool WriteVarInt(uint64_t num);
    bool WriteCompactSize(uint64_t num);
    bool ReadData(const_iterator & pc, int len, std::string& str)const;
    bool ReadDataReverse(const_iterator & pc, int len, std::string& str)const;
    std::string TrimToHumanString(const std::string& str)const;
    bool WriteData(const std::string str);
    bool WriteData(const std::string str, int len);

    bool HasCc(const cctype& cc,const bool requireStandard = true ,int nMaxCC=STANDARD_CONTENT_MAX_CC)const; // if requireStandard = false, this function will be very costly
    bool GetCcContent(const cctype& cc,std::string& content,const bool requireStandard = true ,int nMaxCC=STANDARD_CONTENT_MAX_CC) const;
    bool FirstCc(const cctype& cc)const;
    bool FirstNCc(std::vector<cctype>& ccv,bool& countOverN,const unsigned int n = STANDARD_CONTENT_MAX_CC)const;
    int GetFirstCc(int nIteratrions=0)const;
    bool IsStandard()const;
    bool EncodeP(const int cc, const std::vector<std::pair<int, string> >& vEncoding);
    bool EncodeUnit(const int cc, const string& content);
    bool Decode(std::vector<std::pair<int, string> >& vDecoded)const;
    bool DecodeDomainInfo(string& strAlias, string& strIntro, CLink& iconLink, std::vector<string>& vTags)const;
    bool DecodeDomainForward(int& redirectType, string& redirectTo,vector<unsigned char>& forwardsig)const;
    bool DecodeFileString(std::string& strFile,int nIterations=0);
    //this fucnction is for TagDB
    bool GetTags(std::vector<std::pair<int, std::string> >& vTagList) const;       
    bool _GetTags(std::vector<std::pair<int, std::string> >& vTagList, int ccp = -1) const;
    //this function is for normal use
    bool GetDataByCC(cctype mainCc,std::vector<string> & vDataString,bool fRecursive=true, bool fIncludeTypeCC=false) const; 
    
    
    //bool DecodeTagP(std::vector<std::string> vTag)const;
};
CContent EncodeContentUnitArray(const int cc,const vector<string> vContents);



class CMessage
{
public:
    int nBlockHeight;
    uint256 txid;
    int nTx;
    int nVout;
    CScript IDFrom;
    CScript IDTo;
    CContent content;
    unsigned int nTime;
    //bool fIncoming=true;

    CMessage()
    {
        nBlockHeight = -1;
        txid = 0;
        nTx = -1;
        nVout = 0;
        CScript IDFrom;
        CScript IDTo;
        CContent content;
        nTime = GetTime();
    };
    Value ToJson(bool fLinkOnly = false)const;
    string ToJsonString(bool fLinkOnly = false)const;
    bool SetJson(const Object& json);
};


#endif
