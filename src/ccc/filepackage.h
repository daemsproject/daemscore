/* 
 * File:   filepackage.h
 * Author: 
 *
 * Created on July 26, 2015, 6:59 PM
 */

#ifndef FILEPACKAGE_H
#define	FILEPACKAGE_H
using namespace std;
using namespace json_spirit;
class CFilePackage
{
public:
    //the blockchain link which stores the package list.
    CLink link;
    //the content which stores the package list.
    CContent content;
    std::map<string,string> mapFileList;
    std::string strMainFile;
    bool SetLink(const CLink linkIn);
    bool SetContent(const CContent contentIn);
    bool SetJson(const Value json);
    Value ToJson();
    bool InstallPackage();
    
};


#endif	/* FILEPACKAGE_H */

