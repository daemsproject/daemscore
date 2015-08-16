/* 
 * File:   filepackage.h
 * Author: 
 *
 * Created on July 26, 2015, 6:59 PM
 */

#ifndef FAICOIN_FILEPACKAGE_H
#define	FAICOIN_FILEPACKAGE_H
#include <boost/filesystem.hpp>
#include "json/json_spirit_value.h"
#include "fai/link.h"
using namespace std;
using namespace json_spirit;

class CContent;

//namespace json_spirit
//{
//class Value;
//}
class CFilePackage
{
public:
    //the blockchain link which stores the package list.
    CLink link;
    //the content which stores the package list.
    boost::filesystem::path path;    
    string strPackageName;
    std::map<string,vector<CLink> > mapFileList;
    std::string strMainFile;
    vector<string> vTags;
    bool fValid;
    CFilePackage(){fValid=false;};
    CFilePackage(const CLink linkIn){SetLink(linkIn);};
    CFilePackage(const CLinkUni linkIn){SetLink(linkIn);};
    bool SetLink(const CLink linkIn);
    bool SetLink(const CLinkUni linkIn);
    bool SetContent(const CContent contentIn);
    bool SetJson(const Value json);
    CContent ToContent()const;
    Value ToJson()const;
    bool InstallPackage(const string strDirName,const bool fInternal=false,int timeOut=5000);
    void Clear();
    bool CheckLinks();
    bool IsValid(){return fValid;}
    string FileToContent(const string strFileName) const;
};
bool ReadFilePackageList(const std::string strFileList,std::string& strMainFile,json_spirit::Array& arrFiles);
bool GetFilePackageMain(const string packageName,string& path,const bool fInternal=false);
bool CheckFilePackage(const string packageName);
bool GetFilePackageUrl(const CLink link,string& url);
bool DeleteFilePackage(const CLink link);
bool ClearFilePackageCache(int nMaxSize=1000);
#endif	/* FAICOIN_FILEPACKAGE_H */

