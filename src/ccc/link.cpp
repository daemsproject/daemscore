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

Array CLink::ToJson()
{

    //    iterator pc = begin();
    Array result;
    //    while (pc < end()) {
    //        cctype cc;
    //        CLink contentStr;
    //        if (!GetCcUnit(pc, cc, contentStr))
    //            break;
    //        Object ccUnit;
    //        std::string ccName;
    //        ccName = GetCcName(cc);
    //        ccUnit.push_back(Pair("cc_name", ccName));
    //        ccUnit.push_back(Pair("cc", GetCcHex(cc)));
    //        if (IsCcParent(cc)) {
    //            ccUnit.push_back(Pair("content", contentStr.ToJson()));
    //        } else {
    //            ccUnit.push_back(Pair("content", contentStr));
    //        }
    //        result.push_back(ccUnit);
    //    }
    //    if (pc > end()) {
    //        result.clear();
    //        Object rObj;
    //        rObj.push_back(Pair("content", HexStr(*this)));
    //        result.push_back(rObj);
    //    }
    return result;
}

bool CLink::SetJson(const Array& linkJson)
{

    //    cctype cc = CC_NULL;
    //
    //    BOOST_FOREACH(const Value& input, linkJson)
    //    {
    //        const Object& linkObj = input.get_obj();
    //
    //        BOOST_FOREACH(const Pair& ccUnit, linkObj)
    //        {
    //            std::string ccName;
    //            CLink content;
    //            if (ccUnit.name_ == "cc_name") {
    //                ccName = ccUnit.value_.get_str();
    //                cc = GetCcValue(ccName);
    //                continue;
    //            } else if (ccUnit.name_ == "content") {
    //                IsCcParent(cc) ? content.SetJson(ccUnit.value_.get_array()) : content.SetString(ccUnit.value_.get_str());
    //                WriteVarInt(cc);
    //                WriteCompactSize(content.size());
    //                append(content);
    //            }
    //        }
    //    }
    return true;
}

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
//    std::cout << "1H: " << nHeight << "\n";
//    std::cout << "1T: " << nTx << "\n";
//    std::cout << "1V: " << nVout << "\n";
    // Process Scheme name
    std::size_t posColumn = linkStr.find(":");
    std::string str;
    if (posColumn != std::string::npos) { // full link with ":"
        std::string sn = linkStr.substr(0, posColumn);
        //        std::cout << "sn: " << sn << "\n";
        if (sn != URI_SCHEME_NAME)
            return error("%s: Non-standard link", __func__);
        str = linkStr.substr(posColumn + 1);
    } else
        str = linkStr;

    // Process link content
    std::size_t posFirstDot = str.find(".");
    if (posFirstDot == std::string::npos)
        return error("%s: Non-standard link", __func__);
    std::string nHeightS = str.substr(0, posFirstDot);
    if (!IsStringInteger(nHeightS))
        return error("%s: Non-standard link (nHeight)", __func__);
    nHeight = atoi(nHeightS.c_str());
//    std::cout << "2H: " << nHeight << "\n";
    std::size_t posSecondDot = str.find(".", posFirstDot + 1);
    std::string nTxS = (posSecondDot == std::string::npos) ? str.substr(posFirstDot + 1) :
            str.substr(posFirstDot + 1, posSecondDot - posFirstDot);

//    std::cout << "nTS: " << nTxS << "\n";
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
//    std::cout << "nH: " << nHeight << "\n";
//    std::cout << "nT: " << nTx << "\n";
//    std::cout << "nV: " << nVout << "\n";
    return true;
}

bool CLink::SetString(const vector<unsigned char>& linkVch)
{
    std::string str(linkVch.begin(), linkVch.end());
    return SetString(str);
}

//std::string IntToHexStringH(int i)
//{
//    std::string str;
//    str += "x";
//    str += IntToHexString(i);
//    return str;
//}
//
//std::string IntToB32StringH(int i)
//{
//    std::string str;
//    str += "x";
//    str += EncodeBase32(i);
//    return str;
//}

std::string CLink::ToString(linkformat linkFormat)
{
    if (nHeight == -1 || nTx == -1 || nVout == -1)
        return "";
    std::string r = URI_SCHEME_NAME;
    r += ":";
    std::string nHeightS;
    std::string nTxS;
    std::string nVoutS;
    switch (linkFormat) {
        case LINK_FORMAT_DEC:
            nHeightS = strpatch::to_string(nHeight);
            nTxS = strpatch::to_string(nTx);
            break;
        case LINK_FORMAT_HEX:
            nHeightS = "x";
            nHeightS += IntToHexString(nHeight);
            nTxS = IntToHexString(nTx);
            break;
        case LINK_FORMAT_B32:
            nHeightS = "#";
            nHeightS += EncodeBase32(nHeight);
            nTxS = EncodeBase32(nTx);
            break;
    }
    r += nHeightS;
    r += ".";
    r += nTxS;
    if (nVout > 0) {
        r += ".";
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