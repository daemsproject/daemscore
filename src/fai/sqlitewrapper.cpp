// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sqlitewrapper.h"
#include "fai/domain.h"
#include "fai/link.h"
#include "util.h"
#include "timedata.h"
#include "txdb.h"
#include <boost/filesystem.hpp>
#include <stdio.h>
#include <bits/stl_vector.h>
#include <bits/basic_string.h>
using namespace std;
//class sqlite_error : public std::runtime_error
//{
//public:
//    sqlite_error(const std::string& msg) : std::runtime_error(msg) {}
//};

void CSqliteWrapper::HandleError(const int& status) throw(sqlite_error)
{
    if (!status)
        return;
    sqlite3_close(pdb);
    LogPrintf("%s\n", sqlite3_errmsg(pdb));    
    throw sqlite_error(sqlite3_errmsg(pdb));    
}



CSqliteWrapper::CSqliteWrapper(const boost::filesystem::path& path)
{
    
    TryCreateDirectory(path);
    LogPrintf("Opening sqliteDB in %s\n", path.string());
    
    boost::filesystem::path p=path / "sqlite.db";
    pdb=NULL;
    int status = sqlite3_open(p.string().c_str(), &pdb);
    HandleError(status);
       
    CreateTables();    
    //ClearExpiredTags(GetAdjustedTime());
    LogPrintf("Opened sqliteDB successfully\n"); 
}

CSqliteWrapper::~CSqliteWrapper()
{
    sqlite3_close(pdb);
}

bool CSqliteWrapper::CreateTables()
{
    //LogPrintf("CSqliteWrapper CreateTables  \n");
    CreateDomainTable("domain10000");
    CreateDomainTable("domain100");
    CreateDomainTagTable("domaintag10000");
    CreateDomainTagTable("domaintag100");
    CreateTagIDTable();
    CreateTagTable();
    //CreateScriptIndexTable();
    CreateTxIndexTable();
    CreateContentTable();
    CreateChequeTable();
    CreateBlockDomainTable();
    CreateScript2TxPosTable();
    CreateBlockPosTable();
    return true;
}
bool CSqliteWrapper::CreateDomainTable(const char* tableName)
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    //char* tableName="domainf";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s( \
    domainname VARCHAR(64) PRIMARY KEY, \
    expiredate INTEGER , \
    owner INTEGER, \
    redirecttype INTEGER , \
    redirrectto BLOB(64), \
    alias VARCHAR(64), \
    icon INTEGER, \
    intro VARCHAR(128), \
    redirecthsitory BLOB(64), \
    lockvalue INTEGER \
    );";    
    sprintf(sql,createtablestatement.c_str(),tableName);
    LogPrintf("CSqliteWrapper CreateDomainTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTable done %s\n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),"index_expiredate",tableName,"expiredate");  
    LogPrintf("CSqliteWrapper CreateDomainTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_owner",tableName,"owner");    
    LogPrintf("CSqliteWrapper CreateDomainTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_redirrectto",tableName,"redirrectto");  
    LogPrintf("CSqliteWrapper CreateDomainTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_alias",tableName,"alias");    
    LogPrintf("CSqliteWrapper CreateDomainTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTable createindex done %s\n",zErrMsg);
     sprintf(sql,createindexstatement.c_str(),"index_lockvalue",tableName,"lockvalue");    
    LogPrintf("CSqliteWrapper CreateDomainTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTable createindex done %s\n",zErrMsg);

    //LogPrintf("CSqliteWrapper _CreateTable success \n");
    return true;
}
bool CSqliteWrapper::CreateDomainTagTable(const char* tableName)
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s (domainid INTEGER,tagid INTEGER, PRIMARY KEY(domainid,tagid));";    
    sprintf(sql,createtablestatement.c_str(),tableName);
    LogPrintf("CSqliteWrapper CreateDomainTagTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateDomainTagTable done %s\n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),tableName,"_index_tag",tableName,"tagid");  
    LogPrintf("CSqliteWrapper CreateDomainTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement.c_str(),tableName,"_index_domain",tableName,"domainid");  
    LogPrintf("CSqliteWrapper CreateDomainTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    return true;
}
bool CSqliteWrapper::CreateContentTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    string str="CREATE TABLE IF NOT EXISTS table_content( \
    link INTEGER PRIMARY KEY, \
    pos INTEGER, \
    sender INTEGER, \
    cc INTEGER , \
    lockvalue INTEGER , \
    locktime INTEGER  \
    );";  
    char sql[2000];
    sqlite3_exec(pdb,str.c_str(),0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateContentTable done %s\n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),"index_sender","table_content","sender");  
    LogPrintf("CSqliteWrapper CreateContentTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_sender","table_content","sender");    
    LogPrintf("CSqliteWrapper CreateContentTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateContentTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_cc","table_content","cc");  
    LogPrintf("CSqliteWrapper CreateContentTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateContentTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_lockvalue","table_content","lockvalue");    
    LogPrintf("CSqliteWrapper CreateContentTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateContentTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement.c_str(),"index_locktime","table_content","locktime");    
    LogPrintf("CSqliteWrapper CreateContentTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateContentTable createindex done %s\n",zErrMsg);    
    return true;
}
bool CSqliteWrapper::CreateBlockDomainTable()
{
    //LogPrintf("CSqliteWrapper CreateBlockDomainTable %s \n",tableName);
    char* zErrMsg=0;    
    string tableName="blockdomaintable";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s( blockhash BLOB(32) PRIMARY KEY, mapdomains BLOB);";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper CreateBlockDomainTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateBlockDomainTable done %s \n",zErrMsg);
    return true;
}
bool CSqliteWrapper::CreateTagIDTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    string tableName="tagid";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s( tagid INTEGER PRIMARY KEY, tag VARCHAR(32) UNIQUE);";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper CreateTagIDTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagIDTable done %s \n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),"index_tagtable_tag",tableName.c_str(),"tag");  
    LogPrintf("CSqliteWrapper CreateTagIDTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagIDTable createindex done %s\n",zErrMsg);
   
    return true;
}
//bool CSqliteWrapper::CreateScriptIndexTable()
//{    
//    char* zErrMsg=0;    
//    string tableName="scriptindextable";
//    char sql[2000];
//    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( scriptindex INTEGER PRIMARY KEY AUTOINCREMENT, script BLOB(255));";    
//    sprintf(sql,createtablestatement,tableName.c_str());
//    LogPrintf("CSqliteWrapper CreateScriptIndexTable statement %s \n",sql);
//    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
//    LogPrintf("CSqliteWrapper CreateScriptIndexTable result %s \n",zErrMsg);
//    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
//    sprintf(sql,createindexstatement,"index_scriptindextable_script",tableName.c_str(),"script");  
//    LogPrintf("CSqliteWrapper CreateScriptIndexTable createindexstatement %s \n",sql);
//    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
//    LogPrintf("CSqliteWrapper CreateScriptIndexTable createindex done %s\n",zErrMsg);
//   
//    return true;
//}
bool CSqliteWrapper::CreateScript2TxPosTable()
{    
    char* zErrMsg=0;    
    string tableName="table_script2txpos";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s( scriptindex INTEGER PRIMARY KEY, script BLOB(3000) UNIQUE,vtxpos BLOB);";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper CreateScript2TxPosTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateScript2TxPosTable done %s \n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),"index_table_script2txpos_script",tableName.c_str(),"script");  
    LogPrintf("CSqliteWrapper CreateScript2TxPosTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateScript2TxPosTable createindex done %s\n",zErrMsg);
   
    return true;
}
bool CSqliteWrapper::CreateBlockPosTable()
{    
    char* zErrMsg=0;    
    string tableName="table_blockpos";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s( blockheight INTEGER PRIMARY KEY, blockhash BLOB(32),pos INTEGER);";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper CreateBlockPosTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateBlockPosTable done %s \n",zErrMsg);
    const string createindexstatement1="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement1.c_str(),"index_table_blockpos_height",tableName.c_str(),"blockheight");  
    LogPrintf("CSqliteWrapper CreateBlockPosTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateBlockPosTable createindex done %s\n",zErrMsg);
    //const char * createindexstatement2="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement1.c_str(),"index_table_blockpos_pos",tableName.c_str(),"pos");  
    LogPrintf("CSqliteWrapper CreateBlockPosTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateBlockPosTable createindex done %s\n",zErrMsg);
    return true;
}
bool CSqliteWrapper::CreateTagTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;
    string tableName="tag";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s (link INTEGER,tagid INTEGER,PRIMARY KEY(link,tagid));";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper CreateTagTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagTable done %s\n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),tableName.c_str(),"_index_tag",tableName.c_str(),"tag");  
    LogPrintf("CSqliteWrapper CreateTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);    
    sprintf(sql,createindexstatement.c_str(),tableName.c_str(),"_index_link",tableName.c_str(),"link");    
    LogPrintf("CSqliteWrapper CreateTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    return true;
}

bool CSqliteWrapper::ClearTable(const char* tableName)
{
    char* zErrMsg=0;
    char sql[2000];
    string deletetablement="DELETE FROM %s;";
    sprintf(sql,deletetablement.c_str(),tableName);  
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);    
    if(zErrMsg)
    {
        LogPrintf("CSqliteWrapper ClearTable %s error %s\n",tableName,zErrMsg);
        return false;
    }
    return true;
}
bool CSqliteWrapper::BeginBatch()
{
    char* sErrMsg;
    sqlite3_exec(pdb, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);
   // LogPrintf("sqlite batch begin %s \n",sErrMsg);  
    return true;
}
bool CSqliteWrapper::EndBatch()
{
    char* sErrMsg;
    sqlite3_exec(pdb, "END TRANSACTION", NULL, NULL, &sErrMsg);    
    //LogPrintf("sqlite batch end %s \n",sErrMsg);  
    if(sErrMsg)
        return false;
    return true;
            
}

