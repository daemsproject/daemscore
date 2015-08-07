/* 
 * File:   settings.h
 * Author: alan
 *
 * Created on July 26, 2015, 8:37 PM
 */

#ifndef CCCOIN_SETTINGS_H
#define	CCCOIN_SETTINGS_H
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "ccc/cc.h"
#include <string>
#include <vector>
#include <map>
#include <boost/assign.hpp>

//#include "ccc/link.h"
//#include "ccc/content.h"

using namespace std;
using namespace json_spirit;

int GetPageIDByName(std::string pageName);
string GetPageName(int nPageID);
class CLink;

class CSettings {
public:
    //map<int,CServiceSettings> mapServiceSettings;
    //map<int,string> mapServiceDomain;
    map<int,string> mapPageDomain;
    map<int,CLink>mapPageLink;
    uint64_t nServiceFlags;
    string language;
    
    CSettings();
    bool LoadSettings();
    bool SaveSettings();
    Value ToJson();    
    bool GetSetting(const string settingType,const string key,string& value);
    bool ChangeSetting(const string settingType,const string key,const string& value);
private:
};
class CServiceSettings
{
public:
    int nServiceID;
    //int nDomains;
    //map<int,string> mapDomains;
    bool fServerOn;
    bool fClientOn;
//    CServiceSettings(const int nServiceIDIn);
//    bool LoadSettings();
//    bool SaveSettings();
//    Value ToJson();    
//    bool GetSetting(const string settingType,const string key,string& value);
//    bool ChangeSetting(const string settingType,const string key,const string& value);
    
};
#endif	/* CCCOIN_SETTINGS_H */

