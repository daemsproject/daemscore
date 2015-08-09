// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_TXDB_H
#define BITCOIN_TXDB_H

#include "leveldbwrapper.h"
#include "ccc/sqlitewrapper.h"
#include "main.h"
#include "ccc/link.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

class CCoins;
//class uint256;

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
//class CScript2TxPosViewDB :public CScript2TxPosDBView
//{
//protected:
//    CLevelDBWrapper db;
//public:    
//    CScript2TxPosViewDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);
//    bool GetTxPosList(const CScript scriptPubKey,std::vector<CDiskTxPos> &vTxPos);    
//    bool BatchWrite(const std::map<CScript,std::vector<CDiskTxPos> > &mapScriptTxPosList);  
//    bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
//};
class CScript2TxPosDB 
{
protected:
    CSqliteWrapper* db;
public:    
    CScript2TxPosDB(CSqliteWrapper* dbIn, bool fWipe = false);
    bool GetTxPosList(const CScript scriptPubKey,std::vector<CTxPosItem> &vTxPos);    
    bool BatchWrite(const std::map<CScript,std::vector<CTxPosItem> > &mapScriptTxPosList);  
    bool Write(const CScript &scriptPubKey,const std::vector<CTxPosItem> &vTxPos);
    bool AddNewTxs(const std::map<CScript,CTxPosItem> &mapScriptTxPos);
    bool RemoveTxs(const std::map<CScript,CTxPosItem> &mapScriptTxPos);
     
};
class CBlockPosDB
{
protected:
    CSqliteWrapper* db;
public:    
    CBlockPosDB(CSqliteWrapper* dbIn, bool fWipe = false);
    bool GetByPos(const int nFile,const int nPos,uint256& hashBlock,int& nHeight);
    bool Write(const int nFile,const int nPos,const uint256 hashBlock,const int nHeight);
};
class CDomainViewDB //:public CDomainView
{
protected:
    CSqliteWrapper* db;
public:
    //std::string strtest="db loaded";
    CDomainViewDB(CSqliteWrapper* dbIn, bool fWipe = false);
     //bool GetForward(const std::string strDomainName,CContent& forward)const ;   
     bool _GetDomainByForward(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const ; 
     bool _GetDomainByOwner(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const ;
     bool GetDomainByForward(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupport100=true)const ;
     bool GetDomainByForward(const CScript scriptPubKey,CDomain& domain,bool FSupport100)const;
     bool GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupport100)const ;
     bool GetDomainByName(const string strDomainName,CDomain& domain)const ;
     
    //l bool GetDomainNameByForward(const CScript scriptPubKey,std::vector<string> &vDomainName);    
//      bool GetDomainByName(const std::string strDomainName,CDomain &domain);    
//     bool GetDomainByTags(const std::vector<std::string> vTag,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainByAlias(const std::string strAlias,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI=true);    
//     bool GetDomainNamesToExpire(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpireIn=3600*24,bool FSupportFAI=true);    
//     bool GetDomainNamesExpired(std::vector<CDomain> &vDomain,const int nMax=1000,const uint32_t nExpiredFor=3600*24,bool FSupportFAI=true);       
     bool CDomainViewDB::GetUpdateDomain(const CScript ownerIn,const string& strDomainContent,const uint64_t lockedValue,const uint32_t nLockTimeIn,const CLink link,CDomain& domain);
     bool WriteBlockDomains(const uint256 blockHash,const map<CScript,string>& mapBlockDomains);
    bool GetBlockDomains(const uint256 blockHash,CDataStream& sBlockDomains);
     bool Reverse(const string& strDomainContent);
    //! Do a bulk modification (multiple tam changes).
    //! The passed mapTam can be modified.
     //bool BatchWrite(const std::vector<CDomain> &vDomain);
     bool Write(const CDomain &domain);
     bool WriteTags(const CDomain &domain,const map<string,int>& mapTags);
     bool ClearTables();
    //! As we use CDomainView polymorphically, have a virtual destructor
     ~CDomainViewDB() {}
};
struct CContentDBItem
{
   CLink link;
   int64_t pos;
   CScript sender;
   int cc;
   CAmount lockValue;
   uint32_t lockTime;
   vector<string>vTags;
   CContentDBItem(CLink linkIn,int64_t posIn,CScript senderIn,int ccIn,
   CAmount lockValueIn,  uint32_t lockTimeIn,vector<string>vTagsIn)
   {
       link=linkIn;
       pos=posIn;
       sender=senderIn;
       cc=ccIn;
       lockValue=lockValueIn;
       lockTime=lockTimeIn;
       vTags=vTagsIn;
   }   
};
class CTagViewDB    
{
protected:
    CSqliteWrapper* db;    
public:
    //std::string strtest="db loaded";
    CTagViewDB(CSqliteWrapper* dbIn, bool fWipe = false);
     //bool GetForward(const std::string strDomainName,CContent& forward)const ;   
    bool HasLink(const CLink link)const;
     bool Search(vector<CLink>& vLink,const std::vector<string> &vTag,const int cc=-1,const int nMaxItems=1000,const int nOffset=0)const ;           
     //bool InsertContent(const CContentDBItem);
     bool Insert(const CContentDBItem item);
     bool InsertTags(const vector<string>vTags);
     bool InsertContentTags(const vector<CContentDBItem>& vContents,const map<string,int> mapTags);
     bool ClearExpired(uint32_t nTime);
    bool ClearTables();
     ~CTagViewDB() {}
};
class CCheque
{
public:
    CScript scriptPubKey;
    uint256 txid; 
    int64_t txIndex;
    ushort nOut;
    CAmount nValue;
    uint32_t nLockTime;
    CCheque(){
        txid=uint256(0);
       nOut=0;
        nValue=0;
        nLockTime=0;
        txIndex=0;
    }
    ADD_SERIALIZE_METHODS;   

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {        
        READWRITE(scriptPubKey);
        READWRITE(txid);
        READWRITE(txIndex);
        READWRITE(VARINT(nOut));
        READWRITE(VARINT(nValue));
        READWRITE(VARINT(nLockTime));
    }
};
class CScriptCoinDB 
{
protected:
    CSqliteWrapper* db;
public:
    
