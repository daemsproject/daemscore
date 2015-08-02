// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BROWSER_DOMAIN_H
#define BROWSER_DOMAIN_H

#include "compressor.h"
#include "serialize.h"
#include "uint256.h"
#include "undo.h"
#include "script/script.h"
#include "chain.h"
#include "ccc/link.h"
#include "ccc/content.h"
#include <assert.h>
#include <stdint.h>

#include <boost/foreach.hpp>
//#include <boost/unordered_map.hpp>
using namespace std;
enum DomainGroup
{
    DOMAIN_10000=10000,
    DOMAIN_100=100,
    DOMAIN_ALL=10100
};
class CDomain
{
public:
    int nDomainGroup; 
    std::string strDomain;
    CScript owner;
    CContent redirect;    
    CScript redirectID;//maybe null
    int redirectType;
    string redirectTo;
    vector<unsigned char> forwardsig;
    uint32_t nExpireTime;
    std::string strAlias;
    std::string strIntro;
    CLink iconLink;
    std::vector<std::string> vTags;
    std::vector<CLink> vDirectHistory;
    CDomain()    {        Clear();    };
    CDomain(char** sqlItem,int nOffset=0);    
    bool SetContent(const CContent content,const CScript ownerIn,bool& fRegister,bool& fForward);    
    //bool Update(const CContent content);
    void Clear();
    char* GetInsertSql()const;
    char* GetUpdateSql()const;
    char** GetSqlItems()const;
    bool IsLevel2() const;
    json_spirit::Value ToJson()const;
    CLink GetLastRedirectLink()const;
    
    ADD_SERIALIZE_METHODS;   

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {        
        READWRITE(strDomain);
        READWRITE(owner);
        READWRITE(redirectType);
        READWRITE(redirectTo);
        READWRITE(forwardsig);
        READWRITE(nExpireTime);
        READWRITE(strAlias);
        READWRITE(strIntro);
        READWRITE(iconLink);
        READWRITE(vTags);
        READWRITE(vDirectHistory);
    }
    
    
};
int GetDomainGroup(const string strDomain);
bool IsLevel2Domain(const string strDomain);
std::string GetLevel1Domain(const string strDomain);
bool IsValidDomainFormat(const string strDomain);
/** Abstract view on the open CDomain dataset. */
class CDomainView
{
public:    
    virtual bool GetForward(const int nExtension,const std::string strDomainName,CContent& forward);    
    virtual bool GetDomainByForward(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI=true); 
    //virtual bool GetDomainNameByForward(const CScript scriptPubKey,std::vector<string> &vDomainName);    
    virtual bool GetDomainByName(const std::string strDomainName,CDomain &domain);    
    virtual bool GetDomainByTags(const std::vector<std::string> vTag,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
    virtual bool GetDomainByAlias(const std::string strAlias,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
    virtual bool GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
    virtual bool GetDomainNamesToExpire(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpireIn=3600*24,bool FSupportFAI=true);    
    virtual bool GetDomainNamesExpired(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpiredFor=3600*24,bool FSupportFAI=true);    

    
    virtual bool BatchWrite(const std::vector<CDomain> &vDomain);
    virtual bool Write(const CDomain &domain);
    //! As we use CDomainView polymorphically, have a virtual destructor
    virtual ~CDomainView() {}
    
};

class CDomainViewBacked : public CDomainView
{
protected:
    CDomainView *base;
public:
   CDomainViewBacked(CDomainView *viewIn) : base(viewIn) { }; 
   
};





#endif // BROWSER_DOMAIN_H
