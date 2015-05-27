#include "content.h"
#include "utilstrencodings.h"
#include <string.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <stdio.h>
#include "util.h"

using namespace boost;
using namespace std;

#define to_uint64(n) (*(uint64_t*)(n))
static const int TRIM_READABLE_LEN = 1000;
static const int TRIM_BINARY_LEN = 20;
static const int STR_FORMAT_SUM_MAXSIZE = 512;

std::string GetCcName(const cctype cc)
{
    switch (cc) {
            /** Null * */
        case CC_NULL: return "CC_NULL";

            /** Tag * */
        case CC_TAG: return "CC_TAG";
        case CC_TAG_P: return "CC_TAG_P";

            /** First Level Content Code */
        case CC_TEXT: return "CC_TEXT";
        case CC_TEXT_P: return "CC_TEXT_P";
        case CC_FILE: return "CC_FILE";
        case CC_FILE_P: return "CC_FILE_P";
        case CC_LINK: return "CC_LINK";
        case CC_LINK_P: return "CC_LINK_P";
        case CC_DOMAIN: return "CC_DOMAIN";
        case CC_DOMAIN_P: return "CC_DOMAIN_P";
        case CC_COMMENT: return "CC_COMMENT";
        case CC_COMMENT_P: return "CC_COMMENT_P";
        case CC_DELETE: return "CC_DELETE";
        case CC_DELETE_P: return "CC_DELETE_P";
        case CC_PAYMENT: return "CC_PAYMENT";
        case CC_PAYMENT_P: return "CC_PAYMENT_P";
        case CC_LANG: return "CC_LANG";
        case CC_LANG_P: return "CC_LANG_P";

            /** Second Level Content Code * */
            // Tag
        case CC_TAG_TEXT: return "CC_TAG_TEXT";
        case CC_TAG_FILE: return "CC_TAG_FILE";
        case CC_TAG_LINK: return "CC_TAG_LINK";
        case CC_TAG_DOMAIN: return "CC_TAG_DOMAIN";
        case CC_TAG_COMMENT: return "CC_TAG_COMMENT";
        case CC_TAG_NEWS: return "CC_TAG_NEWS";
        case CC_TAG_IMAGE: return "CC_TAG_IMAGE";
        case CC_TAG_AUDIO: return "CC_TAG_AUDIO";
        case CC_TAG_VIDEO: return "CC_TAG_VIDEO";
        case CC_TAG_MESSAGE: return "CC_TAG_MESSAGE";
        case CC_TAG_ADVERTISMENT: return "CC_TAG_ADVERTISMENT";
        case CC_TAG_STREAM: return "CC_TAG_STREAM";
        case CC_TAG_CONTENT: return "CC_TAG_CONTENT";
        case CC_TAG_COIN: return "CC_TAG_COIN";
        case CC_TAG_ID: return "CC_TAG_ID";
            // Text
        case CC_TEXT_ENCODINGSTRING: return "CC_TEXT_ENCODINGSTRING";
        case CC_TEXT_ENCODING_UTF8: return "CC_TEXT_ENCODING_UTF8";
        case CC_TEXT_ENCODING_ASCII: return "CC_TEXT_ENCODING_ASCII";
        case CC_TEXT_ENCODING_UTF16: return "CC_TEXT_ENCODING_UTF16";
        case CC_TEXT_ENCODING_UTF32: return "CC_TEXT_ENCODING_UTF32";
        case CC_TEXT_ENCODING_ISO8859_1: return "CC_TEXT_ENCODING_ISO8859_1";
        case CC_TEXT_ENCODING_ISO8859_2: return "CC_TEXT_ENCODING_ISO8859_2";
        case CC_TEXT_ENCODING_ISO8859_3: return "CC_TEXT_ENCODING_ISO8859_3";
        case CC_TEXT_ENCODING_ISO8859_4: return "CC_TEXT_ENCODING_ISO8859_4";
        case CC_TEXT_ENCODING_ISO8859_5: return "CC_TEXT_ENCODING_ISO8859_5";
        case CC_TEXT_ENCODING_ISO8859_6: return "CC_TEXT_ENCODING_ISO8859_6";
        case CC_TEXT_ENCODING_ISO8859_7: return "CC_TEXT_ENCODING_ISO8859_7";
        case CC_TEXT_ENCODING_ISO8859_8: return "CC_TEXT_ENCODING_ISO8859_8";
        case CC_TEXT_ENCODING_ISO8859_9: return "CC_TEXT_ENCODING_ISO8859_9";
        case CC_TEXT_ENCODING_ISO8859_10: return "CC_TEXT_ENCODING_ISO8859_10";
        case CC_TEXT_ENCODING_ISO8859_11: return "CC_TEXT_ENCODING_ISO8859_11";
        case CC_TEXT_ENCODING_ISO8859_12: return "CC_TEXT_ENCODING_ISO8859_12";
        case CC_TEXT_ENCODING_ISO8859_13: return "CC_TEXT_ENCODING_ISO8859_13";
        case CC_TEXT_ENCODING_ISO8859_14: return "CC_TEXT_ENCODING_ISO8859_14";
        case CC_TEXT_ENCODING_ISO8859_15: return "CC_TEXT_ENCODING_ISO8859_15";
        case CC_TEXT_ENCODING_ISO8859_16: return "CC_TEXT_ENCODING_ISO8859_16";
        case CC_TEXT_ENCODING_GB2312: return "CC_TEXT_ENCODING_GB2312";
        case CC_TEXT_ENCODING_GBK: return "CC_TEXT_ENCODING_GBK";
        case CC_TEXT_ENCODING_GB18030: return "CC_TEXT_ENCODING_GB18030";
        case CC_TEXT_ENCODING_BIG5: return "CC_TEXT_ENCODING_BIG5";
        case CC_TEXT_ENCODING_HKSCS: return "CC_TEXT_ENCODING_HKSCS";
        case CC_TEXT_ENCODING_SHIFTJIS: return "CC_TEXT_ENCODING_SHIFTJIS";
        case CC_TEXT_ENCODING_EUCJP: return "CC_TEXT_ENCODING_EUCJP";
        case CC_TEXT_ENCODING_ISO2022JP: return "CC_TEXT_ENCODING_ISO2022JP";
        case CC_TEXT_ENCODING_EUCJIS2004: return "CC_TEXT_ENCODING_EUCJIS2004";
        case CC_TEXT_ENCODING_ISO2022JP2004: return "CC_TEXT_ENCODING_ISO2022JP2004";
        case CC_TEXT_ENCODING_KSX1001: return "CC_TEXT_ENCODING_KSX1001";
        case CC_TEXT_ENCODING_EUCKR: return "CC_TEXT_ENCODING_EUCKR";
        case CC_TEXT_ENCODING_ISO2022KR: return "CC_TEXT_ENCODING_ISO2022KR";
        case CC_TEXT_ENCODING_WINDOWS1250: return "CC_TEXT_ENCODING_WINDOWS1250";
        case CC_TEXT_ENCODING_WINDOWS1251: return "CC_TEXT_ENCODING_WINDOWS1251";
        case CC_TEXT_ENCODING_WINDOWS1252: return "CC_TEXT_ENCODING_WINDOWS1252";
        case CC_TEXT_ENCODING_WINDOWS1253: return "CC_TEXT_ENCODING_WINDOWS1253";
        case CC_TEXT_ENCODING_WINDOWS1254: return "CC_TEXT_ENCODING_WINDOWS1254";
        case CC_TEXT_ENCODING_WINDOWS1255: return "CC_TEXT_ENCODING_WINDOWS1255";
        case CC_TEXT_ENCODING_WINDOWS1256: return "CC_TEXT_ENCODING_WINDOWS1256";
        case CC_TEXT_ENCODING_WINDOWS1257: return "CC_TEXT_ENCODING_WINDOWS1257";
        case CC_TEXT_ENCODING_WINDOWS1258: return "CC_TEXT_ENCODING_WINDOWS1258";
        case CC_TEXT_TYPESTRING: return "CC_TEXT_TYPESTRING";
        case CC_TEXT_TYPE_TEXT: return "CC_TEXT_TYPE_TEXT";
        case CC_TEXT_TYPE_HTML: return "CC_TEXT_TYPE_HTML";
        case CC_TEXT_TYPE_XML: return "CC_TEXT_TYPE_XML";
        case CC_TEXT_TYPE_JSON: return "CC_TEXT_TYPE_JSON";
        case CC_TEXT_TYPE_YAML: return "CC_TEXT_TYPE_YAML";
        case CC_TEXT_TYPE_CSV: return "CC_TEXT_TYPE_CSV";
            // File
        case CC_FILE_CONTENT: return "CC_FILE_CONTENT";
        case CC_FILE_CONTENT_P: return "CC_FILE_CONTENT_P";
        case CC_FILE_NAME: return "CC_FILE_NAME";
        case CC_FILE_NAME_P: return "CC_FILE_NAME_P";
        case CC_FILE_TYPESTRING: return "CC_FILE_TYPESTRING";
            // Link
        case CC_LINK_TYPESTRING: return "CC_LINK_TYPESTRING";
        case CC_LINK_TYPE_TXIDOUT: return "CC_LINK_TYPE_TXIDOUT";
        case CC_LINK_TYPE_TXID: return "CC_LINK_TYPE_TXID";
        case CC_LINK_TYPE_COINTO: return "CC_LINK_TYPE_COINTO";
        case CC_LINK_TYPE_HTTP: return "CC_LINK_TYPE_HTTP";
        case CC_LINK_TYPE_HTTPS: return "CC_LINK_TYPE_HTTPS";
        case CC_LINK_TYPE_MAILTO: return "CC_LINK_TYPE_MAILTO";
        case CC_LINK_TYPE_FTP: return "CC_LINK_TYPE_FTP";
        case CC_LINK_TYPE_FILE: return "CC_LINK_TYPE_FILE";
        case CC_LINK_TYPE_CRID: return "CC_LINK_TYPE_CRID";
        case CC_LINK_TYPE_ED2K: return "CC_LINK_TYPE_ED2K";
        case CC_LINK_TYPE_MAGNET: return "CC_LINK_TYPE_MAGNET";
            // Domain
        case CC_DOMAIN_REG: return "CC_DOMAIN_REG";
        case CC_DOMAIN_REG_P: return "CC_DOMAIN_REG_P";
        case CC_DOMAIN_FORWARD: return "CC_DOMAIN_FORWARD";
        case CC_DOMAIN_FORWARD_P: return "CC_DOMAIN_FORWARD_P";
        case CC_DOMAIN_TRANSFER: return "CC_DOMAIN_TRANSFER";
        case CC_DOMAIN_TRANSFER_P: return "CC_DOMAIN_TRANSFER_P";
            // Comment
        case CC_COMMENT_CONTENT: return "CC_COMMENT_CONTENT";
        case CC_COMMENT_CONTENT_P: return "CC_COMMENT_CONTENT_P";
        case CC_COMMENT_LIKE: return "CC_COMMENT_LIKE";
        case CC_COMMENT_LIKE_P: return "CC_COMMENT_LIKE_P";
        case CC_COMMENT_DISLIKE: return "CC_COMMENT_DISLIKE";
        case CC_COMMENT_DISLIKE_P: return "CC_COMMENT_DISLIKE_P";
        case CC_COMMENT_SCORESTRING: return "CC_COMMENT_SCORESTRING";
        case CC_COMMENT_SCORE_0: return "CC_COMMENT_SCORE_0";
        case CC_COMMENT_SCORE_1: return "CC_COMMENT_SCORE_1";
        case CC_COMMENT_SCORE_2: return "CC_COMMENT_SCORE_2";
        case CC_COMMENT_SCORE_3: return "CC_COMMENT_SCORE_3";
        case CC_COMMENT_SCORE_4: return "CC_COMMENT_SCORE_4";
        case CC_COMMENT_SCORE_5: return "CC_COMMENT_SCORE_5";
        case CC_COMMENT_SCORE_6: return "CC_COMMENT_SCORE_6";
        case CC_COMMENT_SCORE_7: return "CC_COMMENT_SCORE_7";
        case CC_COMMENT_SCORE_8: return "CC_COMMENT_SCORE_8";
        case CC_COMMENT_SCORE_9: return "CC_COMMENT_SCORE_9";
        case CC_COMMENT_SCORE_10: return "CC_COMMENT_SCORE_10";
        case CC_COMMENT_SCORE_P: return "CC_COMMENT_SCORE_P";
            // Payment
        case CC_PAYMENT_REQ: return "CC_PAYMENT_REQ";
        case CC_PAYMENT_REQ_P: return "CC_PAYMENT_REQ_P";
        case CC_PAYMENT_PRODID: return "CC_PAYMENT_PRODID";
        case CC_PAYMENT_PRODID_P: return "CC_PAYMENT_PRODID_P";
        case CC_PAYMENT_QUANTITY: return "CC_PAYMENT_QUANTITY";
        case CC_PAYMENT_UNITPRICE: return "CC_PAYMENT_UNITPRICE";
        case CC_PAYMENT_DELIVERADD: return "CC_PAYMENT_DELIVERADD";
        case CC_PAYMENT_CONTACTNUM: return "CC_PAYMENT_CONTACTNUM";
        case CC_PAYMENT_CONTACTEMAIL: return "CC_PAYMENT_CONTACTEMAIL";
        case CC_PAYMENT_REQ_ADDR: return "CC_PAYMENT_REQ_ADDR";
        case CC_PAYMENT_REQ_AMOUNT: return "CC_PAYMENT_REQ_AMOUNT";
        case CC_PAYMENT_REQ_VALIDTILL: return "CC_PAYMENT_REQ_VALIDTILL";
        case CC_PAYMENT_REQ_VALIDTIME: return "CC_PAYMENT_REQ_VALIDTIME";
            // Lang

        case CC_LANG_IETF: return "CC_LANG_IETF";
        case CC_LANG_ISO639_1: return "CC_LANG_ISO639_1";
        case CC_LANG_ISO639_2: return "CC_LANG_ISO639_2";
        case CC_LANG_ISO639_3: return "CC_LANG_ISO639_3";
        case CC_LANG_COUNTRY: return "CC_LANG_COUNTRY";

        case CC_LANG_AB: return "CC_LANG_AB";
        case CC_LANG_AA: return "CC_LANG_AA";
        case CC_LANG_AF: return "CC_LANG_AF";
        case CC_LANG_AK: return "CC_LANG_AK";
        case CC_LANG_SQ: return "CC_LANG_SQ";
        case CC_LANG_AM: return "CC_LANG_AM";
        case CC_LANG_AR: return "CC_LANG_AR";
        case CC_LANG_AN: return "CC_LANG_AN";
        case CC_LANG_HY: return "CC_LANG_HY";
        case CC_LANG_AS: return "CC_LANG_AS";
        case CC_LANG_AV: return "CC_LANG_AV";
        case CC_LANG_AE: return "CC_LANG_AE";
        case CC_LANG_AY: return "CC_LANG_AY";
        case CC_LANG_AZ: return "CC_LANG_AZ";
        case CC_LANG_BM: return "CC_LANG_BM";
        case CC_LANG_BA: return "CC_LANG_BA";
        case CC_LANG_EU: return "CC_LANG_EU";
        case CC_LANG_BE: return "CC_LANG_BE";
        case CC_LANG_BN: return "CC_LANG_BN";
        case CC_LANG_BH: return "CC_LANG_BH";
        case CC_LANG_BI: return "CC_LANG_BI";
        case CC_LANG_BS: return "CC_LANG_BS";
        case CC_LANG_BR: return "CC_LANG_BR";
        case CC_LANG_BG: return "CC_LANG_BG";
        case CC_LANG_MY: return "CC_LANG_MY";
        case CC_LANG_CA: return "CC_LANG_CA";
        case CC_LANG_CH: return "CC_LANG_CH";
        case CC_LANG_CE: return "CC_LANG_CE";
        case CC_LANG_NY: return "CC_LANG_NY";
        case CC_LANG_ZH: return "CC_LANG_ZH";
        case CC_LANG_CV: return "CC_LANG_CV";
        case CC_LANG_KW: return "CC_LANG_KW";
        case CC_LANG_CO: return "CC_LANG_CO";
        case CC_LANG_CR: return "CC_LANG_CR";
        case CC_LANG_HR: return "CC_LANG_HR";
        case CC_LANG_CS: return "CC_LANG_CS";
        case CC_LANG_DA: return "CC_LANG_DA";
        case CC_LANG_DV: return "CC_LANG_DV";
        case CC_LANG_NL: return "CC_LANG_NL";
        case CC_LANG_DZ: return "CC_LANG_DZ";
        case CC_LANG_EN: return "CC_LANG_EN";
        case CC_LANG_EO: return "CC_LANG_EO";
        case CC_LANG_ET: return "CC_LANG_ET";
        case CC_LANG_EE: return "CC_LANG_EE";
        case CC_LANG_FO: return "CC_LANG_FO";
        case CC_LANG_FJ: return "CC_LANG_FJ";
        case CC_LANG_FI: return "CC_LANG_FI";
        case CC_LANG_FR: return "CC_LANG_FR";
        case CC_LANG_FF: return "CC_LANG_FF";
        case CC_LANG_GL: return "CC_LANG_GL";
        case CC_LANG_KA: return "CC_LANG_KA";
        case CC_LANG_DE: return "CC_LANG_DE";
        case CC_LANG_EL: return "CC_LANG_EL";
        case CC_LANG_GN: return "CC_LANG_GN";
        case CC_LANG_GU: return "CC_LANG_GU";
        case CC_LANG_HT: return "CC_LANG_HT";
        case CC_LANG_HA: return "CC_LANG_HA";
        case CC_LANG_HE: return "CC_LANG_HE";
        case CC_LANG_HZ: return "CC_LANG_HZ";
        case CC_LANG_HI: return "CC_LANG_HI";
        case CC_LANG_HO: return "CC_LANG_HO";
        case CC_LANG_HU: return "CC_LANG_HU";
        case CC_LANG_IA: return "CC_LANG_IA";
        case CC_LANG_ID: return "CC_LANG_ID";
        case CC_LANG_IE: return "CC_LANG_IE";
        case CC_LANG_GA: return "CC_LANG_GA";
        case CC_LANG_IG: return "CC_LANG_IG";
        case CC_LANG_IK: return "CC_LANG_IK";
        case CC_LANG_IO: return "CC_LANG_IO";
        case CC_LANG_IS: return "CC_LANG_IS";
        case CC_LANG_IT: return "CC_LANG_IT";
        case CC_LANG_IU: return "CC_LANG_IU";
        case CC_LANG_JA: return "CC_LANG_JA";
        case CC_LANG_JV: return "CC_LANG_JV";
        case CC_LANG_KL: return "CC_LANG_KL";
        case CC_LANG_KN: return "CC_LANG_KN";
        case CC_LANG_KR: return "CC_LANG_KR";
        case CC_LANG_KS: return "CC_LANG_KS";
        case CC_LANG_KK: return "CC_LANG_KK";
        case CC_LANG_KM: return "CC_LANG_KM";
        case CC_LANG_KI: return "CC_LANG_KI";
        case CC_LANG_RW: return "CC_LANG_RW";
        case CC_LANG_KY: return "CC_LANG_KY";
        case CC_LANG_KV: return "CC_LANG_KV";
        case CC_LANG_KG: return "CC_LANG_KG";
        case CC_LANG_KO: return "CC_LANG_KO";
        case CC_LANG_KU: return "CC_LANG_KU";
        case CC_LANG_KJ: return "CC_LANG_KJ";
        case CC_LANG_LA: return "CC_LANG_LA";
        case CC_LANG_LB: return "CC_LANG_LB";
        case CC_LANG_LG: return "CC_LANG_LG";
        case CC_LANG_LI: return "CC_LANG_LI";
        case CC_LANG_LN: return "CC_LANG_LN";
        case CC_LANG_LO: return "CC_LANG_LO";
        case CC_LANG_LT: return "CC_LANG_LT";
        case CC_LANG_LU: return "CC_LANG_LU";
        case CC_LANG_LV: return "CC_LANG_LV";
        case CC_LANG_GV: return "CC_LANG_GV";
        case CC_LANG_MK: return "CC_LANG_MK";
        case CC_LANG_MG: return "CC_LANG_MG";
        case CC_LANG_MS: return "CC_LANG_MS";
        case CC_LANG_ML: return "CC_LANG_ML";
        case CC_LANG_MT: return "CC_LANG_MT";
        case CC_LANG_MI: return "CC_LANG_MI";
        case CC_LANG_MR: return "CC_LANG_MR";
        case CC_LANG_MH: return "CC_LANG_MH";
        case CC_LANG_MN: return "CC_LANG_MN";
        case CC_LANG_NA: return "CC_LANG_NA";
        case CC_LANG_NV: return "CC_LANG_NV";
        case CC_LANG_ND: return "CC_LANG_ND";
        case CC_LANG_NE: return "CC_LANG_NE";
        case CC_LANG_NG: return "CC_LANG_NG";
        case CC_LANG_NB: return "CC_LANG_NB";
        case CC_LANG_NN: return "CC_LANG_NN";
        case CC_LANG_NO: return "CC_LANG_NO";
        case CC_LANG_II: return "CC_LANG_II";
        case CC_LANG_NR: return "CC_LANG_NR";
        case CC_LANG_OC: return "CC_LANG_OC";
        case CC_LANG_OJ: return "CC_LANG_OJ";
        case CC_LANG_CU: return "CC_LANG_CU";
        case CC_LANG_OM: return "CC_LANG_OM";
        case CC_LANG_OR: return "CC_LANG_OR";
        case CC_LANG_OS: return "CC_LANG_OS";
        case CC_LANG_PA: return "CC_LANG_PA";
        case CC_LANG_PI: return "CC_LANG_PI";
        case CC_LANG_FA: return "CC_LANG_FA";
        case CC_LANG_PL: return "CC_LANG_PL";
        case CC_LANG_PS: return "CC_LANG_PS";
        case CC_LANG_PT: return "CC_LANG_PT";
        case CC_LANG_QU: return "CC_LANG_QU";
        case CC_LANG_RM: return "CC_LANG_RM";
        case CC_LANG_RN: return "CC_LANG_RN";
        case CC_LANG_RO: return "CC_LANG_RO";
        case CC_LANG_RU: return "CC_LANG_RU";
        case CC_LANG_SA: return "CC_LANG_SA";
        case CC_LANG_SC: return "CC_LANG_SC";
        case CC_LANG_SD: return "CC_LANG_SD";
        case CC_LANG_SE: return "CC_LANG_SE";
        case CC_LANG_SM: return "CC_LANG_SM";
        case CC_LANG_SG: return "CC_LANG_SG";
        case CC_LANG_SR: return "CC_LANG_SR";
        case CC_LANG_GD: return "CC_LANG_GD";
        case CC_LANG_SN: return "CC_LANG_SN";
        case CC_LANG_SI: return "CC_LANG_SI";
        case CC_LANG_SK: return "CC_LANG_SK";
        case CC_LANG_SL: return "CC_LANG_SL";
        case CC_LANG_SO: return "CC_LANG_SO";
        case CC_LANG_ST: return "CC_LANG_ST";
        case CC_LANG_ES: return "CC_LANG_ES";
        case CC_LANG_SU: return "CC_LANG_SU";
        case CC_LANG_SW: return "CC_LANG_SW";
        case CC_LANG_SS: return "CC_LANG_SS";
        case CC_LANG_SV: return "CC_LANG_SV";
        case CC_LANG_TA: return "CC_LANG_TA";
        case CC_LANG_TE: return "CC_LANG_TE";
        case CC_LANG_TG: return "CC_LANG_TG";
        case CC_LANG_TH: return "CC_LANG_TH";
        case CC_LANG_TI: return "CC_LANG_TI";
        case CC_LANG_BO: return "CC_LANG_BO";
        case CC_LANG_TK: return "CC_LANG_TK";
        case CC_LANG_TL: return "CC_LANG_TL";
        case CC_LANG_TN: return "CC_LANG_TN";
        case CC_LANG_TO: return "CC_LANG_TO";
        case CC_LANG_TR: return "CC_LANG_TR";
        case CC_LANG_TS: return "CC_LANG_TS";
        case CC_LANG_TT: return "CC_LANG_TT";
        case CC_LANG_TW: return "CC_LANG_TW";
        case CC_LANG_TY: return "CC_LANG_TY";
        case CC_LANG_UG: return "CC_LANG_UG";
        case CC_LANG_UK: return "CC_LANG_UK";
        case CC_LANG_UR: return "CC_LANG_UR";
        case CC_LANG_UZ: return "CC_LANG_UZ";
        case CC_LANG_VE: return "CC_LANG_VE";
        case CC_LANG_VI: return "CC_LANG_VI";
        case CC_LANG_VO: return "CC_LANG_VO";
        case CC_LANG_WA: return "CC_LANG_WA";
        case CC_LANG_CY: return "CC_LANG_CY";
        case CC_LANG_WO: return "CC_LANG_WO";
        case CC_LANG_FY: return "CC_LANG_FY";
        case CC_LANG_XH: return "CC_LANG_XH";
        case CC_LANG_YI: return "CC_LANG_YI";
        case CC_LANG_YO: return "CC_LANG_YO";
        case CC_LANG_ZA: return "CC_LANG_ZA";
        case CC_LANG_ZU: return "CC_LANG_ZU";

        default:
            return "CC_UNKNOWN";
    }
}

