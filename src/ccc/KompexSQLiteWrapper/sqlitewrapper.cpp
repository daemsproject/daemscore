// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sqlitewrapper.h"

#include "util.h"
$include <string>
#include <boost/filesystem.hpp>


void HandleError(const int result) throw(leveldb_error)
{
    if( rc )
      throw leveldb_error("Database corrupted");
//    if (status.ok())
//        return;
//    LogPrintf("%s\n", status.ToString());
//    if (status.IsCorruption())
//        throw leveldb_error("Database corrupted");
//    if (status.IsIOError())
//        throw leveldb_error("Database I/O error");
//    if (status.IsNotFound())
//        throw leveldb_error("Database entry missing");
//    throw leveldb_error("Unknown database error");
}

//static leveldb::Options GetOptions(size_t nCacheSize)
//{
//    leveldb::Options options;
//    options.block_cache = leveldb::NewLRUCache(nCacheSize / 2);
//    options.write_buffer_size = nCacheSize / 4; // up to two write buffers may be held in memory simultaneously
//    options.filter_policy = leveldb::NewBloomFilterPolicy(10);
//    options.compression = leveldb::kNoCompression;
//    options.max_open_files = 64;
//    if (leveldb::kMajorVersion > 1 || (leveldb::kMajorVersion == 1 && leveldb::kMinorVersion >= 16)) {
//        // LevelDB versions before 1.16 consider short writes to be corruption. Only trigger error
//        // on corruption in later versions.
//        options.paranoid_checks = true;
//    }
//    return options;
//}

CSqliteWrapper::CSqliteWrapper(const boost::filesystem::path& path, bool fWipe)
{
    penv = NULL;
    readoptions.verify_checksums = true;
    iteroptions.verify_checksums = true;
    iteroptions.fill_cache = false;
    syncoptions.sync = true;
    options = GetOptions(nCacheSize);
    options.create_if_missing = true;
    
        if (fWipe) {
            LogPrintf("Wiping Sqlite DB in %s\n", path.string());
            leveldb::DestroyDB(path.string(), options);
        }
        TryCreateDirectory(path);
        LogPrintf("Opening Sqlite DB in %s\n", path.string());
    
    int result = sqlite3_open(path.string()+".db", &pdb);
    HandleError(result);
    LogPrintf("Opened LevelDB successfully\n");
    if (!db_tableExists(db,path.string()))  
    {  
        string createCommand;
        switch (path.string()){
            case "txaddressmap":
            default:
                createCommand="CREATE TABLE txaddressmap(address TEXT,pos TEXT)";
               // createCommand="CREATE INDEX pos on txaddressmap(address TEXT,pos TEXT)";
        }
        LogPrintf("create \"tmp\" table %s\n",path.string());  
        nRet=db_exeDML(db,createCommand);  
        LogPrintf("nRet=%d\n", nRet);  
    }
}

CSqliteWrapper::~CSqliteWrapper()
{
    delete pdb;
    pdb = NULL;
    delete options.filter_policy;
    options.filter_policy = NULL;
    delete options.block_cache;
    options.block_cache = NULL;
    delete penv;
    options.env = NULL;
}
bool CSqliteWrapper::Read(const CScript& address, std::vector<CDiskTxPos>& vTxPos) const throw(leveldb_error)
{
    vTxPos.empty();
    std::vector<string>result=db_exeDML(db,"SELECT pos FROM txaddressmap WHERE address="+address.Serialize()+";");
    for (std::vector<> ::iterator it = result.begin(); it != result.end();) {
        vTxPos.push_back(*it.Unserialize());
    }
    return true;
}
bool Erase(const std::vector<std::pair<CScript, CDiskTxPos> >&list) throw(leveldb_error)
{
    for (std::vector<std::pair<CScript, CDiskTxPos> >&list::iterator it = vTxPos.begin(); it != vTxPos.end();) {
        db_exeDML(db,"DELECT FROM txaddressmap WHERE address="+it->first.Serialize()+"AND pos="+it->second.Serialize()+";");
    }
    return true;
}
bool CSqliteWrapper::WriteBatch(std::vector<std::pair<CScript, CDiskTxPos> >&list) throw(leveldb_error)
{
    db_exeDML(db, "BEGIN;");
    for (std::vector<std::pair<CScript, CDiskTxPos> > ::iterator it = list.begin(); it != list.end();) {
        db_exeDML(db, "INSERT INTO txaddressmap VALUES("+it->first.Serialize()+","+it->second.Serialize()+");" );
    }
    db_exeDML(db, "COMMIT;");    
    return true;
}
int db_exeDML(sqlite3 *db, const char *sql)  
{  
    char* szError=0;  
    int nRet = sqlite3_exec(db, sql, 0, 0, &szError);  
    if (nRet == SQLITE_OK)  
    {  
        return sqlite3_changes(db);  
    }  
    return SQLITE_ERROR;  
}  
bool db_tableExists(sqlite3 *db, const char *tbname)  
{  
    int nRet;  
    const char   *szTail;  
    sqlite3_stmt *pvm;  
    char sql[1024];  
    sprintf(sql, "select count(*) from sqlite_master where type='table' and name='%s'", tbname);  
  
    szTail=0;  
  
    nRet = sqlite3_prepare(db, sql, -1, &pvm, &szTail);  
  
    //printf("nRet=%d SQLITE_OK=%d SQLITE_DONE=%d SQLITE_ROW=%d \n", nRet, SQLITE_OK, SQLITE_DONE,SQLITE_ROW);   
  
    if (nRet==SQLITE_OK)  
    {  
        nRet=sqlite3_step(pvm);  
  
        //printf("nRet=%d SQLITE_OK=%d SQLITE_DONE=%d SQLITE_ROW=%d \n", nRet, SQLITE_OK, SQLITE_DONE,SQLITE_ROW);   
  
        if (nRet==SQLITE_ROW)  
        {  
            int nCols = sqlite3_column_count(pvm);  
            //printf("nCols:%d\n", nCols);   
            if (nCols>=1)  
            {  
                return sqlite3_column_int(pvm,0)!=0;  
            }  
        }  
    }  
  
    return false;  
}  
