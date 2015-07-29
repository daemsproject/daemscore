#include "link.h"
//#include "utilstrencodings.h"
#include <string.h>
#include <string>
//#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
//#include <stdio.h>
#include "util.h"
#include "base58.h"
#include <iterator>
#include <vector>

#include "utilstrencodings.h"
using namespace boost;
using namespace std;

void CLink::SetEmpty()
{
    nHeight = -1;
    nTx = 0;
    nVout = 0;
}

bool CLink::IsEmpty() const
{
    return (nHeight == -1 && nTx == 0 && nVout == 0);
}

bool CLink::SetInt(const int nHeightIn, const int nTxIn, const int nVoutIn)
{
    nHeight = nHeightIn;
    nTx = (unsigned short) nTxIn;
    nVout = (unsigned short) nVoutIn;
    return true;
}

bool CLink::SetString(const std::string linkStr)
{
    nHeight = -1;
    nTx = 0;
    nVout = 0;
    // Process Scheme name
    std::size_t posColon = linkStr.find(URI_COLON);
    std::string str;
    if (posColon != std::string::npos) { // full link with colon
        std::string sn = linkStr.substr(0, posColon);
        if (sn != URI_SCHEME_NAME) {
            LogPrintf("%s: Non-standard link header %s", __func__, sn);
            return false;
        }
        str = linkStr.substr(posColon + 1);
    } else
        str = linkStr;

    // Process link content
    std::size_t posFirstDot = str.find(URI_SEPERATOR);
    if (posFirstDot == std::string::npos) {
        LogPrintf("CLink %s: Non-standard link %s\n", __func__,linkStr);
        return false;
    }
    std::string nHeightS = str.substr(0, posFirstDot);
    std::size_t posSecondDot = str.find(".", posFirstDot + 1);
    std::string nTxS = (posSecondDot == std::string::npos) ? str.substr(posFirstDot + 1) :
            str.substr(posFirstDot + 1, posSecondDot - posFirstDot - 1);
    if (nHeightS.substr(0, 1) == URI_HEX_HEADER) {
        nHeight = HexStringToInt(nHeightS.substr(1));
        nTx = (unsigned short) HexStringToInt(nTxS);
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            nVout = (unsigned short) HexStringToInt(nVoutS);
        }
    } else if (nHeightS.substr(0, 1) == URI_B32_HEADER) {
        nHeight = DecodeBase32ToInt(nHeightS.substr(1));
        nTx = (unsigned short) DecodeBase32ToInt(nTxS);
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            nVout = (unsigned short) DecodeBase32ToInt(nVoutS);
        }
    } else {
        if (!IsStringInteger(nHeightS)) {
            LogPrintf("%s: Non-standard link (nHeight)", __func__);
            return false;
        }
        nHeight = atoi(nHeightS.c_str());
        if (!IsStringInteger(nTxS))
            return error("%s: Non-standard link (nTx)", __func__);
        nTx = (unsigned short) atoi(nTxS.c_str());
        if (posSecondDot == std::string::npos) {
            nVout = 0;
        } else {
            std::string nVoutS = str.substr(posSecondDot + 1);
            if (!IsStringInteger(nVoutS)) {
                LogPrintf("%s: Non-standard link (nVout)", __func__);
                return false;
            }

            nVout = (unsigned short) atoi(nVoutS.c_str());
        }
    }
    return true;
}

string CLink::Serialize()const
{
    string str;
    WriteVarInt(nHeight, str);
    WriteVarInt(nTx, str);
    WriteVarInt(nVout, str);
    // str.append((char*)&(nHeight), sizeof(nHeight));
    // str.append((char*)&(nTx), sizeof(nTx));
    // str.append((char*)&(nVout), sizeof(nVout)); 
    LogPrintf("CLink::Serialize() %s", HexStr(str.begin(), str.end()));
    return str;
}

bool CLink::Unserialize(string& str)
{
    //    if(str.size()!=8)
    //        return false;    
    //     nHeight = str[3]<<24|(unsigned char)str[2]<<16|(unsigned char)str[1]<<8|(unsigned char)str[0];
    //    nTx = (unsigned char)str[5]<<8|(unsigned char)str[4];
    //    nVout = (unsigned char)str[7]<<8|(unsigned char)str[6];
    LogPrintf("CLink::UnSerialize() %s\n", HexStr(str.begin(), str.end()));
    if (!ReadVarInt(str, nHeight))
        return false;
    int n = 0;
    if (!ReadVarInt(str, n))
        return false;
    if (n < 0 || n > 65535)
        return false;
    nTx = (unsigned short) n;
    if (!ReadVarInt(str, n))
        return false;
    if (n < 0 || n > 65535)
        return false;
    nVout = (unsigned short) n;
    LogPrintf("CLink::UnSerialize() %i,%i,%i \n", nHeight, nTx, nVout);
    return true;
}
bool CLink::UnserializeConst(const string& str)
{
    string strlink=str;
    return Unserialize(strlink);
}
bool CLink::WriteVarInt(const int nIn, string& str) const
{
    int n = nIn;
    char tmp[8];
    int len = 0;
    while (true) {
        tmp[len] = (n & 0x7F) | (len ? 0x80 : 0x00);
        if (n <= 0x7F)
            break;
        n = (n >> 7) - 1;
        len++;
    }
    do {
        std::string tstr;
        tstr += tmp[len];
        str.append(tstr, 0, 1);
    } while (len--);
    return true;
}

bool CLink::ReadVarInt(string& str, int& n)const
{
    string::iterator pc = str.begin();
    n = 0;
    unsigned char chData = 0xff;
    int offset = 0;
    while (pc < str.end()) {
        if (offset > 3)
            return false;
        chData = *pc++;
        n = (n << 7) | (chData & 0x7F);
        offset++;
        if (chData & 0x80)
            n++;
        else
            break;
    }
    str = str.substr(offset);
    return (chData & 0x80) ? false : true;
}

bool CLink::SetString(const vector<unsigned char>& linkVch)
{
    std::string str(linkVch.begin(), linkVch.end());
    return SetString(str);
}

std::string CLink::ToString(linkformat linkFormat)const
{
    if (IsEmpty())
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