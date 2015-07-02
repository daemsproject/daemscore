#include "content.h"
#include "utilstrencodings.h"
#include <string.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <stdio.h>
#include "util.h"
#include "utiltime.h"
#include "base58.h"
#include "streams.h"

#include "json/json_spirit_writer_template.h"
using namespace boost;
using namespace std;

#define to_uint64(n) (*(uint64_t*)(n))
static const int TRIM_READABLE_LEN = 1000;
static const int TRIM_BINARY_LEN = 20;
static const int STR_FORMAT_SUM_MAXSIZE = 512;

std::string GetCcName(const cctype cc)
{
    if(mapCC.count(cc))
        return mapCC[cc];
    return "";
}

cctype GetCcValue(const std::string ccName)
{
    for(map<int,std::string>::iterator it=mapCC.begin();it!=mapCC.end();it++)
    {
        if (it->second==ccName)
            return (cctype)it->first;        
    }
    return CC_NULL;
}

std::string GetCcHex(const cctype cc)
{
    std::ostringstream stm;
    stm << (char) cc;
    return HexStr(stm.str());
}

bool CContent::IsStandard()const
{
    const_iterator pc = begin();
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            return false;
    }
    return (pc > end()) ? false : true;
}

Array CContent::ToJson(stringformat fFormat, bool fRecursive)const
{
    const_iterator pc = begin();
    Array result;
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        Object ccUnit;
        std::string ccName;
        ccName = GetCcName(cc);
        //LogPrintf("CContent Tojson cc:%i,ccname:%s\n",IntArray2HexStr(&cc,&cc+1),ccName);
        ccUnit.push_back(Pair("encoding", fFormat));
        ccUnit.push_back(Pair("cc_name", ccName));
        ccUnit.push_back(Pair("cc", GetCcHex(cc)));
        if (IsCcParent(cc) && fRecursive) {
            if (contentStr.IsStandard())
                ccUnit.push_back(Pair("content", contentStr.ToJson(fFormat)));
            else
                ccUnit.push_back(Pair("content", "non-standard"));
        } else {
            switch (fFormat) {
                case STR_FORMAT_BIN:
                case STR_FORMAT_BIN_SUM:
                    if (fFormat == STR_FORMAT_BIN_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", contentStr));
                    break;
                case STR_FORMAT_HEX:
                case STR_FORMAT_HEX_SUM:
                    if (fFormat == STR_FORMAT_HEX_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", HexStr(contentStr)));
                    break;
                case STR_FORMAT_B64:
                case STR_FORMAT_B64_SUM:
                    if (fFormat == STR_FORMAT_B64_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", EncodeBase64(contentStr)));
                    break;

            }

        }
        result.push_back(ccUnit);
    }
    if (pc > end()) {
        result.clear();
        Object rObj;
        rObj.push_back(Pair("content", "non-standard"));
        result.push_back(rObj);
    }
    return result;
}

std::string CContent::TrimToHumanString(const std::string& str)const
{
    std::string lenStr = " ... (";
    lenStr += strpatch::to_string(str.size());
    lenStr += " bytes) ";
    std::string str2;
    if (IsStringPrint(str)) {
        str2 = str.size() > TRIM_READABLE_LEN ? str.substr(0, TRIM_READABLE_LEN) + lenStr : str;
    } else {
        str2 = str.size() > TRIM_BINARY_LEN ? HexStr(str.substr(0, TRIM_BINARY_LEN)) + lenStr : HexStr(str);
    }
    trim(str2);
    return str2;
}

std::string CContent::ToHumanString()
{
    std::string ccUnit;
    const_iterator pc = begin();
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        std::string ccName;
        ccName = GetCcName(cc);
        ccUnit += ccName;
        if (IsCcParent(cc)) {
            if (contentStr.IsStandard())
                ccUnit += " " + contentStr.ToHumanString() + " ";
            else
                ccUnit = strpatch::to_string(size()) + " bytes binary";
        } else {
            ccUnit += " " + TrimToHumanString(contentStr) + " ";
        }
    }
    if (pc > end()) {
        return strpatch::to_string(size()) + " bytes binary";
    }
    trim(ccUnit);
    return ccUnit;
}

bool CContent::HasCc(const cctype& ccIn) const// Very costly !!! Try to use FirstCc()
{
    const_iterator pc = begin();
    bool r = false;
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        if (cc == ccIn)
            return true;
        if (IsCcParent(cc)) {
            r = contentStr.HasCc(ccIn);
        }
    }
    if (pc > end())
        return 0;
    return r;
}

