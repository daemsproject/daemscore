// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CCCOIN_SQLITEWRAPPER_H
#define CCCOIN_SQLITEWRAPPER_H

#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "util.h"
#include "ccc/link.h"
#include "version.h"
//#include "script/script.h"
#include "utilstrencodings.h"

#include <boost/filesystem/path.hpp>
#include <sqlite3.h>
class CDomain;
class uint256;

using namespace std;
enum sqlitedatatype
{
    SQLITEDATATYPE_INT,
    SQLITEDATATYPE_INT64,
    SQLITEDATATYPE_TEXT,
    SQLITEDATATYPE_BOOL,
    SQLITEDATATYPE_CHAR,
    SQLITEDATATYPE_VARCHAR,
    SQLITEDATATYPE_BLOB,
};

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
class CContentDBItem
{
public:
   CLink link;
   int64_t pos;
   CScript sender;
   int cc;
   CAmount lockValue;
   uint32_t lockTime;
   vector<string>vTags;
   string senderDomain;
   //string strContent;
   CContentDBItem()
   {
       pos=0;
       cc=0;
       lockValue=0;
       lockTime=0;
   }
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
   ADD_SERIALIZE_METHODS;   

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {        
        READWRITE(link);         
        READWRITE(sender);  
        READWRITE(senderDomain);   
        READWRITE(VARINT(cc));
        READWRITE(VARINT(lockValue));
        READWRITE(VARINT(lockTime));
        READWRITE(vTags);
    }
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
        txIndex=-1<<16;
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
class CSqliteWrapper
{
private:
    
    //! the database itself
    sqlite3* pdb;
    void HandleError(const int& status) throw(sqlite_error);
public:
    CSqliteWrapper(const boost::filesystem::path& path);
    ~CSqliteWrapper();
    bool CreateTables();
    bool ClearTable(const char* tableName);
    bool CreateDomainTable(const char* tableName);
    bool CreateDomainTagTable(const char* tableName);
    bool CreateScript2TxPosTable();
    bool CreateBlockPosTable();
    bool CreateBlockDomainTable();
    bool CreateContentTable();
    bool CreateTagIDTable();
    bool CreateTagTable();
    bool CreateTxIndexTable();
    bool CreateChequeTable();
    
