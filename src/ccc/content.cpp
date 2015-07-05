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
    //LogPrintf("CContent::GetTags contentunits: %i \n",vDecoded.size());
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
                //LogPrintf("CContent::GetTags parent unit \n");
                CContent(str)._GetTags(vTagList,cc-cc%2);
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
    //LogPrintf("CContent::_GetTags contentunits: %i \n",vDecoded.size());
    for(unsigned int i=0; i<vDecoded.size();i++)
    {
        int cc=   vDecoded[i].first;      
        //LogPrintf("CContent::_GetTags cc: %i \n",cc);
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
Value CProduct::ToJson(bool fLinkOnly)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("link",Value(link.ToString())));
    obj.push_back(Pair("id",Value(id)));
    obj.push_back(Pair("name",Value(name)));    
    obj.push_back(Pair("price",_ValueFromAmount(price)));
    if(shipmentFee!=-1)
        obj.push_back(Pair("shipmentfee",_ValueFromAmount(shipmentFee)));    
    if(recipient.size()>0)
    {
        string strID;
        ScriptPubKeyToString(recipient, strID);
        obj.push_back(Pair("recipient",Value(strID)));    
    }
    if(seller.size()>0)
    {
        Object objSeller;
        string strID;
        ScriptPubKeyToString(seller, strID);
        objSeller.push_back(Pair("id",Value(strID)));    
        if(vSellerDomain.size()>0)
        {
            Array arr;
            for(unsigned int i=0;i<vSellerDomain.size();i++)
                arr.push_back(Value(vSellerDomain[i]));
            objSeller.push_back(Pair("domain",arr));
        }
        obj.push_back(Pair("seller",objSeller));  
    }
    if(!icon.IsEmpty())
        obj.push_back(Pair("icon",Value(icon.ToString()))); 
    if(intro.size()>0)
        obj.push_back(Pair("intro",Value(intro)));
    if(nExpireTime>0)
        obj.push_back(Pair("expiretime",Value((uint64_t)nExpireTime)));
    Array arrTag;
    for(unsigned int j=0;j<vTag.size();j++)    
        arrTag.push_back(vTag[j]);    
    obj.push_back(Pair("tags",arrTag));
    return Value(obj);
}
string CProduct::ToJsonString(bool fLinkOnly )const
{
    return write_string(ToJson(fLinkOnly), false);
}
bool CProduct::SetJson(const Object& obj,string& strError)
{
    Value tmp = find_value(obj, "id");
    if (tmp.type() != str_type)
    {            
        strError="invalid product id";
        return false;
    }     
    id=tmp.get_str();
    LogPrintf("CProduct::SetJson product id %s\n",id);
    tmp = find_value(obj, "name");
    if (tmp.type() != str_type)
    {            
        strError="invalid product name";
        return false;
    }     
    name=tmp.get_str();    
    LogPrintf("CProduct::SetJson product name %s\n", name);
    tmp = find_value(obj, "price");    
    try{
        price=_AmountFromValue(tmp);
    }
    catch (Object& objError)
    {
        strError="price is not valid format";        
        return false;
    }
    if(price<0)
    {
        strError="negative price";        
        price=0;
        return false;
    }    
    LogPrintf("CProduct::SetJson product price %i\n",price);
    tmp = find_value(obj, "shipmentfee");
    if (tmp.type() != null_type)
    { 
        try{
            shipmentFee=_AmountFromValue(tmp);
        }
        catch (Object& objError)
        {
            strError="shipment fee is not valid fromat";        
            return false;
        }        
        LogPrintf("CProduct::SetJson product shipment fee %i\n",shipmentFee);
    }            
    tmp = find_value(obj, "recipient");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=str_type)
        {
            strError="recipient is not string type";
            return false;
        }             
        if(!StringToScriptPubKey(tmp.get_str(),recipient)){
            strError="scriptPubKey is not valid format";
            return false;
        }        
        LogPrintf("CProduct::SetJson recipient %s\n", recipient.ToString());
    } 
    tmp = find_value(obj, "seller");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=str_type)
        {
            strError="seller is not string type";
            return false;
        }             
        if(!StringToScriptPubKey(tmp.get_str(),seller)){
            strError="scriptPubKey is not valid format";
            return false;
        }        
        LogPrintf("CProduct::SetJson seller %s\n", seller.ToString());
    } 
    tmp = find_value(obj, "icon");
    if (tmp.type() != null_type)
    {   
        if(tmp.type() != str_type||!icon.SetString(tmp.get_str()))
        {
            strError="invalid product icon";
            return false;
        }           
        LogPrintf("CProduct::SetJson icon %s\n", tmp.get_str());
    }     
    tmp = find_value(obj, "intro");
    if (tmp.type() != null_type)
    {            
        if(tmp.type() != str_type)
        {
            strError="intro is not str type";
            return false;
        }
        intro=tmp.get_str();        
        LogPrintf("CProduct::SetJson intro %s\n", intro);
    }     
    tmp = find_value(obj, "tags");
    if (tmp.type() != null_type)
    {            
        if(tmp.type() != array_type)
        {
            strError="tags is not array type";
            return false;
        }
        Array arrTags=tmp.get_array();
        for(unsigned int j=0;j<arrTags.size();j++)
        {
            if(arrTags[j].type()!=str_type)
            {
                strError="tag is not str type";
                return false;
            }
            if(arrTags[j].get_str()!="")
            {
                vTag.push_back(arrTags[j].get_str());
                LogPrintf("CProduct::SetJson tag %s\n", arrTags[j].get_str());
            }
        }
    }
    tmp = find_value(obj, "expiretime");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=int_type)
        {
            strError="nExpireTime is not int type";
            return false;
        } 
        nExpireTime=tmp.get_int64();        
        LogPrintf("CProduct::SetJson nExpireTime %i\n", nExpireTime);
    }
    return true;
}

