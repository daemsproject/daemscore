/* 
 * File:   filepackage.cpp
 * Author: htl
 * 
 * Created on July 26, 2015, 7:00 PM
 */

#include <map>

#include "filepackage.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader_template.h"
#include "ccc/link.h"
#include "util.h"
#include "script/script.h"
#include "ccc/content.h"
#include "ccc/contentutil.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost;
bool CFilePackage::SetLink(const CLink linkIn)
{
    link=linkIn;
    CContent content;
    if(!GetContentByLink(linkIn,content))
        return false;
    return SetContent(content);
}
bool CFilePackage::SetContent(const CContent contentIn)
{
    Clear();
    std::vector<std::pair<int, string> > vDecoded;
    contentIn.Decode(vDecoded);
    if(vDecoded.size()>0&&vDecoded[0].first==CC_FILE_PACKAGE_P)
    {
        string content2;
        vDecoded.clear();
        if(!CContent(content2).Decode(vDecoded))
            return false;
        for(unsigned int i=0;i<vDecoded.size();i++)
        {
            if(vDecoded[i].first==CC_FILE_P)
            {
                 vector<CLink> vlink;            
                std::vector<std::pair<int, string> > vDecoded1;
                 if(!CContent(vDecoded[i].second).Decode(vDecoded1))
                     return false;
                bool fFound=false;
                string strFileName;
                for(unsigned int i=0;i<vDecoded1.size();i++)
                    if(vDecoded1[i].first==CC_FILE_NAME)
                    {
                        strFileName=vDecoded1[i].second;
                        fFound=true;
                        break;
                    }
                if (!fFound)
                    return false;
                fFound=false;
                for(unsigned int i=0;i<vDecoded1.size();i++)
                    if(vDecoded1[i].first==CC_FILE_NAME)
                    {
                        string strFileLink=vDecoded1[i].second;
                        CLink fileLink;
                        if(!fileLink.Unserialize(strFileLink))
                            return false;                   
                        vlink.push_back(fileLink);
                        mapFileList[strFileName]=vlink;
                        fFound=true;
                        break;
                    }
                
                if (!fFound)
                {
                    fFound=false;
                    for(unsigned int i=0;i<vDecoded1.size();i++)
                    if(vDecoded1[i].first==CC_FILE_COMBINE_P)
                    {                    
                        std::vector<std::pair<int, string> > vDecoded2;
                        if(!CContent(vDecoded1[i].second).Decode(vDecoded2))
                            return false;
                        for(unsigned int j=0;j<vDecoded.size();j++)
                        {
                            if(vDecoded[j].first!=CC_LINK)
                                return false;
                            CLink fileLink;
                            if(!fileLink.Unserialize(vDecoded[j].second))
                                return false;
                            vlink.push_back(fileLink);
                        }
                        break;
                    }
                    if (!fFound)
                    return false;
                }
                
            }
        }
        if(mapFileList.size()==0)
            return false;        
        for(unsigned int i=0;i<vDecoded.size();i++)
        {
            if(vDecoded[i].first==CC_FILE_PACKAGE_MAINFILE)
            {
                if(vDecoded[i].second.size()==0)
                    return false;
                if(mapFileList.find(vDecoded[i].second)==mapFileList.end())
                    return false;                
                strMainFile=vDecoded[i].second;
                return true;
            }
        }
    }
    return false;
}
bool CFilePackage::InstallPackage(string strDirName)
{
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / strDirName;
     if(!boost::filesystem::create_directories(fpPath))
         return false;
     for(map<string,vector<CLink> >::iterator it=mapFileList.begin();it!=mapFileList.end();it++)
     {
         string strFilecontent;
        if(GetFileFromLinks(it->second,strFilecontent))
        {
            boost::filesystem::path fpFile=fpPath / it->first;
            if(!StringToFile(fpPath.string(),strFilecontent))
                return false;
        }
         return false;
     }  
     return true;    
}

void CFilePackage::Clear()
{
    link=CLink();    
    path="";    
    strPackageName="";
    mapFileList.clear();
    strMainFile="";
    fValid=false;            
}
bool GetFilePackageMain(const string packageName,string& path)
{
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / packageName / "filepackage"  ;    
    boost::filesystem::path fpFile=fpPath / (packageName+".package.json");
    LogPrintf("GetFilePackageMain fpPath:%s,fpFile %s \n",fpPath.string(),fpFile.string());    
    json_spirit::Array arrFiles;
    std::string strMainFile;
    if(boost::filesystem::exists(fpFile))
    {        
        LogPrintf("GetFilePackageMain file exists \n");    
        std::string str;        
        if(ReadFileToString(fpFile.string(),str)&&ReadFilePackageList(str,strMainFile,arrFiles))
        {
            path=fpPath.string();            
            path.append("/").append(strMainFile);   
            return true;         
        }
        LogPrintf("GetFilePackageMain file to string %s \n",str);  
    }
    return false;
}
bool CheckFilePackage(const string packageName)
{
     boost::filesystem::path fpPath=GetDataDir()  / "appdata" / packageName / "filepackage"  ;
    boost::filesystem::path fpFile=fpPath / (packageName+".package.json");
    string filename=fpFile.string();   
    json_spirit::Array arrFiles;
    std::string strMainFile;
    bool fIntegrite=false;
    if(boost::filesystem::exists(fpFile))
    {
        fIntegrite=true;
        std::string str;        
        if(FileToString(fpFile.string(),str)&&ReadFilePackageList(str,strMainFile,arrFiles))
        {
            for(unsigned int i=0;i<arrFiles.size();i++)
            {
                Object obj=arrFiles[i].get_obj();
                
                boost::filesystem::path fpFile2=fpPath / obj[0].name_;
                if(!boost::filesystem::exists(fpFile2)) 
                {
                    fIntegrite=false;
                    break;
                }
            }            
        }        
    }    
    return fIntegrite;
}
bool ReadFilePackageList(const std::string strFileList,std::string& strMainFile,json_spirit::Array& arrFiles)
{
        json_spirit::Value fileData;
        if (!json_spirit::read_string(strFileList,fileData)){
            LogPrintf("readFileList %s: fail2 \n",strFileList);
            return false;
        }    
        if(fileData.type() != json_spirit::obj_type)
        {
           LogPrintf("readFileList %s:  fail3 \n",strFileList);
            return false;
        }
        json_spirit::Object obj= fileData.get_obj();
        json_spirit::Value val = find_value(obj, "files");
        if (val.type()!=array_type)
        {
            LogPrintf("readFileList %s:  fail4 %i\n",val.type());
            return false;
        }
        arrFiles = val.get_array();
        val = find_value(obj, "mainfile");
        if (val.type()!=str_type)
        {
            LogPrintf("readFileList %s:  fail5 \n",strFileList);
            return false;
        }
        strMainFile = val.get_str();
        return true;
}