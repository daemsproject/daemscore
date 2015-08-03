// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CCCOIN_SQLITEWRAPPER_H
#define CCCOIN_SQLITEWRAPPER_H

#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "util.h"
#include "version.h"
#include "utilstrencodings.h"

#include <boost/filesystem/path.hpp>
#include <sqlite3.h>
class CDomain;
class CLink;
class CCheque;
class uint256;
using namespace std;
class sqlite_error : public std::runtime_error
{
public:
    sqlite_error(const std::string& msg) : std::runtime_error(msg) {}
};

//void HandleError(const leveldb::Status& status) throw(sqlite_error);

/** Batch of changes queued to be written to a CSqliteWrapper */
class CSqliteDBBatch
{
    
};

class CSqliteWrapper
{
private:
    
    //! the database itself
    sqlite3* pdb;
    void HandleError(const int& status) throw(sqlite_error);
public:
    CSqliteWrapper(const boost::filesystem::path& path);
    ~CSqliteWrapper();
    bool ClearTable(const char* tableName);
    bool Write(const char* sql);
    bool Insert(const CDomain& domain);
    bool Update(const CDomain& domain);
    bool Delete(const CDomain& domain);
    bool Get(const char* tableName,const char* searchColumn,const char* searchValue,char**& result,int& nRow,int& nColumn) const;
    bool GetDomain(const char* tableName,const char* searchColumn,const char* searchValue,std::vector<CDomain>& vDomain) const;
    bool CreateTables();
    bool _CreateTable(const char* tableName);
    bool CreateBlockDomainTable();
    bool Insert(const uint256 blockHash,const CDataStream& sBlockDomains);
    bool GetBlockDomains(const uint256 blockHash,CDataStream& sBlockDomains);
    
    bool CreateTagIDTable();
    bool CreateTagTable();
    bool InsertTag(const int cc,const int tagID,const CLink link,const unsigned int nExpireTime);
    bool GetLinks(const vector<string> vTag,const int cc,const CLink link,std::vector<CLink>& vLink,const int nMaxItems=1000,const int nOffset=0) const;
    bool InsertTagID(const string tag,int& tagID);
    bool GetTagID(const string tag,int& tagID) const;
    bool ClearExpiredTags(const unsigned int nTime);
    
    bool CreateScriptIndexTable();
    bool CreateTxIndexTable();
    bool CreateChequeTable();
    bool InsertScriptIndex(const CScript script,int& scriptIndex);
    bool GetScriptIndex(const CScript script,int& scriptIndex) const;
    bool InsertTxIndex(const uint256 txid,int& txIndex);
    bool GetTxIndex(const uint256 txid,int& txIndex) const;
    bool GetTxidByTxIndex(const int txIndex, uint256& txid) const;
    bool InsertCheque(int scriptIndex,int txIndex,ushort nOut, uint64_t nValue,uint32_t nLockTime);
    bool GetCheques(const vector<CScript>& vScript,vector<CCheque> & vCheques,const int nMaxItems=1000,const int nOffset=0)const;
    bool EraseCheque(const int txindex, const uint32_t nOut);
};
#endif // BITCOIN_LEVELDBWRAPPER_H
