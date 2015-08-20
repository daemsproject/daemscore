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
#include "json/json_spirit_writer_template.h"
#include "fai/link.h"
#include "util.h"
#include "script/script.h"
#include "fai/content.h"
#include "fai/contentutil.h"

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost;

bool CFilePackage::SetLink(const CLink linkIn)
{
    link = linkIn;
    CContent content;

    if (!GetContentByLink(linkIn, content)) {
        LogPrintf("CFilePackage::SetLink link: %s content:%s\n", link.ToString(), content);
        return false;
    }
    LogPrintf("CFilePackage::SetLink link: %s content:%s\n", link.ToString(), content);
    return SetContent(content);
}

bool CFilePackage::SetLink(const CLinkUni linkIn)
{
    if (linkIn.linkType == CC_LINK_TYPE_BLOCKCHAIN)
        link = CLink(linkIn.nHeight, linkIn.nTx, linkIn.nVout);
    else if (linkIn.linkType == CC_LINK_TYPE_TXIDOUT) {
        if (!TxidOutLink2BlockChainLink(linkIn.txid, linkIn.nVout, link))
            return true;
    } else
        return false;
    CContent content;
    if (!GetContentByLink(linkIn, content)) {
        LogPrintf("CFilePackage::SetLink link: %s content:%s\n", link.ToString(), content);
        return false;
    }
    LogPrintf("CFilePackage::SetLink link: %s content:%s\n", link.ToString(), content);
    return SetContent(content);
}

bool CFilePackage::SetContent(const CContent contentIn)
{
    Clear();
    std::vector<std::pair<int, string> > vDecoded;
    if (!contentIn.Decode(vDecoded))
        return false;
    LogPrintf("CFilePackage::SetContent decoded size:%i\n", vDecoded.size());
    if (vDecoded.size() > 0 && vDecoded[0].first == CC_FILE_PACKAGE_P) {
        CContent content2 = vDecoded[0].second;
        std::vector<std::pair<int, string> > vDecoded1;
        if (!content2.Decode(vDecoded1))
            return false;
        LogPrintf("CFilePackage::SetContent decoded1 size %i\n", vDecoded1.size());
        for (unsigned int i = 0; i < vDecoded1.size(); i++) {
            LogPrintf("CFilePackage::SetContent decoded1 n %i,cc:%s,content:%s\n", i, GetCcName((cctype) vDecoded1[i].first), vDecoded1[i].second);
            if (vDecoded1[i].first == CC_FILE_P) {
                LogPrintf("CFilePackage::SetContent filep:%i\n", i);
                vector<CLink> vlink;
                std::vector<std::pair<int, string> > vDecoded2;
                if (!CContent(vDecoded1[i].second).Decode(vDecoded2))
                    return false;
                LogPrintf("CFilePackage::SetContent filep subs:%i\n", vDecoded2.size());
                bool fFound = false;
                string strFileName;
                for (unsigned int j = 0; j < vDecoded2.size(); j++)
                    if (vDecoded2[j].first == CC_NAME) {
                        strFileName = vDecoded2[j].second;
                        fFound = true;
                        break;
                    }
                LogPrintf("CFilePackage::SetContent file name:%s\n", strFileName);
                if (!fFound)
                    return false;
                fFound = false;
                for (unsigned int j = 0; j < vDecoded2.size(); j++)
                    if (vDecoded2[j].first == CC_LINK) {
                        string strFileLink = vDecoded2[j].second;
                        CLink fileLink;
                        if (!fileLink.UnserializeConst(strFileLink))
                            return false;
                        vlink.push_back(fileLink);
                        mapFileList[strFileName] = vlink;
                        LogPrintf("CFilePackage::SetContent file name:%s,link:%s\n", strFileName, fileLink.ToString());
                        fFound = true;
                        break;
                    }

                if (!fFound) {                    
                    for (unsigned int j = 0; j < vDecoded2.size(); j++)
                        if (vDecoded2[j].first == CC_FILE_COMBINE_P) {
                            LogPrintf("CFilePackage::SetContent file combined\n");
                            std::vector<std::pair<int, string> > vDecoded3;
                            if (!CContent(vDecoded2[j].second).Decode(vDecoded3))
                                return false;
                            for (unsigned int k = 0; k < vDecoded3.size(); k++) {
                                if (vDecoded3[k].first != CC_LINK)
                                    return false;
                                CLink fileLink;
                                if (!fileLink.UnserializeConst(vDecoded3[k].second))
                                    return false;
                                vlink.push_back(fileLink);
                            }
                            if(vlink.size()>0)
                            {
                                LogPrintf("CFilePackage::SetContent file combined %i\n",vlink.size());
                            
                                mapFileList[strFileName] = vlink;
                                fFound=true;
                            }
                            break;
                        }
                    if (!fFound)
                        return false;
                }

            }
        }
        if (mapFileList.size() == 0)
            return false;
        for (unsigned int i = 0; i < vDecoded1.size(); i++) {
            if (vDecoded1[i].first == CC_FILE_PACKAGE_MAINFILE) {
                if (vDecoded1[i].second.size() == 0)
                    return false;
                if (mapFileList.find(vDecoded1[i].second) == mapFileList.end())
                    return false;
                strMainFile = vDecoded1[i].second;
            }
            if (vDecoded1[i].first == CC_NAME) {
                strPackageName = vDecoded1[i].second;
            }
        }
        CContent(vDecoded1[0].second).GetDataByCC(CC_TAG, vTags, true, true);
        fValid = CheckLinks();
        return true;
    }
    return false;
}