bool CSqliteWrapper::Write(const char* sql)
{
    char* zErrMsg=0;
     //LogPrintf("CSqliteWrapper Write \n");
     sqlite3_exec(pdb,sql,0,0,&zErrMsg);
     //LogPrintf("CSqliteWrapper Write done \n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:write error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::SearchInt(const char* tableName,const char* columnName,const char* columnValue,const char* resultColumnName,int64_t& result)const
{
    char sql[2000]; 
    const char* selectstatement="SELECT %s FROM %s WHERE %s = %s;";    
     sprintf(sql,selectstatement,resultColumnName,tableName,columnName,columnValue);
    LogPrintf("CSqliteWrapper SearchInt sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {            
            result=sqlite3_column_int64(stmt,0);
           sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }    
    return false;  
}
//bool CSqliteWrapper::DeleteBy1Col(const char* tableName,const char* columnName, const char* comlumnValue)
//{
//    //LogPrintf("GetdeleteSql\n");
//    
//    char deletetatement[2000];    
//    sprintf(deletetatement,"DELETE FROM %s WHERE %s = %s;",tableName,columnName,comlumnValue);
//    char* zErrMsg=0;
//     //LogPrintf("CSqliteWrapper Delete \n");
//     sqlite3_exec(pdb,deletetatement,0,0,&zErrMsg);
//     //LogPrintf("CSqliteWrapper Delete done\n");     
//     if(zErrMsg)
//     {
//         LogPrintf("sqlitewrapper:DeleteBy1 error:%s",zErrMsg);
//         return false;
//     }
//     return true;
//}
bool CSqliteWrapper::SearchStr(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,string& searchResult) const
{
    //LogPrintf("CSqliteWrapper GetTagID \n");
    //char* zErrMsg=0;
    char sql[7000]; 
    const char* selectstatement="SELECT %s FROM %s WHERE %s = %s;";    
     sprintf(sql,selectstatement,searchForColumn,tableName,searchByColumn,searchByValue);
    //LogPrintf("CSqliteWrapper GetTagID sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW)
    {
        switch (searchResultFormat)
        {
            case SQLITEDATATYPE_TEXT:
                searchResult=(char*)sqlite3_column_text(stmt,  0);

                break;
            case SQLITEDATATYPE_BLOB:
                searchResult=string((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));                    
                //LogPrintf("CSqliteWrapper SearchStr %s result %s\n",tableName,searchResult);
                break;
            default:
                return false;              
        }                     
       sqlite3_finalize(stmt);
        return true;
    }
    else
    {
        if(rc!=101)
            LogPrintf("CSqliteWrapper SearchStr  %s error %i\n",sql,rc);
        sqlite3_finalize(stmt);
        return false;
    }    
    return false;     
}
//bool CSqliteWrapper::SearchStrsIn(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,vector<string>& searchResult,int nMax) const
//{
//    //LogPrintf("CSqliteWrapper GetTagID \n");
//    //char* zErrMsg=0;
//    char sql[2000]; 
//    const char* selectstatement="SELECT %s FROM %s WHERE %s IN(%s) LIMIT %i;";    
//     sprintf(sql,selectstatement,searchForColumn,tableName,searchByColumn,searchByValue,nMax);
//    //LogPrintf("CSqliteWrapper GetTagID sql %s\n",sql); 
//    sqlite3_stmt  *stmt = NULL;
//    int rc;
//    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
//    if(rc != SQLITE_OK)
//    {
//        if(stmt)        
//            sqlite3_finalize(stmt);               
//        return false;
//    }
//    string str;
//    do{ 
//        rc = sqlite3_step(stmt);
//        if(rc == SQLITE_ROW)
//        {
//            switch (searchResultFormat)
//            {
//                case SQLITEDATATYPE_TEXT:
//                    str=(char*)sqlite3_column_text(stmt,  0);
//                    break;
//                case SQLITEDATATYPE_BLOB:
//                    str=string((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));                    
//                    break;
//                default:
//                    return false;              
//            }                     
//            searchResult.push_back(str);
//        }         
//        else if(rc == SQLITE_DONE)
//        {            
//            sqlite3_finalize(stmt);
//            return true;
//        }
//        else
//        {
//            sqlite3_finalize(stmt);
//            return false;
//        }
//    }while(1);   
//    return false;     
//}
bool CSqliteWrapper::SearchStrs(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,vector<string>& searchResult,const char* chOperator,int nMax) const
{
    //LogPrintf("CSqliteWrapper GetTagID \n");
    //char* zErrMsg=0;
    char sql[7000]; 
    const char* selectstatement="SELECT %s FROM %s WHERE %s %s %s LIMIT %i;";    
     sprintf(sql,selectstatement,searchForColumn,tableName,searchByColumn,chOperator,searchByValue,nMax);
    //LogPrintf("CSqliteWrapper GetTagID sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    string str;
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
            switch (searchResultFormat)
            {
                case SQLITEDATATYPE_TEXT:
                    str=(char*)sqlite3_column_text(stmt,  0);
                    break;
                case SQLITEDATATYPE_BLOB:
                    str=string((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));                    
                    break;
                default:
                    return false;              
            }                     
            searchResult.push_back(str);
        }         
        else if(rc == SQLITE_DONE)
        {            
            sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }while(1);   
    return false;     
}
bool CSqliteWrapper::SearchInts(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,vector<int64_t>& searchResult,const char* chOperator) const
{
    //LogPrintf("CSqliteWrapper SearchInts \n");
    //char* zErrMsg=0;
    char sql[7000]; 
    const char* selectstatement="SELECT %s FROM %s WHERE %s %s %s;";    
     sprintf(sql,selectstatement,searchForColumn,tableName,searchByColumn,chOperator,searchByValue);
    //LogPrintf("CSqliteWrapper SearchInts sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    //LogPrintf("CSqliteWrapper SearchInts sql %s rc:%i\n",sql,rc); 
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    string str;
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {            
            searchResult.push_back(sqlite3_column_int64(stmt,  0));
        }         
        else if(rc == SQLITE_DONE)
        {            
            sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }
    }while(1);   
    return false;     
}
bool CSqliteWrapper::Insert(const uint256 blockHash,const CDataStream& sBlockDomains)
{
    //LogPrintf("InsertCheque\n");    
    string insertstatement="INSERT OR IGNORE INTO blockdomaintable VALUES (?,?);";
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_blob( stat, 1, (char*)blockHash.begin(), 32, NULL );   
   //LogPrintf("GetInsertSql2 %i\n",result); 
   result=sqlite3_bind_blob( stat, 2, (const char*)&sBlockDomains[0], sBlockDomains.size(), NULL );   
   //LogPrintf("GetInsertSql3 %i\n",result);   
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
        LogPrintf("InsertCheque failed result %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK);
}
bool CSqliteWrapper::GetBlockDomains(const uint256 blockHash,CDataStream& sBlockDomains)
{
    //LogPrintf("CSqliteWrapper GetBlockDomains \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT mapdomains FROM blockdomaintable WHERE blockhash = ?;";
    //sprintf(sql,selectstatement,blockHash.GetHex().c_str());
    
    //LogPrintf("CSqliteWrapper GetBlockDomains sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , selectstatement , strlen(selectstatement) , &stmt , NULL);
    rc= sqlite3_bind_blob( stmt, 1, (char*)blockHash.begin(), 32, NULL );   
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           sBlockDomains.write((char*)sqlite3_column_blob(stmt,0),sqlite3_column_bytes(stmt,0));            
           sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }    
    return false;     
}
bool CSqliteWrapper::Insert(const CDomain& domain,const int64_t ownerID)
{
    //LogPrintf("GetInsertSql\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domain10000":"domain100");
    char insertstatement[2000];    
    sprintf(insertstatement,"INSERT OR IGNORE INTO %s VALUES (?,?,?,?,?,?,?,?,?,?)",tableName);
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);
   result=sqlite3_bind_text(stat,1, domain.strDomain.c_str(),domain.strDomain.size(),NULL);       
   //LogPrintf("GetInsertSql2 %i\n",result);
   result=sqlite3_bind_int64(stat,2, (int64_t)domain.nExpireTime);
   //LogPrintf("GetInsertSql3 %i\n",result);
   //LogPrintf("GetInsertSql owner size %i,owner%s:\n",domain.owner.size(),domain.owner.ToString());
   result=sqlite3_bind_int64( stat, 3, ownerID); 