cctype GetCcValue(std::string ccName)
{
    /** Null * */
    if (ccName == "CC_NULL") return CC_NULL;

        /** Tag * */
    else if (ccName == "CC_TAG") return CC_TAG;
    else if (ccName == "CC_TAG_P") return CC_TAG_P;

        /** First Level Content Code */
    else if (ccName == "CC_TEXT") return CC_TEXT;
    else if (ccName == "CC_TEXT_P") return CC_TEXT_P;
    else if (ccName == "CC_FILE") return CC_FILE;
    else if (ccName == "CC_FILE_P") return CC_FILE_P;
    else if (ccName == "CC_LINK") return CC_LINK;
    else if (ccName == "CC_LINK_P") return CC_LINK_P;
    else if (ccName == "CC_DOMAIN") return CC_DOMAIN;
    else if (ccName == "CC_DOMAIN_P") return CC_DOMAIN_P;
    else if (ccName == "CC_COMMENT") return CC_COMMENT;
    else if (ccName == "CC_COMMENT_P") return CC_COMMENT_P;
    else if (ccName == "CC_DELETE") return CC_DELETE;
    else if (ccName == "CC_DELETE_P") return CC_DELETE_P;
    else if (ccName == "CC_PAYMENT") return CC_PAYMENT;
    else if (ccName == "CC_PAYMENT_P") return CC_PAYMENT_P;
    else if (ccName == "CC_LANG") return CC_LANG;
    else if (ccName == "CC_LANG_P") return CC_LANG_P;

        /** Second Level Content Code * */
        // Tag
    else if (ccName == "CC_TAG_TEXT") return CC_TAG_TEXT;
    else if (ccName == "CC_TAG_FILE") return CC_TAG_FILE;
    else if (ccName == "CC_TAG_LINK") return CC_TAG_LINK;
    else if (ccName == "CC_TAG_DOMAIN") return CC_TAG_DOMAIN;
    else if (ccName == "CC_TAG_COMMENT") return CC_TAG_COMMENT;
    else if (ccName == "CC_TAG_NEWS") return CC_TAG_NEWS;
    else if (ccName == "CC_TAG_IMAGE") return CC_TAG_IMAGE;
    else if (ccName == "CC_TAG_AUDIO") return CC_TAG_AUDIO;
    else if (ccName == "CC_TAG_VIDEO") return CC_TAG_VIDEO;
    else if (ccName == "CC_TAG_MESSAGE") return CC_TAG_MESSAGE;
    else if (ccName == "CC_TAG_ADVERTISMENT") return CC_TAG_ADVERTISMENT;
    else if (ccName == "CC_TAG_STREAM") return CC_TAG_STREAM;
    else if (ccName == "CC_TAG_CONTENT") return CC_TAG_CONTENT;
    else if (ccName == "CC_TAG_COIN") return CC_TAG_COIN;
    else if (ccName == "CC_TAG_ID") return CC_TAG_ID;
        // Text
    else if (ccName == "CC_TEXT_ENCODINGSTRING") return CC_TEXT_ENCODINGSTRING;
    else if (ccName == "CC_TEXT_ENCODING_UTF8") return CC_TEXT_ENCODING_UTF8;
    else if (ccName == "CC_TEXT_ENCODING_ASCII") return CC_TEXT_ENCODING_ASCII;
    else if (ccName == "CC_TEXT_ENCODING_UTF16") return CC_TEXT_ENCODING_UTF16;
    else if (ccName == "CC_TEXT_ENCODING_UTF32") return CC_TEXT_ENCODING_UTF32;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_1") return CC_TEXT_ENCODING_ISO8859_1;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_2") return CC_TEXT_ENCODING_ISO8859_2;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_3") return CC_TEXT_ENCODING_ISO8859_3;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_4") return CC_TEXT_ENCODING_ISO8859_4;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_5") return CC_TEXT_ENCODING_ISO8859_5;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_6") return CC_TEXT_ENCODING_ISO8859_6;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_7") return CC_TEXT_ENCODING_ISO8859_7;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_8") return CC_TEXT_ENCODING_ISO8859_8;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_9") return CC_TEXT_ENCODING_ISO8859_9;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_10") return CC_TEXT_ENCODING_ISO8859_10;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_11") return CC_TEXT_ENCODING_ISO8859_11;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_12") return CC_TEXT_ENCODING_ISO8859_12;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_13") return CC_TEXT_ENCODING_ISO8859_13;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_14") return CC_TEXT_ENCODING_ISO8859_14;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_15") return CC_TEXT_ENCODING_ISO8859_15;
    else if (ccName == "CC_TEXT_ENCODING_ISO8859_16") return CC_TEXT_ENCODING_ISO8859_16;
    else if (ccName == "CC_TEXT_ENCODING_GB2312") return CC_TEXT_ENCODING_GB2312;
    else if (ccName == "CC_TEXT_ENCODING_GBK") return CC_TEXT_ENCODING_GBK;
    else if (ccName == "CC_TEXT_ENCODING_GB18030") return CC_TEXT_ENCODING_GB18030;
    else if (ccName == "CC_TEXT_ENCODING_BIG5") return CC_TEXT_ENCODING_BIG5;
    else if (ccName == "CC_TEXT_ENCODING_HKSCS") return CC_TEXT_ENCODING_HKSCS;
    else if (ccName == "CC_TEXT_ENCODING_SHIFTJIS") return CC_TEXT_ENCODING_SHIFTJIS;
    else if (ccName == "CC_TEXT_ENCODING_EUCJP") return CC_TEXT_ENCODING_EUCJP;
    else if (ccName == "CC_TEXT_ENCODING_ISO2022JP") return CC_TEXT_ENCODING_ISO2022JP;
    else if (ccName == "CC_TEXT_ENCODING_EUCJIS2004") return CC_TEXT_ENCODING_EUCJIS2004;
    else if (ccName == "CC_TEXT_ENCODING_ISO2022JP2004") return CC_TEXT_ENCODING_ISO2022JP2004;
    else if (ccName == "CC_TEXT_ENCODING_KSX1001") return CC_TEXT_ENCODING_KSX1001;
    else if (ccName == "CC_TEXT_ENCODING_EUCKR") return CC_TEXT_ENCODING_EUCKR;
    else if (ccName == "CC_TEXT_ENCODING_ISO2022KR") return CC_TEXT_ENCODING_ISO2022KR;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1250") return CC_TEXT_ENCODING_WINDOWS1250;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1251") return CC_TEXT_ENCODING_WINDOWS1251;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1252") return CC_TEXT_ENCODING_WINDOWS1252;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1253") return CC_TEXT_ENCODING_WINDOWS1253;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1254") return CC_TEXT_ENCODING_WINDOWS1254;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1255") return CC_TEXT_ENCODING_WINDOWS1255;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1256") return CC_TEXT_ENCODING_WINDOWS1256;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1257") return CC_TEXT_ENCODING_WINDOWS1257;
    else if (ccName == "CC_TEXT_ENCODING_WINDOWS1258") return CC_TEXT_ENCODING_WINDOWS1258;
    else if (ccName == "CC_TEXT_TYPESTRING") return CC_TEXT_TYPESTRING;
    else if (ccName == "CC_TEXT_TYPE_TEXT") return CC_TEXT_TYPE_TEXT;
    else if (ccName == "CC_TEXT_TYPE_HTML") return CC_TEXT_TYPE_HTML;
    else if (ccName == "CC_TEXT_TYPE_XML") return CC_TEXT_TYPE_XML;
    else if (ccName == "CC_TEXT_TYPE_JSON") return CC_TEXT_TYPE_JSON;
    else if (ccName == "CC_TEXT_TYPE_YAML") return CC_TEXT_TYPE_YAML;
    else if (ccName == "CC_TEXT_TYPE_CSV") return CC_TEXT_TYPE_CSV;
        // File
    else if (ccName == "CC_FILE_CONTENT") return CC_FILE_CONTENT;
    else if (ccName == "CC_FILE_CONTENT_P") return CC_FILE_CONTENT_P;
    else if (ccName == "CC_FILE_NAME") return CC_FILE_NAME;
    else if (ccName == "CC_FILE_NAME_P") return CC_FILE_NAME_P;
    else if (ccName == "CC_FILE_TYPESTRING") return CC_FILE_TYPESTRING;
        // Link
    else if (ccName == "CC_LINK_TYPESTRING") return CC_LINK_TYPESTRING;
    else if (ccName == "CC_LINK_TYPE_TXIDOUT") return CC_LINK_TYPE_TXIDOUT;
    else if (ccName == "CC_LINK_TYPE_TXID") return CC_LINK_TYPE_TXID;
    else if (ccName == "CC_LINK_TYPE_COINTO") return CC_LINK_TYPE_COINTO;
    else if (ccName == "CC_LINK_TYPE_HTTP") return CC_LINK_TYPE_HTTP;
    else if (ccName == "CC_LINK_TYPE_HTTPS") return CC_LINK_TYPE_HTTPS;
    else if (ccName == "CC_LINK_TYPE_MAILTO") return CC_LINK_TYPE_MAILTO;
    else if (ccName == "CC_LINK_TYPE_FTP") return CC_LINK_TYPE_FTP;
    else if (ccName == "CC_LINK_TYPE_FILE") return CC_LINK_TYPE_FILE;
    else if (ccName == "CC_LINK_TYPE_CRID") return CC_LINK_TYPE_CRID;
    else if (ccName == "CC_LINK_TYPE_ED2K") return CC_LINK_TYPE_ED2K;
    else if (ccName == "CC_LINK_TYPE_MAGNET") return CC_LINK_TYPE_MAGNET;
        // Domain
    else if (ccName == "CC_DOMAIN_REG") return CC_DOMAIN_REG;
    else if (ccName == "CC_DOMAIN_REG_P") return CC_DOMAIN_REG_P;
    else if (ccName == "CC_DOMAIN_FORWARD") return CC_DOMAIN_FORWARD;
    else if (ccName == "CC_DOMAIN_FORWARD_P") return CC_DOMAIN_FORWARD_P;
    else if (ccName == "CC_DOMAIN_TRANSFER") return CC_DOMAIN_TRANSFER;
    else if (ccName == "CC_DOMAIN_TRANSFER_P") return CC_DOMAIN_TRANSFER_P;
        // Comment
    else if (ccName == "CC_COMMENT_CONTENT") return CC_COMMENT_CONTENT;
    else if (ccName == "CC_COMMENT_CONTENT_P") return CC_COMMENT_CONTENT_P;
    else if (ccName == "CC_COMMENT_LIKE") return CC_COMMENT_LIKE;
    else if (ccName == "CC_COMMENT_LIKE_P") return CC_COMMENT_LIKE_P;
    else if (ccName == "CC_COMMENT_DISLIKE") return CC_COMMENT_DISLIKE;
    else if (ccName == "CC_COMMENT_DISLIKE_P") return CC_COMMENT_DISLIKE_P;
    else if (ccName == "CC_COMMENT_SCORESTRING") return CC_COMMENT_SCORESTRING;
    else if (ccName == "CC_COMMENT_SCORE_0") return CC_COMMENT_SCORE_0;
    else if (ccName == "CC_COMMENT_SCORE_1") return CC_COMMENT_SCORE_1;
    else if (ccName == "CC_COMMENT_SCORE_2") return CC_COMMENT_SCORE_2;
    else if (ccName == "CC_COMMENT_SCORE_3") return CC_COMMENT_SCORE_3;
    else if (ccName == "CC_COMMENT_SCORE_4") return CC_COMMENT_SCORE_4;
    else if (ccName == "CC_COMMENT_SCORE_5") return CC_COMMENT_SCORE_5;
    else if (ccName == "CC_COMMENT_SCORE_6") return CC_COMMENT_SCORE_6;
    else if (ccName == "CC_COMMENT_SCORE_7") return CC_COMMENT_SCORE_7;
    else if (ccName == "CC_COMMENT_SCORE_8") return CC_COMMENT_SCORE_8;
    else if (ccName == "CC_COMMENT_SCORE_9") return CC_COMMENT_SCORE_9;
    else if (ccName == "CC_COMMENT_SCORE_10") return CC_COMMENT_SCORE_10;
    else if (ccName == "CC_COMMENT_SCORE_P") return CC_COMMENT_SCORE_P;
        // Payment
    else if (ccName == "CC_PAYMENT_REQ") return CC_PAYMENT_REQ;
    else if (ccName == "CC_PAYMENT_REQ_P") return CC_PAYMENT_REQ_P;
    else if (ccName == "CC_PAYMENT_PRODID") return CC_PAYMENT_PRODID;
    else if (ccName == "CC_PAYMENT_PRODID_P") return CC_PAYMENT_PRODID_P;
    else if (ccName == "CC_PAYMENT_QUANTITY") return CC_PAYMENT_QUANTITY;
    else if (ccName == "CC_PAYMENT_UNITPRICE") return CC_PAYMENT_UNITPRICE;
    else if (ccName == "CC_PAYMENT_DELIVERADD") return CC_PAYMENT_DELIVERADD;
    else if (ccName == "CC_PAYMENT_CONTACTNUM") return CC_PAYMENT_CONTACTNUM;
    else if (ccName == "CC_PAYMENT_CONTACTEMAIL") return CC_PAYMENT_CONTACTEMAIL;
    else if (ccName == "CC_PAYMENT_REQ_ADDR") return CC_PAYMENT_REQ_ADDR;
    else if (ccName == "CC_PAYMENT_REQ_AMOUNT") return CC_PAYMENT_REQ_AMOUNT;
    else if (ccName == "CC_PAYMENT_REQ_VALIDTILL") return CC_PAYMENT_REQ_VALIDTILL;
    else if (ccName == "CC_PAYMENT_REQ_VALIDTIME") return CC_PAYMENT_REQ_VALIDTIME;
        // Lang

    else if (ccName == "CC_LANG_IETF") return CC_LANG_IETF;
    else if (ccName == "CC_LANG_ISO639_1") return CC_LANG_ISO639_1;
    else if (ccName == "CC_LANG_ISO639_2") return CC_LANG_ISO639_2;
    else if (ccName == "CC_LANG_ISO639_3") return CC_LANG_ISO639_3;
    else if (ccName == "CC_LANG_COUNTRY") return CC_LANG_COUNTRY;

    else if (ccName == "CC_LANG_AB") return CC_LANG_AB;
    else if (ccName == "CC_LANG_AA") return CC_LANG_AA;
    else if (ccName == "CC_LANG_AF") return CC_LANG_AF;
    else if (ccName == "CC_LANG_AK") return CC_LANG_AK;
    else if (ccName == "CC_LANG_SQ") return CC_LANG_SQ;
    else if (ccName == "CC_LANG_AM") return CC_LANG_AM;
    else if (ccName == "CC_LANG_AR") return CC_LANG_AR;
    else if (ccName == "CC_LANG_AN") return CC_LANG_AN;
    else if (ccName == "CC_LANG_HY") return CC_LANG_HY;
    else if (ccName == "CC_LANG_AS") return CC_LANG_AS;
    else if (ccName == "CC_LANG_AV") return CC_LANG_AV;
    else if (ccName == "CC_LANG_AE") return CC_LANG_AE;
    else if (ccName == "CC_LANG_AY") return CC_LANG_AY;
    else if (ccName == "CC_LANG_AZ") return CC_LANG_AZ;
    else if (ccName == "CC_LANG_BM") return CC_LANG_BM;
    else if (ccName == "CC_LANG_BA") return CC_LANG_BA;
    else if (ccName == "CC_LANG_EU") return CC_LANG_EU;
    else if (ccName == "CC_LANG_BE") return CC_LANG_BE;
    else if (ccName == "CC_LANG_BN") return CC_LANG_BN;
    else if (ccName == "CC_LANG_BH") return CC_LANG_BH;
    else if (ccName == "CC_LANG_BI") return CC_LANG_BI;
    else if (ccName == "CC_LANG_BS") return CC_LANG_BS;
    else if (ccName == "CC_LANG_BR") return CC_LANG_BR;
    else if (ccName == "CC_LANG_BG") return CC_LANG_BG;
    else if (ccName == "CC_LANG_MY") return CC_LANG_MY;
    else if (ccName == "CC_LANG_CA") return CC_LANG_CA;
    else if (ccName == "CC_LANG_CH") return CC_LANG_CH;
    else if (ccName == "CC_LANG_CE") return CC_LANG_CE;
    else if (ccName == "CC_LANG_NY") return CC_LANG_NY;
    else if (ccName == "CC_LANG_ZH") return CC_LANG_ZH;
    else if (ccName == "CC_LANG_CV") return CC_LANG_CV;
    else if (ccName == "CC_LANG_KW") return CC_LANG_KW;
    else if (ccName == "CC_LANG_CO") return CC_LANG_CO;
    else if (ccName == "CC_LANG_CR") return CC_LANG_CR;
    else if (ccName == "CC_LANG_HR") return CC_LANG_HR;
    else if (ccName == "CC_LANG_CS") return CC_LANG_CS;
    else if (ccName == "CC_LANG_DA") return CC_LANG_DA;
    else if (ccName == "CC_LANG_DV") return CC_LANG_DV;
    else if (ccName == "CC_LANG_NL") return CC_LANG_NL;
    else if (ccName == "CC_LANG_DZ") return CC_LANG_DZ;
    else if (ccName == "CC_LANG_EN") return CC_LANG_EN;
    else if (ccName == "CC_LANG_EO") return CC_LANG_EO;
    else if (ccName == "CC_LANG_ET") return CC_LANG_ET;
    else if (ccName == "CC_LANG_EE") return CC_LANG_EE;
    else if (ccName == "CC_LANG_FO") return CC_LANG_FO;
    else if (ccName == "CC_LANG_FJ") return CC_LANG_FJ;
    else if (ccName == "CC_LANG_FI") return CC_LANG_FI;
    else if (ccName == "CC_LANG_FR") return CC_LANG_FR;
    else if (ccName == "CC_LANG_FF") return CC_LANG_FF;
    else if (ccName == "CC_LANG_GL") return CC_LANG_GL;
    else if (ccName == "CC_LANG_KA") return CC_LANG_KA;
    else if (ccName == "CC_LANG_DE") return CC_LANG_DE;
    else if (ccName == "CC_LANG_EL") return CC_LANG_EL;
    else if (ccName == "CC_LANG_GN") return CC_LANG_GN;
    else if (ccName == "CC_LANG_GU") return CC_LANG_GU;
    else if (ccName == "CC_LANG_HT") return CC_LANG_HT;
    else if (ccName == "CC_LANG_HA") return CC_LANG_HA;
    else if (ccName == "CC_LANG_HE") return CC_LANG_HE;
    else if (ccName == "CC_LANG_HZ") return CC_LANG_HZ;
    else if (ccName == "CC_LANG_HI") return CC_LANG_HI;
    else if (ccName == "CC_LANG_HO") return CC_LANG_HO;
    else if (ccName == "CC_LANG_HU") return CC_LANG_HU;
    else if (ccName == "CC_LANG_IA") return CC_LANG_IA;
    else if (ccName == "CC_LANG_ID") return CC_LANG_ID;
    else if (ccName == "CC_LANG_IE") return CC_LANG_IE;
    else if (ccName == "CC_LANG_GA") return CC_LANG_GA;
    else if (ccName == "CC_LANG_IG") return CC_LANG_IG;
    else if (ccName == "CC_LANG_IK") return CC_LANG_IK;
    else if (ccName == "CC_LANG_IO") return CC_LANG_IO;
    else if (ccName == "CC_LANG_IS") return CC_LANG_IS;
    else if (ccName == "CC_LANG_IT") return CC_LANG_IT;
    else if (ccName == "CC_LANG_IU") return CC_LANG_IU;
    else if (ccName == "CC_LANG_JA") return CC_LANG_JA;
    else if (ccName == "CC_LANG_JV") return CC_LANG_JV;
    else if (ccName == "CC_LANG_KL") return CC_LANG_KL;
    else if (ccName == "CC_LANG_KN") return CC_LANG_KN;
    else if (ccName == "CC_LANG_KR") return CC_LANG_KR;
    else if (ccName == "CC_LANG_KS") return CC_LANG_KS;
    else if (ccName == "CC_LANG_KK") return CC_LANG_KK;
    else if (ccName == "CC_LANG_KM") return CC_LANG_KM;
    else if (ccName == "CC_LANG_KI") return CC_LANG_KI;
    else if (ccName == "CC_LANG_RW") return CC_LANG_RW;
    else if (ccName == "CC_LANG_KY") return CC_LANG_KY;
    else if (ccName == "CC_LANG_KV") return CC_LANG_KV;
    else if (ccName == "CC_LANG_KG") return CC_LANG_KG;
    else if (ccName == "CC_LANG_KO") return CC_LANG_KO;
    else if (ccName == "CC_LANG_KU") return CC_LANG_KU;
    else if (ccName == "CC_LANG_KJ") return CC_LANG_KJ;
    else if (ccName == "CC_LANG_LA") return CC_LANG_LA;
    else if (ccName == "CC_LANG_LB") return CC_LANG_LB;
    else if (ccName == "CC_LANG_LG") return CC_LANG_LG;
    else if (ccName == "CC_LANG_LI") return CC_LANG_LI;
    else if (ccName == "CC_LANG_LN") return CC_LANG_LN;
    else if (ccName == "CC_LANG_LO") return CC_LANG_LO;
    else if (ccName == "CC_LANG_LT") return CC_LANG_LT;
    else if (ccName == "CC_LANG_LU") return CC_LANG_LU;
    else if (ccName == "CC_LANG_LV") return CC_LANG_LV;
    else if (ccName == "CC_LANG_GV") return CC_LANG_GV;
    else if (ccName == "CC_LANG_MK") return CC_LANG_MK;
    else if (ccName == "CC_LANG_MG") return CC_LANG_MG;
    else if (ccName == "CC_LANG_MS") return CC_LANG_MS;
    else if (ccName == "CC_LANG_ML") return CC_LANG_ML;
    else if (ccName == "CC_LANG_MT") return CC_LANG_MT;
    else if (ccName == "CC_LANG_MI") return CC_LANG_MI;
    else if (ccName == "CC_LANG_MR") return CC_LANG_MR;
    else if (ccName == "CC_LANG_MH") return CC_LANG_MH;
    else if (ccName == "CC_LANG_MN") return CC_LANG_MN;
    else if (ccName == "CC_LANG_NA") return CC_LANG_NA;
    else if (ccName == "CC_LANG_NV") return CC_LANG_NV;
    else if (ccName == "CC_LANG_ND") return CC_LANG_ND;
    else if (ccName == "CC_LANG_NE") return CC_LANG_NE;
    else if (ccName == "CC_LANG_NG") return CC_LANG_NG;
    else if (ccName == "CC_LANG_NB") return CC_LANG_NB;
    else if (ccName == "CC_LANG_NN") return CC_LANG_NN;
    else if (ccName == "CC_LANG_NO") return CC_LANG_NO;
    else if (ccName == "CC_LANG_II") return CC_LANG_II;
    else if (ccName == "CC_LANG_NR") return CC_LANG_NR;
    else if (ccName == "CC_LANG_OC") return CC_LANG_OC;
    else if (ccName == "CC_LANG_OJ") return CC_LANG_OJ;
    else if (ccName == "CC_LANG_CU") return CC_LANG_CU;
    else if (ccName == "CC_LANG_OM") return CC_LANG_OM;
    else if (ccName == "CC_LANG_OR") return CC_LANG_OR;
    else if (ccName == "CC_LANG_OS") return CC_LANG_OS;
    else if (ccName == "CC_LANG_PA") return CC_LANG_PA;
    else if (ccName == "CC_LANG_PI") return CC_LANG_PI;
    else if (ccName == "CC_LANG_FA") return CC_LANG_FA;
    else if (ccName == "CC_LANG_PL") return CC_LANG_PL;
    else if (ccName == "CC_LANG_PS") return CC_LANG_PS;
    else if (ccName == "CC_LANG_PT") return CC_LANG_PT;
    else if (ccName == "CC_LANG_QU") return CC_LANG_QU;
    else if (ccName == "CC_LANG_RM") return CC_LANG_RM;
    else if (ccName == "CC_LANG_RN") return CC_LANG_RN;
    else if (ccName == "CC_LANG_RO") return CC_LANG_RO;
    else if (ccName == "CC_LANG_RU") return CC_LANG_RU;
    else if (ccName == "CC_LANG_SA") return CC_LANG_SA;
    else if (ccName == "CC_LANG_SC") return CC_LANG_SC;
    else if (ccName == "CC_LANG_SD") return CC_LANG_SD;
    else if (ccName == "CC_LANG_SE") return CC_LANG_SE;
    else if (ccName == "CC_LANG_SM") return CC_LANG_SM;
    else if (ccName == "CC_LANG_SG") return CC_LANG_SG;
    else if (ccName == "CC_LANG_SR") return CC_LANG_SR;
    else if (ccName == "CC_LANG_GD") return CC_LANG_GD;
    else if (ccName == "CC_LANG_SN") return CC_LANG_SN;
    else if (ccName == "CC_LANG_SI") return CC_LANG_SI;
    else if (ccName == "CC_LANG_SK") return CC_LANG_SK;
    else if (ccName == "CC_LANG_SL") return CC_LANG_SL;
    else if (ccName == "CC_LANG_SO") return CC_LANG_SO;
    else if (ccName == "CC_LANG_ST") return CC_LANG_ST;
    else if (ccName == "CC_LANG_ES") return CC_LANG_ES;
    else if (ccName == "CC_LANG_SU") return CC_LANG_SU;
    else if (ccName == "CC_LANG_SW") return CC_LANG_SW;
    else if (ccName == "CC_LANG_SS") return CC_LANG_SS;
    else if (ccName == "CC_LANG_SV") return CC_LANG_SV;
    else if (ccName == "CC_LANG_TA") return CC_LANG_TA;
    else if (ccName == "CC_LANG_TE") return CC_LANG_TE;
    else if (ccName == "CC_LANG_TG") return CC_LANG_TG;
    else if (ccName == "CC_LANG_TH") return CC_LANG_TH;
    else if (ccName == "CC_LANG_TI") return CC_LANG_TI;
    else if (ccName == "CC_LANG_BO") return CC_LANG_BO;
    else if (ccName == "CC_LANG_TK") return CC_LANG_TK;
    else if (ccName == "CC_LANG_TL") return CC_LANG_TL;
    else if (ccName == "CC_LANG_TN") return CC_LANG_TN;
    else if (ccName == "CC_LANG_TO") return CC_LANG_TO;
    else if (ccName == "CC_LANG_TR") return CC_LANG_TR;
    else if (ccName == "CC_LANG_TS") return CC_LANG_TS;
    else if (ccName == "CC_LANG_TT") return CC_LANG_TT;
    else if (ccName == "CC_LANG_TW") return CC_LANG_TW;
    else if (ccName == "CC_LANG_TY") return CC_LANG_TY;
    else if (ccName == "CC_LANG_UG") return CC_LANG_UG;
    else if (ccName == "CC_LANG_UK") return CC_LANG_UK;
    else if (ccName == "CC_LANG_UR") return CC_LANG_UR;
    else if (ccName == "CC_LANG_UZ") return CC_LANG_UZ;
    else if (ccName == "CC_LANG_VE") return CC_LANG_VE;
    else if (ccName == "CC_LANG_VI") return CC_LANG_VI;
    else if (ccName == "CC_LANG_VO") return CC_LANG_VO;
    else if (ccName == "CC_LANG_WA") return CC_LANG_WA;
    else if (ccName == "CC_LANG_CY") return CC_LANG_CY;
    else if (ccName == "CC_LANG_WO") return CC_LANG_WO;
    else if (ccName == "CC_LANG_FY") return CC_LANG_FY;
    else if (ccName == "CC_LANG_XH") return CC_LANG_XH;
    else if (ccName == "CC_LANG_YI") return CC_LANG_YI;
    else if (ccName == "CC_LANG_YO") return CC_LANG_YO;
    else if (ccName == "CC_LANG_ZA") return CC_LANG_ZA;
    else if (ccName == "CC_LANG_ZU") return CC_LANG_ZU;

    else return CC_NULL;
}

