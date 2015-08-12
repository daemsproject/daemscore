#include "ccc/link.h"
#include "ccc/cc.h"
#include <string.h>
#include <string>
//#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

//#include <stdio.h>
#include "util.h"
#include "ccc/domain.h"
#include "ccc/content.h"
#include "utilstrencodings.h"
#include "base58.h"
#include "json/json_spirit_writer_template.h"
#include <iterator>
#include <vector>

using namespace boost;
using namespace std;

void CLink::SetEmpty()
{
    nHeight = -1;
    nTx = 0;
    nVout = 0;
}

bool CLink::IsEmpty() const
{
    return (nHeight == -1 && nTx == 0 && nVout == 0);
}

bool CLink::IsValid() const
{
    return nHeight != -1;
}

bool CLink::SetInt(const int nHeightIn, const int nTxIn, const int nVoutIn)
{
    nHeight = nHeightIn;
    nTx = (unsigned short) nTxIn;
    nVout = (unsigned short) nVoutIn;
    return true;
}

bool CLink::SetString(const std::string linkStr)
{
    nHeight = -1;
    nTx = 0;
    nVout = 0;
    // Process Scheme name
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn != URI_SCHEME_NAME) {
            LogPrintf("%s: Non-standard link header %s", __func__, sn);
            return false;
        }
        str = linkStr.substr(posColon + 1);
    } else
        str = linkStr;

    // Process link content
    std::size_t posFirstDot = str.find(URI_SEPERATOR);
    if (posFirstDot == std::string::npos) {
        LogPrintf("CLink %s: Non-standard link %s\n", __func__,linkStr);
        return false;
    }
    std::string nHeightS = str.substr(0, posFirstDot);
    std::size_t posSecondDot = str.find(".", posFirstDot + 1);
    std::string nTxS = (posSecondDot == std::string::npos) ? str.substr(posFirstDot + 1) :
            str.substr(posFirstDot + 1, posSecondDot - posFirstDot - 1);
//    if (nHeightS.substr(0, 1) == URI_HEX_HEADER) {
//        nHeight = HexStringToInt(nHeightS.substr(1));
//        nTx = (unsigned short) HexStringToInt(nTxS);
//        if (posSecondDot == std::string::npos) {
//            nVout = 0;
//        } else {
//            std::string nVoutS = str.substr(posSecondDot + 1);
//            nVout = (unsigned short) HexStringToInt(nVoutS);
//        }
//    } else
//        if (nHeightS.substr(0, 1) == URI_B32_HEADER) {
//        nHeight = DecodeBase32ToInt(nHeightS.substr(1));
//        nTx = (unsigned short) DecodeBase32ToInt(nTxS);
//        if (posSecondDot == std::string::npos) {
//            nVout = 0;
//        } else {
//            std::string nVoutS = str.substr(posSecondDot + 1);
//            nVout = (unsigned short) DecodeBase32ToInt(nVoutS);
//        }
//    } else 
    {
        if (!IsStringInteger(nHeightS)) {
            LogPrintf("%s: Non-standard link (nHeight) \n", __func__);
            return false;
        }
        nHeight = atoi(nHeightS.c_str());
        if (!IsStringInteger(nTxS))
        {
             LogPrintf("%s: Non-standard link (nTx) \n", __func__);
            return false;
        }
        nTx = (unsigned short) atoi(nTxS.c_str());
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            if (!IsStringInteger(nVoutS)) {
                LogPrintf("%s: Non-standard link (nVout)", __func__);
                return false;
            }
            nVout = (unsigned short) atoi(nVoutS.c_str());
        }
    }
    return true;
}

