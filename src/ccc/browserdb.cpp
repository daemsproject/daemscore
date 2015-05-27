
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>

#include "browserdb.h"
using namespace std;

bool CBrowserConf::getValueFrFile()
{
    if (!ReadFileToJson(confFile.string(), value))
        return false;
    return true;
}

Value CBrowserConf::getValue()
{
    return value;
}

void CBrowserConf::init()
{
    confPath = GetDataDir() / "conf";
    boost::filesystem::create_directories(confPath);
    confFile = GetDataDir() / "conf" / (category + ".conf");
    getValueFrFile();
}

bool CBrowserConf::setValue(const Value& valueIn)
{
    value = valueIn;
    return WriteJsonToFile(value, confFile.string());
}

bool CBrowserConf::save()
{
    return WriteJsonToFile(value.get_array(), confFile.string());
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