bool CContent::FirstCc(const cctype& ccIn)const
{
    const_iterator pc = begin();
    cctype cc;
    u_int64_t n;
    if (!ReadVarInt(pc, n))
        return false;
    cc = (cctype) n;
    if (cc != ccIn)
        return false;
    if (!IsStandard())
        return false;
    return true;

}

int CContent::GetFirstCc()const
{
    if (IsEmpty())
        return 0;
    const_iterator pc = begin();
    //cctype cc;
    u_int64_t n;
    if (!ReadVarInt(pc, n))
        return 0;
    return n;
}

bool CContent::SetEmpty()
{
    clear();
    return true;
}

bool CContent::IsEmpty()const
{
    return size() == 0;
}

bool CContent::SetJson(const Array& cttJson)
{
    cctype cc = CC_NULL;
    bool fSuccess = true;

    BOOST_FOREACH(const Value& input, cttJson)
    {
        const Object& cttObj = input.get_obj();
        std::string ccName;
        stringformat format = STR_FORMAT_BIN;
        CContent content;
        bool fHasName = false;
        bool fHasContent = false;
        Array arrContent;

        BOOST_FOREACH(const Pair& ccUnit, cttObj)
        {
            //LogPrintf(" ccunit name:%s\n",ccUnit.name_);            
            if (ccUnit.name_ == "cc_name") {
                ccName = ccUnit.value_.get_str();
                cc = GetCcValue(ccName);
                fHasName = true;
            }
            if (ccUnit.name_ == "encoding") {
                format = (stringformat) ccUnit.value_.get_int();
            }
        }

        BOOST_FOREACH(const Pair& ccUnit, cttObj)
        {
            if (ccUnit.name_ == "content") {
                if (IsCcParent(cc)) {
                    if (content.SetJson(ccUnit.value_.get_array()))
                        fHasContent = true;
                } else {
                    string formatedcontent = ccUnit.value_.get_str();
                    string bincontent;
                    std::vector<unsigned char> vch1;
                    switch (format) {
                        case STR_FORMAT_BIN:
                        case STR_FORMAT_BIN_SUM:
                            bincontent = formatedcontent;
                            break;
                        case STR_FORMAT_B64:
                        case STR_FORMAT_B64_SUM:
                            bincontent = DecodeBase64(formatedcontent);
                            break;
                        case STR_FORMAT_HEX:
                        case STR_FORMAT_HEX_SUM:
                            vch1 = ParseHex(formatedcontent);
                            bincontent = std::string(vch1.begin(), vch1.end());
                            break;
                        default:
                            bincontent = formatedcontent;
                    }
                    content.SetString(bincontent);
                    fHasContent = true;
                }
                break;
            }
        }
        if (fHasName && fHasContent) {
            //LogPrintf("setjson success:%s\n");
            WriteVarInt(cc);
            WriteCompactSize(content.size());
            append(content);
        } else
            fSuccess = false;
    }
    return fSuccess;
}

bool CContent::SetString(const std::string& cttStr)
{
    clear();
    append(cttStr);
    return true;
}

bool CContent::SetString(const vector<unsigned char>& cttVch)
{
    std::string str(cttVch.begin(), cttVch.end());
    return SetString(str);
}

bool CContent::SetUnit(const cctype& cc, const std::string& cttStr)
{
    WriteVarInt(cc);
    WriteCompactSize(cttStr.size());
    append(cttStr);
    return true;
}

bool CContent::SetUnit(const std::string& ccname, const std::string& cttStr)
{
    cctype cc = GetCcValue(ccname);
    return SetUnit(cc, cttStr);
}

bool CContent::GetCcUnit(const_iterator& pc, cctype& ccRet, std::string& content)const
{
    ccRet = CC_NULL;
    if (pc >= end())
        return false;
    u_int64_t n;
    if (!ReadVarInt(pc, n))
        return false;
    ccRet = (cctype) n;
    u_int64_t len;
    if (!ReadCompactSize(pc, len))
        return false;
    if (len > 0) {
        if (!ReadData(pc, len, content)) {
            content = "";
            return false;
        }
    } else if (len == 0)
        content = "";
    else
        return false;
    return pc <= end() ? true : false;
}

bool CContent::WriteVarInt(u_int64_t n)
{
    char tmp[8];
    int len = 0;
    while (true) {
        tmp[len] = (n & 0x7F) | (len ? 0x80 : 0x00);
        if (n <= 0x7F)
            break;
        n = (n >> 7) - 1;
        len++;
    }
    do {
        std::string tstr;
        tstr += tmp[len];
        WriteData(tstr, 1);
    } while (len--);
    return true;
}