    CScriptCoinDB(CSqliteWrapper* dbIn, bool fWipe = false);
    
     bool Search(const vector<CScript>& vScriptPubKey,vector<CCheque> & vCheques)const ;    
     bool Insert(const CCheque cheque);
     //bool BatchInsert(vector<CCheque> vCheque);
     //bool BatchErase(vector<pair<uint256,uint32_t> >vChequeErase);
     bool Erase(const uint256 txid, const uint32_t nOut);
    bool ClearTables();
    
     ~CScriptCoinDB() {}
};
void UpdateSqliteDB(const CBlock& block,const vector<pair<uint256, CDiskTxPos> >& vPos,const vector<vector<pair<CScript,uint32_t> > >& vPrevouts,bool fErase);
void GetBlockScript2TxPosList(const CBlock& block,const vector<pair<uint256, CDiskTxPos> >& vPos,const vector<vector<CScript> >& vPrevouts,map<CScript,vector<CTxPosItem> > mapScript2TxPos,bool fErase);
void MergeScript2TxPosList(map<CScript,vector<CTxPosItem> >& parent,const map<CScript,CTxPosItem>& child);
void GetBlockContentAndTagList(const CBlock& block,const vector<pair<uint256, CDiskTxPos> >& vPos,const vector<vector<CScript> >& vPrevouts,vector<string>& vTags,vector<CContentDBItem>& vContents);
void FindBlockTagIDAndNewTags(const vector<string>& vTags,map<string,int>& mapTags,vector<string>& vTagNew);
void PrePareBlockTxIndex(const CBlock& block,map<uint256,uint64_t>& mapTxIndex);
void GetBlockSenderDomains(const CBlock& block,const vector<vector<CScript> >& vPrevouts,map<CScript,string>& mapBlockDomains);
void GetBlockChequeUpdates(const CBlock& block,const vector<vector<pair<CScript,uint32_t> > >& vPrevouts,vector<CCheque> vChequeAdd,vector<pair<uint256,uint32_t> >vChequeErase,bool fReverse);

#endif // BITCOIN_TXDB_H
