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
#include <stdlib.h>
using namespace std;
//CDomain::CDomain(char** sqlItem,int nOffset)
//{
//    
//    LogPrintf("CDomain CDomain(sql)size  %i\n",sizeof sqlItem); 
////    LogPrintf("CDomain CDomain(sql)1 %s\n",sqlItem[nOffset+1]); 
////    LogPrintf("CDomain CDomain(sql)11 %s\n",sqlItem[nOffset+11]); 
//    
//    //if(sqlItem.size()<nOffset+12)
//    //    return;
//    nOffset+=12;
////    LogPrintf("CDomain CDomain(sql)9 %s\n",sqlItem[nOffset+9]); 
////    LogPrintf("CDomain CDomain(sql)10 %s\n",sqlItem[nOffset+10]); 
////    LogPrintf("CDomain CDomain(sql)11 %s\n",sqlItem[nOffset+11]); 
//    strDomain=sqlItem[nOffset+0];
//    LogPrintf("CDomain CDomain() strDomain %s \n",strDomain); 
//    nDomainGroup=GetDomainGroup(strDomain);
//    //strDomain.append(nDomainGroup==DOMAIN_EXTENSION_F?".f":".fai");
//    nExpireTime=atoi(sqlItem[nOffset+1]);
//    LogPrintf("CDomain CDomain() expiretime %i \n",nExpireTime); 
//    owner=CScript((unsigned char*)sqlItem[nOffset+2],(unsigned char*)(sqlItem[nOffset+2]+strlen(sqlItem[nOffset+2])));
//    //owner=CScript(ParseHex(sqlItem[nOffset+2]));
//    LogPrintf("CDomain CDomain() owner %i,%s %s\n",strlen(sqlItem[nOffset+2]),HexStr(owner.begin(),owner.end()),owner.ToString());     
//    string str(sqlItem[nOffset+2]);    
//    LogPrintf("CDomain CDomain() owner %i,%s \n",str.size(),HexStr(str.begin(),str.begin()+35));     
//    redirectType=atoi(sqlItem[nOffset+3]);
//    LogPrintf("CDomain CDomain() redirectType %i \n",redirectType); 
//    //std::vector<unsigned char> vuc=ParseHex(sqlItem[nOffset+4]);
//    //redirectTo=string(vuc.begin(),vuc.end());
//    redirectTo=sqlItem[nOffset+4];
//    LogPrintf("CDomain CDomain() redirectTo %s \n",HexStr(redirectTo.begin(),redirectTo.end()));   
////    if(redirectType==CC_LINK_TYPE_SCRIPTPUBKEY)
////        redirectID=CScript(redirectTo);
////    redirect.EncodeUnit(redirectType,redirectTo);    
//    if(redirectType>=0&&redirectTo.size()>0)
//        redirect.EncodeUnit(redirectType,redirectTo);
//    strAlias=sqlItem[nOffset+5];
//    LogPrintf("CDomain CDomain() strAlias %s \n",strAlias); 
//    LogPrintf("CDomain CDomain() iconlink len %i \n",strlen(sqlItem[nOffset+6])); 
//    if(strlen(sqlItem[nOffset+6])>0)
//    {
//        CDataStream s(sqlItem[nOffset+6],sqlItem[nOffset+6]+strlen(sqlItem[nOffset+6]),0,0);
//        //vuc=sqlItem[nOffset+6];
//        //CDataStream s(vuc,0,0);
//        iconLink.Unserialize(s,0,0);
//        //iconLink.Unserialize(sqlItem[nOffset+6],0,0);
//    }
//    LogPrintf("CDomain() iconLink %s \n",iconLink.ToString()); 
//    strIntro=sqlItem[nOffset+7];       
//    LogPrintf("CDomain() strIntro %s \n",strIntro); 
//    //vuc=ParseHex(sqlItem[nOffset+8]);
//    //vuc.push_back('\0');
//    
//    //int len=(int)(vuc.size()/8);
//    int len=(int)(strlen(sqlItem[nOffset+8])/8);
//    if(len>0)
//    {
//        for (int i=0;i<len;i++)
//        {
//        //char chlink[8] ;
//        //strncpy(chlink,sqlItem[nOffset+8]+8*i,8);
//        
//            CLink link;        
//        //CDataStream s((char*)&vuc.front()+8*i,(char*)&vuc.front()+8*(i+1),0,0);
//            CDataStream s(sqlItem[nOffset+8]+8*i,sqlItem[nOffset+8]+8*(i+1),0,0);
//        
//            link.Unserialize(s,0,0);
//        //link.Unserialize(chlink,0,0);
//            vDirectHistory.push_back(link);
//        
//        }
//    }
//    //vDirectHistory.Unserialize(sqlItem[nOffset+8],0,0);
//    LogPrintf("CDomain() vDirectHistory %i \n",vDirectHistory.size()); 
//    for(int i=9;i<12;i++)
//    {
//        LogPrintf("CDomain() vTags %i %i\n",i,strlen(sqlItem[nOffset+i])); 
//       if(strlen(sqlItem[nOffset+i])>0) 
//       {
//            string tag=sqlItem[nOffset+i];        
//            vTags.push_back(tag);
//       }
//       //LogPrintf("CDomain() vTags  %i \n",i); 
//    }    
//    LogPrintf("CDomain() vTags size %i \n",vTags.size()); 
//}
bool CDomain::SetContent(const CContent content,const CScript ownerIn,bool& fRegister,bool& fForward)
{
    LogPrintf("SetContent \n"); 
    fRegister=false;
    fForward=false;
    std::vector<std::pair<int, string> > vDecoded;
    CContent c=content;
    c.Decode(vDecoded);
    LogPrintf("SetContent vDecoded %i\n",vDecoded.size());     
    bool fHasDomain=false;
    //CDomain tmpDomain;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
        string str=vDecoded[i].second;
        if(vDecoded[i].first==CC_DOMAIN)
        {
            if(strDomain==str)
            {
                LogPrintf("SetContent domain found %s\n",str);    
                fHasDomain=true;
                break;
            }
            if(strDomain!="")
                return false;
            if(str.size()<3||str.size()>64)
                return false;
            nDomainGroup=GetDomainGroup(str);
            if(nDomainGroup==0)
                return false;
                    
            LogPrintf("SetContent nDomainGroup %i,domain %s\n",nDomainGroup,str); 
            fHasDomain=true;
            strDomain=str;//.substr(0,str.size()-nDomainGroup=DOMAIN_EXTENSION_F?2:4);
        }
    }
    if(!fHasDomain)
        return false;
    if(ownerIn!=CScript())
    {
        owner=ownerIn;
        LogPrintf("SetContent owner %s\n",owner.ToString()); 
    }
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
        string str=vDecoded[i].second;
        vector<unsigned char>vuc;
        switch (vDecoded[i].first)
        {
            LogPrintf("SetContent cc code %i\n",vDecoded[i].first); 
            case CC_DOMAIN_FORWARD_P:     
                LogPrintf("SetContent forward\n"); 
                if(CContent(str).DecodeLink(redirectType,redirectTo))
                    fForward=true;
                LogPrintf("SetContent forward %i %s\n",redirectType,HexStr(redirectTo.begin(),redirectTo.end()));
                break;
            case CC_DOMAIN_INFO_P:
                LogPrintf("SetContent info\n"); 
                CContent(str).DecodeDomainInfo(strAlias,strIntro,iconLink,vTags);    
                LogPrintf("SetContent info\n");
                break;
            case CC_DOMAIN_TRANSFER:
                LogPrintf("SetContent CC_TRANSFER %s\n",HexStr(str));
                //fTransfer=true;
                if(IsLevel2())//no transfer for level2
                    break;                
                //vuc.resize(str.size());
                //vuc.assign(str.begin(),str.end());
                if(str.size()>35)
                    break;
                owner.resize(str.size());
                owner.assign(str.begin(),str.end());
                LogPrintf("SetContent CC_TRANSFER owner %s,%s\n",HexStr(owner.begin(),owner.end()),owner.ToString());
                break;
            case CC_DOMAIN_REG:
                fRegister=true;
                LogPrintf("SetContent CC_DOMAIN_REGISTER\n");
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
}
//char* CDomain::GetInsertSql()const
//{
//    LogPrintf("GetInsertSql\n");
//    const char* insertstatement="INSERT INTO %s VALUES ('%s',%i,x'%s',%i,x'%s','%s',x'%s','%s',x'%s','%s','%s','%s');";
//    const char* tableName=nDomainGroup==DOMAIN_10000?"domainf":"domainfai";
//    char sql[2000];  
//    char** ch=GetSqlItems();
//    //
//    
//
//    sprintf(sql,insertstatement,tableName,ch[0],nExpireTime,ch[2],redirectType,ch[4],ch[5],ch[6],ch[7],ch[8],ch[9],ch[10],ch[11]);
//    LogPrintf("GetInsertSql12 %s\n",sql);
//    return sql;
//}
//char* CDomain::GetUpdateSql()const
//{
//LogPrintf("GetUpdateSql\n");
//    const char* tableName=(nDomainGroup==DOMAIN_10000?"domainf":"domainfai");
//    LogPrintf("GetInsertSql 10%s \n",tableName);
//    const char* updateStateMent="UPDATE %s SET expiredate =%i, owner= x'%s', redirecttype=%i,redirrectto =x'%s', alias='%s', icon=x'%s',intro='%s',redirecthsitory=x'%s',tag1='%s',tag2='%s',tag3='%s' WHERE domainname = '%s';";
//    char sql[2000];    
////    char* ch[12];
////    LogPrintf("GetInsertSql4 owner %s\n",HexStr(domain.owner.begin(),domain.owner.end()));
////    LogPrintf("GetInsertSql4 redirectTo %s\n",HexStr(domain.redirectTo.begin(),domain.redirectTo.end()));    
////    char* icon;
////    if(!domain.iconLink.IsEmpty())
////    {
////        CDataStream s(0,0);
////        s<<domain.iconLink;   
////        icon=new char[s.size()];
////        s.read(icon,s.size());   
////    }
////    else
////        icon="";
////    LogPrintf("GetInsertSql4 icon %s\n",HexStr(icon,icon+strlen(icon)));
////    char* history;
////    if(domain.vDirectHistory.size()>0)
////    {
////        CDataStream ss(0,0);
////        ss<<domain.vDirectHistory;
////        char history[ss.size()];
////        ss.read(history,ss.size());
////    }
////    else
////    {
////        history="";
////    }    
////    LogPrintf("GetInsertSql4 history %s\n",HexStr(history,history+strlen(history)));    
////    char* tag1="";
////    if (domain.vTags.size()>0)
////        tag1=(char*)domain.vTags[0].data();
////    char* tag2="";
////    if (domain.vTags.size()>1)
////        tag2=(char*)domain.vTags[1].data();
////    char* tag3="";
////    if (domain.vTags.size()>2)
////        tag3=(char*)domain.vTags[2].data(); 
////    LogPrintf("GetInsertSql4 tags %s\n",tag1,tag2,tag3);    
////    sprintf(sql,,
////            tableName,domain.nExpireTime,HexStr(domain.owner.begin(),domain.owner.end()),
////            domain.redirectType,HexStr(domain.redirectTo.begin(),domain.redirectTo.end()),domain.strAlias.c_str(),
////            HexStr(icon,icon+strlen(icon)),domain.strIntro.c_str(),HexStr(history,history+strlen(history)),
////            tag1,tag2,tag3,(char*)domain.strDomain.data());
//    char **ch=GetSqlItems();
//    sprintf(sql,updateStateMent,tableName,nExpireTime,ch[2],redirectType,ch[4],ch[5],ch[6],ch[7],ch[8],ch[9],ch[10],ch[11],ch[0]);
//    LogPrintf("GetInsertSql4 sql %s\n",sql);
//    return sql;
//}
//char** CDomain::GetSqlItems()const
//{
//    char* ch[12];
//    LogPrintf("GetInsertSql1\n");
//    ch[0]=(char*)strDomain.data();
//    LogPrintf("GetInsertSql domain %s\n",ch[0]);
//    LogPrintf("GetInsertSql2 expire time %i\n",nExpireTime);
//    string hexstr=HexStr(owner.begin(),owner.end());
//    ch[2]=(char*)hexstr.data();
//    LogPrintf("GetInsertSql owner %s\n",ch[2]);
//    LogPrintf("GetInsertSql redirect type %i \n",redirectType);
//    if(redirectTo.size()==0)
//        ch[4]="";
//    else
//    {
//        hexstr=HexStr(redirectTo.begin(),redirectTo.end());
//        ch[4]=(char*)hexstr.data();
//    }
//    LogPrintf("GetInsertSql redirectTo %s \n",ch[4]);
//    if(strAlias.size()==0)
//        ch[5]="";
//    else
//        ch[5]=(char*)strAlias.data();
//    LogPrintf("GetInsertSql strAlias %s \n",ch[5]);
//    if(iconLink.IsEmpty())
//        ch[6]="";
//    else
//    {        
//        CDataStream s(0,0);
//        s<<iconLink;
//        hexstr=HexStr(s.begin(),s.end());        
//        ch[6]=(char*)hexstr.data();
//    }
//    LogPrintf("GetInsertSql iconLink %s \n",ch[6]);
//    if(strIntro.size()==0)
//        ch[7]="";
//    else
//        ch[7]=(char*)strIntro.data();
//    LogPrintf("GetInsertSql strIntro %s \n",ch[7]);
//    if(vDirectHistory.size()==0)
//        ch[8]="";
//    else
//    {
//        CDataStream ss(0,0);
//        ss<<vDirectHistory;
//    
//        LogPrintf("GetInsertSql81 \n");
//        hexstr=HexStr(ss.begin(),ss.end());        
//        ch[8]=(char*)hexstr.data();
//    }
//    LogPrintf("GetInsertSql83 vDirectHistory %s\n",ch[8]);
//    for(int i=0;i<3;i++)
//    {
//        if(i<vTags.size())
//            ch[9+i]=(char*)vTags[i].data();
//        else
//            ch[9+i]="";
//        LogPrintf("GetInsertSql tag %s\n",ch[9+i]);
//    }    
//    
//    LogPrintf("GetInsertSql 101%s \n",ch[0]);
//    LogPrintf("GetInsertSql 102%i \n",nExpireTime);
//    LogPrintf("GetInsertSql 103%s \n",ch[2]);
//    LogPrintf("GetInsertSql 104%i \n",redirectType);
//    LogPrintf("GetInsertSql 105%s \n",ch[4]);
//    LogPrintf("GetInsertSql 106%s \n",ch[5]);
//    LogPrintf("GetInsertSql 107%s \n",ch[6]);
//    LogPrintf("GetInsertSql 108%s \n",ch[7]);
//    LogPrintf("GetInsertSql 109%s \n",ch[8]);
//    LogPrintf("GetInsertSql 110%s \n",ch[9]);
//    LogPrintf("GetInsertSql 111%s \n",ch[10]);
//    LogPrintf("GetInsertSql 112%s \n",ch[11]);
//    return ch;
//}
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
            LogPrintf("scriptPubKey %s\n",scriptPubKey.ToString());
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
        default:
            break;
    }
    //obj.push_back(json_spirit::Pair("forward",redirect.ToJson()));
    obj.push_back(json_spirit::Pair("expireTime",(uint64_t)(nExpireTime)));
    obj.push_back(json_spirit::Pair("alias",strAlias));
    obj.push_back(json_spirit::Pair("intro",strIntro));
    obj.push_back(json_spirit::Pair("icon",iconLink.ToString(LINK_FORMAT_DEC)));
    Array arrHistory;
    for(unsigned int i=0;i<vDirectHistory.size();i++)
        arrHistory.push_back(Value(vDirectHistory[i].ToString(LINK_FORMAT_DEC)));
    obj.push_back(json_spirit::Pair("forwardHistory",arrHistory));
    Array arrTags;
    for(unsigned int i=0;i<vTags.size();i++)
        arrTags.push_back(Value(vTags[i]));
    obj.push_back(json_spirit::Pair("tags",arrTags));    
    return Value(obj);
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
bool IsValidDomainFormat(const string strDomain){
    if(strDomain.find(".")==0)
        return false;
    if(strDomain.find("..")!=strDomain.npos)
        return false;
    if(strDomain.find("@")!=strDomain.npos)
        return false;
    if(strDomain.size()<3||strDomain.size()>64)
        return false;            
    if(GetDomainGroup(strDomain)==0)
        return false;
    return true;
}
int GetDomainGroup(const string strDomain)
{
    if(strDomain.substr(strDomain.size()-2)==".f")
    {
        if(!IsLevel2Domain(strDomain))
            return DOMAIN_10000;
        else
           return DOMAIN_100;
    }            
    else if(strDomain.size()>4&&strDomain.substr(strDomain.size()-4)==".fai")
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