bool CContent::ReadVarInt(const_iterator& pc, u_int64_t& n)const
{
    n = 0;
    unsigned char chData = 0xff;
    int offset = 0;
    while (pc < end()) {
        if (offset > 3)
            return false;
        chData = *pc++;
        n = (n << 7) | (chData & 0x7F);
        offset++;
        if (chData & 0x80)
            n++;
        else
            break;
    }
    return (chData & 0x80) ? false : true;
}

bool CContent::WriteCompactSize(u_int64_t n)
{
    std::ostringstream os;
    if (n < 253) {
        os << (char) n;
        WriteData(os.str());
    } else if (n <= std::numeric_limits<unsigned short>::max()) {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += n >> 8;
        os << (char) 253 << tmp;
        WriteData(os.str());
    } else if (n <= std::numeric_limits<unsigned int>::max()) {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 16) & 0xFF;
        tmp += (n >> 24) & 0xFF;
        os << (char) 254 << tmp;
        WriteData(os.str());
    } else {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 16) & 0xFF;
        tmp += (n >> 24) & 0xFF;
        tmp += (n >> 32) & 0xFF;
        tmp += (n >> 40) & 0xFF;
        tmp += (n >> 48) & 0xFF;
        tmp += (n >> 56) & 0xFF;
        os << (char) 255 << tmp;
        WriteData(os.str());
    }
    return true;
}

bool CContent::ReadCompactSize(const_iterator& pc, u_int64_t& nSizeRet)const
{
    if (pc == end())
        return false;
    unsigned char chSize;
    std::string chData;
    chSize = *pc++;
    nSizeRet = 0;
    if (chSize < 253) {
        nSizeRet = chSize;
    } else if (chSize == 253) {
        if (!ReadDataReverse(pc, 2, chData))
            return false;
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 253) {
            throw std::ios_base::failure("non-canonical ReadCompactSize()" + nSizeRet);
        }
    } else if (chSize == 254) {
        if (!ReadDataReverse(pc, 4, chData))
            return false;
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 0x10000u)
            throw std::ios_base::failure("non-canonical ReadCompactSize()");
    } else {
        if (!ReadDataReverse(pc, 8, chData))
            return false;
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 0x100000000ULL)
            throw std::ios_base::failure("non-canonical ReadCompactSize()");
    }
    return true;
}

bool CContent::WriteData(const std::string str)
{
    append(str);
    return true;
}

bool CContent::WriteData(const std::string str, int len)
{
    append(str, 0, len);
    return true;
}

bool CContent::ReadData(const_iterator& pc, int len, std::string& result)const
{
    int i = 0;
    while (i < len) {
        result += *pc++;
        if (pc > end())
            return false;
        i++;
    }
    return true;
}

bool CContent::ReadDataReverse(const_iterator& pc, int len, std::string& result)const
{
    int i = len;
    const_iterator pc2 = pc + len;
    if (pc2 > end())
        return false;
    while (i > 0) {
        result += *--pc2;
        i--;
        pc++;
    }
    return true;
}

bool IsCcParent(const cctype& cc)
{
    u_int64_t cc2 = cc;
    return (cc2 % 2 == 1) ? true : false;
}

bool CContent::EncodeP(const int cc, const std::vector<std::pair<int, string> >& vEncoding)
{
    CContent integrated;
    for (unsigned int i = 0; i < vEncoding.size(); i++)
        integrated.EncodeUnit(vEncoding[i].first, vEncoding[i].second);
    EncodeUnit(cc, integrated);
    return true;
}

bool CContent::EncodeUnit(int cc, const string& content)
{
    WriteVarInt(cc);
    WriteCompactSize(content.size());
    append(content);
    return true;
}

bool CContent::Decode(std::vector<std::pair<int, string> >& vDecoded)const
{
    const_iterator pc = begin();
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        vDecoded.push_back(make_pair(cc, contentStr));
    }
    if (pc > end())
        return false;
    return true;
}

bool CContent::DecodeLink(int& redirectType, string& redirectTo)const
{
    LogPrintf("CContent DecodeLink\n");
    std::vector<std::pair<int, string> > vDecoded;
    Decode(vDecoded);
    LogPrintf("CContent DecodeLink1\n");
    bool fHasLinkType = false;
    bool fHasLinkContent = false;
    int cc;
    int nLinkType = 0;
    string str;
    for (unsigned int i = 0; i < vDecoded.size(); i++) {
        cc = vDecoded[i].first;
        if (cc >= CC_LINK_TYPESTRING && cc <= CC_LINK_TYPE_DOMAIN) {
            LogPrintf("CContent DecodeLink2\n");
            fHasLinkType = true;
            nLinkType = cc;
        } else if (cc == CC_LINK) {
            LogPrintf("CContent DecodeLink3\n");
            str = vDecoded[i].second;
            if (str.size() > 64)
                return false;
            fHasLinkContent = true;
        }

    }
    if (fHasLinkType&&fHasLinkContent)
    {
        LogPrintf("CContent DecodeLink success\n");
        redirectType = nLinkType;
        redirectTo = str;
        return true;
    }
    return false;
}

