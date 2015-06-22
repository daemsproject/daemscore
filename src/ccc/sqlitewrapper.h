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
    CSqliteWrapper(const boost::filesystem::path& path,bool fWipe = false);
    ~CSqliteWrapper();
    bool Write(const char* sql);
    bool Insert(const CDomain& domain);
    bool Update(const CDomain& domain);
    bool Delete(const CDomain& domain);
    bool Get(const char* tableName,const char* searchColumn,const char* searchValue,char**& result,int& nRow,int& nColumn) const;
    bool GetDomain(const char* tableName,const char* searchColumn,const char* searchValue,std::vector<CDomain>& vDomain) const;
    bool CreateTables();
    bool _CreateTable(const char* tableName);
};

#endif // BITCOIN_LEVELDBWRAPPER_H
