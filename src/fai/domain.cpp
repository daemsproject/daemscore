// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "domain.h"
#include "base58.h"
#include "content.h"
#include "streams.h"
#include "utilstrencodings.h"
#include <assert.h>
#include <util.h>
#include "json/json_spirit_writer_template.h"
#include <boost/algorithm/string.hpp>   

using namespace std;
bool CheckDomainName(const string str)
{
    if(str.find("-")==0||str.find(".")==0)
        return false;
    if(str.find("--")!=str.npos)
        return false;
    if(str.find("-.")!=str.npos||str.find(".-")!=str.npos)
        return false;
    if(str.find("..")!=str.npos)
        return false;
    const char* pszBaseDomain = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-";  
    const char* ch=str.c_str();
    for(int i=0;i<(int)str.size();i++)
    {
        const char* v = strchr(pszBaseDomain, *ch);
    
       // LogPrintf("%s ",ch);
        if (v == NULL)
        {
            LogPrintf("check domain failed:%s \n",str);
            return false;    
        }   
         ch++;
    }
    return true;
}
bool CDomain::SetContent(const CContent content,const CScript ownerIn,bool& fRegister,bool& fForward)
{
    string str;
    content.ToJsonString(str);
    
    //LogPrintf("SetContent %s\n",str.c_str()); 
    fRegister=false;
    fForward=false;
    std::vector<std::pair<int, string> > vDecoded;
    CContent c=content;
    if(!c.Decode(vDecoded))
        return false;
    //LogPrintf("SetContent vDecoded %i\n",vDecoded.size());     
    bool fHasDomain=false;
    //CDomain tmpDomain;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
        string str=vDecoded[i].second;
        if(vDecoded[i].first==CC_DOMAIN)
        {
            //LogPrintf("domain SetContent  cc_domain 1\n");    
            
              
            if(str.size()<3||str.size()>64)
                return false;
           // LogPrintf("domain SetContent  cc_domain 3\n");    
            if(!CheckDomainName(str))
                return false;
            //LogPrintf("domain SetContent  cc_domain 4\n");    
            boost::algorithm::to_lower(str);
            if(strDomain==str)
            {
               // LogPrintf("SetContent domain found %s\n",str);    
                fHasDomain=true;
                break;
            }           
            
            if(strDomain!="")//if domain name in is different from existing one, and existing is not null ,reture false
                return false;
            //LogPrintf("domain SetContent  cc_domain 5\n");    
            nDomainGroup=GetDomainGroup(str);
           // LogPrintf("domain SetContent  cc_domain 6\n");    
            if(nDomainGroup==0)
                return false;
                    
           // LogPrintf("SetContent nDomainGroup %i,domain %s\n",nDomainGroup,str); 
            fHasDomain=true;
            strDomain=str;//.substr(0,str.size()-nDomainGroup=DOMAIN_EXTENSION_F?2:4);
        }
    }
    if(!fHasDomain)
        return false;
    if(ownerIn!=CScript())
    {
        owner=ownerIn;
     //   LogPrintf("SetContent owner %s\n",owner.ToString()); 
    }
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
        string str=vDecoded[i].second;
        vector<unsigned char>vuc;
        switch (vDecoded[i].first)
        {
          //  LogPrintf("SetContent cc code %s\n",GetCcName((cctype)vDecoded[i].first)); 
            case CC_DOMAIN_FORWARD_P:     
           //     LogPrintf("SetContent forward\n"); 
                if(str.size()==0)
                {
                    redirectType=CC_NULL;
                    redirectTo="";        
                     fForward=true;
                }
                else if(CContent(str).DecodeDomainForward(redirectType,redirectTo,forwardsig))
                    fForward=true;
           //     LogPrintf("SetContent forward %i %s\n",redirectType,HexStr(redirectTo.begin(),redirectTo.end()));
                break;
            case CC_DOMAIN_INFO_P:
              //  LogPrintf("SetContent info\n"); 
                CContent(str).DecodeDomainInfo(strAlias,strIntro,iconLink,vTags);    
              //  LogPrintf("SetContent info\n");
                break;
            case CC_DOMAIN_TRANSFER:
              //  LogPrintf("SetContent CC_TRANSFER %s\n",HexStr(str));
                //fTransfer=true;
                if(IsLevel2())//no transfer for level2
                    break;                
                //vuc.resize(str.size());
                //vuc.assign(str.begin(),str.end());
                if(str.size()>35)
                    break;
                owner.resize(str.size());
                owner.assign(str.begin(),str.end());
              //  LogPrintf("SetContent CC_TRANSFER owner %s,%s\n",HexStr(owner.begin(),owner.end()),owner.ToString());
                break;
            case CC_DOMAIN_REG:
                fRegister=true;
               // LogPrintf("SetContent CC_DOMAIN_REGISTER\n");
                break;
            default:
                break;
        }
    }          
    return true;
}
void CDomain::Clear()
{
    nDomainGroup=-1;
        redirectType=-1;
        nExpireTime=0;   
        nLockValue=0;
}
bool CDomain::IsEmpty() const{
    return nDomainGroup == -1;
}
bool CDomain::IsLevel2()const
{
    return (strDomain.find(".")!=strDomain.rfind("."));
}
using namespace json_spirit;
json_spirit::Value CDomain::ToJson()const
{
    json_spirit::Object obj;
    obj.push_back(json_spirit::Pair("domain",strDomain));
    obj.push_back(json_spirit::Pair("group",nDomainGroup));
    string str;
    ScriptPubKeyToString(owner,str);
    obj.push_back(json_spirit::Pair("owner",str));
    Object obj1;
    switch (redirectType)
    {
        case CC_LINK_TYPE_SCRIPTPUBKEY:  
        {
            string strID;
            CScript scriptPubKey((unsigned char*)redirectTo.c_str(),(unsigned char*)redirectTo.c_str()+redirectTo.size());
            //LogPrintf("scriptPubKey %s\n",scriptPubKey.ToString());
            if(ScriptPubKeyToString(scriptPubKey,strID))
            {                
                obj1.push_back(json_spirit::Pair("linkType","ID"));            
                obj1.push_back(json_spirit::Pair("target",strID));
                obj.push_back(json_spirit::Pair("forward",obj1));
            }   
            break;
        }
        case CC_LINK_TYPE_BLOCKCHAIN:
        {
            obj1.push_back(json_spirit::Pair("linkType","blockchain"));   
            CLink link;
            link.UnserializeConst(redirectTo);
            obj1.push_back(json_spirit::Pair("target",link.ToString(LINK_FORMAT_DEC)));
            obj.push_back(json_spirit::Pair("forward",obj1));
        }
        break;
        default:
            obj1.push_back(json_spirit::Pair("linkType","other"));
            obj1.push_back(json_spirit::Pair("target",redirectTo));
            obj.push_back(json_spirit::Pair("forward",obj1));
            break;
    }
    //obj.push_back(json_spirit::Pair("forward",redirect.ToJson()));
    obj.push_back(json_spirit::Pair("expireTime",(uint64_t)(nExpireTime)));
    obj.push_back(json_spirit::Pair("lockvalue",nLockValue));
    obj.push_back(json_spirit::Pair("alias",EncodeBase64(strAlias)));
    obj.push_back(json_spirit::Pair("intro",EncodeBase64(strIntro)));
    obj.push_back(json_spirit::Pair("icon",iconLink.ToString(LINK_FORMAT_DEC)));
    Array arrHistory;
    for(unsigned int i=0;i<vDirectHistory.size();i++)
        arrHistory.push_back(Value(vDirectHistory[i].ToString(LINK_FORMAT_DEC)));
    obj.push_back(json_spirit::Pair("forwardHistory",arrHistory));
    Array arrTags;
    for(unsigned int i=0;i<vTags.size();i++)
        arrTags.push_back(Value(EncodeBase64(vTags[i])));
    obj.push_back(json_spirit::Pair("tags",arrTags));    
    return Value(obj);
}
string CDomain::ToJsonString()const
{
    return write_string(ToJson(), false);
}
CLink CDomain::GetLastRedirectLink()const
{
    CLink link;
    if(vDirectHistory.size()>0)    
        link=vDirectHistory[vDirectHistory.size()-1];        
    return link;
}


