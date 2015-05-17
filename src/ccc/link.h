#include <string.h>
#include <string>
#include <vector>
//
//#include "serialize.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
//
using namespace json_spirit;
using namespace std;
//using std::string;

static const std::string URI_SCHEME_NAME = "ccc";

enum linkformat
{
    LINK_FORMAT_DEC = 0,
    LINK_FORMAT_HEX = 1,
    LINK_FORMAT_B32 = 2,
};

class CLink
{
public:
    int nHeight;
    int nTx;
    int nVout;

    CLink()
    {
        SetEmpty();
    }

    CLink(const std::string& linkStr)
    {
        SetString(linkStr);
    }

    CLink(const vector<unsigned char>& linkVch)
    {
        SetString(linkVch);
    }

    CLink(const Array& linkJson)
    {
        SetJson(linkJson);
    }
    
    CLink(const int nHeightIn,const int nTxIn,const int nVoutIn = 0)
    {
        nHeight = nHeightIn;
        nTx = nTxIn;
        nVout = nVoutIn;
    }
    Array ToJson();
    bool ToJsonString(std::string& entry);
    std::string ToString(linkformat linkFormat = LINK_FORMAT_DEC);
    void SetEmpty();
    bool SetString(const std::string linkStr);
    bool SetString(const vector<unsigned char>& linkVch);
    bool SetJson(const Array& linkJson);
};