/* 
 * File:   settings.cpp
 * Author: 
 * 
 * Created on July 26, 2015, 8:37 PM
 */

#include "settings.h"
#include "util.h"
#include "contentutil.h"
#include "domain.h"
#include "main.h"
#include "filepackage.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
using namespace std;
using namespace json_spirit;
using namespace boost;
CSettings::CSettings()
{
    //load default settings    
    nServiceFlags=0x00000009;
    language="en";
    mapPageDomain=mapDefaultPageDomain;
    
}
bool CSettings::LoadSettings()
{
    filesystem::path fpFile=GetDataDir() / "appdata" / "settings.json";
    if(!FileExists(fpFile.string()))
        return SaveSettings();
    Value val;
    if(!ReadFileToJson(fpFile.string(),val)||!(val.type() == obj_type))       
    {
         SaveSettings();
        return false;
    }
    Object& obj= val.get_obj();
//    json_spirit::Value& val1 = find_value(obj, "servicedomains");
//    if (val1.type!=obj_type)
//    {
//        LogPrintf("LoadSettings %s:  servicedomains fail \n",);
//       SaveSettings();
//        return false;
//    }
//    Object& obj1 = val1.get_obj();
//    BOOST_FOREACH(PAIRTYPE(int,string)& pair, obj1)
//    {
//        if(pair.first>=0&&pair.first<=31&&IsValidDomain(pair.second))                
//            mapServiceDomain[pair.first]=pair.second;
//    }
    json_spirit::Value val2 = find_value(obj, "pagedomains");
    if (val2.type()!=obj_type)
    {
        LogPrintf("LoadSettings:  pagedomains fail \n");
       SaveSettings();
        return false;
    }
    Object& obj2 = val.get_obj();
    //BOOST_FOREACH(Object& pair, obj2)
    for(unsigned int i=0;i<obj2.size();i++)
    {
        //pair<string,Value> pair=obj2[i];
        int name=atoi(obj2[i].name_);
        string str=obj2[i].value_.get_str();
        if(name>=1&&name<=11&&IsValidDomainFormat(str))
        {
            mapPageDomain[name]=str;
            CLink link;
            if(GetDomainLink(str,link)&&link!=mapPageLink[name])
            {
                mapPageLink[name]=link;                
                CFilePackage(link).InstallPackage(mapPageNames[name]);
            }
            
            
        }
    }
    json_spirit::Value val3 = find_value(obj, "serviceflags");
    if (val3.type()!=int_type)
    {
        LogPrintf("LoadSettings:  serviceflags fail \n");
       SaveSettings();
        return false;
    }
    nServiceFlags=val3.get_uint64();
    json_spirit::Value val4 = find_value(obj, "language");
    if (val4.type()!=str_type)
    {
        LogPrintf("LoadSettings:  language fail \n");
       SaveSettings();
        return false;
    }
    language=val4.get_str();
    return true;
   
}

bool CSettings::SaveSettings()
{
    filesystem::path fpFile=GetDataDir() / "appdata" / "settings.json";
    return WriteJsonToFile(ToJson(),fpFile.string());
}
Value CSettings::ToJson()
{
    Object obj;
    obj.push_back(Pair("language",language));
    obj.push_back(Pair("serviceflags",nServiceFlags));
//    Object obj1;
//    BOOST_FOREACH(PAIRTYPE(int,string)& pair, mapServiceDomain)
//        obj1.push_back(Pair(mapServiceNames[pair.first],pair.second));
//    obj.push_back(Pair("servicedomain",obj1));
    Object obj2;
    BOOST_FOREACH(PAIRTYPE(const int,string)& pair, mapPageDomain)
        obj2.push_back(Pair(mapPageNames[pair.first],pair.second));
    obj.push_back(Pair("pagedomain",obj2));
    return Value(obj);
}
bool CSettings::GetSetting(const string settingType,const string key,string& value)
{
//    if(settingType=="servicedomain")
//    {
//        map<int,string>::iterator it=find(mapServiceNames,key);
//        if(it!=mapServiceNames.end())
//        {
//            value=mapServiceDomain[it->first];
//            return true;
//        }
//    }
//    else
    if(settingType=="pagedomain")
    {
        BOOST_FOREACH(PAIRTYPE(const int,string)& pair,mapPageNames)
            if(pair.second==key)
        {
            value=mapPageDomain[pair.first];
            return true;
        }
    }
    else if(settingType=="language")
    {
        value=language;
        return true;
    }
    else if(settingType=="serviceflages")
    {
        BOOST_FOREACH(PAIRTYPE(const int,string)& pair,mapServiceNames)
            if(pair.second==key)        
        {
            value=((nServiceFlags>>pair.first)&1)?"true":"false";
            return true;
        }
    }
    return false;
}
 bool CSettings::ChangeSetting(const string settingType,const string key,const string& value)
{
//    if(settingType=="servicedomain"&&IsValidDomain(value))
//    {
//        map<int,string>::iterator it=find(mapServiceNames,key);
//        if(it!=mapServiceNames.end())
//        {
//            mapServiceDomain[it->first]=value;
//            return true;
//        }
//    }
//    else
        if(settingType=="pagedomain"&&IsValidDomainFormat(value))
    {
        BOOST_FOREACH(PAIRTYPE(const int,string)& pair,mapPageNames)
            if(pair.second==key)
            {
                mapPageDomain[pair.first]=value;
            
                return SaveSettings();
            }
    }
    else if(settingType=="language")
    {
        language=value;
        return SaveSettings();
    }
    else if(settingType=="serviceflages")
    {
       BOOST_FOREACH(PAIRTYPE(const int,string)& pair,mapServiceNames)
            if(pair.second==key)        
            {
                uint64_t mask=0xffffffff-(1<<(pair.first));             
                
                nServiceFlags&=mask;
                if(value=="true")
                    nServiceFlags |=1<<pair.first;
                return SaveSettings();
            }
    }
    return false;
}
