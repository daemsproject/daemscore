// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sqlitewrapper.h"
#include "ccc/domain.h"
#include "ccc/link.h"
#include "util.h"
#include "timedata.h"
#include "txdb.h"
#include <boost/filesystem.hpp>
#include <stdio.h>
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



CSqliteWrapper::CSqliteWrapper(const boost::filesystem::path& path, bool fWipe)
{
    if (fWipe) {
            LogPrintf("Wiping sqliteDB in %s\n", path.string());
            //leveldb::DestroyDB(path.string(), options);
        }
    TryCreateDirectory(path);
    LogPrintf("Opening sqliteDB in %s\n", path.string());
    
    boost::filesystem::path p=path / "sqlite.db";
    pdb=NULL;
    int status = sqlite3_open(p.string().c_str(), &pdb);
    HandleError(status);
       
    CreateTables();    
    ClearExpiredTags(GetAdjustedTime());
    LogPrintf("Opened sqliteDB successfully\n"); 
}

CSqliteWrapper::~CSqliteWrapper()
{
    sqlite3_close(pdb);
}
bool CSqliteWrapper::CreateTables()
{
    //LogPrintf("CSqliteWrapper CreateTables  \n");
    _CreateTable("domainf");
    _CreateTable("domainfai");
    CreateTagIDTable();
    CreateTagTable();
    CreateScriptIndexTable();
    CreateTxIndexTable();
    CreateChequeTable();
    
    return true;
}
bool CSqliteWrapper::_CreateTable(const char* tableName)
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    //char* tableName="domainf";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( \
    domainname VARCHAR(64) PRIMARY KEY, \
    expiredate INTEGER , \
    owner BLOB(35), \
    redirecttype INTEGER , \
    redirrectto BLOB(64), \
    alias VARCHAR(64), \
    icon BLOB(8), \
    intro VARCHAR(128), \
    redirecthsitory BLOB(64), \
    tag1 VARCHAR(32), \
    tag2 VARCHAR(32), \
    tag3 VARCHAR(32) \
    );";    
    sprintf(sql,createtablestatement,tableName);
    //LogPrintf("CSqliteWrapper _CreateTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable done %s\n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName,"_index_expiredate",tableName,"expiredate");  
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_owner",tableName,"owner");    
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_redirrectto",tableName,"redirrectto");  
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_alias",tableName,"alias");    
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_tag1",tableName,"tag1");    
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_tag2",tableName,"tag2");    
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_tag3",tableName,"tag3");    
    //LogPrintf("CSqliteWrapper _CreateTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable createindex done %s\n",zErrMsg);
    //LogPrintf("CSqliteWrapper _CreateTable success \n");
    return true;
}
bool CSqliteWrapper::CreateTagIDTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;    
    char* tableName="tagid";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( tagid INTEGER PRIMARY KEY AUTOINCREMENT, tag VARCHAR(32));";    
    sprintf(sql,createtablestatement,tableName);
    LogPrintf("CSqliteWrapper CreateTagIDTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagIDTable done %s \n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName,"_index_tagid_tag",tableName,"tag");  
    LogPrintf("CSqliteWrapper CreateTagIDTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagIDTable createindex done %s\n",zErrMsg);
   
    return true;
}
bool CSqliteWrapper::CreateTagTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;
    char* tableName="tag";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s ( cc INTEGER , tagid INTEGER,link BLOB(8),expiretime INTEGER);";    
    sprintf(sql,createtablestatement,tableName);
    LogPrintf("CSqliteWrapper CreateTagTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTagTable done %s\n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName,"_index_tag",tableName,"tag");  
    LogPrintf("CSqliteWrapper CreateTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_cc",tableName,"cc");       
    LogPrintf("CSqliteWrapper CreateTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    sprintf(sql,createindexstatement,tableName,"_index_link",tableName,"link");    
    LogPrintf("CSqliteWrapper CreateTagTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    return true;
}

bool CSqliteWrapper::Write(const char* sql)
{
    char* zErrMsg=0;
     LogPrintf("CSqliteWrapper Write \n");
     sqlite3_exec(pdb,sql,0,0,&zErrMsg);
     LogPrintf("CSqliteWrapper Write done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:write error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::Insert(const CDomain& domain)
{
    LogPrintf("GetInsertSql\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domainf":"domainfai");
    char insertstatement[2000];    
    sprintf(insertstatement,"INSERT INTO %s VALUES (?,?,?,?,?,?,?,?,?,?,?,?)",tableName);
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );
   LogPrintf("GetInsertSql1 %i\n",result);
   result=sqlite3_bind_text(stat,1, domain.strDomain.c_str(),domain.strDomain.size(),NULL);       
   LogPrintf("GetInsertSql2 %i\n",result);
   result=sqlite3_bind_int(stat,2, domain.nExpireTime);
   LogPrintf("GetInsertSql3 %i\n",result);
   LogPrintf("GetInsertSql owner size %i,owner%s:\n",domain.owner.size(),domain.owner.ToString());
   result=sqlite3_bind_blob( stat, 3, &domain.owner[0], domain.owner.size(), NULL ); 
//   char* owner=(char*)sqlite3_column_blob(stat,3);   
//   LogPrintf("GetInsertSql after bind owner %s:\n",HexStr(owner,owner+domain.owner.size()) );
   LogPrintf("GetInsertSql4 %i\n",result);
   result=sqlite3_bind_int(stat,4, domain.redirectType);
   LogPrintf("GetInsertSql5 %i\n",result);
   result=sqlite3_bind_blob( stat, 5, domain.redirectTo.c_str(), domain.redirectTo.size(), NULL ); 
   LogPrintf("GetInsertSql6 %i\n",result);
   result=sqlite3_bind_text( stat, 6, domain.strAlias.c_str(), domain.strAlias.size(), NULL ); 
   LogPrintf("GetInsertSql7 %i\n",result);
//   CDataStream s(0,0);
//   s<<domain.iconLink;   
//   char ch[s.size()];
//   s.read(ch,s.size());
   string str1;
   if(domain.iconLink.IsEmpty())
       str1="";
   else
    str1=domain.iconLink.Serialize();
   result=sqlite3_bind_blob( stat, 7, str1.c_str(), str1.size(), NULL ); 
   
   LogPrintf("GetInsertSql8 %i\n",result);
    result=sqlite3_bind_text( stat, 8, domain.strIntro.c_str(), domain.strIntro.size(), NULL ); 
    LogPrintf("GetInsertSql9 %i\n",result);
//    CDataStream ss(0,0);
//    ss<<domain.vDirectHistory;
//    char ch1[ss.size()];
//    ss.read(ch1,ss.size());
//    result=sqlite3_bind_blob( stat, 9, ch1, ss.size(), NULL ); 
    string str="";
    for(unsigned int i=0;i<domain.vDirectHistory.size();i++)    
        str.append(domain.vDirectHistory[i].Serialize());
    
    result=sqlite3_bind_blob( stat, 9, str.c_str(), str.size(), NULL ); 
    LogPrintf("GetInsertSql10 %i\n",result);
    
    //for(unsigned int i=0;i<domain.vTags.size();i++)
        for(unsigned int i=0;i<3;i++)
    {
            string str=(i<domain.vTags.size()?domain.vTags[i]:"");
            result=sqlite3_bind_text( stat, 10+i, str.c_str(), str.size(), NULL ); 
        //result=sqlite3_bind_text( stat, 10+i, domain.vTags[i].c_str(), domain.vTags[i].size(), NULL ); 
           LogPrintf("GetInsertSql%i %i\n",i+1,result);
    }
    result=sqlite3_step( stat );
    LogPrintf("GetInsertSql %i\n",result);
       sqlite3_finalize( stat );
       return (result==SQLITE_OK);
}
bool CSqliteWrapper::Update(const CDomain& domain)
{  
    LogPrintf("CSqliteWrapper::Update\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domainf":"domainfai");
    char updatestatement[2000];    
    sprintf(updatestatement,"UPDATE %s SET expiredate =?, owner= ?, redirecttype=?,redirrectto =?, \
            alias=?, icon=?,intro=?,redirecthsitory=?,tag1=?,tag2=?,tag3=? WHERE domainname = ?",tableName);
    LogPrintf("CSqliteWrapper::Update %s\n",updatestatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, updatestatement, -1, &stat, 0 );
   LogPrintf("GetUpdateSql1 %i\n",result);   
   result=sqlite3_bind_int(stat,1, domain.nExpireTime);
   LogPrintf("GetUpdateSql2 %i\n",result);
   LogPrintf("GetUpdateSql owner size %i,owner%s:\n",domain.owner.size(),domain.owner.ToString());
   result=sqlite3_bind_blob( stat, 2, &domain.owner[0], domain.owner.size(), NULL ); 
//   char* owner=(char*)sqlite3_column_blob(stat,2);
//   LogPrintf("GetUpdateSql22 owner %s:\n",HexStr(owner,owner+domain.owner.size()) );
   LogPrintf("GetUpdateSql3 %i\n",result);
   result=sqlite3_bind_int(stat,3, domain.redirectType);
   LogPrintf("GetUpdateSql4 %i\n",result);
   result=sqlite3_bind_blob( stat, 4, domain.redirectTo.c_str(), domain.redirectTo.size(), NULL ); 
   LogPrintf("GetUpdateSql5 %i\n",result);
   result=sqlite3_bind_text( stat, 5, domain.strAlias.c_str(), domain.strAlias.size(), NULL ); 
   LogPrintf("GetUpdateSql6 %i\n",result);
//   CDataStream s(0,0);
//   s<<domain.iconLink;   
//   char ch[s.size()];
//   s.read(ch,s.size());
   string str1;
   if(domain.iconLink.IsEmpty())
       str1="";
   else
        str1=domain.iconLink.Serialize();
   result=sqlite3_bind_blob( stat, 6, str1.c_str(), str1.size(), NULL ); 
   LogPrintf("GetUpdateSql7 %i %s\n",result,HexStr(str1.begin(),str1.end()));
    result=sqlite3_bind_text( stat, 7, domain.strIntro.c_str(), domain.strIntro.size(), NULL ); 
    LogPrintf("GetUpdateSql8 %i\n",result);
    //CDataStream ss(0,0);
    //ss<<domain.vDirectHistory;
    //char ch1[ss.size()];
    //ss.read(ch1,ss.size());
    string str="";
    for(unsigned int i=0;i<domain.vDirectHistory.size();i++)
    {    
        LogPrintf("GetUpdateSql9 vDirectHistory %s\n",domain.vDirectHistory[i].ToString());
        str+=domain.vDirectHistory[i].Serialize();
    }
    LogPrintf("GetUpdateSql9 str size %i\n",str.size());
    result=sqlite3_bind_blob( stat, 8, str.c_str(), str.size(), NULL ); 
    LogPrintf("GetUpdateSql9 %i\n",result);
    
    for(unsigned int i=0;i<3;i++)
    {
            string str=(i<domain.vTags.size()?domain.vTags[i]:"");
            result=sqlite3_bind_text( stat, 9+i, str.c_str(), str.size(), NULL );       
           LogPrintf("GetUpdateSql %i, %i\n",9+i,result);
    }
    result=sqlite3_bind_text(stat,12, domain.strDomain.c_str(),domain.strDomain.size(),NULL);       
   LogPrintf("GetUpdateSql11 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("CSqliteWrapper::Update %i\n",result);
       sqlite3_finalize( stat );
       LogPrintf("CSqliteWrapper::Update finalized %i\n",result);
       return (result==SQLITE_OK||result==101);
}
bool CSqliteWrapper::Delete(const CDomain& domain)
{
    LogPrintf("GetdeleteSql\n");
    const char* tableName=(domain.nDomainGroup==DOMAIN_10000?"domainf":"domainfai");
    char deletetatement[2000];    
    sprintf(deletetatement,"DELETE FROM %s WHERE domainname = %s;",tableName,domain.strDomain.c_str());
    char* zErrMsg=0;
     LogPrintf("CSqliteWrapper Delete \n");
     sqlite3_exec(pdb,deletetatement,0,0,&zErrMsg);
     LogPrintf("CSqliteWrapper Delete done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:Delete error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::GetDomain(const char* tableName,const char* searchColumn,const char* searchValue,std::vector<CDomain>& vDomain) const
{
    LogPrintf("CSqliteWrapper Get \n");
    //char* zErrMsg=0;
    const char* selectstatement="SELECT * FROM %s WHERE %s = %s;";    
    char sql[2000];
    sprintf(sql,selectstatement,tableName,searchColumn,searchValue);
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
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
           domain.strDomain=(char*)sqlite3_column_text(stmt,  0);
            LogPrintf("GetDomain strDomain %s \n",domain.strDomain); 
            domain.nDomainGroup=GetDomainGroup(domain.strDomain);    
            domain.nExpireTime= sqlite3_column_int(stmt,  1);
            LogPrintf("CDomain CDomain() expiretime %i \n",domain.nExpireTime); 
            domain.owner=CScript((unsigned char*)sqlite3_column_blob(stmt,  2),(unsigned char*)sqlite3_column_blob(stmt,  2)+sqlite3_column_bytes(stmt,  2));
            LogPrintf("CDomain CDomain() owner %i,%s %s\n",sqlite3_column_bytes(stmt,  2),HexStr(domain.owner.begin(),domain.owner.end()),domain.owner.ToString());                    
            domain.redirectType=sqlite3_column_int(stmt,  3);
            LogPrintf("CDomain CDomain() redirectType %i \n",domain.redirectType);     
            domain.redirectTo=string((char*)sqlite3_column_blob(stmt,4),(char*)sqlite3_column_blob(stmt,4)+sqlite3_column_bytes(stmt,4));
            LogPrintf("CDomain CDomain() redirectTo %s \n",HexStr(domain.redirectTo.begin(),domain.redirectTo.end()));   
            if(domain.redirectType>=0&&domain.redirectTo.size()>0)
                domain.redirect.EncodeUnit(domain.redirectType,domain.redirectTo);
            domain.strAlias=(char*)sqlite3_column_text(stmt,5);
            LogPrintf("CDomain CDomain() strAlias %s \n",domain.strAlias); 
            LogPrintf("CDomain CDomain() iconlink len %i \n",sqlite3_column_bytes(stmt,6)); 
            if(sqlite3_column_bytes(stmt,6)>0)
            {
                //CDataStream s((char*)sqlite3_column_blob(stmt,6),(char*)sqlite3_column_blob(stmt,6)+sqlite3_column_bytes(stmt,6),0,0);
                string str((char*)sqlite3_column_blob(stmt,6),(char*)sqlite3_column_blob(stmt,6)+sqlite3_column_bytes(stmt,6));
                domain.iconLink.Unserialize(str);        
            }
            LogPrintf("CDomain() iconLink %s \n",domain.iconLink.ToString()); 
            domain.strIntro=(char*)sqlite3_column_text(stmt,7);  
            LogPrintf("CDomain() strIntro %s \n",domain.strIntro);     
            //int len=(int)(sqlite3_column_bytes(stmt,8)/8);
            //if(len>0)
//            {
//                for (int i=0;i<len;i++)
                //{       
                    CLink link; 
                    //CDataStream s((char*)sqlite3_column_blob(stmt,8)+8*i,(char*)sqlite3_column_blob(stmt,8)+8*(i+1),0,0);  
                    //string str((char*)sqlite3_column_blob(stmt,8)+8*i,(char*)sqlite3_column_blob(stmt,8)+8*(i+1));  
                    string str((char*)sqlite3_column_blob(stmt,8),(char*)sqlite3_column_blob(stmt,8)+sqlite3_column_bytes(stmt,8));  
                    while(link.Unserialize(str))
                        domain.vDirectHistory.push_back(link);        
                //}
            //}    
            LogPrintf("CDomain() vDirectHistory %i \n",domain.vDirectHistory.size()); 
            for(int i=9;i<12;i++)
            {
                LogPrintf("CDomain() vTags %i %i\n",i,sqlite3_column_bytes(stmt,i)); 
                if(sqlite3_column_bytes(stmt,i)>0) 
                {
                    string tag((char*)sqlite3_column_text(stmt,i));        
                    domain.vTags.push_back(tag);
                }       
            }    
                //sqlItem[i]=(char*)sqlite3_column_blob(stmt,  i);
                //LogPrintf("i:%i,size:%i,content:%s \n",i, sqlite3_column_bytes(stmt,  i),HexStr((unsigned char*)sqlite3_column_blob(stmt,  i),(unsigned char*)sqlite3_column_blob(stmt,  i)+sqlite3_column_bytes(stmt,  i)));
                      
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
bool CSqliteWrapper::Get(const char* tableName,const char* searchColumn,const char* searchValue,char**& result,int& nRow,int& nColumn) const
{
    LogPrintf("CSqliteWrapper Get \n");
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
bool CSqliteWrapper::InsertTag(const int cc,const int tagID,const CLink link,const unsigned int nExpireTime)
{
    LogPrintf("InsertTag\n");    
    char* insertstatement="INSERT INTO tag VALUES (?,?,?,?)";
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );
   LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_int(stat,1, cc);   
   LogPrintf("GetInsertSql2 %i\n",result);   
   result=sqlite3_bind_int(stat,2, tagID);  
   LogPrintf("GetInsertSql3 %i\n",result);
   string str =link.Serialize();
   LogPrintf("GetInsertSql link %s\n",HexStr(str.begin(),str.end()));
   result=sqlite3_bind_blob( stat, 3, str.c_str(), str.size(), NULL ); 
   LogPrintf("GetInsertSql4 %i\n",result);
   result=sqlite3_bind_int(stat,4, nExpireTime);  
   LogPrintf("GetInsertSql5 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("InsertTag %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK);
}
bool CSqliteWrapper::GetLinks(const vector<string> vTag,const int cc,const CLink link,std::vector<CLink>& vLink,const int nMaxItems,const int nOffset) const
{
    LogPrintf("CSqliteWrapper GetLinks \n");
    //char* zErrMsg=0;
    const char* selectstatement="SELECT link FROM tag WHERE %s = %s;";    
    char sql[2000];    
    if(!link.IsEmpty())
    {
        string strLink="x'";
        string strLink1=link.Serialize();
        strLink.append(HexStr(strLink1.begin(),strLink1.end()));
        strLink.append("'");
        sprintf(sql,selectstatement,"link",strLink.c_str());
    }
    else
    {        
        vector<int> vTagID;
        for(unsigned int i=0;i<vTag.size();i++)
        {
            int nTagID;
           if(GetTagID(vTag[i],nTagID)) 
               vTagID.push_back(nTagID);
        }
        if(vTagID.size()==0&&cc==-1)
            return false;
        if(cc!=-1)
        {
            
            selectstatement="SELECT link FROM tag WHERE cc = %i";    
            sprintf(sql,selectstatement,cc);
            if(vTag.size()>0)
            {
                selectstatement="%s AND tagid= %i";    
                sprintf(sql,selectstatement,sql,vTagID[0]);
            }
        }
        else
        {
            selectstatement="SELECT link FROM tag WHERE tagid= %i";    
            sprintf(sql,selectstatement,vTagID[0]);
        }
        LogPrintf("CSqliteWrapper GetLinks sql %s\n",sql);
        for(unsigned int i=1;i<vTagID.size();i++)
        {
            selectstatement="SELECT link FROM tag WHERE tagid= %i AND link IN(%s)";    
            sprintf(sql,selectstatement,vTagID[i],sql);
            
        }
        selectstatement="%s limit %i offset %i;"; 
        sprintf(sql,selectstatement,sql,nMaxItems,nOffset);
    }    
    
     LogPrintf("CSqliteWrapper GetLinks sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
    if(rc != SQLITE_OK)
    {
        if(stmt)
        {
            sqlite3_finalize(stmt);
        }        
        return false;
    }
    //int nColumn = sqlite3_column_count(stmt);
    //int nItems=0;
    do{ 
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
           CLink link1;
            if(sqlite3_column_bytes(stmt,0)>0)
            {                
                string str((char*)sqlite3_column_blob(stmt,0),(char*)sqlite3_column_blob(stmt,0)+sqlite3_column_bytes(stmt,0));
                LogPrintf("db.getlink link length %i, hex %s \n",sqlite3_column_bytes(stmt,0),HexStr(str.begin(),str.end())); 
                link1.Unserialize(str);        
            }
            LogPrintf("Link %s \n",link1.ToString());             
            vLink.push_back(link1);               
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
bool CSqliteWrapper::InsertTagID(const string tag,int& tagID)
{
    if (tag.size()>32)
        return false;    
    if(GetTagID(tag,tagID))
        return true;
    LogPrintf("InsertTagID\n");    
    char* insertstatement="INSERT INTO tagid(tag) VALUES (?)";
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );   
   result=sqlite3_bind_text( stat, 1, tag.c_str(), tag.size(), NULL ); 
   LogPrintf("GetInsertSql4 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("InsertTagID %i\n",result);
   sqlite3_finalize( stat );
   if(!result==SQLITE_OK)
       return false;
   return GetTagID(tag,tagID);   
}
bool CSqliteWrapper::GetTagID(const string tag,int& tagID) const
{
    LogPrintf("CSqliteWrapper GetTagID \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT tagid FROM tagid WHERE tag = '%s';";    
     sprintf(sql,selectstatement,tag.c_str());
    LogPrintf("CSqliteWrapper GetTagID sql %s\n",sql); 
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
           tagID= sqlite3_column_int(stmt,  0);             
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
bool CSqliteWrapper::ClearExpiredTags(const unsigned int nTime)
{
    LogPrintf("GetdeleteSql\n");    
    char deletestatement[2000];    
    sprintf(deletestatement,"DELETE FROM tag WHERE expiretime< %i;",nTime);
    char* zErrMsg=0;
     LogPrintf("CSqliteWrapper Delete \n");
     sqlite3_exec(pdb,deletestatement,0,0,&zErrMsg);
     LogPrintf("CSqliteWrapper Delete done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:Delete error:%s",zErrMsg);
         return false;
     }
     return true;
}
bool CSqliteWrapper::CreateScriptIndexTable()
{    
    char* zErrMsg=0;    
    string tableName="scriptindextable";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( scriptindex INTEGER PRIMARY KEY AUTOINCREMENT, script BLOB(255));";    
    sprintf(sql,createtablestatement,tableName.c_str());
    LogPrintf("CSqliteWrapper CreateScriptIndexTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateScriptIndexTable done %s \n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName.c_str(),"_index_scriptindextable_script",tableName.c_str(),"script");  
    LogPrintf("CSqliteWrapper CreateScriptIndexTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateScriptIndexTable createindex done %s\n",zErrMsg);
   
    return true;
}
bool CSqliteWrapper::CreateTxIndexTable()
{    
    char* zErrMsg=0;    
    string tableName="txindextable";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s( txindex INTEGER PRIMARY KEY AUTOINCREMENT, txid BLOB(32));";    
    sprintf(sql,createtablestatement,tableName.c_str());
    LogPrintf("CSqliteWrapper CreateTxIndexTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTxIndexTable done %s \n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName.c_str(),"_index_txindextable_txid",tableName.c_str(),"txid");  
    LogPrintf("CSqliteWrapper CreateTxIndexTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateTxIndexTable createindex done %s\n",zErrMsg);
   
    return true;
}
bool CSqliteWrapper::CreateChequeTable()
{
    //LogPrintf("CSqliteWrapper _CreateTable %s \n",tableName);
    char* zErrMsg=0;
    char* tableName="chequetable";
    char sql[2000];
    const char* createtablestatement="CREATE TABLE IF NOT EXISTS %s ( txindex INTEGER , nout INTEGER,scriptindex INTEGER,value INTEGER，locktime INTEGER, PRIMARY KEY (txidex,nout) );";    
    sprintf(sql,createtablestatement,tableName);
    LogPrintf("CSqliteWrapper CreateChequeTable statement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);
    LogPrintf("CSqliteWrapper CreateChequeTable done %s\n",zErrMsg);
    const char * createindexstatement="CREATE INDEX IF NOT EXISTS %s %s on %s(%s)";    
    sprintf(sql,createindexstatement,tableName,"_index_chequetable_scriptindex",tableName,"scriptindex");  
    LogPrintf("CSqliteWrapper CreateChequeTable createindexstatement %s \n",sql);
    sqlite3_exec(pdb,sql,0,0,&zErrMsg);    
    return true;
}
bool CSqliteWrapper::InsertScriptIndex(const CScript script,int& scriptIndex)
{
    if(script.size()==0)
        return false;
    CScript script1=script;
    if (script1.size()>255)
        script1.resize(255);
    if(GetScriptIndex(script1,scriptIndex))
        return true;
    LogPrintf("InsertScriptIndex\n");    
    char* insertstatement="INSERT INTO scriptindextable(script) VALUES (?)";
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );   
   result=sqlite3_bind_blob( stat, 1, (char*)&script1[0], script1.size(), NULL ); 
   LogPrintf("GetInsertSql4 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("InsertTagID %i\n",result);
   sqlite3_finalize( stat );
   if(!result==SQLITE_OK)
       return false;
   return GetScriptIndex(script,scriptIndex);   
}
bool CSqliteWrapper::GetScriptIndex(const CScript script,int& scriptIndex) const
{
    LogPrintf("CSqliteWrapper GetScriptIndex \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT scriptindex FROM scriptindextable WHERE script = %s;";  
    string strScript="x'";        
    strScript.append(HexStr(script.begin(),script.end()));
    strScript.append("'");
    sprintf(sql,selectstatement,strScript.c_str());
    
    LogPrintf("CSqliteWrapper GetScriptIndex sql %s\n",sql); 
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
           scriptIndex= sqlite3_column_int(stmt,  0);             
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
bool CSqliteWrapper::InsertTxIndex(const uint256 txid,int& txIndex)
{
    
    if(GetTxIndex(txid,txIndex))
        return true;
    LogPrintf("InsertTxIndex\n");    
    char* insertstatement="INSERT INTO txindextable(txid) VALUES (?)";
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );   
   result=sqlite3_bind_blob( stat, 1, (char*)txid.begin(), 32, NULL ); 
   LogPrintf("GetInsertSql4 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("InsertTagID %i\n",result);
   sqlite3_finalize( stat );
   if(!result==SQLITE_OK)
       return false;
   return GetTxIndex(txid,txIndex);   
}
bool CSqliteWrapper::GetTxIndex(const uint256 txid,int& txIndex) const
{
    LogPrintf("CSqliteWrapper GetTxIndex \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT txindex FROM txindextable WHERE txid = %s;";  
    string strTxid="x'";        
    strTxid.append(txid.GetHex());
    strTxid.append("'");
    sprintf(sql,selectstatement,strTxid.c_str());
    
    LogPrintf("CSqliteWrapper GetTxIndex sql %s\n",sql); 
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
           txIndex= sqlite3_column_int(stmt,  0);             
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
bool CSqliteWrapper::GetTxidByTxIndex(const int txIndex, uint256& txid) const
{
    LogPrintf("CSqliteWrapper GetTxIndex \n");
    //char* zErrMsg=0;
    char sql[2000]; 
    const char* selectstatement="SELECT txid FROM txindextable WHERE txIndex = %i;";   
    sprintf(sql,selectstatement,txIndex);
    
    LogPrintf("CSqliteWrapper GetTxidByTxIndex sql %s\n",sql); 
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
bool CSqliteWrapper::InsertCheque(int scriptIndex,int txIndex,ushort nOut, uint64_t nValue,uint32_t nLockTime)
{
    
    LogPrintf("InsertCheque\n");    
    char* insertstatement="INSERT INTO chequetable VALUES (?,?,?,?,?)";
    LogPrintf("GetInsertSql %s\n",insertstatement);
    int result;
    sqlite3_stmt  *stat;    
   result=sqlite3_prepare_v2( pdb, insertstatement, -1, &stat, 0 );
   LogPrintf("GetInsertSql1 %i\n",result);   
   result=sqlite3_bind_int(stat,1, txIndex);   
   LogPrintf("GetInsertSql2 %i\n",result);   
   result=sqlite3_bind_int(stat,2, (int)nOut);  
   LogPrintf("GetInsertSql3 %i\n",result);
   result=sqlite3_bind_int(stat,3, scriptIndex); 
   LogPrintf("GetInsertSql4 %i\n",result);
   result=sqlite3_bind_int64(stat,4, (int64_t)nValue);  
   LogPrintf("GetInsertSql5 %i\n",result);
   result=sqlite3_bind_int(stat,5, (int)nLockTime); 
   LogPrintf("GetInsertSql6 %i\n",result);
    result=sqlite3_step( stat );
    LogPrintf("InsertCheque result %i\n",result);
   sqlite3_finalize( stat );
   return (result==SQLITE_OK);
}
bool CSqliteWrapper::GetCheques(const vector<CScript>& vScript,vector<CCheque> & vCheques)const
{
    LogPrintf("CSqliteWrapper GetCheques \n");
    if(vScript.size()>100)
        return false;
    //char* zErrMsg=0;
    const char* selectstatement="SELECT * FROM chequetable WHERE scriptindex IN(%s);";    
    char sql[2000]; 
    vector<int> vScriptIndex;
    map<int,CScript> mapScriptIndex;
    for(unsigned int i=0;i<vScript.size();i++)
    {
        int scriptIndex;
       if(GetScriptIndex(vScript[i],scriptIndex)) 
           vScriptIndex.push_back(scriptIndex);
        mapScriptIndex[scriptIndex]=vScript[i];
    }
    if(vScriptIndex.size()==0)
        return false;
    char chScriptList[2000];
        sprintf(chScriptList,"%i",vScriptIndex[0]);
    for(unsigned int i=1;i<vScriptIndex.size();i++)
    {
        sprintf(chScriptList,"%s,%i",chScriptList,vScriptIndex[i]);
    }
     sprintf(sql,selectstatement,chScriptList);
     LogPrintf("CSqliteWrapper GetCheques sql %s\n",sql); 
    sqlite3_stmt  *stmt = NULL;
    int rc;
    rc = sqlite3_prepare_v2(pdb , sql , strlen(sql) , &stmt , NULL);
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
           int txindex=sqlite3_column_int(stmt,  0);
           GetTxidByTxIndex(txindex,cheque.txid);
           LogPrintf("GetCheques txid %s \n",cheque.txid.GetHex());
           cheque.nOut=(ushort)sqlite3_column_int(stmt,  1);
           LogPrintf("GetCheques nOut %i \n",cheque.nOut);
           int scriptIndex=sqlite3_column_int(stmt,  2);           
           cheque.scriptPubKey=mapScriptIndex[scriptIndex];
           LogPrintf("GetCheques scriptPubKey %s \n",cheque.scriptPubKey.ToString());
           cheque.nValue=(uint64_t)sqlite3_column_int64(stmt,  3);
           LogPrintf("GetCheques nValue %i \n",cheque.nValue);
           cheque.nLockTime=(uint32_t)sqlite3_column_int64(stmt,  4);
           LogPrintf("GetCheques nLockTime %i \n",cheque.nLockTime);
           vCheques.push_back(cheque);  
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
bool CSqliteWrapper::EraseCheque(const uint256 txid, const uint32_t nOut)
{
    LogPrintf("GetdeleteSql\n");    
    char deletestatement[2000];    
    sprintf(deletestatement,"DELETE FROM chequetable WHERE txid = x'%s' AND nout = %i;",txid.GetHex().c_str(),nOut);
    char* zErrMsg=0;
     LogPrintf("CSqliteWrapper Delete \n");
     sqlite3_exec(pdb,deletestatement,0,0,&zErrMsg);
     LogPrintf("CSqliteWrapper Delete done\n");     
     if(zErrMsg)
     {
         LogPrintf("sqlitewrapper:Delete error:%s",zErrMsg);
         return false;
     }
     return true;
}