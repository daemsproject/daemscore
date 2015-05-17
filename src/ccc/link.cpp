#include "link.h"
//#include "utilstrencodings.h"
#include <string.h>
#include <string>
//#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
//#include <stdio.h>
#include "util.h"
#include "base58.h"



#include "utilstrencodings.h"
using namespace boost;
using namespace std;

void CLink::SetEmpty()
{
    nHeight = -1;
    nTx = -1;
    nVout = -1;
}

bool CLink::SetString(const std::string linkStr)
{
    nHeight = -1;
    nTx = -1;
    nVout = -1;
    // Process Scheme name
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn != URI_SCHEME_NAME)
            return error("%s: Non-standard link", __func__);
        str = linkStr.substr(posColon + 1);
    } else
        str = linkStr;

    // Process link content
    std::size_t posFirstDot = str.find(URI_SEPERATOR);
    if (posFirstDot == std::string::npos)
        return error("%s: Non-standard link", __func__);
    std::string nHeightS = str.substr(0, posFirstDot);
    std::size_t posSecondDot = str.find(".", posFirstDot + 1);
    std::string nTxS = (posSecondDot == std::string::npos) ? str.substr(posFirstDot + 1) :
            str.substr(posFirstDot + 1, posSecondDot - posFirstDot);
    if (nHeightS.substr(0, 1) == URI_HEX_HEADER) {
        nHeight = HexStringToInt(nHeightS.substr(1));
        nTx = HexStringToInt(nTxS);
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            nVout = HexStringToInt(nVoutS);
        }
    } else if (nHeightS.substr(0, 1) == URI_B32_HEADER) {
        nHeight = DecodeBase32ToInt(nHeightS.substr(1));
        nTx = DecodeBase32ToInt(nTxS);
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            nVout = DecodeBase32ToInt(nVoutS);
        }
    } else {
        if (!IsStringInteger(nHeightS))
            return error("%s: Non-standard link (nHeight)", __func__);
        nHeight = atoi(nHeightS.c_str());
        if (!IsStringInteger(nTxS))
            return error("%s: Non-standard link (nTx)", __func__);
        nTx = atoi(nTxS.c_str());
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            if (!IsStringInteger(nVoutS))
                return error("%s: Non-standard link (nVout)", __func__);
            nVout = atoi(nVoutS.c_str());
        }
    }
    return true;
}

bool CLink::SetString(const vector<unsigned char>& linkVch)
{
    std::string str(linkVch.begin(), linkVch.end());
    return SetString(str);
}

std::string CLink::ToString(linkformat linkFormat)
{
    if (nHeight == -1 || nTx == -1 || nVout == -1)
        return "";
    std::string r = URI_SCHEME_NAME;
    r += URI_COLON;
    std::string nHeightS;
    std::string nTxS;
    std::string nVoutS;
    switch (linkFormat) {
        case LINK_FORMAT_DEC:
            nHeightS = strpatch::to_string(nHeight);
            nTxS = strpatch::to_string(nTx);
            break;
        case LINK_FORMAT_HEX:
            nHeightS = URI_HEX_HEADER;
            nHeightS += IntToHexString(nHeight);
            nTxS = IntToHexString(nTx);
            break;
        case LINK_FORMAT_B32:
            nHeightS = URI_B32_HEADER;
            nHeightS += EncodeBase32(nHeight);
            nTxS = EncodeBase32(nTx);
            break;
    }
    r += nHeightS;
    r += URI_SEPERATOR;
    r += nTxS;
    if (nVout > 0) {
        r += URI_SEPERATOR;
        switch (linkFormat) {
            case LINK_FORMAT_DEC:
                r += strpatch::to_string(nVout);
                break;
            case LINK_FORMAT_HEX:
                r += IntToHexString(nVout);
                break;
            case LINK_FORMAT_B32:
                r += EncodeBase32(nVout);
                break;
        }
    }
    return r;
}