bool CProduct::SetContent(const CContent content)
{
    LogPrintf("CProduct::SetContent\n");
    std::vector<std::pair<int, string> > vDecoded0;
    content.Decode(vDecoded0);   
    if(vDecoded0[0].first!=CC_PRODUCT_P)
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    CContent(vDecoded0[0].second).Decode(vDecoded);   
    int cc;
    string str;    
    for(unsigned int i=0;i<vDecoded.size();i++)
    {   
        cc=vDecoded[i].first;
        str=vDecoded[i].second;
        //LogPrintf("CProduct::SetContent cc:%i,str:%s \n",cc,str);
        if(cc==CC_PRODUCT_ID)
        {
            id=str; 
            //LogPrintf("CProduct::SetContent id:%s \n",id);
        }
        else if(cc==CC_PRODUCT_NAME)
        {
            name=str;
            //LogPrintf("CProduct::SetContent name:%s \n",name);
        }
        else if(cc==CC_PRODUCT_PRICE)
        {
            CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong price format\n");
                return false;            
            }
            price=CAmount(u);
        }
        else if(cc==CC_PRODUCT_SHIPMENTFEE)
        {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong shipmentfee format\n");            
                return false;            
            }
            shipmentFee=CAmount(u);
        }
         else if(cc==CC_PRODUCT_PAYTO)
         {
             recipient=CScript((unsigned char*)str.c_str(),(unsigned char*)(str.c_str()+str.size()));             
             LogPrintf("CProduct::SetContent recipient str %s script %s \n",HexStr(str.begin(),str.end()),recipient.ToString());            
                 
         }
        else if(cc==CC_PRODUCT_EXPIRETIME)
        {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(cp.ReadVarInt(pc,u))     
                nExpireTime=uint32_t(u);
        }
        else if(cc==CC_PRODUCT_ICON&&str.size()>=3)       
            icon.Unserialize(str);
        else if(cc==CC_PRODUCT_INTRO&&str.size()>0)       
            intro=str;
        else if(cc==CC_TAG&&str.size()>0)
            vTag.push_back(str);
        else if(cc==CC_TAG_P)
        {
            std::vector<std::pair<int, string> > vDecodedTag;
            CContent(str).Decode(vDecodedTag);
            for(unsigned int i=0;i<vDecodedTag.size();i++)
            { 
                if(vDecodedTag[i].first==CC_TAG)
                vTag.push_back(vDecodedTag[i].second);
            }
        }