string CLink::Serialize()const
{
    string str;
    WriteVarInt(nHeight, str);
    WriteVarInt(nTx, str);
    WriteVarInt(nVout, str);
    // str.append((char*)&(nHeight), sizeof(nHeight));
    // str.append((char*)&(nTx), sizeof(nTx));
    // str.append((char*)&(nVout), sizeof(nVout)); 
    LogPrintf("CLink::Serialize() %s", HexStr(str.begin(), str.end()));
    return str;
}
int64_t CLink::SerializeInt()const
{
    int64_t nLink;
    nLink=nHeight;
    nLink<<=16;
    nLink|=nTx;
    nLink<<=16;
    nLink|=nVout;
    return nLink;
}
bool CLink::Unserialize(const int64_t nLink)
{
    nVout=nLink&0xffff;
    nTx=nLink>>16&0xffff;
    nHeight=nLink>>32;
    return true;
}
bool CLink::Unserialize(string& str)
{
    //    if(str.size()!=8)
    //        return false;    
    //     nHeight = str[3]<<24|(unsigned char)str[2]<<16|(unsigned char)str[1]<<8|(unsigned char)str[0];
    //    nTx = (unsigned char)str[5]<<8|(unsigned char)str[4];
    //    nVout = (unsigned char)str[7]<<8|(unsigned char)str[6];
    //LogPrintf("CLink::UnSerialize() %s\n", HexStr(str.begin(), str.end()));
    if (!ReadVarInt(str, nHeight))
        return false;
    int n = 0;
    if (!ReadVarInt(str, n))
        return false;
    if (n < 0 || n > 65535)
        return false;
    nTx = (unsigned short) n;
    if (!ReadVarInt(str, n))
        return false;
    if (n < 0 || n > 65535)
        return false;
    nVout = (unsigned short) n;
    //LogPrintf("CLink::UnSerialize() %i,%i,%i \n", nHeight, nTx, nVout);
    return true;
}
bool CLink::UnserializeConst(const string& str)
{
    string strlink=str;
    return Unserialize(strlink);
}
bool CLink::WriteVarInt(const int nIn, string& str) const
{
    int n = nIn;
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
        str.append(tstr, 0, 1);
    } while (len--);
    return true;
}

bool CLink::ReadVarInt(string& str, int& n)const
{
    string::iterator pc = str.begin();
    n = 0;
    unsigned char chData = 0xff;
    int offset = 0;
    while (pc < str.end()) {
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
    str = str.substr(offset);
    return (chData & 0x80) ? false : true;
}

bool CLink::SetString(const vector<unsigned char>& linkVch)
{
    std::string str(linkVch.begin(), linkVch.end());
    return SetString(str);
}

std::string CLink::ToString(const linkformat linkFormat)const
{
    if (IsEmpty())
        return "";
    std::string r = URI_SCHEME_NAME;
    r += URI_COLON;
    std::string nHeightS;
    std::string nTxS;
    std::string nVoutS;
    switch (linkFormat) {
        case LINK_FORMAT_DEC:
            nHeightS = strpatch::to_string(nHeight);
            nTxS = strpatch::to_string(nTx);
            break;
        case LINK_FORMAT_HEX:
            nHeightS = URI_HEX_HEADER;
            nHeightS += IntToHexString(nHeight);
            nTxS = IntToHexString(nTx);
            break;
        case LINK_FORMAT_B32:
            nHeightS = URI_B32_HEADER;
            nHeightS += EncodeBase32(nHeight);
            nTxS = EncodeBase32(nTx);
            break;
    }
    r += nHeightS;
    r += URI_SEPERATOR;
    r += nTxS;
    if (nVout > 0) {
        r += URI_SEPERATOR;
        switch (linkFormat) {
            case LINK_FORMAT_DEC:
                r += strpatch::to_string(nVout);
                break;
            case LINK_FORMAT_HEX:
                r += IntToHexString(nVout);
                break;
            case LINK_FORMAT_B32:
                r += EncodeBase32(nVout);
                break;
        }
    }
    return r;
}

//CLinkUni

bool CLinkUni::SetString(const std::string linkStr)
{
    strLink=linkStr;
    if(SetStringNative(linkStr))
        return true;
    if(SetStringBlockChain(linkStr))
    return true;
    if(SetStringTxidOut(linkStr))
    return true;
    if(SetStringScriptPubKey(linkStr))
    return true;
//    if(strLink.size()>64)
//        return false;
    if(IsValidDomainFormat(linkStr))
    {
        linkType=CC_LINK_TYPE_DOMAIN;
        string str=strDomain.substr(0,strDomain.find("/"));
        strDomain=str;
        strDomainExtension=str.substr(strDomain.find("/"));
        return true;
    }
    else if(IsValidHttpFormat(linkStr))   
    {
        LogPrintf("valid http link \n");
        linkType=CC_LINK_TYPE_HTTP; 
    }
    else if(IsValidFileFormat(linkStr))   
        linkType=CC_LINK_TYPE_FILE; 
    else if(linkStr.find("magnet:")==0)
        linkType=CC_LINK_TYPE_MAGNET; 
    else if(linkStr.find("mailto:")==0)
        linkType=CC_LINK_TYPE_MAILTO; 
    else if(linkStr.find("ftp:")==0)
        linkType=CC_LINK_TYPE_FTP; 
    else
        linkType=CC_LINK_TYPE_UNKNOWN; 
    return true;    
}
bool CLinkUni::SetStringNative(const std::string linkStr)
{
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) 
    { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn == URI_SCHEME_NAME) 
        {//ccc url, to be parsed as app or link.
            str = linkStr.substr(posColon + 1);
        
            for(int i=1;i<=11;i++)            
                if (str==mapPageNames[i])
                {
                    linkType=CC_LINK_TYPE_NATIVE;            
                    return true;
                }
        }
    }
        return false;
}
bool CLinkUni::SetStringBlockChain(const std::string linkStr)
{
    CLink link;
    if(!link.SetString(strLink))           
        return false;           
    nHeight=link.nHeight;
    nTx=link.nTx;
    nVout=link.nVout; 
    linkType=CC_LINK_TYPE_BLOCKCHAIN;
    return true;
}
bool CLinkUni::SetStringTxidOut(const std::string linkStr)
{
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn != URI_SCHEME_NAME) {
            LogPrintf("%s: Non-standard link header %s", __func__, sn);
            return false;
        }
        str = linkStr.substr(posColon + 1);
    } else
        str = linkStr;
    if(str.size()<64)
        return false;
    std::string strTxidHex = str.substr(0, 64);
    if(!IsHex(strTxidHex))
        return false;    
    txid.SetHex(strTxidHex);    
    if(str.size()==64)
    {
        nVout=0;
        linkType=CC_LINK_TYPE_TXIDOUT;
        return true;
    }
    if(str.substr(64,1)!=URI_SEPERATOR)
        return false;
    str=str.substr(65);
    if (!IsStringInteger(str))
    {
                LogPrintf("%s: Non-standard link (nVout)", __func__);
                return false;
            }
    nVout = (unsigned short) atoi(str.c_str());  
    linkType=CC_LINK_TYPE_TXIDOUT;
    return true;
}
bool CLinkUni::SetStringScriptPubKey(const std::string linkStr)
{
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn != URI_SCHEME_NAME) {
            LogPrintf("%s: Non-standard link header %s", __func__, sn);
            return false;
        }
        str = linkStr.substr(posColon + 1);
    } else
        str = linkStr;
    if(!StringToScriptPubKey(str,scriptPubKey))
        return false;