bool CFilePackage::InstallPackage(const string strDirName, const bool fInternal, int timeOut)
{
    LogPrintf("InstallPackage to dir %s nFIles:%i\n", strDirName, mapFileList.size());
    boost::filesystem::path fpPath;
    if (fInternal)
        fpPath = GetDataDir() / "appdata" / strDirName / "filepackage";
    else
        fpPath = GetDataDir() / "appdata" / "filepackages" / strDirName;
    boost::filesystem::create_directories(fpPath);
    int64_t startTime = GetTimeMillis();
    for (map<string, vector<CLink> >::iterator it = mapFileList.begin(); it != mapFileList.end(); it++) {
        LogPrintf("InstallPackage filename %s link:%s\n", it->first, it->second[0].ToString());
        string strFilecontent;
        if (GetFileFromLinks(it->second, strFilecontent, timeOut)) {
            LogPrintf("InstallPackage filename %s length:%i\n", it->first, strFilecontent.size());
            boost::filesystem::path fpFile= fpPath;
            //parse path included in file name
            string strFileName=it->first;
            if(strFileName.find("..")!=strFileName.npos||strFileName.find("//")!=strFileName.npos)
                return false;
            while(strFileName.find("/")==0)
            {
                if(strFileName.size()==1)
                    return false;
                strFileName=strFileName.substr(1);
            }
            
            while(true)
            {
                int pos=strFileName.find("/");
                if(pos==(int)strFileName.size()-1)
                    return false;
                if(pos==(int)strFileName.npos)
                {                    
                    break;
                }
                else
                {
                    fpFile /=strFileName.substr(0,pos);
                    strFileName=strFileName.substr(pos+1);
                }            
            }
            boost::filesystem::create_directories(fpFile);
            fpFile /=strFileName;
            if (!StringToFile(fpFile.string(), strFilecontent))
                return false;
        } else
            return false;
        if ((GetTimeMillis() - startTime) > timeOut)
            return false;
    }
    LogPrintf("InstallPackage time %lld\n", GetTimeMillis() - startTime);
    boost::filesystem::path fpFile = fpPath / (strDirName + ".package.json");
    LogPrintf("InstallPackage packagefile %s length:%i\n", fpFile.string(), write_string(ToJson(), true));
    WriteJsonToFile(ToJson(), fpFile.string());
    return true;
}

void CFilePackage::Clear()
{
    link = CLink();
    path = "";
    strPackageName = "";
    mapFileList.clear();
    strMainFile = "";
    fValid = false;
    vTags.clear();
}

bool GetFilePackageMain(const string packageName, string& path, const bool fInternal)
{
    boost::filesystem::path fpPath;
    if (fInternal)
        fpPath = GetDataDir() / "appdata" / packageName / "filepackage";
    else
        fpPath = GetDataDir() / "appdata" / "filepackages" / packageName;
    boost::filesystem::path fpFile = fpPath / (packageName + ".package.json");
    LogPrintf("GetFilePackageMain fpPath:%s,fpFile %s \n", fpPath.string(), fpFile.string());
    json_spirit::Object objFiles;
    std::string strMainFile;
    if (boost::filesystem::exists(fpFile)) {
        LogPrintf("GetFilePackageMain file exists \n");
        std::string str;
        if (ReadFileToString(fpFile.string(), str) && ReadFilePackageList(str, strMainFile, objFiles)) {
            boost::filesystem::path fpMainfile = fpPath / strMainFile;
            path = "file://" + fpMainfile.string();
            return true;
        }
        LogPrintf("GetFilePackageMain file to string %s \n", str);
    }
    return false;
}