//   char* owner=(char*)sqlite3_column_blob(stat,3);   
//   LogPrintf("GetInsertSql after bind owner %s:\n",HexStr(owner,owner+domain.owner.size()) );
   //LogPrintf("GetInsertSql4 %i\n",result);
   result=sqlite3_bind_int(stat,4, domain.redirectType);
   //LogPrintf("GetInsertSql5 %i\n",result);
   result=sqlite3_bind_blob( stat, 5, domain.redirectTo.c_str(), domain.redirectTo.size(), NULL ); 
   //LogPrintf("GetInsertSql6 %i\n",result);
   result=sqlite3_bind_text( stat, 6, domain.strAlias.c_str(), domain.strAlias.size(), NULL ); 
   //LogPrintf("GetInsertSql7 %i\n",result);

   int64_t nLink=domain.iconLink.SerializeInt();
   result=sqlite3_bind_int64( stat, 7, nLink); 
   
   //LogPrintf("GetInsertSql8 %i\n",result);
    result=sqlite3_bind_text( stat, 8, domain.strIntro.c_str(), domain.strIntro.size(), NULL ); 
   // LogPrintf("GetInsertSql9 %i\n",result);

    string str="";
    for(unsigned int i=0;i<domain.vDirectHistory.size();i++)    
        str.append(domain.vDirectHistory[i].Serialize());
    
    result=sqlite3_bind_blob( stat, 9, str.c_str(), str.size(), NULL ); 
    result=sqlite3_bind_int64( stat, 10, domain.nLockValue); 
    //LogPrintf("GetInsertSql10 %i\n",result);
    
    //for(unsigned int i=0;i<domain.vTags.size();i++)