std::string GetCcHex(const cctype cc)
{
    std::ostringstream stm;
    stm << (char) cc;
    return HexStr(stm.str());
}

bool CContent::IsStandard()
{
    iterator pc = begin();
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            return false;
    }
    return (pc > end()) ? false : true;
}

Array CContent::ToJson(stringformat fFormat)
{
    iterator pc = begin();
    Array result;
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        Object ccUnit;
        std::string ccName;
        ccName = GetCcName(cc);
        ccUnit.push_back(Pair("cc_name", ccName));
        ccUnit.push_back(Pair("cc", GetCcHex(cc)));
        if (IsCcParent(cc)) {
            if (contentStr.IsStandard())
                ccUnit.push_back(Pair("content", contentStr.ToJson(fFormat)));
            else
                ccUnit.push_back(Pair("content", "non-standard"));
        } else {
            switch (fFormat) {
                case STR_FORMAT_BIN:
                case STR_FORMAT_BIN_SUM:
                    if (fFormat == STR_FORMAT_BIN_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", contentStr));
                    break;
                case STR_FORMAT_HEX:
                case STR_FORMAT_HEX_SUM:
                    if (fFormat == STR_FORMAT_HEX_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", HexStr(contentStr)));
                    break;
                case STR_FORMAT_B64:
                case STR_FORMAT_B64_SUM:
                    if (fFormat == STR_FORMAT_B64_SUM && contentStr.size() > STR_FORMAT_SUM_MAXSIZE)
                        ccUnit.push_back(Pair("length", contentStr.size()));
                    else
                        ccUnit.push_back(Pair("content", EncodeBase64(contentStr)));
                    break;

            }

        }
        result.push_back(ccUnit);
    }
    if (pc > end()) {
        result.clear();
        Object rObj;
        rObj.push_back(Pair("content", "non-standard"));
        result.push_back(rObj);
    }
    return result;
}

std::string CContent::TrimToHumanString(const std::string& str)
{
    std::string lenStr = " ... (";
    lenStr += strpatch::to_string(str.size());
    lenStr += " bytes) ";
    std::string str2;
    if (IsStringPrint(str)) {
        str2 = str.size() > TRIM_READABLE_LEN ? str.substr(0, TRIM_READABLE_LEN) + lenStr : str;
    } else {
        str2 = str.size() > TRIM_BINARY_LEN ? HexStr(str.substr(0, TRIM_BINARY_LEN)) + lenStr : HexStr(str);
    }
    trim(str2);
    return str2;
}

std::string CContent::ToHumanString()
{
    std::string ccUnit;
    iterator pc = begin();
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        std::string ccName;
        ccName = GetCcName(cc);
        ccUnit += ccName;
        if (IsCcParent(cc)) {
            if (contentStr.IsStandard())
                ccUnit += " " + contentStr.ToHumanString() + " ";
            else
                ccUnit = strpatch::to_string(size()) + " bytes binary";
        } else {
            ccUnit += " " + TrimToHumanString(contentStr) + " ";
        }
    }
    if (pc > end()) {
        return strpatch::to_string(size()) + " bytes binary";
    }
    trim(ccUnit);
    return ccUnit;
}