bool CheckFilePackage(const string packageName)
{
    boost::filesystem::path fpPath = GetDataDir() / "appdata" / packageName / "filepackage";
    boost::filesystem::path fpFile = fpPath / (packageName + ".package.json");
    string filename = fpFile.string();
    json_spirit::Object objFiles;
    std::string strMainFile;
    bool fIntegrite = false;
    if (boost::filesystem::exists(fpFile)) {
        fIntegrite = true;
        std::string str;
        if (FileToString(fpFile.string(), str) && ReadFilePackageList(str, strMainFile, objFiles)) {
            for (unsigned int i = 0; i < objFiles.size(); i++) {
                boost::filesystem::path fpFile2 = fpPath / objFiles[i].name_;
                if (!boost::filesystem::exists(fpFile2)) {
                    fIntegrite = false;
                    break;
                }
            }
        }
    }
    return fIntegrite;
}

bool ReadFilePackageList(const std::string strFileList, std::string& strMainFile, json_spirit::Object& objFiles)
{
    json_spirit::Value fileData;
    if (!json_spirit::read_string(strFileList, fileData)) {
        LogPrintf("readFileList %s: fail2 \n", strFileList);
        return false;
    }
    if (fileData.type() != json_spirit::obj_type) {
        LogPrintf("readFileList %s:  fail3 \n", strFileList);
        return false;
    }
    json_spirit::Object obj = fileData.get_obj();
    json_spirit::Value val = find_value(obj, "files");
    if (val.type() != obj_type) {
        LogPrintf("readFileList   fail4 %i\n", val.type());
        return false;
    }
    objFiles = val.get_obj();
    val = find_value(obj, "mainfile");
    if (val.type() != str_type) {
        LogPrintf("readFileList %s:  fail5 \n", strFileList);
        return false;
    }
    strMainFile = val.get_str();
    return true;
}

bool CFilePackage::SetJson(const Value json)
{
    Clear();
    if (json.type() != json_spirit::obj_type) {
        LogPrintf("SetJson :  Value is not object \n");
        return false;
    }
    json_spirit::Object obj = json.get_obj();
    json_spirit::Value val = find_value(obj, "files");
    if (val.type() != obj_type) {
        LogPrintf("SetJson :  files is not obj \n");
        return false;
    }
    Object objFiles = val.get_obj();
    
    for (unsigned int i = 0; i < objFiles.size(); i++) {
        
        string strFileName = objFiles[i].name_;
        LogPrintf("CFilePackage SetJson :  file %s\n",strFileName.c_str());
        vector<CLink> vlinks;
        
        std::vector<string> vstr;
        if (!GetStringVectorFromValue(objFiles[i].value_, vstr)) {
            LogPrintf("SetJson :  link is not valid \n");
            return false;
        }
        
        for (unsigned int j = 0; j < vstr.size(); j++) {
            CLink flink;
            LogPrintf("CFilePackage SetJson :  file %s\n",vstr[j]);
            if (!flink.SetString(vstr[j])) {
                LogPrintf("SetJson :  link is not valid \n");
                return false;
            }
            LogPrintf("CFilePackage SetJson :  link %s, height:%i,ntx:%i,nvout:%i \n", vstr[j], flink.nHeight, flink.nTx, flink.nVout);
            vlinks.push_back(flink);
        }
        LogPrintf("CFilePackage SetJson :  file 3\n");
        mapFileList[strFileName] = vlinks;
        LogPrintf("CFilePackage SetJson :  file 4\n");
    }
    val = find_value(obj, "mainfile");
    if (val.type() != str_type) {
        LogPrintf("SetJson :  mainfile is not str \n");
        return false;
    }
    strMainFile = val.get_str();
    val = find_value(obj, "packagename");
    if (val.type() == str_type) {
        strPackageName = val.get_str();
    }
    val = find_value(obj, "tags");
    GetStringVectorFromValue(val, vTags);
    fValid = CheckLinks();
    return true;
}

