/* 
 * File:   settings.h
 * Author: alan
 *
 * Created on July 26, 2015, 8:37 PM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H
using namespace std;
using namespace json_spirit;
enum servicecode
{
    SERVICE_FULLNODE=0,
    SERVICE_NAT=1,
    SERVICE_STUN=2,
    SERVICE_FULLNODEPLUS=3,
    SERVICE_ICQ=4,
    SERVICE_RELAY=5,
    SERVICE_SEARCHENGINE=7,
    SERVICE_MININGPOOL=8,
    SERVICE_OFFCHAIN_MESSAGE=9,
    SERVICE_OFFCHAIN_PRIVATE=10,
    SERVICE_OFFCHAIN_PUBLIC=11,
    SERVICE_MALL=13,
    SERVICE_BANK=14,
    SERVICE_TORRENTFILE=15,
    SERVICE_EMAIL=16,
    SERVICE_NOBLOCKCHAINDATA=30,
    SERVICE_APP=31,
    

};
static std::map<int,std::string> mapServiceNames=boost::assign::map_list_of
(SERVICE_FULLNODE,"full node service")
    (SERVICE_NAT,"NAT peer")
    (SERVICE_STUN,"STUN service")
    (SERVICE_FULLNODEPLUS,"full node plus service")
    (SERVICE_ICQ,"ICQ service")
    (SERVICE_RELAY,"relay service")
    (SERVICE_SEARCHENGINE,"search engine service")
    (SERVICE_MININGPOOL,"mining pool servcie")
    (SERVICE_OFFCHAIN_MESSAGE,"offchain message storage service")
    (SERVICE_OFFCHAIN_PRIVATE,"offchain private storage service")
    (SERVICE_OFFCHAIN_PUBLIC,"offchain public storage service")
    (SERVICE_MALL,"offchain mall service")
    (SERVICE_BANK,"bank service")
    (SERVICE_TORRENTFILE,"torrent file service")
    (SERVICE_EMAIL,"email service")
    (SERVICE_NOBLOCKCHAINDATA,"no block chain data")
    (SERVICE_APP,"app service");
enum pageid
{
    WALLETPAGE_ID=1,
    BROWSERPAGE_ID=2,
    PUBLISHERPAGE_ID=3,
    MESSENGERPAGE_ID=4,
    MINERPAGE_ID=5,
    DOMAINPAGE_ID=6,
    SETTINGPAGE_ID=7,
    SERVICEPAGE_ID=8,
    SHOPPAGE_ID=9,
    TVPAGE_ID=10,
    DOWNLOADERPAGE_ID=11
};
static std::map<int,std::string> mapPageNames=boost::assign::map_list_of
(WALLETPAGE_ID,"wallet")
(BROWSERPAGE_ID,"browser")
(PUBLISHERPAGE_ID,"publisher")
(MESSENGERPAGE_ID,"messenger")
(DOMAINPAGE_ID,"domain")
(SETTINGPAGE_ID,"settings")
(SERVICEPAGE_ID,"service")
(SHOPPAGE_ID,"shop")
(TVPAGE_ID,"tv")
(DOWNLOADERPAGE_ID,"downloader")
;
static std::map<int,std::string> mapDefaultServiceDomain=boost::assign::map_list_of
            (SERVICE_FULLNODE,"")
    (SERVICE_NAT,"")
    (SERVICE_STUN,"stun.f")
    (SERVICE_FULLNODEPLUS,"fullnodeplus.f")
    (SERVICE_ICQ,"icq.f"),
    (SERVICE_RELAY,"relay.f")
    (SERVICE_SEARCHENGINE,"search.f")
    (SERVICE_MININGPOOL,"pool.f"),
    (SERVICE_OFFCHAIN_MESSAGE,"offchainmessage.f")
    (SERVICE_OFFCHAIN_PRIVATE,"offchainprivate.f")
    (SERVICE_OFFCHAIN_PUBLIC,"offchain.f")
    (SERVICE_MALL,"mall.f")
    (SERVICE_BANK,"bank.f")
    (SERVICE_TORRENTFILE,"torrent.f")
    (SERVICE_EMAIL,"email.f"),
    (SERVICE_NOBLOCKCHAINDATA,"")
    (SERVICE_APP,"");

static std::map<int,std::string> mapDefaultPageDomain=boost::assign::map_list_of
            (WALLETPAGE_ID,"wallet.f")
(BROWSERPAGE_ID,"browser.f")
(PUBLISHERPAGE_ID,"publisher.f")
(MESSENGERPAGE_ID,"messenger.f")
(DOMAINPAGE_ID,"domainpage.f")
(SETTINGPAGE_ID,"settings.f")
(SERVICEPAGE_ID,"servicepage.f")
(SHOPPAGE_ID,"shop.f")
(TVPAGE_ID,"tv.f")
(DOWNLOADERPAGE_ID,"downloader.f");


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
#endif	/* SETTINGS_H */

