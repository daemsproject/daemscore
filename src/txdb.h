// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_TXDB_H
#define BITCOIN_TXDB_H

#include "leveldbwrapper.h"
#include "ccc/sqlitewrapper.h"
#include "main.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CCoins;
class uint256;

//! -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
//! max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
//! min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;

/** CCoinsView backed by the LevelDB coin database (chainstate/) */
class CCoinsViewDB : public CCoinsView
{
protected:
    CLevelDBWrapper db;
public:
    CCoinsViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    bool GetCoins(const uint256 &txid, CCoins &coins) const;
    bool HaveCoins(const uint256 &txid) const;
    uint256 GetBestBlock() const;
    bool BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock);
    bool GetStats(CCoinsStats &stats) const;
};

/** Access to the block database (blocks/index/) */
class CBlockTreeDB : public CLevelDBWrapper
{
public:
    CBlockTreeDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
private:
    CBlockTreeDB(const CBlockTreeDB&);
    void operator=(const CBlockTreeDB&);
public:
    bool WriteBlockIndex(const CDiskBlockIndex& blockindex);
    bool ReadBlockFileInfo(int nFile, CBlockFileInfo &fileinfo);
    bool WriteBlockFileInfo(int nFile, const CBlockFileInfo &fileinfo);
    bool ReadLastBlockFile(int &nFile);
    bool WriteLastBlockFile(int nFile);
    bool WriteReindexing(bool fReindex);
    bool ReadReindexing(bool &fReindex);
    bool ReadTxIndex(const uint256 &txid, CDiskTxPos &pos);
    bool WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> > &list);
    bool WriteFlag(const std::string &name, bool fValue);
    bool ReadFlag(const std::string &name, bool &fValue);
    bool LoadBlockIndexGuts();
};
//ccc:CScript2TxPosViewDB, because this map is very big, no memory cache is available
class CScript2TxPosViewDB :public CScript2TxPosDBView
{
protected:
    CLevelDBWrapper db;
public:    
    CScript2TxPosViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
    bool GetTxPosList(const CScript scriptPubKey,std::vector<CDiskTxPos> &vTxPos);    
    bool BatchWrite(const std::map<CScript,std::vector<CDiskTxPos> > &mapScriptTxPosList);  
    bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
};
class CDomainViewDB //:public CDomainView
{
protected:
    CSqliteWrapper db;
public:
    //std::string strtest="db loaded";
    CDomainViewDB( bool fWipe = false);
     //bool GetForward(const std::string strDomainName,CContent& forward)const ;   
     bool _GetDomainByForward(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const ; 
     bool _GetDomainByOwner(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const ;
     bool GetDomainByForward(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI=true)const ;
     bool GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI)const ;
     bool GetDomainByName(const string strDomainName,CDomain& domain)const ;
     
    //l bool GetDomainNameByForward(const CScript scriptPubKey,std::vector<string> &vDomainName);    
//      bool GetDomainByName(const std::string strDomainName,CDomain &domain);    
//     bool GetDomainByTags(const std::vector<std::string> vTag,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainByAlias(const std::string strAlias,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainNamesToExpire(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpireIn=3600*24,bool FSupportFAI=true);    
//     bool GetDomainNamesExpired(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpiredFor=3600*24,bool FSupportFAI=true);       
     bool Update(const CScript ownerIn,const string& strDomainContent,const uint64_t lockedValue,const uint32_t nLockTimeIn,const CLink link);

     bool Reverse(const string& strDomainContent);
    //! Do a bulk modification (multiple tam changes).
    //! The passed mapTam can be modified.
     //bool BatchWrite(const std::vector<CDomain> &vDomain);
     bool Write(const CDomain &domain);
    //! As we use CDomainView polymorphically, have a virtual destructor
     ~CDomainViewDB() {}
};
class CTagViewDB //:public CDomainView
{
protected:
    CSqliteWrapper db;
public:
    //std::string strtest="db loaded";
    CTagViewDB( bool fWipe = false);
     //bool GetForward(const std::string strDomainName,CContent& forward)const ;   
    bool HasLink(const CLink link)const;
     bool Search(vector<CLink>& vLink,const std::vector<string> &vTag,const int cc=-1,const int nMaxItems=1000,const int nOffset=0)const ;           
     bool Insert(const int cc,const string tag,const CLink link,const int nExpireTime);
     bool ClearExpired();
    
     ~CTagViewDB() {}
};
#endif // BITCOIN_TXDB_H