bool CFilePackage::CheckLinks()
{
    for (std::map<string, vector<CLink> >::iterator it = mapFileList.begin(); it != mapFileList.end(); it++) {
        CContent content;

        if (it->second.size() == 1) {
            LogPrintf("CFilePackage::CheckLinks :  link height:%i,ntx:%i,nvout:%i \n", it->second[0].nHeight, it->second[0].nTx, it->second[0].nVout);
            if (!GetContentByLink(it->second[0], content))
                return false;
            LogPrintf("CFilePackage::CheckLinks :  link content %s \n", content.substr(0, 100));
            vector<string> vFiles;
            if (!content.GetDataByCC(CC_FILE, vFiles, true, false))
                return false;
        } else {
            for (unsigned int i = 0; i < it->second.size(); i++) {
                if (!GetContentByLink(it->second[0], content))
                    return false;
                vector<string> vParts;
                if (!content.GetDataByCC(CC_FILE_PART, vParts, true, false))
                    return false;
            }
        }
    }
    return true;
}

CContent CFilePackage::ToContent()const
{
    std::vector<std::pair<int, string> > vcc;
    vcc.push_back(make_pair(CC_FILE_PACKAGE_MAINFILE, strMainFile));
    vcc.push_back(make_pair(CC_NAME, strPackageName));
    for (std::map<string, vector<CLink> >::const_iterator it = mapFileList.begin(); it != mapFileList.end(); it++)
        vcc.push_back(make_pair(CC_FILE_P, FileToContent(it->first)));
    if (vTags.size() > 0)
        vcc.push_back(make_pair(CC_TAG_P, EncodeContentUnitArray(CC_TAG, vTags)));


    CContent ctt;
    ctt.EncodeP(CC_FILE_PACKAGE_P, vcc);
    return ctt;

}

string CFilePackage::FileToContent(const string strFileName) const
{

    std::map<string, vector<CLink> >::const_iterator it = mapFileList.find(strFileName);
    //LogPrintf("CFilePackage::FileToContent filename:%s \n",strFileName);
    CContent content;

    if (it == mapFileList.end()) {
        content.EncodeUnit(CC_LINK, "");
        return content;
    }
    content.EncodeUnit(CC_NAME, it->first);
    if (it->second.size() == 1) {
        content.EncodeUnit(CC_LINK, it->second[0].Serialize());

    } else {
        CContent content1;
        for (unsigned i = 0; i < it->second.size(); i++)
            content1.EncodeUnit(CC_LINK, it->second[i].Serialize());
        content.EncodeUnit(CC_FILE_COMBINE_P, content1);

    }
    return content;
}

Value CFilePackage::ToJson()const
{
    json_spirit::Object obj;
    obj.push_back(Pair("packagename", Value(strPackageName)));
    obj.push_back(Pair("mainfile", Value(strMainFile)));
    Object objFiles;
    for (std::map<string, vector<CLink> >::const_iterator it = mapFileList.begin(); it != mapFileList.end(); it++) {

        vector<CLink> vlink = it->second;
        if (vlink.size() == 1) {
            objFiles.push_back(Pair(it->first, Value(vlink[0].ToString())));

        } else {
            Array arr2;
            for (unsigned int i = 0; i < vlink.size(); i++) {
                arr2.push_back(Value(vlink[i].ToString()));
            }
            objFiles.push_back(Pair(it->first, arr2));

        }
    }
    obj.push_back(Pair("files", objFiles));
    if (vTags.size() > 0) {
        Array arr3;
        for (unsigned int i = 0; i < vTags.size(); i++) {
            arr3.push_back(Value(vTags[i]));
        }
        obj.push_back(Pair("tags", arr3));
    }
    return Value(obj);
}

bool GetFilePackageUrl(const CLink link, string& url)
{
    string packageName = link.ToString(LINK_FORMAT_DEC_NOSCHEMA);    
    if (GetFilePackageMain(packageName, url, false))
        return true;
    CFilePackage package(link);
    return package.InstallPackage(packageName, false) && package.IsValid() && GetFilePackageMain(packageName, url, false);
}

bool DeleteFilePackage(const CLink link)
{

    boost::filesystem::path fpPath = GetDataDir() / "appdata" / "filepackages" / link.ToString();
    //fpPath.clear();
    if (!boost::filesystem::exists(fpPath))
        return false;
    boost::filesystem::remove_all(fpPath);
    return true;
}

bool ClearFilePackageCache(int nMaxSize)
{
    boost::filesystem::path fpPath = GetDataDir() / "appdata" / "filepackages";
    //fpPath.clear();
    if (!boost::filesystem::exists(fpPath))
        return false;
    boost::filesystem::remove_all(fpPath);
    return true;
}