bool CContent::HasCc(const cctype& ccIn) // Very costly !!! Try to use FirstCc()
{
    iterator pc = begin();
    bool r = false;
    while (pc < end()) {
        cctype cc;
        CContent contentStr;
        if (!GetCcUnit(pc, cc, contentStr))
            break;
        if (cc == ccIn)
            return true;
        if (IsCcParent(cc)) {
            r = contentStr.HasCc(ccIn);
        }
    }
    if (pc > end())
        return 0;
    return r;
}

bool CContent::FirstCc(const cctype& ccIn)
{
    iterator pc = begin();
    cctype cc = (cctype) ReadVarInt(pc);
    if (cc != ccIn)
        return false;
    if (!IsStandard())
        return false;
    return true;

}

bool CContent::SetEmpty()
{
    clear();
    return true;
}

bool CContent::SetJson(const Array& cttJson)
{
    cctype cc = CC_NULL;

    BOOST_FOREACH(const Value& input, cttJson)
    {
        const Object& cttObj = input.get_obj();

        BOOST_FOREACH(const Pair& ccUnit, cttObj)
        {
            std::string ccName;
            CContent content;
            if (ccUnit.name_ == "cc_name") {
                ccName = ccUnit.value_.get_str();
                cc = GetCcValue(ccName);
                continue;
            } else if (ccUnit.name_ == "content") {
                IsCcParent(cc) ? content.SetJson(ccUnit.value_.get_array()) : content.SetString(ccUnit.value_.get_str());
                WriteVarInt(cc);
                WriteCompactSize(content.size());
                append(content);
            }
        }
    }
    return true;
}