bool CContent::DecodeDomainInfo(string& strAlias, string& strIntro, CLink& iconLink, std::vector<string>& vTags)const
{
    LogPrintf("CContent DecodeDomainInfo\n");
    std::vector<std::pair<int, string> > vDecoded;
    Decode(vDecoded);
    int cc;
    string str;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {        
        cc=vDecoded[i].first;
        str=vDecoded[i].second;
        if(cc==CC_DOMAIN_INFO_ALIAS)
        {
            if(str.size()>64)            
                str=str.substr(0,64);
            strAlias=str;        
        }
        else if(cc==CC_DOMAIN_INFO_INTRO)
        {
            if(str.size()>128)
                str=str.substr(0,128);
            strIntro=str;
        }
        else if(cc==CC_DOMAIN_INFO_ICON&&str.size()>=3)
        {
            //CDataStream s(str.c_str(),str.c_str()+str.size(),0,0);
            iconLink.Unserialize(str);
        }
        else if(cc==CC_TAG&&str.size()<=32)
            vTags.push_back(str);
    }
    LogPrintf("CContent DecodeDomainInfo done\n");
    return true;
}
bool CContent::GetTags(std::vector<std::pair<int,std::string> >& vTagList) const
{
    if (!IsStandard())
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    Decode(vDecoded);
    bool fccp=false;
    int ccp=-1;    
    for(unsigned int i=0; i<vDecoded.size();i++)
    {
         int cc=   vDecoded[i].first;         
         string str=vDecoded[i].second;
         if(cc==CC_P)
        {
            CContent(str).GetTags(vTagList);
             fccp=true;
        }
         else if(cc<255)
         {
            if(cc%2==1&&cc!=CC_TAG_P)
            {
                CContent(str)._GetTags(vTagList,cc&0);
                fccp=true;
    }
            else if(cc!=CC_TAG)
                ccp=cc;
    }
}
    if (!fccp&&ccp!=-1)
{
        _GetTags(vTagList,ccp);
     }        
    return vTagList.size()>0;    
}
bool CContent::_GetTags(std::vector<std::pair<int,std::string> >& vTagList,int ccp) const
{
    if(!IsStandard())
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    Decode(vDecoded);
    for(unsigned int i=0; i<vDecoded.size();i++)
        {
            int cc=   vDecoded[i].first;         
            string str=vDecoded[i].second;            
        std::vector<std::string> vTag;
        if (cc == CC_TAG && str.size() > 0 && str.size() <= 32)
            vTagList.push_back(make_pair(ccp, str));
        if (cc == CC_TAG_P) {
            std::vector<std::pair<int, string> > vDecoded2;
            for (unsigned int i = 0; i < vDecoded.size(); i++) {
                int cc1 = vDecoded[i].first;
                string str1 = vDecoded[i].second;
                if (cc1 == CC_TAG && str1.size() > 0 && str1.size() <= 32)
                    vTagList.push_back(make_pair(ccp, str1));
            }
        }
    }
    return vTagList.size() > 0;
}

Value CMessage::ToJson(bool fLinkOnly)const
{ // to test
    json_spirit::Object obj;
    string strID;
    ScriptPubKeyToString(IDFrom, strID);
    obj.push_back(Pair("IDFrom", Value(strID)));
    ScriptPubKeyToString(IDTo, strID);
    obj.push_back(Pair("IDTo", Value(strID)));
    obj.push_back(Pair("txid", Value(txid.GetHex())));
    obj.push_back(Pair("nVout", Value(nVout)));
    obj.push_back(Pair("nBlockHeight", Value(nBlockHeight)));
    obj.push_back(Pair("nTime", Value((uint64_t) nTime)));
    obj.push_back(Pair("nTx", Value(nTx)));
    if (content != CContent())
        obj.push_back(Pair("content", content.ToJson(fLinkOnly ? STR_FORMAT_B64_SUM : STR_FORMAT_B64)));
    return Value(obj);
}

string CMessage::ToJsonString(bool fLinkOnly)const
{
    return write_string(ToJson(fLinkOnly), false);

}

bool CMessage::SetJson(const Object& json)
{
    return false;
}