//    if(scriptPubKey.size()>64)
//        return false;
    linkType=CC_LINK_TYPE_SCRIPTPUBKEY;
    return true;
}
std::string CLinkUni::ToString(const linkformat linkFormat)const
{
    if (IsEmpty())
        return "";
    switch (linkType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:
            return ToStringBlockChain(linkFormat);
        case CC_LINK_TYPE_TXIDOUT:
            return ToStringTxidOut();
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            return ToStringScriptPubKey();
        case CC_LINK_TYPE_DOMAIN:
            return strDomain+strDomainExtension;
        default:
            return strLink;            
    }
}
std::string CLinkUni::ToStringBlockChain(const linkformat linkFormat)const
{
    std::string r = URI_SCHEME_NAME;
    r += URI_COLON;
    std::string nHeightS;
    std::string nTxS;
    std::string nVoutS;
    switch (linkFormat) {
        case LINK_FORMAT_DEC:
            nHeightS = strpatch::to_string(nHeight);
            nTxS = strpatch::to_string(nTx);
            break;
        case LINK_FORMAT_HEX:
            nHeightS = URI_HEX_HEADER;
            nHeightS += IntToHexString(nHeight);
            nTxS = IntToHexString(nTx);
            break;
        case LINK_FORMAT_B32:
            nHeightS = URI_B32_HEADER;
            nHeightS += EncodeBase32(nHeight);
            nTxS = EncodeBase32(nTx);
            break;
    }
    r += nHeightS;
    r += URI_SEPERATOR;
    r += nTxS;
    if (nVout > 0) {
        r += URI_SEPERATOR;
        switch (linkFormat) {
            case LINK_FORMAT_DEC:
                r += strpatch::to_string(nVout);
                break;
            case LINK_FORMAT_HEX:
                r += IntToHexString(nVout);
                break;
            case LINK_FORMAT_B32:
                r += EncodeBase32(nVout);
                break;
        }
    }
    return r;
}
std::string CLinkUni::ToStringTxidOut()const
{
    std::string r = URI_SCHEME_NAME;
    r += URI_COLON; 
    r += txid.GetHex();   
    if (nVout > 0) {
        r += URI_SEPERATOR;       
        r += strpatch::to_string(nVout);                
    }
    return r;
}
std::string CLinkUni::ToStringScriptPubKey()const
{    
    string str;
     ScriptPubKeyToString(scriptPubKey,str);
     return str;
}
bool CLinkUni::SetContent(const string& str)
{
    LogPrintf("CLinkUni SetContent \n");
    std::vector<std::pair<int, string> > vDecoded;
    CContent c=str;
    if(!c.Decode(vDecoded))
        return false;
    LogPrintf("SetContent vDecoded %i\n",vDecoded.size()); 
    switch(vDecoded[0].first)
    {
        case  CC_LINK:
        {
            CLink link;
            if(!link.UnserializeConst(vDecoded[0].second))
                return false;
            nHeight=link.nHeight;
            nTx=link.nTx;
            nVout=link.nVout;
            linkType=CC_LINK_TYPE_BLOCKCHAIN;
            return true;
        }
        case  CC_LINK_P:
        {
            std::vector<std::pair<int, string> > vDecoded2;
            if(!CContent(vDecoded[0].second).Decode(vDecoded2))
                return false;
            bool fHasType=false;
            for(unsigned int i=0;i<vDecoded2.size();i++)
            {
                if(vDecoded[i].first>=CC_LINK_TYPESTRING&&vDecoded[i].first<=CC_LINK_TYPE_UNKNOWN)
                {
                    fHasType=true;
                    linkType=(cctype)vDecoded[i].first;
                    break;
                }
            }
            if(!fHasType)
                return false;
            bool fHasLink=false;
            for(unsigned int i=0;i<vDecoded2.size();i++)
            {
                string str1=vDecoded2[i].second;
                switch (vDecoded2[i].first)
                {
                    case CC_LINK:
                    {
                        strLink=str1;
                        switch ((int)linkType)
                        {
                            case CC_LINK_TYPE_BLOCKCHAIN:
                            {
                                CLink link;
                                if(!link.UnserializeConst(vDecoded[0].second))
                                    return false;
                                nHeight=link.nHeight;
                                nTx=link.nTx;
                                nVout=link.nVout;  
                                break;
                            }
                            case CC_LINK_TYPE_TXIDOUT:
                            {
                                if(str1.size()<33)
                                    return false;
                                memcpy(txid.begin(),(unsigned char*)&str1[0],32);
                                str1=str1.substr(32);
                                uint64_t n;
                                if(!DecodeVarInt(str1,n))
                                    return false;
                                nVout=(unsigned short)n;
                                break;
                            }
                            case CC_LINK_TYPE_SCRIPTPUBKEY:
                                scriptPubKey.assign(str1.begin(),str1.end());
                                break;   
                            case CC_LINK_TYPE_DOMAIN:
                                strDomain=str1.substr(0,str1.find("/"));
                                strDomainExtension=str1.substr(str1.find("/"));
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case CC_NAME:
                        strLinkName=str1;
                        break;
                    case CC_TAG:
                        vTags.push_back(str1);
                        break;
                    case CC_TAG_P:
                        CContent(str1).GetDataByCC(CC_TAG,vTags,true,true);
                        break;
                    default:
                        break;
                }
            }
            return fHasLink;
        }        
        default:
            return false;
    }
}
    
string CLinkUni::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(linkType,""));    
    switch ((int)linkType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:
        {
            CLink link(nHeight,nTx,nVout);
            vcc.push_back(make_pair(CC_LINK,link.Serialize()));  
        }
            break;
        case CC_LINK_TYPE_TXIDOUT:
        {
            string str(txid.begin(),txid.end());                                
            EncodeVarInt((uint64_t)nVout,str);
            vcc.push_back(make_pair(CC_LINK,str));  
        }
            break;                        
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            vcc.push_back(make_pair(CC_LINK,string(scriptPubKey.begin(),scriptPubKey.end())));  
            break;
        case CC_LINK_TYPE_DOMAIN:
            vcc.push_back(make_pair(CC_LINK,strDomain)); 
            break;
        default:
            return strLink; 
    }
    if(strLinkName.size()>0)
       vcc.push_back(make_pair(CC_NAME,strLinkName));   
    if(vTags.size()>0)
            vcc.push_back(make_pair(CC_TAG_P,EncodeContentUnitArray(CC_TAG,vTags))); 
    CContent ctt;
    ctt.EncodeP(CC_LINK_P,vcc); 
    return ctt;
} 
bool CLinkUni::SetJson(const Object& obj,string& strError)
{
   Value tmp = find_value(obj, "linktype");
    if (tmp.type() != str_type)
    {            
        strError="invalid link type";
        return false;
    }     
    linkType=GetCcValue(tmp.get_str());
    LogPrintf("CLinkUni::SetJson linkType %s\n", GetCcName(linkType));
    tmp = find_value(obj, "link");
    if (tmp.type() != str_type)
    {            
        strError="invalid payment items type";
        return false;
    }    
    strLink=tmp.get_str();
    switch ((int)linkType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:              
            if(!SetStringBlockChain(strLink))
            {
                strError="invalid blockchai link format";
                return false;
            }
            break;
        case CC_LINK_TYPE_TXIDOUT:
            if(!SetStringTxidOut(strLink))
               {
                strError="invalid txidout link format";
                return false;
            }
            break;
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            if(!SetStringScriptPubKey(strLink))
                {
                strError="invalid scriptpubkey link format";
                return false;
            }
            break;   
        case CC_LINK_TYPE_DOMAIN:
        {
            if(!IsValidDomainFormat(strLink))
                {
                strError="invalid domain link format";
                return false;
            }
            string str2=strLink.substr(0,strDomain.find("/"));
            strDomain=str2;
            strDomainExtension=str2.substr(strDomain.find("/"));  
        }
            break;
        case CC_LINK_TYPE_HTTP:
            if(!IsValidHttpFormat(strLink))   
            {
                strError="invalid domain link format";
                return false;
            }
        default:
            break;
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
                vTags.push_back(arrTags[j].get_str());
                LogPrintf("linkuni::SetJson tag %s\n", arrTags[j].get_str());
            }
        }
    }
    tmp = find_value(obj, "linkname");
    if (tmp.type() == str_type)    
        strLinkName=tmp.get_str();  
    return true; 
}
Value CLinkUni::ToJson(const linkformat linkFormat)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("linktype",Value(GetCcName(linkType))));
    switch ((int)linkType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:
            obj.push_back(Pair("link",ToStringBlockChain(linkFormat)));
        case CC_LINK_TYPE_TXIDOUT:
            obj.push_back(Pair("link",ToStringTxidOut()));
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            obj.push_back(Pair("link",ToStringScriptPubKey()));
        case CC_LINK_TYPE_DOMAIN:
           obj.push_back(Pair("link",strDomain+strDomainExtension));
        default:
            obj.push_back(Pair("link",strLink));            
    }
    if(strLinkName.size()>0)    
        obj.push_back(Pair("linkname",Value(strLinkName)));    
    
    if(vTags.size()>0)
    {
        Array arr;
        for(unsigned int i=0;i<vTags.size();i++)        
            arr.push_back(vTags[i]);
       
        obj.push_back(Pair("tags",arr)); 
    }
    return Value(obj);
}
string CLinkUni::ToJsonString(const linkformat linkFormat)const
{ 
    return write_string(ToJson(linkFormat), false);
}
void CLinkUni::SetEmpty()
{
    linkType=CC_NULL;
}
bool CLinkUni::IsEmpty() const
{
    return linkType==CC_NULL;
}
bool operator==(const CLinkUni& a, const CLinkUni& b)
{
    if(a.linkType!=b.linkType)
        return false;
    switch ((int)a.linkType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:
            return a.nHeight==b.nHeight&&a.nTx==b.nTx&&a.nVout==b.nVout;
        case CC_LINK_TYPE_TXIDOUT:
            return a.txid==b.txid&&a.nVout==b.nVout;
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            return a.scriptPubKey==b.scriptPubKey;        
        default:
            return a.strLink==b.strLink;
    }
}