bool IsLevel2Domain(const string strDomain)
{
    return (strDomain.find(".")!=strDomain.rfind("."));
}
 std::string GetLevel1Domain(const string strDomain)
{
    if (!IsValidDomainFormat(strDomain))
        return "";
    if(!IsLevel2Domain(strDomain))
        return strDomain;
    //LogPrintf("GetLevel1Domain1 %i",strDomain.rfind("."));
    //LogPrintf("GetLevel1Domain2 %i",strDomain.rfind(".",strDomain.rfind(".")-1));
    //LogPrintf("GetLevel1Domain3 %i",strDomain.substr(0,strDomain.rfind(".")).rfind("."));
    //LogPrintf("GetLevel1Domain4 %s",strDomain.substr(strDomain.rfind(".",strDomain.rfind(".")-1)+1));
    return strDomain.substr(strDomain.rfind(".",strDomain.rfind(".")-1)+1);
}
bool IsValidDomainFormat(const string strDomain){//TODO:enable extension
    string str=strDomain.substr(0,strDomain.find("/"));  
    
    if(str.size()<3||str.size()>64)
        return false;
    std::size_t posColon = str.find(URI_COLON);    
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = str.substr(0, posColon);
        if (sn != URI_SCHEME_NAME) {
            LogPrintf("%s: Non-standard link header %s", __func__, sn);
            return false;
        }
        str = str.substr(posColon + 1);
    } 
    if (str.find(":")!=str.npos)
        return false;
    if (str.find("\\")!=str.npos)
        return false;
    if(str.find(".")==0)
        return false;
    if(str.find("..")!=str.npos)
        return false;
    if(str.find("@")!=str.npos)
        return false;                
    if(GetDomainGroup(str)==0)
        return false;
    return true;
}
int GetDomainGroup(const string strDomain)
{
    
    string str=strDomain.substr(0,strDomain.find("/"));
    if(!CheckDomainName(str))
        return 0;
    boost::algorithm::to_lower(str);
    if(str.substr(str.size()-2)==".f")
    {
        if(!IsLevel2Domain(str))
            return DOMAIN_10000;
        else
           return DOMAIN_100;
    }            
    else if(str.size()>4&&str.substr(str.size()-4)==".fai")
        return DOMAIN_100;
    return 0;    
}
bool CDomainView::GetForward(const int nExtension,const std::string strDomainName,CContent& forward){return false;}    
bool CDomainView::GetDomainByForward(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI){return false;}  
    //virtual bool GetDomainNameByForward(const CScript scriptPubKey,std::vector<string> &vDomainName);    
bool CDomainView::GetDomainByName(const std::string strDomainName,CDomain &domain){return false;}     
bool CDomainView::GetDomainByTags(const std::vector<std::string> vTag,std::vector<CDomain> &vDomain,bool FSupportFAI){return false;}     
bool CDomainView::GetDomainByAlias(const std::string strAlias,std::vector<CDomain> &vDomain,bool FSupportFAI){return false;}     
bool CDomainView::GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI){return false;}     
bool CDomainView::GetDomainNamesToExpire(std::vector<CDomain> &vDomain,const int nMax,const uint32_t nExpireIn,bool FSupportFAI){return false;}     
bool CDomainView::GetDomainNamesExpired(std::vector<CDomain> &vDomain,const int nMax,const uint32_t nExpiredFor,bool FSupportFAI){return false;}     
bool CDomainView::BatchWrite(const std::vector<CDomain> &vDomain){return false;}  
bool CDomainView::Write(const CDomain &domain){return false;}  

