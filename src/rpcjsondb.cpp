#include "ccc/jsondb.h"
#include "rpcserver.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;
using namespace boost::assign;
Value getbrowserconf(const json_spirit::Array& params, bool fHelp) //  To Do
{
    Object r;

    return r;
}

Value getfollowed(const json_spirit::Array& params, bool fHelp)
{
    Array r;
    CBrowserFollow fll;
    std::vector<CBitcoinAddress> tmp;
    fll.getFollowed(tmp);

    BOOST_FOREACH(const CBitcoinAddress& addr, tmp)
    {
        r.push_back(addr.ToString());
    }
    return r;
}

Value setfollow(const json_spirit::Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error("Wrong number of parameters");
    RPCTypeCheck(params, list_of(array_type));
    Array addrs = params[0].get_array();
    CBrowserFollow fll;

    BOOST_FOREACH(const Value& addrV, addrs)
    {
        if (!fll.isFollowed(addrV.get_str())) {
            CBitcoinAddress addr;
            if (addr.SetString(addrV.get_str()))
                fll.value.get_array().push_back(addrV.get_str());
        }
    }
    fll.save();
    Array empty;
    return getfollowed(empty, false);
}