bool CContent::SetString(const std::string& cttStr)
{
    clear();
    append(cttStr);
    return true;
}

bool CContent::SetString(const vector<unsigned char>& cttVch)
{
    std::string str(cttVch.begin(), cttVch.end());
    return SetString(str);
}

bool CContent::GetCcUnit(iterator& pc, cctype& ccRet, std::string& content)
{
    ccRet = CC_NULL;
    if (pc >= end())
        return false;
    ccRet = (cctype) ReadVarInt(pc);
    int len = ReadCompactSize(pc);
    if (len > 0 && len <= end() - pc)
        content = ReadData(pc, len);
    else if (len == 0)
        content = "";
    else
        return false;
    return pc <= end() ? true : false;
}

bool CContent::WriteVarInt(u_int64_t n)
{
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
        WriteData(tstr, 1);
    } while (len--);
    return true;
}

u_int64_t CContent::ReadVarInt(iterator& pc)
{
    u_int64_t n = 0;
    while (true) {
        unsigned char chData;
        chData = *pc++;
        n = (n << 7) | (chData & 0x7F);
        if (chData & 0x80)
            n++;
        else
            return n;
    }
}

bool CContent::WriteCompactSize(u_int64_t n)
{
    std::ostringstream os;
    if (n < 253) {
        os << (char) n;
        WriteData(os.str());
    } else if (n <= std::numeric_limits<unsigned short>::max()) {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += n >> 8;
        os << (char) 253 << tmp;
        WriteData(os.str());
    } else if (n <= std::numeric_limits<unsigned int>::max()) {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        os << (char) 254 << tmp;
        WriteData(os.str());
    } else {
        std::string tmp;
        tmp += n & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        tmp += (n >> 8) & 0xFF;
        os << (char) 255 << tmp;
        WriteData(os.str());
    }
    return true;
}

