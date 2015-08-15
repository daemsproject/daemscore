#ifndef FAICOIN_SETTINGS_H
#define	FAICOIN_SETTINGS_H
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "fai/cc.h"
#include <string>
#include <vector>
#include <map>
#include <boost/assign.hpp>

//#include "fai/link.h"
//#include "fai/content.h"

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
#endif	/* FAICOIN_SETTINGS_H */