//        for(unsigned int i=0;i<3;i++)
//    {
//            string str=(i<domain.vTags.size()?domain.vTags[i]:"");
//            result=sqlite3_bind_text( stat, 10+i, str.c_str(), str.size(), NULL ); 
//        //result=sqlite3_bind_text( stat, 10+i, domain.vTags[i].c_str(), domain.vTags[i].size(), NULL ); 
//           //LogPrintf("GetInsertSql%i %i\n",i+1,result);
//    }
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
        LogPrintf("insert domain failed %i\n",result);
       sqlite3_finalize( stat );
       return (result==SQLITE_OK);
}
bool CSqliteWrapper::BatchUpdate(const char* tableName,const char* indexColumnName,const int format1,const char* changedColumnName,const int format2,const vector<pair<string,string> >& vValue)
    {
       
    //LogPrintf("BatchUpdate\n");  
    char sql[7000]; 
    sprintf(sql,"UPDATE %s SET %s =? WHERE %s = ?",tableName,changedColumnName,indexColumnName);
    
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   //LogPrintf("BatchUpdate sql %i\n",result);   
   for(unsigned int i=0;i<vValue.size();i++)
   {
       if(!BindValue(stat,1,format2,vValue[i].second))
        return false;
       if(!BindValue(stat,2,format1,vValue[i].first))
       return false;        
        result=sqlite3_step( stat );
        if(result!=0&&result!=101)
            LogPrintf("BatchUpdate failed result %i\n",result);
        sqlite3_clear_bindings(stat);
        sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::Update(const CDomain& domain,const int64_t ownerID)
{  
   // LogPrintf("CSqliteWrapper::Update domain\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domain10000":"domain100");
    char updatestatement[2000];    
    sprintf(updatestatement,"UPDATE %s SET expiredate =?, owner= ?, redirecttype=?,redirrectto =?, \
            alias=?, icon=?,intro=?,redirecthsitory=?,lockvalue=? WHERE domainname = ?",tableName);
    //LogPrintf("CSqliteWrapper::Update domain%s\n",updatestatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, updatestatement, -1, &stat, 0 );
   //LogPrintf("Updatedomain1 %i\n",result);   
   result=sqlite3_bind_int64(stat,1, (int64_t)domain.nExpireTime);
   //LogPrintf("GetUpdateSql2 %i\n",result);
  // LogPrintf("GetUpdateSql owner size %i,owner%s:\n",domain.owner.size(),domain.owner.ToString());
   result=sqlite3_bind_int64( stat, 2, ownerID);

   //LogPrintf("GetUpdateSql3 %i\n",result);
   result=sqlite3_bind_int(stat,3, domain.redirectType);
   //LogPrintf("GetUpdateSql4 %i\n",result);
   result=sqlite3_bind_blob( stat, 4, domain.redirectTo.c_str(), domain.redirectTo.size(), NULL ); 
   //LogPrintf("GetUpdateSql5 %i\n",result);
   result=sqlite3_bind_text( stat, 5, domain.strAlias.c_str(), domain.strAlias.size(), NULL ); 
   //LogPrintf("GetUpdateSql6 %i\n",result);
   result=sqlite3_bind_int64( stat, 6, domain.iconLink.SerializeInt()); 
   //LogPrintf("GetUpdateSql7 %i %s\n",result,HexStr(str1.begin(),str1.end()));
    result=sqlite3_bind_text( stat, 7, domain.strIntro.c_str(), domain.strIntro.size(), NULL ); 
    //LogPrintf("GetUpdateSql8 %i\n",result);
    
    string str="";
    for(unsigned int i=0;i<domain.vDirectHistory.size();i++)
    {    
        //LogPrintf("GetUpdateSql9 vDirectHistory %s\n",domain.vDirectHistory[i].ToString());
        str+=domain.vDirectHistory[i].Serialize();
    }
    //LogPrintf("GetUpdateSql9 str size %i\n",str.size());
    result=sqlite3_bind_blob( stat, 8, str.c_str(), str.size(), NULL ); 
    //LogPrintf("GetUpdateSql9 %i\n",result);
    result=sqlite3_bind_int64( stat, 9, domain.nLockValue); 
    
    result=sqlite3_bind_text(stat,10, domain.strDomain.c_str(),domain.strDomain.size(),NULL);       
  // LogPrintf("GetUpdateSql11 %i\n",result);
    result=sqlite3_step( stat );
    //LogPrintf("CSqliteWrapper::Update domain%i\n",result);
   sqlite3_finalize( stat );
   if(result!=0&&result!=101)
   LogPrintf("CSqliteWrapper::Update domain failed %i\n",result);
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::Delete(const CDomain& domain)
{
    //LogPrintf("GetdeleteSql\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domain10000":"domain100");
    char deletetatement[2000];    
    sprintf(deletetatement,"DELETE FROM %s WHERE domainname = %s;",tableName,domain.strDomain.c_str());
    char* zErrMsg=0;
     //LogPrintf("CSqliteWrapper Delete \n");
     sqlite3_exec(pdb,deletetatement,0,0,&zErrMsg);
     //LogPrintf("CSqliteWrapper Delete done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:Delete domain error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::Delete(const char* tableName,const char* searchColumn,const char* searchValue,const char* chOperator)
{
    //LogPrintf("GetdeleteSql\n");    
    char deletetatement[2000];    
    sprintf(deletetatement,"DELETE FROM %s WHERE %s %s %s;",tableName,searchColumn,chOperator,searchValue);
    char* zErrMsg=0;
    sqlite3_exec(pdb,deletetatement,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper Delete statement:%s,r:%s\n",deletetatement,zErrMsg);
     //LogPrintf("CSqliteWrapper Delete done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:Delete error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::GetDomain(const char* tableName,const char* searchColumn,const char* searchOperator,const char* searchValue,std::vector<CDomain>& vDomain,const int nMaxResult,bool fGetTags) const
{
    //LogPrintf("CSqliteWrapper GetDomain \n");
    //char* zErrMsg=0;
    const char* selectstatement="SELECT rowid,domainname,expiredate,owner,redirecttype,redirrectto\
    ,alias,icon,intro,redirecthsitory,lockvalue FROM %s WHERE %s %s %s ORDER BY lockvalue DESC LIMIT %i;";    
    char sql[2000];
    sprintf(sql,selectstatement,tableName,searchColumn,searchOperator,searchValue,nMaxResult);
    
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    //LogPrintf("CSqliteWrapper::GetDomain sql %s rx:%i\n",sql,rc); 
    if(rc != SQLITE_OK)
    {
        if(stmt)
        {
            sqlite3_finalize(stmt);
        }        
        return false;
    }
    //int nColumn = sqlite3_column_count(stmt);
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           CDomain domain;
           int64_t domainid=sqlite3_column_int64(stmt,  0);
           domain.strDomain=(char*)sqlite3_column_text(stmt,  1);
            //LogPrintf("GetDomain strDomain %s \n",domain.strDomain); 
            domain.nDomainGroup=GetDomainGroup(domain.strDomain);    
            domain.nExpireTime= (uint32_t)sqlite3_column_int64(stmt,  2);
            //LogPrintf("CDomain CDomain() expiretime %i \n",domain.nExpireTime); 
            int64_t ownerID=sqlite3_column_int64(stmt,  3);
            char chOwnerID[20];
            sprintf(chOwnerID,"%lld",ownerID);
            string ownerStr;
            SearchStr("table_script2txpos","scriptindex",chOwnerID,"script",SQLITEDATATYPE_BLOB,ownerStr);
            domain.owner.assign(ownerStr.begin(),ownerStr.end());
            //domain.owner=CScript((unsigned char*)sqlite3_column_blob(stmt,  3),(unsigned char*)sqlite3_column_blob(stmt,  3)+sqlite3_column_bytes(stmt,  3));
            //LogPrintf("CDomain CDomain() owner %s\n",domain.owner.ToString());                    
            domain.redirectType=sqlite3_column_int(stmt,  4);
            //LogPrintf("CDomain CDomain() redirectType %i \n",domain.redirectType);     
            domain.redirectTo=string((char*)sqlite3_column_blob(stmt,5),(char*)sqlite3_column_blob(stmt,5)+sqlite3_column_bytes(stmt,5));
            //LogPrintf("CDomain CDomain() redirectTo %s \n",HexStr(domain.redirectTo.begin(),domain.redirectTo.end()));   
            if(domain.redirectType>=0&&domain.redirectTo.size()>0)
                domain.redirect.EncodeUnit(domain.redirectType,domain.redirectTo);
            domain.strAlias=(char*)sqlite3_column_text(stmt,6);
            //LogPrintf("CDomain CDomain() strAlias %s \n",domain.strAlias); 
            //LogPrintf("CDomain CDomain() iconlink len %i \n",sqlite3_column_bytes(stmt,7)); 
            //if(sqlite3_column_bytes(stmt,6)>0)
            //{
                //CDataStream s((char*)sqlite3_column_blob(stmt,6),(char*)sqlite3_column_blob(stmt,6)+sqlite3_column_bytes(stmt,6),0,0);
                //string str((char*)sqlite3_column_blob(stmt,6),(char*)sqlite3_column_blob(stmt,6)+sqlite3_column_bytes(stmt,6));
                int64_t nLink=sqlite3_column_int64(stmt,  7);
                domain.iconLink.Unserialize(nLink);        
            //}
            //LogPrintf("CDomain() iconLink %s \n",domain.iconLink.ToString()); 
            domain.strIntro=(char*)sqlite3_column_text(stmt,8);  
            //LogPrintf("CDomain() strIntro %s \n",domain.strIntro);     
            //int len=(int)(sqlite3_column_bytes(stmt,8)/8);
            //if(len>0)
//            {
//                for (int i=0;i<len;i++)
                //{       
                    CLink link; 
                    //CDataStream s((char*)sqlite3_column_blob(stmt,8)+8*i,(char*)sqlite3_column_blob(stmt,8)+8*(i+1),0,0);  
                    //string str((char*)sqlite3_column_blob(stmt,8)+8*i,(char*)sqlite3_column_blob(stmt,8)+8*(i+1));  
                    string str((char*)sqlite3_column_blob(stmt,9),(char*)sqlite3_column_blob(stmt,9)+sqlite3_column_bytes(stmt,9));  
                    while(link.Unserialize(str))
                        domain.vDirectHistory.push_back(link); 
                    //LogPrintf("CDomain() vDirectHistory %i \n",domain.vDirectHistory.size()); 
            domain.nLockValue=sqlite3_column_int64(stmt,  10);        
                //}
            //}    
            
            //LogPrintf("CDomain() nLockValue %i \n",domain.nLockValue); 
            if(fGetTags)
            {
                vector<int64_t> vTagIDs;
                char chDomainID[20];
                sprintf(chDomainID,"%lld",domainid);
                //LogPrintf("Getdomain,search tags, tablename:%s,tablename=domain10000:%b",tableName,tableName=="domain10000");
                SearchInts((string(tableName)=="domain10000")?"domaintag10000":"domaintag100","domainid",chDomainID,"tagid",vTagIDs);
                char chTagList[2000];
                if(vTagIDs.size()>0)
                {
                    string tmp;
                    sprintf(chTagList,"(%lld",vTagIDs[0]);
                    tmp.assign(chTagList);
                    for(unsigned int i=1;i<vTagIDs.size();i++)
                    {
                        sprintf(chTagList,"%s,%lld",tmp.c_str(),vTagIDs[i]);
                        tmp.assign(chTagList);
                    }
                     sprintf(chTagList,"%s)",tmp.c_str());
                    // LogPrintf("CDomain() search tages sql %s\n",chTagList);
                    SearchStrs("tagid","tagid",chTagList,"tag",SQLITEDATATYPE_TEXT,domain.vTags,"IN",10);
                }
           }      
            vDomain.push_back(domain);
               
        }
        else if(rc == SQLITE_DONE)
        {            
            sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }
     
         
    }while(1);
    return false;
     
}
//bool CSqliteWrapper::GetExpiredDomainIDs(const char* tableName,vector<int64_t>& vDomainIDs,const uint32_t time)
//{
//  LogPrintf("CSqliteWrapper GetDomain \n");
//    //char* zErrMsg=0;
//    const char* selectstatement="SELECT rowid FROM %s WHERE expiredate < %i;";    
//    char sql[2000];
//    sprintf(sql,selectstatement,tableName,time);
//    sqlite3_stmt  *stmt = NULL;
//    int rc;
//    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
//    if(rc != SQLITE_OK)
//    {
//        if(stmt)
//        {
//            sqlite3_finalize(stmt);
//        }        
//        return false;
//    }
//    //int nColumn = sqlite3_column_count(stmt);
//    do{ 
//        rc = sqlite3_step(stmt);
//        if(rc == SQLITE_ROW)
//        {  
//            vDomainIDs.push_back(sqlite3_column_int64(stmt,  0));
//        }
//        else if(rc == SQLITE_DONE)
//        {            
//            sqlite3_finalize(stmt);
//            return true;
//        }
//        else
//        {
//            sqlite3_finalize(stmt);
//            return false;
//        }
//    }while(1);
//    return true;
//}
bool CSqliteWrapper::Get(const char* tableName,const char* searchColumn,const char* searchValue,char**& result,int& nRow,int& nColumn) const
{
    //LogPrintf("CSqliteWrapper Get \n");
    char* zErrMsg=0;
    const char* selectstatement="SELECT * FROM %s WHERE %s = %s;";    
    char sql[2000];
    sprintf(sql,selectstatement,tableName,searchColumn,searchValue);
     sqlite3_get_table(pdb,sql,&result,&nRow,&nColumn,&zErrMsg);    
     LogPrintf("CSqliteWrapper Get done\n");
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:get error:tableName %s,columnname %s,value%s,error %s",tableName,searchColumn,searchValue,zErrMsg);
         return false;
     }
    
     return true;
}
bool CSqliteWrapper::InsertTag(const char* tableName,const int64_t tagID,const int64_t nLink)
{
    LogPrintf("InsertTag\n");    
    string insertstatement="INSERT INTO %s VALUES (?,?)";
    char sql[2000]; 
     sprintf(sql,insertstatement.c_str(),tableName);
    //LogPrintf("GetInsertSql %s\n",sql);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_int64(stat,1, nLink);   
   //LogPrintf("GetInsertSql2 %i\n",result);   
   result=sqlite3_bind_int64(stat,2, tagID);  
   //LogPrintf("GetInsertSql3 %i\n",result);  
    result=sqlite3_step( stat );
    LogPrintf("InsertTag %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK);
}
bool CSqliteWrapper::InsertTags(const char* tableName,const int64_t nLink,const vector<int64_t>& vTagID)
{
   // LogPrintf("InsertTags ntags:%i\n",vTagID.size());    
    char sql[2000]; 
    string insertstatement="INSERT OR IGNORE INTO %s VALUES (?,?)";
     sprintf(sql,insertstatement.c_str(),tableName);
  //  LogPrintf("GetInsertSql %s\n",sql);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   for(unsigned int i=0;i<vTagID.size();i++)
   {
       result=sqlite3_bind_int64(stat,1,nLink);   
       //LogPrintf("GetInsertSql2 %i\n",result);   
       result=sqlite3_bind_int64(stat,2,vTagID[i]  );  
      // LogPrintf("GetInsertSql3 %i\n",result);  
        result=sqlite3_step( stat );
       // LogPrintf("InsertTag result %i\n",result);
        if(result!=0&&result!=101)
                LogPrintf("InsertTags failed result %i\n",result);
        sqlite3_clear_bindings(stat);
        sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}

bool CSqliteWrapper::InsertContent(const int64_t nLink,const int64_t pos,const int64_t sender, const int cc, const int64_t lockValue,const uint32_t lockTime)
{
    
    //LogPrintf("InsertContent\n");    
    string insertstatement="INSERT OR IGNORE INTO table_content VALUES (?,?,?,?,?,?)";
    //LogPrintf("GetInsertSql %s\n",insertstatement.c_str());
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_int64(stat,1, nLink);   
   //LogPrintf("GetInsertSql2 %i\n",result);   
   result=sqlite3_bind_int64(stat,2, pos);  
   //LogPrintf("GetInsertSql3 %i\n",result); 
   result=sqlite3_bind_int64(stat,3, sender);  
   //LogPrintf("GetInsertSql3 %i\n",result); 
   result=sqlite3_bind_int(stat,4, cc);  
   //LogPrintf("GetInsertSql3 %i\n",result); 
   result=sqlite3_bind_int64(stat,5, lockValue);  
   //LogPrintf("GetInsertSql3 %i\n",result); 
   result=sqlite3_bind_int64(stat,6, (int64_t)lockTime);  
   //LogPrintf("GetInsertSql3 %i\n",result); 
    result=sqlite3_step( stat );
    if(result!=SQLITE_OK&&result!=101)
    LogPrintf("InsertContent failed %i\n",result);
    
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::InsertContents(const vector<CContentDBItem>& vContents,const map<CScript,int64_t>& mapScriptIndex)
{
    
    //LogPrintf("InsertContents %i\n",vContents.size());    
    string insertstatement="INSERT OR IGNORE INTO table_content VALUES (?,?,?,?,?,?)";
    //LogPrintf("GetInsertSql %s\n",insertstatement.c_str());
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   for(unsigned int i=0;i<vContents.size();i++)
   {
       //LogPrintf("GetInsertSql1 %i\n",result);   
       result=sqlite3_bind_int64(stat,1, vContents[i].link.SerializeInt());   
       //LogPrintf("GetInsertSql2 %i\n",result);   
       result=sqlite3_bind_int64(stat,2, vContents[i].pos);  
       //LogPrintf("GetInsertSql3 %i\n",result); 
       if(mapScriptIndex.find(vContents[i].sender)==mapScriptIndex.end())
       {
           LogPrintf("script not found %s\n",vContents[i].sender.ToString()); 
           return false;
       }
       result=sqlite3_bind_int64(stat,3, mapScriptIndex.find(vContents[i].sender)->second);  
       //LogPrintf("GetInsertSql3 %i\n",result); 
       result=sqlite3_bind_int(stat,4, vContents[i].cc);  
       //LogPrintf("GetInsertSql3 %i\n",result); 
       result=sqlite3_bind_int64(stat,5, vContents[i].lockValue);  
       //LogPrintf("GetInsertSql3 %i\n",result); 
       result=sqlite3_bind_int64(stat,6, (int64_t)vContents[i].lockTime);  
       //LogPrintf("GetInsertSql3 %i\n",result); 
        result=sqlite3_step( stat );
        //LogPrintf("InsertContent %i\n",result);
        if(result!=0&&result!=101)
                    LogPrintf("InsertContent failed result %i\n",result);
        sqlite3_clear_bindings(stat);
        sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::DeleteContents(const vector<CContentDBItem>& vContents)
{
    bool result = true;
    const char* tableName = "table_content";
    for (unsigned int ii = 0; ii < vContents.size(); ii++)
    {
        char chInt[100];
        sprintf(chInt,"%lld",vContents[ii].link.SerializeInt());
        result &= Delete(tableName,"link",chInt ,"=" );
    }
    return result;
}
    
bool CSqliteWrapper::SearchContents(const vector<int64_t>& vSenderIDs,const vector<int>& vCCs,const vector<int64_t>& vTagIDs,vector<CContentDBItem>& vContents,const int nMaxResults,const int nOffset)
{
    //LogPrintf("CSqliteWrapper GetContents \n");    
    char sql[100000];
    const char* selectstatement="SELECT * FROM table_content %s%s%s%s ORDER BY lockvalue DESC LIMIT %i OFFSET %i;";  
    string strWhere="";
    if((vSenderIDs.size()+vCCs.size()+vTagIDs.size())>0)
        strWhere="WHERE ";    
    char chTag[1000];
    chTag[0]=0;
    string strtmp;
    if(vTagIDs.size()>0)
    {
        if(vTagIDs.size()>10)
            return false;  
        const char* tagselectstatement="SELECT link FROM tag WHERE tagid =%lld ";  
        sprintf(chTag,tagselectstatement,vTagIDs[0]);
        const char* tagselectstatement2="SELECT link FROM tag WHERE link IN(%s) AND tagid=%lld";
        for(unsigned int i=1;i<vTagIDs.size();i++)   
        {
            strtmp.assign(chTag);
            sprintf (chTag,tagselectstatement2,strtmp.c_str(),vTagIDs[i]);
        }
        strtmp.assign(chTag);
        sprintf(chTag,"link IN(%s) ",strtmp.c_str());
    }
    char chSender[2000];
    chSender[0]=0;
    const char* chAnd="AND ";
    const char* chEmpty="";
    
    if(vSenderIDs.size()>0)
    {
        if(vSenderIDs.size()>100)
            return false;     
        sprintf(chSender,"%lld",vSenderIDs[0]);     
        for(unsigned int i=1;i<vSenderIDs.size();i++)      
        {
            strtmp.assign(chSender);
            sprintf(chSender,"%s,%lld",strtmp.c_str(),vSenderIDs[i]);    
        }
        strtmp.assign(chSender);
        sprintf(chSender,"%ssender IN(%s) ",(vTagIDs.size()>0)?chAnd:chEmpty,strtmp.c_str());
    } 
    char chCC[2000];
    chCC[0]=0;
    string strCC;
    if(vCCs.size()>0)
    {
        if(vCCs.size()>10)
            return false;     
        sprintf(chCC,"%lld",vCCs[0]);     
        //LogPrintf("CSqliteWrapper GetContents chCC: %s\n",chCC); 
        for(unsigned int i=1;i<vCCs.size();i++)     
        {
            strCC.assign(chCC);
            sprintf(chCC,"%s,%lld",strCC.c_str(),vCCs[i]);  
        }
       // LogPrintf("CSqliteWrapper GetContents chCC: %s\n",chCC);   
        strCC.assign(chCC);
        sprintf(chCC,"%scc IN(%s) ",(vTagIDs.size()+vSenderIDs.size())>0?chAnd:chEmpty,strCC.c_str());
       // LogPrintf("CSqliteWrapper GetContents chCC: %s\n",chCC); 
    } 
    sprintf(sql,selectstatement,strWhere.c_str(),chSender,chTag,chCC,nMaxResults,nOffset);
    
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    LogPrintf("CSqliteWrapper GetContents sql: %s,rc:%i\n",sql,rc); 
    if(rc != SQLITE_OK)
    {
        if(stmt)
        {
            sqlite3_finalize(stmt);
        }        
        return false;
    }    
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
            CContentDBItem item;
           int64_t nLink=sqlite3_column_int64(stmt,0);
           item.link.Unserialize(nLink);
           item.pos=sqlite3_column_int64(stmt,1);
           int64_t nSender=sqlite3_column_int64(stmt,2);
           string senderStr;
           char chSenderID[30];
           sprintf (chSenderID,"%lld",nSender);
            SearchStr("table_script2txpos","scriptindex",chSenderID,"script",SQLITEDATATYPE_BLOB,senderStr);
            item.sender.assign(senderStr.begin(),senderStr.end());       
            item.cc=sqlite3_column_int(stmt,3);
            item.lockValue=sqlite3_column_int64(stmt,4);
            item.lockTime=(uint32_t)sqlite3_column_int64(stmt,5);                
            vContents.push_back(item);               
        }
        else if(rc == SQLITE_DONE)
        {            
            sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }
        //nItems++;
    }while(1);//(nItems<nLimit);
    return false;         
}

//bool CSqliteWrapper::GetLinks(const vector<string>& vTag,const int cc,const CLink link,std::vector<CLink>& vLink,const int nMaxItems,const int nOffset) const
//{
//    LogPrintf("CSqliteWrapper GetLinks \n");
//    //char* zErrMsg=0;
//    const char* selectstatement="SELECT link FROM tag WHERE %s = %s;";    
//    char sql[2000];    
//    if(!link.IsEmpty())
//    {
//        string strLink="x'";
//        string strLink1=link.Serialize();
//        strLink.append(HexStr(strLink1.begin(),strLink1.end()));
//        strLink.append("'");
//        sprintf(sql,selectstatement,"link",strLink.c_str());
//    }
//    else
//    {        
//        vector<int> vTagID;
//        for(unsigned int i=0;i<vTag.size();i++)
//        {
//            int64_t nTagID;
//           if(GetTagID(vTag[i],nTagID)) 
//               vTagID.push_back(nTagID);
//        }
//        if(vTagID.size()==0&&cc==-1)
//            return false;
//        if(cc!=-1)
//        {
//            
//            selectstatement="SELECT link FROM tag WHERE cc = %i";    
//            sprintf(sql,selectstatement,cc);
//            if(vTag.size()>0)
//            {
//                selectstatement="%s AND tagid= %i";    
//                sprintf(sql,selectstatement,sql,vTagID[0]);
//            }
//        }
//        else
//        {
//            selectstatement="SELECT link FROM tag WHERE tagid= %i";    
//            sprintf(sql,selectstatement,vTagID[0]);
//        }
//        LogPrintf("CSqliteWrapper GetLinks sql %s\n",sql);
//        for(unsigned int i=1;i<vTagID.size();i++)
//        {
//            selectstatement="SELECT link FROM tag WHERE tagid= %i AND link IN(%s)";    
//            sprintf(sql,selectstatement,vTagID[i],sql);
//            
//        }
//        selectstatement="%s limit %i offset %i;"; 
//        sprintf(sql,selectstatement,sql,nMaxItems,nOffset);
//    }    
//    
//     LogPrintf("CSqliteWrapper GetLinks sql %s\n",sql); 
//    sqlite3_stmt  *stmt = NULL;
//    int rc;
//    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
//    if(rc != SQLITE_OK)
//    {
//        if(stmt)
//        {
//            sqlite3_finalize(stmt);
//        }        
//        return false;
//    }
//    //int nColumn = sqlite3_column_count(stmt);
//    //int nItems=0;
//    do{ 
//        rc = sqlite3_step(stmt);
//        if(rc == SQLITE_ROW)
//        {
//           CLink link1;
//            if(sqlite3_column_bytes(stmt,0)>0)
//            {                
//                string str((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));
//                LogPrintf("db.getlink link length %i, hex %s \n",sqlite3_column_bytes(stmt,0),HexStr(str.begin(),str.end())); 
//                link1.Unserialize(str);        
//            }
//            LogPrintf("Link %s \n",link1.ToString());             
//            vLink.push_back(link1);               
//        }
//        else if(rc == SQLITE_DONE)
//        {            
//            sqlite3_finalize(stmt);
//            return true;
//        }
//        else
//        {
//            sqlite3_finalize(stmt);
//            return false;
//        }
//        //nItems++;
//    }while(1);//(nItems<nLimit);
//    return false;     
//}
bool CSqliteWrapper::InsertTagID(const string tag,int64_t& tagID)
{
    if (tag.size()>32)
        return false;    
    if(GetTagID(tag,tagID))
        return true;
    //LogPrintf("InsertTagID\n");    
    string insertstatement="INSERT INTO tagid(tag) VALUES (?)";
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );   
   result=sqlite3_bind_text( stat, 1, tag.c_str(), tag.size(), NULL ); 
   //LogPrintf("GetInsertSql4 %i\n",result);
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
    LogPrintf("InsertTagID failed %i\n",result);
   sqlite3_finalize( stat );
   if(!result==SQLITE_OK)
       return false;
   return GetTagID(tag,tagID);   
}
bool CSqliteWrapper::GetTagID(const string tag,int64_t& tagID) const
{
    //LogPrintf("CSqliteWrapper GetTagID \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT tagid FROM tagid WHERE tag = '%s';";    //NOte:for varchar, need to cover by ''
     sprintf(sql,selectstatement,tag.c_str());
    //LogPrintf("CSqliteWrapper GetTagID sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           tagID= sqlite3_column_int64(stmt,  0);             
           sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }    
    return false;     
}
//template <typename K>
//bool CSqliteWrapper::Select141(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,K& searchResult) const

//bool CSqliteWrapper::ClearExpiredTags(const unsigned int nTime)
//{
//    LogPrintf("ClearExpiredTags\n");    
//    char deletestatement[2000];    
//    sprintf(deletestatement,"DELETE FROM tag WHERE expiredate< %i;",nTime);
//    char* zErrMsg=0;
//     //LogPrintf("CSqliteWrapper Delete \n");
//     sqlite3_exec(pdb,deletestatement,0,0,&zErrMsg);
//     //LogPrintf("CSqliteWrapper Delete done\n");     
//     if(zErrMsg)
//     {
//         LogPrintf("sqlitewrapper:ClearExpiredTags error:%s",zErrMsg);
//         return false;
//     }
//     return true;
//}

bool CSqliteWrapper::CreateTxIndexTable()
{    
    char* zErrMsg=0;    
    string tableName="txindextable";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( txindex INTEGER PRIMARY KEY, txid BLOB(32));";    
    sprintf(sql,createtablestatement,tableName.c_str());
    LogPrintf("CSqliteWrapper CreateTxIndexTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTxIndexTable done %s \n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement,"index_txidindextable_txid",tableName.c_str(),"txid");  
    LogPrintf("CSqliteWrapper CreateTxIndexTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTxIndexTable createindex done %s\n",zErrMsg);
   
    return true;
}
bool CSqliteWrapper::CreateChequeTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;
    string tableName="table_unspent";
    char sql[2000];
    const string createtablestatement="CREATE TABLE IF NOT EXISTS %s ( link INTEGER PRIMARY KEY, scriptindex INTEGER, value INTEGER, locktime INTEGER);";    
    sprintf(sql,createtablestatement.c_str(),tableName.c_str());
    LogPrintf("CSqliteWrapper Createtable_unspent statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg); 
    LogPrintf("CSqliteWrapper Createtable_unspent done %s\n",zErrMsg);
    const string createindexstatement="CREATE INDEX IF NOT EXISTS %s on %s(%s)";    
    sprintf(sql,createindexstatement.c_str(),"index_table_unspent_scriptindex",tableName.c_str(),"scriptindex");  
    LogPrintf("CSqliteWrapper Createtable_unspent createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg); 
    LogPrintf("CSqliteWrapper table_unspent createindex done %s \n",zErrMsg);
    return true;
}
//bool CSqliteWrapper::InsertScriptIndex(const CScript script,int& scriptIndex)
//{
//    if(script.size()==0)
//        return false;
//    CScript script1=script;
//    if (script1.size()>255)
//        script1.resize(255);
//    if(GetScriptIndex(script1,scriptIndex))
//        return true;
//    //LogPrintf("InsertScriptIndex\n");    
//    char* insertstatement="INSERT INTO scriptindextable(script) VALUES (?)";
//    //LogPrintf("GetInsertSql %s\n",insertstatement);
//    int result;
//    sqlite3_stmt  *stat;    
//   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );   
//   result=sqlite3_bind_blob( stat, 1,&script1[0], script1.size(), NULL ); 
//   //LogPrintf("GetInsertSql4 %i\n",result);
//    result=sqlite3_step( stat );
//   sqlite3_finalize( stat );
//   if(result!=SQLITE_OK&&result!=101)
//   {
//       LogPrintf("InsertScriptIndex failed %i\n",result);
//       return false;
//   }
//   return GetScriptIndex(script,scriptIndex);   
//}
bool CSqliteWrapper::GetScriptIndex(const CScript script,int64_t& scriptIndex) const
{
    //LogPrintf("CSqliteWrapper GetScriptIndex \n");
    //char* zErrMsg=0;
    //char sql[2000]; 
    if(script.size()>3000)
        return false;
    const char* sql="SELECT scriptindex FROM table_script2txpos WHERE script = ?;";  
    //string strScript=HexStr(script.begin(),script.end());    
    //sprintf(sql,selectstatement.c_str(),strScript.c_str());
    
    //LogPrintf("CSqliteWrapper GetScriptIndex sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    rc=sqlite3_bind_blob( stmt, 1, (char*)&script[0], script.size(), NULL ); 
    if(rc != SQLITE_OK)
    {
        LogPrintf("CSqliteWrapper GetScriptIndex failed %i\n",rc);
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           scriptIndex= sqlite3_column_int64(stmt,  0);             
           sqlite3_finalize(stmt);
           //LogPrintf("CSqliteWrapper GetScriptIndex result %i\n",scriptIndex); 
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }    
    return false;     
}
bool CSqliteWrapper::InsertTxIndex(const uint256 txid,const int64_t& txIndex)
{
    
   // if(GetTxIndex(txid,txIndex))
    //    return true;
    //LogPrintf("InsertTxIndex\n");    
    string insertstatement="INSERT OR IGNORE INTO txindextable VALUES (?,?)";
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );   
  // LogPrintf("GetInsertSql2 %i\n",result);
   result=sqlite3_bind_int64( stat, 1, txIndex);   
   //std::vector<unsigned char> vch=ParseHex(txid.GetHex());
   //result=sqlite3_bind_blob( stat, 2, &vch[0], 32, NULL ); 
   result=sqlite3_bind_blob( stat, 2, txid.begin(), 32, NULL ); 
   //LogPrintf("GetInsertSql4 %i\n",result);
    result=sqlite3_step( stat );
   // LogPrintf("InsertTxIndex result %i\n",result);
   sqlite3_finalize( stat );
   if(result!=SQLITE_OK&&result!=101)
   {
       LogPrintf("InsertTxIndex failed code:%i \n",result);
              return false;
   }
   return true;
   //return GetTxIndex(txid,txIndex);   
}
bool CSqliteWrapper::InsertTxIndice(const map<uint256, int64_t>& mapTxIndex)
{
    
   // if(GetTxIndex(txid,txIndex))
    //    return true;
    //LogPrintf("InsertTxIndex\n");    
    string insertstatement="INSERT OR REPLACE INTO txindextable VALUES (?,?)";
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );   
  // LogPrintf("GetInsertSql2 %i\n",result);
   for(map<uint256, int64_t>::const_iterator it=mapTxIndex.begin();it!=mapTxIndex.end();it++)
   {
       result=sqlite3_bind_int64( stat, 1, it->second);   
       //std::vector<unsigned char> vch=ParseHex(it->first.GetHex());
       //result=sqlite3_bind_blob( stat, 2, &vch[0], 32, NULL ); 
       uint256 txid=it->first;
       result=sqlite3_bind_blob( stat, 2, txid.begin(), 32, NULL ); 
       //LogPrintf("GetInsertSql4 %i\n",result);
        result=sqlite3_step( stat );
       // LogPrintf("InsertTxIndex result %i\n",result);
       
       if(result!=SQLITE_OK&&result!=101)
       {
           LogPrintf("InsertTxIndice failed code:%i \n",result);
                  //return false;
       }
       sqlite3_clear_bindings(stat);
       sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return true;
   //return GetTxIndex(txid,txIndex);   
}
bool CSqliteWrapper::GetTxIndex(const uint256 txid,int64_t& txIndex) const
{
    //LogPrintf("CSqliteWrapper GetTxIndex \n");
    //char* zErrMsg=0;
    const char* sql="SELECT txindex FROM txindextable WHERE txid = ?;";  
    //string strTxid=txid.GetHex();
    //sprintf(sql,selectstatement.c_str());//,strTxid.c_str());
    
    //LogPrintf("CSqliteWrapper GetTxIndex sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        LogPrintf("CSqliteWrapper GetTxIndex failed sql:%s rc:%i \n",sql,rc); 
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc=sqlite3_bind_blob( stmt, 1, txid.begin(), 32, NULL ); 
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           txIndex= sqlite3_column_int64(stmt,  0);             
           sqlite3_finalize(stmt);
           //LogPrintf("CSqliteWrapper GetTxIndex success %i\n",txIndex);
            return true;
        } 
     LogPrintf("CSqliteWrapper GetTxIndex notfound txid: %s\n",txid.GetHex());
            sqlite3_finalize(stmt);
            return false;
         
}
bool CSqliteWrapper::GetTxidByTxIndex(const int64_t txIndex, uint256& txid) const
{
    //LogPrintf("CSqliteWrapper GetTxIndex \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT txid FROM txindextable WHERE txIndex = %lld;";   
    sprintf(sql,selectstatement,txIndex);
    
    //LogPrintf("CSqliteWrapper GetTxidByTxIndex sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        LogPrintf("CSqliteWrapper GetTxidByTxIndex failed %i\n",rc); 
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           string strtxid(); 
           if(sqlite3_column_bytes(stmt,  0)!=32)
               return false;
           memcpy(txid.begin(),(unsigned char*)sqlite3_column_blob(stmt,  0),32);
           sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            sqlite3_finalize(stmt);
            return false;
        }    
    return false;     
}
bool CSqliteWrapper::InsertCheque(int64_t scriptIndex,int64_t link, int64_t nValue,uint32_t nLockTime)
{
    
    //LogPrintf("InsertCheque\n");    
    string insertstatement="INSERT OR IGNORE INTO table_unspent VALUES (?,?,?,?);";
    //LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_int64(stat,1, link);   
  // LogPrintf("GetInsertSql2 %i\n",result);   
   result=sqlite3_bind_int64(stat,2, scriptIndex);  
  // LogPrintf("GetInsertSql3 %i\n",result);
   result=sqlite3_bind_int64(stat,3, nValue); 
   //LogPrintf("GetInsertSql4 %i\n",result);
   result=sqlite3_bind_int64(stat,4, (int64_t)nLockTime);  
  // LogPrintf("GetInsertSql5 %i\n",result);   
    result=sqlite3_step( stat );
    if(!result==SQLITE_OK&&!result==101)
        LogPrintf("InsertCheque failed %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::BatchInsertCheque(vector<CCheque>& vCheque,const map<CScript,int64_t>& mapScriptIndex)
{
    
    //LogPrintf("BatchInsertCheque %i\n",vCheque.size());    
    string insertstatement="INSERT OR IGNORE INTO table_unspent VALUES (?,?,?,?);";
    //LogPrintf("GetInsertSql %s\n",insertstatement.c_str());
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   for(unsigned int i=0;i<vCheque.size();i++)
   {
       //assert(mapScriptIndex.find(vCheque[i].scriptPubKey)!=mapScriptIndex.end());
       int64_t scriptIndex=mapScriptIndex.find(vCheque[i].scriptPubKey)->second;
       int64_t link=(vCheque[i].txIndex<<16)+vCheque[i].nOut;
       //LogPrintf("batch insert cheque: link %lld,scriptIndex %lld,value:%lld,locktime:%lld \n",link,scriptIndex,vCheque[i].nValue,vCheque[i].nLockTime);
       //LogPrintf("GetInsertSql1 %i\n",result);   
       result=sqlite3_bind_int64(stat,1, link);   
       //LogPrintf("GetInsertSql2 %i\n",result);   
       result=sqlite3_bind_int64(stat,2, scriptIndex);  
       //LogPrintf("GetInsertSql3 %i\n",result);
       result=sqlite3_bind_int64(stat,3, vCheque[i].nValue); 
       //LogPrintf("GetInsertSql4 %i\n",result);
       result=sqlite3_bind_int64(stat,4, (int64_t)vCheque[i].nLockTime);  
       //LogPrintf("GetInsertSql5 %i\n",result);   
        result=sqlite3_step( stat );
        if(!result==SQLITE_OK&&!result==101)
            LogPrintf("InsertCheque failed %i\n",result);
        sqlite3_clear_bindings(stat);
        sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::GetCheques(const vector<CScript>& vScript,vector<CCheque> & vCheques,const int nMaxItems,const int nOffset)const
{
    //LogPrintf("CSqliteWrapper GetCheques \n");
    if(vScript.size()>100)
        return false;
    //char* zErrMsg=0;
    const char* selectstatement="SELECT * FROM table_unspent WHERE scriptindex IN(%s) ORDER BY link DESC LIMIT %i OFFSET %i;";    
    char sql[2000]; 
    vector<int64_t> vScriptIndex;
    map<int64_t,CScript> mapScriptIndex;
    for(unsigned int i=0;i<vScript.size();i++)
    {
        int64_t scriptIndex;
       if(!GetScriptIndex(vScript[i],scriptIndex)) 
           continue;
        vScriptIndex.push_back(scriptIndex);
        mapScriptIndex[scriptIndex]=vScript[i];
    }
    if(vScriptIndex.size()==0)
        return false;
    char chScriptList[2000];
    string tmp;
        sprintf(chScriptList,"%lld",vScriptIndex[0]);
        tmp.assign(chScriptList);
    for(unsigned int i=1;i<vScriptIndex.size();i++)
    {
        sprintf(chScriptList,"%s,%lld",tmp.c_str(),vScriptIndex[i]);
        tmp.assign(chScriptList);
    }
     sprintf(sql,selectstatement,chScriptList,nMaxItems,nOffset);
     //LogPrintf("CSqliteWrapper GetCheques sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    //LogPrintf("CSqliteWrapper GetCheques result %i\n",rc);
    if(rc != SQLITE_OK)
    {
        if(stmt)
        {
            sqlite3_finalize(stmt);
        }        
        return false;
    }
    do{ 
        rc = sqlite3_step(stmt);
        
        if(rc == SQLITE_ROW)
        {
           CCheque cheque;
           int64_t nLink=sqlite3_column_int64(stmt,  0);
           GetTxidByTxIndex(nLink>>16,cheque.txid);
           cheque.txIndex=nLink>>16;
           //LogPrintf("GetCheques txid %s \n",cheque.txid.GetHex());
           cheque.nOut=nLink&0xffff;
           //LogPrintf("GetCheques nOut %i \n",cheque.nOut);
           int64_t scriptIndex=sqlite3_column_int64(stmt,  1);           
           cheque.scriptPubKey=mapScriptIndex[scriptIndex];
          // LogPrintf("GetCheques scriptPubKey %s \n",cheque.scriptPubKey.ToString());
           cheque.nValue=(uint64_t)sqlite3_column_int64(stmt,  2);
          // LogPrintf("GetCheques nValue %i \n",cheque.nValue);
           cheque.nLockTime=(uint32_t)sqlite3_column_int64(stmt,  3);
         //  LogPrintf("GetCheques nLockTime %i \n",cheque.nLockTime);
           vCheques.push_back(cheque);  
        }
        else if(rc == SQLITE_DONE)
        {            
            sqlite3_finalize(stmt);
            return true;
        }
        else
        {
            LogPrintf("CSqliteWrapper GetCheques error %i\n",rc);
            sqlite3_finalize(stmt);
            return false;
        }
    }while(1);
    return false;     
}
//bool CSqliteWrapper::EraseCheque(const int64_t txindex)
//{
//    //LogPrintf("EraseCheque\n");    
//    char deletestatement[2000];    
//    sprintf(deletestatement,"DELETE FROM table_unspent WHERE txindex = %lld ;",txindex);
//    char* zErrMsg=0;
//    // LogPrintf("CSqliteWrapper Delete \n");
//     sqlite3_exec(pdb,deletestatement,0,0,&zErrMsg);
//    // LogPrintf("CSqliteWrapper Delete done\n");     
//     if(zErrMsg)
//     {
//         LogPrintf("sqlitewrapper:EraseCheque error:%s",zErrMsg);
//         return false;
//     }
//     return true;
//}
//bool CSqliteWrapper::BatchEraseCheque(vector<int64_t> vChequeErase)
//{
//    //LogPrintf("EraseCheque\n");    
//    char deletestatement[2000];   
//    char strList[100000];
//    if(vChequeErase.size()==0)
//        return false;
//    sprintf(strList,"%lld",vChequeErase[0]);
//    for(unsigned int i=1;i<vChequeErase.size();i++)
//    {
//        sprintf(strList,"%s,%lld",strList,vChequeErase[i]);
//    }
//    sprintf(deletestatement,"DELETE FROM table_unspent WHERE txindex IN(%s);",strList);
//    char* zErrMsg=0;
//    // LogPrintf("CSqliteWrapper Delete \n");
//     sqlite3_exec(pdb,deletestatement,0,0,&zErrMsg);
//    // LogPrintf("CSqliteWrapper Delete done\n");     
//     if(zErrMsg)
//     {
//         LogPrintf("sqlitewrapper:EraseCheque error:%s",zErrMsg);
//         return false;
//     }
//     return true;
//}

bool CSqliteWrapper::GetBlockPosItem(const int64_t nPosDB,uint256& hashBlock,int& nHeight)
{
    //char sql[2000]; 
    string tableName="table_blockpos";
    const char* sql="SELECT blockhash,blockheight FROM table_blockpos WHERE pos< ? ORDER BY pos DESC LIMIT 1;";    
    // sprintf(sql,selectstatement,nPosDB);
    //LogPrintf("CSqliteWrapper GetBlockPosItem sql %s\n",sql); 
    
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);    
    if(rc != SQLITE_OK)
    {
        if(stmt)        
            sqlite3_finalize(stmt);               
        return false;
    }
    rc=sqlite3_bind_int64( stmt, 1, nPosDB); 
    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW)
    {
        //string strHash=string((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));                    
        //hashBlock.SetHex(HexStr(strHash.begin(),strHash.end()));
        //LogPrintf("GetBlockPosItem:hash:%s \n",hashBlock.GetHex());
        memcpy(hashBlock.begin(),(unsigned char*)sqlite3_column_blob(stmt,  0),32);
        nHeight=   sqlite3_column_int(stmt,  1); 
        //LogPrintf("GetBlockPosItem: height:%i \n",nHeight);
        sqlite3_finalize(stmt);
        return true;
    }
    else    
        sqlite3_finalize(stmt);        
    return false;      
}
bool CSqliteWrapper::WriteBlockPos(const int64_t nPosDB,const uint256& hashBlock,const int& nHeight)
{
    //LogPrintf("WriteBlockPos\n");  
    //char sql[2000]; 
    const string insertstatement="INSERT OR REPLACE INTO table_blockpos VALUES (?,?,?);";
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement.c_str(), -1, &stat, 0 );
   //LogPrintf("WriteBlockPos %i\n",result);   
    result=sqlite3_bind_int( stat, 1, nHeight);    
    //LogPrintf("WriteBlockPos %i\n",result);   
    //std::vector<unsigned char> vch=ParseHex(hashBlock.GetHex());
    //LogPrintf("WriteBlockPos %i\n",result);   
    //result=sqlite3_bind_blob( stat, 2, &vch[0], 32, NULL );
    result=sqlite3_bind_blob( stat, 2, hashBlock.begin(), 32, NULL );
    result=sqlite3_bind_int64( stat, 3, nPosDB);    
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
        LogPrintf("WriteBlockPos failed result %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}