//        else if(cc==CC_ATTRIBUTE_P)
//        {
//            
//        }
//        else if(cc==CC_PRICE_P)
//        {
//            
//        }
//        else if(cc==CC_SHIPMENTFEE_P)
//        {
//            
//        }
    }
    LogPrintf("CProduct::SetContent done\n");
    return IsValid();
}
CContent CProduct::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_PRODUCT_ID,id));
    vcc.push_back(make_pair(CC_PRODUCT_NAME,name));
    CContent cPrice;
    cPrice.WriteVarInt(price);
    vcc.push_back(make_pair(CC_PRODUCT_PRICE,cPrice));
    if(shipmentFee!=-1)
    {
        CContent cshipmentfee;
        cshipmentfee.WriteVarInt(shipmentFee);
        vcc.push_back(make_pair(CC_PRODUCT_SHIPMENTFEE,cshipmentfee));
    }
    if(recipient.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_PAYTO,string(recipient.begin(),recipient.end())));
    if(!icon.IsEmpty())
        vcc.push_back(make_pair(CC_PRODUCT_ICON,icon.Serialize())); 
    if(intro.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_INTRO,intro));
    if(nExpireTime>0)
    {
        CContent cexpiretime;
        cexpiretime.WriteVarInt(nExpireTime);
        vcc.push_back(make_pair(CC_PRODUCT_EXPIRETIME,cexpiretime));
    }
    for(unsigned int j=0;j<vTag.size();j++)    
        vcc.push_back(make_pair(CC_TAG,vTag[j]));    
    CContent ctt;
    ctt.EncodeP(CC_PRODUCT_P,vcc); 
    return ctt;
}
Value CPayment::ToJson(bool fLinkOnly)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("type",Value(GetCcName(ccPaymentType))));
    if(ccPyamentType==CC_PAYMENT_TYPE_PRODUCT)
    {
        obj.push_back(Pair("productid",Value(productID)));
        if(!linkPayTo.IsEmpty())
            obj.push_back(Pair("productlink",Value(linkPayTo.ToString())));
    }
    obj.push_back(Pair("price",_ValueFromAmount(price)));
    if(shipmentFee!=-1)
        obj.push_back(Pair("shipmentfee",_ValueFromAmount(shipmentFee)));    
    if(recipient.size()>0)
    {
        string strID;
        ScriptPubKeyToString(recipient, strID);
        obj.push_back(Pair("recipient",Value(strID)));    
    }    
    if(strMemo.size()>0)
        obj.push_back(Pair("memo",Value(strMemo)));    
    return Value(obj);
}
string CPayment::ToJsonString(bool fLinkOnly )const
{
    return write_string(ToJson(fLinkOnly), false);
}
bool CPayment::SetJson(const Object& obj,string& strError)
{
    Value tmp = find_value(obj, "type");
    if (tmp.type() != str_type)
    {            
        strError="invalid pyament type";
        return false;
    }     
    ccPaymentType=GetCcValue(tmp.get_str());
    switch (ccPaymentType)
    {
        case  CC_PAYMENT_TYPE_PRODUCT:
        {
             tmp = find_value(obj, "productid");
            if (tmp.type() != str_type)
            {            
                strError="invalid product id";
                return false;
            }     
            productID=tmp.get_str();
            LogPrintf("CProduct::SetJson product id %s\n",productID);
            tmp = find_value(obj, "productlink");
            if (tmp.type() == str_type)
            {     
                linkPayTo=tmp.get_str();
                LogPrintf("CProduct::SetJson productlink %s\n",linkPayTo);
            }
            break;
        }
        default://TODO other payment types
            break;
    }
    
    tmp = find_value(obj, "price");    
    try{
        price=_AmountFromValue(tmp);
    }
    catch (Object& objError)
    {
        strError="price is not valid format";        
        return false;
    }
    if(price<0)
    {
        strError="negative price";        
        price=0;
        return false;
    }    
    LogPrintf("CProduct::SetJson product price %i\n",price);
    tmp = find_value(obj, "shipmentfee");
    if (tmp.type() != null_type)
    { 
        try{
            shipmentFee=_AmountFromValue(tmp);
        }
        catch (Object& objError)
        {
            strError="shipment fee is not valid fromat";        
            return false;
        }        
        LogPrintf("CProduct::SetJson product shipment fee %i\n",shipmentFee);
    }            
    tmp = find_value(obj, "recipient");
    if (tmp.type() != null_type||tmp.type()!=str_type)
    {
            strError="recipient type error";
            return false;
    }             
    if(!StringToScriptPubKey(tmp.get_str(),recipient)){
        strError="scriptPubKey is not valid format";
        return false;
    }        
    tmp = find_value(obj, "memo");
    if (tmp.type() == str_type)
    {     
        strMemo=tmp.get_str();
        LogPrintf("CProduct::SetJson memo %s\n",strMemo);
    }
    LogPrintf("CProduct::SetJson recipient %s\n", recipient.ToString());
     
    return true;
}