u_int64_t CContent::ReadCompactSize(iterator& pc)
{
    unsigned char chSize;
    std::string chData;
    chSize = *pc++;
    u_int64_t nSizeRet = 0;
    if (chSize < 253) {
        nSizeRet = chSize;
    } else if (chSize == 253) {
        chData = ReadDataReverse(pc, 2);
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 253) {
            throw std::ios_base::failure("non-canonical ReadCompactSize()" + nSizeRet);
        }
    } else if (chSize == 254) {
        chData = ReadDataReverse(pc, 4);
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 0x10000u)
            throw std::ios_base::failure("non-canonical ReadCompactSize()");
    } else {
        chData = ReadDataReverse(pc, 8);
        nSizeRet = strtoull(HexStr(chData).data(), NULL, 16);
        if (nSizeRet < 0x100000000ULL)
            throw std::ios_base::failure("non-canonical ReadCompactSize()");
    }
    return nSizeRet;
}

bool CContent::WriteData(const std::string str)
{
    append(str);
    return true;
}

bool CContent::WriteData(const std::string str, int len)
{
    append(str, 0, len);
    return true;
}

std::string CContent::ReadData(iterator& pc, int len)
{
    std::string result;
    int i = 0;
    while (i < len) {
        result += *pc++;
        i++;
    }
    return result;
}

std::string CContent::ReadDataReverse(iterator& pc, int len)
{
    std::string result;
    int i = len;
    iterator pc2 = pc + len;
    while (i > 0) {
        result += *--pc2;
        i--;
        pc++;
    }
    return result;
}

bool CContent::IsCcParent(const cctype& cc)
{
    u_int64_t cc2 = cc;
    return (cc2 % 2 == 1) ? true : false;
}