    bool BeginBatch();
    bool EndBatch();
    template <typename V>
    bool BindValue(sqlite3_stmt* stat,const int n,const int format,const V& value)
    {
        int result;
        switch (format)
         {
//            case SQLITEDATATYPE_INT:
//               result=sqlite3_bind_int( stat, n, (int)value);              
//                    break; 
//            case SQLITEDATATYPE_INT64:
//               result=sqlite3_bind_int64( stat, n, (int64_t)value);              
//                    break;  
            case SQLITEDATATYPE_TEXT:
                    result=sqlite3_bind_text(stat,n, value.c_str(),value.size(),NULL);
                    break;            
            case SQLITEDATATYPE_BLOB:
                result=sqlite3_bind_blob( stat, n, (const char*)&value[0], value.size(), NULL );
                break;                 
            default:
                return false;              
        }
        if(result!=SQLITE_OK)
            return false;
        return true;
    }
    //string only
    template <typename V1,typename V2>
    bool Insert(const char* tableName,const char* columnName1,const int format1,const V1& value1,const char* columnName2,const int format2,const V2& value2,const bool fReplace)
    {
    LogPrintf("Insert2\n");  
    char sql[2000]; 
    const char* insertstatement="INSERT OR %s INTO %s(%s,%s) VALUES (?,?);";    
     sprintf(sql,insertstatement,fReplace?"REPLACE":"IGNORE",tableName,columnName1,columnName2);
     LogPrintf("Insert sql %s\n",sql);  
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   LogPrintf("GetInsertSql1 %i\n",result);   
   if(!BindValue(stat,1,format1,value1))
       return false;
   if(!BindValue(stat,2,format2,value2))
       return false; 
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
        LogPrintf("Insert2 failed result %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
    }
    template <typename V1,typename V2>
    bool InsertBatch(const char* tableName,const char* columnName1,const int format1,const char* columnName2,const int format2,const vector<pair<V1,V2> >& vValue,const bool fReplace=true)
    {
        {
    //LogPrintf("InsertBatch2\n");  
    char sql[2000]; 
    const char* insertstatement="INSERT OR %s INTO %s(%s,%s) VALUES (?,?);";    
     sprintf(sql,insertstatement,fReplace?"REPLACE":"IGNORE",tableName,columnName1,columnName2);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   //LogPrintf("GetInsertSql1 %i\n",result);   
   for(unsigned int i=0;i<vValue.size();i++)
   {
       if(!BindValue(stat,1,format1,vValue[i].first))
       return false;
        if(!BindValue(stat,2,format2,vValue[i].second))
        return false;
//        switch (format1)
//            {
//                case SQLITEDATATYPE_BLOB:
//                    result=sqlite3_bind_blob( stat, 1, (const char*)&vValue[i]->first[0], vValue[i]->first.size(), NULL );
//                    
//                    break;                 
//                default:
//                    return false;              
//            }
//        LogPrintf("GetInsertSql2 %i\n",result); 
//        switch (format2)
//            {
//                case SQLITEDATATYPE_BLOB:
//                    result=sqlite3_bind_blob( stat, 2, (const char*)&vValue[i]->second[0], vValue[i]->second.size(), NULL );
//                    
//                    break;                 
//                default:
//                    return false;              
//            }  
       // LogPrintf("GetInsertSql2 %i\n",result); 
        result=sqlite3_step( stat );
        if(result!=0&&result!=101)
            LogPrintf("Insert2 failed result %i\n",result);
        sqlite3_clear_bindings(stat);
        sqlite3_reset(stat);
   }
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
    }
    template <typename V1>
    bool InsertBatch(const char* tableName,const char* columnName1,const int format1,const vector<V1>& vValue,const bool fReplace=true)
    {
        
        //LogPrintf("InsertBatch1\n");  
        char sql[2000]; 
        const string insertstatement="INSERT OR %s INTO %s(%s) VALUES (?)";    
         sprintf(sql,insertstatement.c_str(),fReplace?"REPLACE":"IGNORE",tableName,columnName1);
        int result;
        sqlite3_stmt  *stat;    
       result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
       //LogPrintf("GetInsertSql1 %i\n",result);   
       for(unsigned int i=0;i<vValue.size();i++)
       {
           if(!BindValue(stat,1,format1,vValue[i]))
           return false;
            //LogPrintf("GetInsertSql2 %i\n",result); 
            result=sqlite3_step( stat );
            if(result!=0&&result!=101)
                LogPrintf("Insert2 failed result %i\n",result);
            sqlite3_clear_bindings(stat);
            sqlite3_reset(stat);
       }
       sqlite3_finalize( stat );
       return (result==SQLITE_OK||result==101);
    
    }
    template <typename V1,typename V2,typename V3>
    bool Insert(const char* tableName,
        const char* columnName1,const int format1,const V1& value1,
        const char*columnName2,const int format2,const V2& value2,
        const char*columnName3,const int format3,const V3& value3,const bool fReplace)
    {
        {
    LogPrintf("Insert3\n");  
    char sql[2000]; 
    const char* insertstatement="INSERT OR %s INTO %s(%s,%s,%s) VALUES (?,?,?);";    
     sprintf(sql,insertstatement,fReplace?"REPLACE":"IGNORE",tableName,columnName1,columnName2,columnName3);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, sql, -1, &stat, 0 );
   LogPrintf("GetInsertSql1 %i\n",result);   
   if(!BindValue(stat,1,format1,value1))
       return false;
   if(!BindValue(stat,2,format2,value2))
       return false;
   if(!BindValue(stat,3,format3,value3))
       return false;   
    result=sqlite3_step( stat );
    if(result!=0&&result!=101)
        LogPrintf("Insert2 failed result %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK||result==101);
}
    }
    bool BatchUpdate(const char* tableName,const char* indexColumnName,const int format1,const char* changedColumnName,const int format2,const vector<pair<string,string> >& vValue);

    bool SearchStr(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,string& searchResult) const;
    //bool SearchStrsIn(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,vector<string>& searchResult,int nMax=1000) const;
    bool SearchStrs(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,int searchResultFormat,vector<string>& searchResult,const char* chOperator="=",int nMax=1000) const;
    bool SearchInts(const char* tableName,const char* searchByColumn,const char* searchByValue,const char* searchForColumn,vector<int64_t>& searchResult,const char* chOperator="=") const;
    bool SearchInt(const char* tableName,const char* columnName,const char* columnValue,const char* resultColumnName,int64_t& result)const;
    bool Delete(const char* tableName,const char* searchColumn,const char* searchValue,const char* chOperator);

    bool Write(const char* sql);
    // bool DeleteBy1Col(const char* tableName,const char* columnName, const char* comlumnValue);
    bool Insert(const CDomain& domain,const int64_t ownerID);
    bool Update(const CDomain& domain,const int64_t ownerID);
    bool Delete(const CDomain& domain);
    bool Get(const char* tableName,const char* searchColumn,const char* searchValue,char**& result,int& nRow,int& nColumn) const;
    bool GetDomain(const char* tableName,const char* searchColumn,const char* searchOperator,const char* searchValue,std::vector<CDomain>& vDomain,bool fGetTags=true) const;
    //bool GetExpiredDomainIDs(const char* tableName,vector<int64_t>& vDomainIDs,const uint32_t time);


    bool Insert(const uint256 blockHash,const CDataStream& sBlockDomains);
    bool GetBlockDomains(const uint256 blockHash,CDataStream& sBlockDomains);
    

    bool InsertContent(const int64_t nLink,const int64_t pos,const int64_t sender, const int cc, const int64_t lockValue,const uint32_t lockTime);
    bool InsertContents(const vector<CContentDBItem>& vContents,const map<CScript,int64_t>& mapScriptIndex);
    bool SearchContents(const vector<int64_t>& vSenders,const vector<int>& vCCs,const vector<int64_t>& vTagIDs,vector<CContentDBItem>& vContents,const int nMaxResult=30,const int nOffset=0);

    bool InsertTag(const char* tableName,const int64_t tagID,const int64_t nLink);
    bool InsertTags(const char* tableName,const int64_t nLink,const vector<int64_t>& vTagID);
    //bool GetLinks(const vector<string>& vTag,const int cc,const CLink link,std::vector<CLink>& vLink,const int nMaxItems=1000,const int nOffset=0) const;
    bool InsertTagID(const string tag,int64_t& tagID);
    bool GetTagID(const string tag,int64_t& tagID) const;
    //bool ClearExpiredTags(const unsigned int nTime);
    
    //bool CreateScriptIndexTable();

    //bool InsertScriptIndex(const CScript script,int& scriptIndex);
    bool GetScriptIndex(const CScript script,int64_t& scriptIndex) const;
    bool InsertTxIndex(const uint256 txid,const int64_t& txIndex);
    bool InsertTxIndice(const map<uint256, int64_t>& mapTxIndex);
    bool GetTxIndex(const uint256 txid,int64_t& txIndex) const;
    bool GetTxidByTxIndex(const int64_t txIndex, uint256& txid) const;
    bool InsertCheque(int64_t scriptIndex,int64_t nLink, int64_t nValue,uint32_t nLockTime);
    bool BatchInsertCheque(vector<CCheque>& vCheque,const map<CScript,int64_t>& mapScriptIndex);
    bool GetCheques(const vector<CScript>& vScript,vector<CCheque> & vCheques,const int nMaxItems=1000,const int nOffset=0)const;
    //bool EraseCheque(const int64_t txindex);
    //bool BatchEraseCheque(vector<int64_t> vChequeErase);
            
    bool GetBlockPosItem(const int64_t nPosDB,uint256& hashBlock,int& nHeight);
    bool WriteBlockPos(const int64_t nPosDB,const uint256& hashBlock,const int& nHeight);
};

#endif // BITCOIN_LEVELDBWRAPPER_H
