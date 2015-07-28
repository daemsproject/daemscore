/* 
 * File:   filepackage.h
 * Author: 
 *
 * Created on July 26, 2015, 6:59 PM
 */

#ifndef CCCOIN_FILEPACKAGE_H
#define	CCCOIN_FILEPACKAGE_H
#include <boost/filesystem.hpp>
#include "json/json_spirit_value.h"
#include "ccc/link.h"
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
    bool fValid=false;
    CFilePackage(const CLink linkIn){SetLink(linkIn);};
    bool SetLink(const CLink linkIn);
    bool SetContent(const CContent contentIn);
    bool SetJson(const Value json);
    Value ToJson();
    bool InstallPackage( string strDirName);
    void Clear();
    
};
bool ReadFilePackageList(const std::string strFileList,std::string& strMainFile,json_spirit::Array& arrFiles);
bool GetFilePackageMain(const string packageName,string& path);
bool CheckFilePackage(const string packageName);

#endif	/* CCCOIN_FILEPACKAGE_H */

