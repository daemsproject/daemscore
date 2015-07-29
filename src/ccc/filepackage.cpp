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
                    if(vDecoded1[i].first==CC_NAME)
                    {
                        strFileName=vDecoded1[i].second;
                        fFound=true;
                        break;
                    }
                if (!fFound)
                    return false;
                fFound=false;
                for(unsigned int i=0;i<vDecoded1.size();i++)
                    if(vDecoded1[i].first==CC_LINK)
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
            }                        
            if(vDecoded[i].first==CC_NAME)
            {
                strPackageName=vDecoded[i].second;
            }
        }
        CContent(vDecoded[0].second).GetDataByCC(CC_TAG,vTags,true,true);
            return true;
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
    vTags.clear();
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
bool CFilePackage::SetJson(const Value json)
{
    Clear();
    if(json.type() != json_spirit::obj_type)
    {
       LogPrintf("SetJson :  Value is not object \n");
        return false;
    }
    json_spirit::Object obj= json.get_obj();
    json_spirit::Value val = find_value(obj, "files");
    if (val.type()!=array_type)
    {
        LogPrintf("SetJson :  files is not array \n");
        return false;
    }
    Array arrFiles = val.get_array();        
    for(unsigned int i=0;i<arrFiles.size();i++)
    {
        if(arrFiles[i].type()!=obj_type)
        {
            LogPrintf("SetJson :  file is not obj \n");
        return false;
        }
        Object obj=arrFiles[i].get_obj();
        string strFileName=obj[0].name_;
        vector<CLink> vlinks;
        CLink flink;        
        std::vector<string> vstr;
        if(!GetStringVectorFromValue(obj[0].value_,vstr))
         {
              LogPrintf("SetJson :  link is not valid \n");
                return false;
         }
        for(unsigned int j=0;j<vstr.size();j++)         
        {
            
            if(!flink.SetString(vstr[j]))
            {
                LogPrintf("SetJson :  link is not valid \n");
                return false;
            }
            LogPrintf("CFilePackage SetJson :  link %s, height:%i,ntx:%i,nvout:%i \n",vstr[i],flink.nHeight,flink.nTx,flink.nVout);
            vlinks.push_back(flink);            
        }        
        mapFileList[strFileName]=vlinks;
    }
    val = find_value(obj, "mainfile");
    if (val.type()!=str_type)
    {
        LogPrintf("SetJson :  mainfile is not str \n");
        return false;
    }
    strMainFile = val.get_str();
    val = find_value(obj, "packagename");
    if (val.type()==str_type)
    {
        strPackageName=val.get_str();
    }
    val = find_value(obj, "tags");    
    GetStringVectorFromValue(val,vTags);    
    fValid=CheckLinks();
    return true;
}
bool CFilePackage::CheckLinks()
{
    for(std::map<string,vector<CLink> >::iterator it=mapFileList.begin();it!=mapFileList.end();it++)
    {
        CContent content;
        
        if(it->second.size()==1)
        {
            LogPrintf("CFilePackage::CheckLinks :  link height:%i,ntx:%i,nvout:%i \n",it->second[0].nHeight,it->second[0].nTx,it->second[0].nVout);
            if(!GetContentByLink(it->second[0],content))
                    return false;
            LogPrintf("CFilePackage::CheckLinks :  link content %s \n",content);
            vector<string> vFiles;
            if(!content.GetDataByCC(CC_FILE,vFiles,true,false))
                    return false;            
        }
        else
        {
            for(unsigned int i=0;i<it->second.size();i++)
            {
                if(!GetContentByLink(it->second[0],content))
                    return false;
                vector<string> vParts;
                if(!content.GetDataByCC(CC_FILE_PART,vParts,true,false))
                    return false;     
            }
        }
    }
    return true;
}
CContent CFilePackage::ToContent()const
{   
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_FILE_PACKAGE_MAINFILE,strMainFile));
    vcc.push_back(make_pair(CC_NAME,strPackageName));
    for(std::map<string,vector<CLink> >::const_iterator it=mapFileList.begin();it!=mapFileList.end();it++)   
        vcc.push_back(FileToContent(it->first));    
    if(vTags.size()>0)
        vcc.push_back(make_pair(CC_TAG_P,EncodeContentUnitArray(CC_TAG,vTags)));
       
    
    CContent ctt;
    ctt.EncodeP(CC_FILE_PACKAGE_P,vcc); 
    return ctt;
    
}
pair<int,string> CFilePackage::FileToContent(const string strFileName) const
{
    std::map<string,vector<CLink> >::const_iterator it=mapFileList.find(strFileName);
    if(it==mapFileList.end())
        return make_pair(CC_LINK,"");
    if(it->second.size()==1)
    {       
        return make_pair(CC_LINK,it->second[0].Serialize());
    }
    else 
    {        
        CContent content;
         for(unsigned i=0;i<it->second.size();i++)
             content.EncodeUnit(CC_LINK,it->second[i].Serialize());
        return make_pair(CC_FILE_COMBINE_P,content);
    }
}
