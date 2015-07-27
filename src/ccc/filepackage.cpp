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
#include "ccc/link.h"
#include "util.h"
#include "ccc/content.h"
#include "ccc/contentutil.h"
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
        vDecoded.clear();
        if(!CContent(vDecoded[0].second).Decode(vDecoded))
            return false;
        for(unsigned int i=0;i<vDecoded.size();i++)
        {
            if(vDecoded[i].first==CC_FILE_P)
            {
                 vector<CLink> vlink;            
                std::vector<std::pair<int, string> > vDecoded1;
                 if(!CContent(vDecoded[i].second).Decode(vDecoded1))
                     return false;
                std::vector<std::pair<int, string> > it=find(vDecoded1.begin(),vDecoded1.end(),CC_FILE_NAME);
                if (it==vDecoded1.end())
                    return false;
                string strFileName=it->second;
                it=find(vDecoded1.begin(),vDecoded1.end(),CC_LINK);
                if (it==vDecoded1.end())
                {
                    it=find(vDecoded1.begin(),vDecoded1.end(),CC_COMBINE_P);
                    if (it==vDecoded1.end())
                        return false;
                    std::vector<std::pair<int, string> > vDecoded2;
                    if(!CContent(it->second).Decode(vDecoded2))
                        return false;
                    for(unsigned int j=0;j<vDecoded.size();j++)
                    {
                        if(vDecoded[j].first!=CC_LINK)
                            return false;
                        CLink fileLink;
                        if(!fileLink.Unserialize(strFileLink)))
                            return false;
                        vlink.push_back(fileLink);
                    }
                }
                else
                {
                    string strFileLink=it->second;
                   
                    if(!fileLink.Unserialize(strFileLink)))
                        return false;
                   
                    vlink.push_back(fileLink);
                    mapFileList.push_back(strFileName,vlink);
                }
            }
            
            switch (vDecoded[i].first)
            {
                case CC_FILE_PACKAGE_MAINFILE
            }
        }
        if(mapFileList.size()==0)
            return false;
        std::vector<std::pair<int, string> > it=find(vDecoded.begin(),vDecoded.end(),CC_FILE_PACKAGE_MAINFILE);
        if (it!=vDecoded1.end())
        {
            if(it->second.size()==0)
                return false;
            if(find(mapFileList.begin(),mapFileList.end(),it->second)==mapFileList.end())
                return false;                
            strMainFile=it->second;
        }
    }
    return false;
}
bool CFilePackage::InstallPackage(string strDirName)
{
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / strDirName;
     if(!boost::filesystem::create_directories(fpPath))
         return false;
     BOOST_FOREACH(Pair<string,vector<CLink> >& pair,mapFileList)
     {
         string strFilecontent;
        if(GetFileFromLinks(pair.second,strFilecontent))
        {
            boost::filesystem::path fpFile=fpPath / pair.first;
            if(!StringToFile(fpPath,strFilecontent))
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

