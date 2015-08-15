
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>

#include "jsondb.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"
using namespace std;

bool CJsonDb::getValueFrFile()
{
    if (!ReadFileToJson(confFile.string(), value))
        return false;
    return true;
}

Value CJsonDb::getValue()
{
    return value;
}

void CJsonDb::init()
{
    confPath = GetDataDir() / "conf";
    boost::filesystem::create_directories(confPath);
    confFile = GetDataDir() / "conf" / (category + ".conf");
    //getValueFrFile();
}

bool CJsonDb::setValue(const Value& valueIn)
{
    value = valueIn;
    return WriteJsonToFile(value, confFile.string());
}

bool CJsonDb::save()
{
    return WriteJsonToFile(value.get_array(), confFile.string());
}
bool CJsonDb::WriteFile(const std::string appName,const std::string path,const std::string filename,const std::string filecontent)
{   
    boost::filesystem::path filePath=GetDataDir() / "appdata" / appName / path;
    if (!boost::filesystem::exists(filePath))   
        boost::filesystem::create_directories(filePath);
    boost::filesystem::path fullPath=filePath / filename;
    //remove file if content is empty
    if(filecontent.size()==0)
    {
        boost::filesystem::remove(fullPath);
        return true;
    }
    else
        return StringToFile(fullPath.string(),filecontent);
}
bool CJsonDb::ReadFile(const std::string appName,const std::string path,const std::string filename,std::string& filecontent)
{    
    boost::filesystem::path fullPath=GetDataDir() / "appdata" / appName / path / filename;
    string strFullPath=fullPath.string();
    return ReadFileToString(strFullPath,filecontent);
}
bool CJsonDb::WriteSetting(const std::string appName,const std::string IDLocal,const std::string IDForeign,const std::string key,const std::string value)
{
    json_spirit::Object objConf;
    boost::filesystem::path file;
    bool fFound=false;
    if (GetConfObj(appName,IDLocal,IDForeign,objConf,file)){
        for(unsigned int i = 0; i != objConf.size(); ++i )
        {
            json_spirit::Pair& pair = objConf[i];
            const std::string name  = pair.name_;
            json_spirit::Value&  vvalue = pair.value_;
            if (name==key){
                fFound=true;
                vvalue=json_spirit::Value(value);                
                break;
            }            
        }
    }
    
    if (!fFound)
        objConf.push_back(Pair(key, Value(value)));
    json_spirit::Value val=json_spirit::Value(objConf);    
    return WriteJsonToFile(val,file.string());   
}
bool CJsonDb::ReadSetting(const std::string appName,const std::string IDLocal,const std::string IDForeign,const std::string& key,std::string& value)
{
    json_spirit::Object objConf;    
    boost::filesystem::path file;
    json_spirit::Value valConfValue;
    if (GetConfObj(appName,IDLocal,IDForeign,objConf,file)){         
        valConfValue = find_value(objConf, key);
        if (valConfValue.type()!=null_type){            
            value=json_spirit::write_string(valConfValue,false);
            return true;
        }            
    }    
    return false;
}
bool CJsonDb::GetConfObj(const std::string appName,const std::string IDLocal,const std::string IDForeign,json_spirit::Object& objConf,boost::filesystem::path& file){ 
    boost::filesystem::path fullPath;
    std::string fileName;    
    fullPath=GetDataDir() / "appdata" / appName;    
    if(IDLocal=="")
        fileName=appName;
    else
    {
        if (IDForeign=="")
            fileName=IDLocal;
        else
        {            
            fullPath /=IDLocal;
            fileName=IDForeign;
        }
    }
    file= fullPath / fileName.append(".conf");
    if (!boost::filesystem::exists(fullPath))   
        boost::filesystem::create_directories(fullPath);
    std::string strConfFileName=file.string();    
    //LogPrintf("getconfobj file:%s \n",strConfFileName);
    json_spirit::Value valConf;    
    if(boost::filesystem::exists(file)&&ReadFileToJson(strConfFileName,valConf)){        
            if (valConf.type()==obj_type){
                objConf = valConf.get_obj();
                return true;
            }
    }    
    return false;
}   

bool CBrowserFollow::getFollowed(std::vector<CBitcoinAddress>& followList)
{

    BOOST_FOREACH(const Value& addrV, value.get_array())
    {
        CBitcoinAddress addr;
        if (addr.SetString(addrV.get_str()))
            followList.push_back(addr);
    }
    return true;
}

bool CBrowserFollow::setFollow(const CBitcoinAddress& addr)
{
    if (!isFollowed(addr)) {
        value.get_array().push_back(addr.ToString());
        return save();
    }
    return true;
}

bool CBrowserFollow::setUnfollow(const CBitcoinAddress& addr)
{
    for (json_spirit::Array::iterator it = value.get_array().begin(); it <= value.get_array().end(); it++) {
        if ((*it).get_str() == addr.ToString()) {
            value.get_array().erase(it);
        }
    }
    return save();
}

bool CBrowserFollow::isFollowed(const CBitcoinAddress& addr)
{
    return isFollowed(addr.ToString());
}

bool CBrowserFollow::isFollowed(const string& addrStr)
{

    BOOST_FOREACH(const Value& addrV, value.get_array())
    {
        if (addrV.get_str() == addrStr)
            return true;
    }
    return false;
}