bool CPayment::SetContent(const CContent content)
{
    LogPrintf("CPayment::SetContent\n");
    std::vector<std::pair<int, string> > vDecoded0;
    content.Decode(vDecoded0);   
    if(vDecoded0[0].first!=CC_PAYMENT_P)
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    CContent(vDecoded0[0].second).Decode(vDecoded);   
    int cc;
    string str;    
    for(unsigned int i=0;i<vDecoded.size();i++)
    {   
        cc=vDecoded[i].first;
        str=vDecoded[i].second;
        //LogPrintf("CProduct::SetContent cc:%i,str:%s \n",cc,str);
        if(cc==CC_PRODUCT_ID)
        {
            productID=str; 
            //LogPrintf("CProduct::SetContent id:%s \n",id);
        }        
        else if(cc==CC_PRODUCT_PRICE)
        {
            CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong price format\n");
                return false;            
            }
            price=CAmount(u);
        }
        else if(cc==CC_PRODUCT_SHIPMENTFEE)
        {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong shipmentfee format\n");            
                return false;            
            }
            shipmentFee=CAmount(u);
        }
         else if(cc==CC_PRODUCT_PAYTO)
         {
             recipient=CScript((unsigned char*)str.c_str(),(unsigned char*)(str.c_str()+str.size()));             
             LogPrintf("CProduct::SetContent recipient str %s script %s \n",HexStr(str.begin(),str.end()),recipient.ToString());            
                 
         } 
         else if (cc==CC_QUANTITY)
         {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong quantity format\n");
                return false;            
            }
            nQuantity=CAmount(u);
         }
        else if (cc==CC_QUANTITY)
         {
            
        }
    }
    LogPrintf("CProduct::SetContent done\n");
    return IsValid();
}
CContent CPayment::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_PRODUCT_ID,id));
    vcc.push_back(make_pair(CC_PRODUCT_NAME,name));
    CContent cPrice;
    cPrice.WriteVarInt(price);
    vcc.push_back(make_pair(CC_PRODUCT_PRICE,cPrice));
    if(shipmentFee!=-1)
    {
        CContent cshipmentfee;
        cshipmentfee.WriteVarInt(shipmentFee);
        vcc.push_back(make_pair(CC_PRODUCT_SHIPMENTFEE,cshipmentfee));
    }
    if(recipient.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_PAYTO,string(recipient.begin(),recipient.end())));
    if(!icon.IsEmpty())
        vcc.push_back(make_pair(CC_PRODUCT_ICON,icon.Serialize())); 
    if(intro.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_INTRO,intro));
    if(nExpireTime>0)
    {
        CContent cexpiretime;
        cexpiretime.WriteVarInt(nExpireTime);
        vcc.push_back(make_pair(CC_PRODUCT_EXPIRETIME,cexpiretime));
    }
    for(unsigned int j=0;j<vTag.size();j++)    
        vcc.push_back(make_pair(CC_TAG,vTag[j]));    
    CContent ctt;
    ctt.EncodeP(CC_PRODUCT_P,vcc); 
    return ctt;
}
