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

#include <boost/filesystem/path.hpp>
#include <stdio.h>  
#include <stdlib.h>  
#include <sqlite3.h>

class sqlite_error : public std::runtime_error
{
public:
    sqlite_error(const std::string& msg) : std::runtime_error(msg) {}
};

void HandleError(const leveldb::Status& status) throw(sqlite_error);

/** Batch of changes queued to be written to a CLevelDBWrapper */
class CSqliteBatch
{
    friend class CSqliteWrapper;

private:
    leveldb::WriteBatch batch;

public:
    template <typename K, typename V>
    void Write(const K& key, const V& value)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(ssValue.GetSerializeSize(value));
        ssValue << value;
        leveldb::Slice slValue(&ssValue[0], ssValue.size());

        batch.Put(slKey, slValue);
    }

    template <typename K>
    void Erase(const K& key)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        batch.Delete(slKey);
    }
};

class CSqliteWrapper
{
private:
    //! custom environment this database is using (may be NULL in case of default environment)
    leveldb::Env* penv;

    //! database options used
    leveldb::Options options;

    //! options used when reading from the database
    leveldb::ReadOptions readoptions;

    //! options used when iterating over values of the database
    leveldb::ReadOptions iteroptions;

    //! options used when writing to the database
    leveldb::WriteOptions writeoptions;

    //! options used when sync writing to the database
    leveldb::WriteOptions syncoptions;

    //! the database itself
    sqlite3* db pdb;

public:
    CSqliteWrapper(const boost::filesystem::path& path,bool fWipe = false);
    ~CSqliteWrapper();

    
    bool Read(const CScript& address, std::vector<CDiskTxPos>& vTxPos) const throw(leveldb_error);
    

    template <typename K>
    bool Exists(const K& key) const throw(leveldb_error)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok()) {
            if (status.IsNotFound())
                return false;
            LogPrintf("LevelDB read failure: %s\n", status.ToString());
            HandleError(status);
        }
        return true;
    }

   
    bool Erase(const std::vector<CDiskTxPos>& vTxPos) throw(sqlite_error);
  

    bool WriteBatch(std::vector<std::pair<CScript, CDiskTxPos> >&list)throw(sqlite_error);

    // not available for LevelDB; provide for compatibility with BDB
    bool Flush()
    {
        return true;
    }

    bool Sync() throw(sqlite_error)
    {
        CLevelDBBatch batch;
        return WriteBatch(batch, true);
    }

    // not exactly clean encapsulation, but it's easiest for now
    leveldb::Iterator* NewIterator()
    {
        return pdb->NewIterator(iteroptions);
    }
};

#endif // BITCOIN_LEVELDBWRAPPER_H
