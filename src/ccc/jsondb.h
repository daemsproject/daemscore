#ifndef CCC_BROWSERDB_H
#define	CCC_BROWSERDB_H

#include "base58.h"
#include "json/json_spirit_value.h"
#include "../util.h"
#include <boost/filesystem.hpp>
using namespace json_spirit;

class CJsonDb
{
public:
    boost::filesystem::path confPath;
    boost::filesystem::path confFile;
    std::string category; // category defines filename of conf file
    std::string name;
    Value value;

    CJsonDb()
    {
        init();
    }
    void init();
    bool getValueFrFile();
    Value getValue();
    bool setValue(const Value& value);
    bool save();
};

class CBrowserFollow : public CJsonDb
{
public:
    std::string strCurrentWallet;
    CBrowserFollow()
    {
        category = "follow";
        value = Array();
        init();
    }
    bool isFollowed(const CBitcoinAddress& addr);
    bool isFollowed(const std::string& addrStr);
    bool getFollowed(std::vector<CBitcoinAddress>& followList);
    bool setFollow(const CBitcoinAddress& addr);
};

#endif	// CCC_BROWSERDB_H 