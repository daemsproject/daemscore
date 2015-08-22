#include "fai/jsondb.h"
#include "rpcserver.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;
using namespace boost::assign;
using namespace json_spirit;

//Value getbrowserconf(const json_spirit::Array& params, bool fHelp) //  To Do
//{
//    Object r;
//
//    return r;
//}
//
//Value getfollowed(const json_spirit::Array& params, bool fHelp)
//{
//    Array r;
//    CBrowserFollow fll;
//    std::vector<CBitcoinAddress> tmp;
//    fll.getFollowed(tmp);
//
//    BOOST_FOREACH(const CBitcoinAddress& addr, tmp)
//    {
//        r.push_back(addr.ToString());
//    }
//    return r;
//}
//
//Value setfollow(const json_spirit::Array& params, bool fHelp)
//{
//    if (fHelp || params.size() != 1)
//        throw runtime_error("Wrong number of parameters");
//    RPCTypeCheck(params, list_of(array_type));
//    Array addrs = params[0].get_array();
//    CBrowserFollow fll;
//
//    BOOST_FOREACH(const Value& addrV, addrs)
//    {
//        if (!fll.isFollowed(addrV.get_str())) {
//            CBitcoinAddress addr;
//            if (addr.SetString(addrV.get_str()))
//                fll.value.get_array().push_back(addrV.get_str());
//        }
//    }
//    fll.save();
//    Array empty;
//    return getfollowed(empty, false);
//}
//
//Value setunfollow(const json_spirit::Array& params, bool fHelp)
//{
//    if (fHelp || params.size() != 1)
//        throw runtime_error("Wrong number of parameters");
//    RPCTypeCheck(params, list_of(array_type));
//    Array addrs = params[0].get_array();
//    CBrowserFollow fll;
//
//    BOOST_FOREACH(const Value& addrV, addrs)
//    {
//        if (fll.isFollowed(addrV.get_str())) {
//            CBitcoinAddress addr;
//            if (addr.SetString(addrV.get_str()))
//                fll.setUnfollow(addr);
//            }
//    }
//    fll.save();
//    Array empty;
//    return true;
//}
Value writefile(const json_spirit::Array& params, bool fHelp) //  To Do
{
    if (fHelp || params.size() !=4)
        throw runtime_error("writefile Wrong number of parameters");
    RPCTypeCheck(params, list_of(str_type));
//    for(unsigned int i=0;i<4;i++)
//        if(params[i].type()!=str_type)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected string");
    if(CJsonDb().WriteFile(params[0].get_str(),params[1].get_str(),params[2].get_str(),params[3].get_str()))
        return Value("success");
    throw JSONRPCError(RPC_INVALID_PARAMETER, "write file failed");
   
}
Value readfile(const json_spirit::Array& params, bool fHelp) //  To Do
{
    if (fHelp || params.size() !=3)
        throw runtime_error("readfile Wrong number of parameters");
    RPCTypeCheck(params, list_of(str_type));
//    for(unsigned int i=0;i<4;i++)
//        if(params[i].type()!=str_type)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected string");
    std::string filecontent;
    if(!CJsonDb().ReadFile(params[0].get_str(),params[1].get_str(),params[2].get_str(),filecontent))        
        throw JSONRPCError(RPC_INVALID_PARAMETER, "read file failed");
    return Value(filecontent);
}
Value setconf(const json_spirit::Array& params, bool fHelp) //  To Do
{
    if (fHelp || params.size() !=5)
        throw runtime_error("setconf Wrong number of parameters");
   // RPCTypeCheck(params, list_of(str_type));
    for(unsigned int i=0;i<4;i++)
        if(params[i].type()!=str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected string");
    std::string value;
    //LogPrintf("params4 type:%i\n",params[4].type());
    if (params[4].type()==null_type)
        value="";
    else if(params[4].type()==str_type)
        value=params[4].get_str();
    else
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter value, expected string");
    if(CJsonDb().WriteSetting(params[0].get_str(),params[1].get_str(),params[2].get_str(),params[3].get_str(),value))
        return Value("success");
    throw JSONRPCError(RPC_INVALID_PARAMETER, "write setting failed");
}
Value getconf(const json_spirit::Array& params, bool fHelp) //  To Do
{
    if (fHelp || params.size() !=4)
        throw runtime_error("getconf Wrong number of parameters");
    RPCTypeCheck(params, list_of(str_type));
//    for(unsigned int i=0;i<4;i++)
//        if(params[i].type()!=str_type)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected string");
    std::string strConf;
    if(!CJsonDb().ReadSetting(params[0].get_str(),params[1].get_str(),params[2].get_str(),params[3].get_str(),strConf))        
        throw JSONRPCError(RPC_INVALID_PARAMETER, "read setting failed");
    return Value(strConf);
}
