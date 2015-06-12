#ifndef CCC_CONTENT_H
#define CCC_CONTENT_H

#include <string.h>
#include <string>
#include <vector>
#include "utiltime.h"
#include "serialize.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "uint256.h"
#include "script/script.h"
using namespace json_spirit;
using namespace std;
using std::string;

enum stringformat
{
    STR_FORMAT_BIN = 0,
    STR_FORMAT_HEX = 1,
    STR_FORMAT_B64 = 2,
    STR_FORMAT_BIN_SUM = 3,
    STR_FORMAT_HEX_SUM = 4,
    STR_FORMAT_B64_SUM = 5,
};
enum directionFilter
{
    BI_DIRECTION =0,
    INCOMING_ONLY =1,
    OUTPUT_ONLY =2,    
};
/** Content codes */
enum cctype
{
    /** Null * */
    CC_NULL = 0x00,

    /** Tag * */
    CC_TAG = 0x02,
    CC_TAG_P = 0x03,

    /** First Level Content Code */
    CC_TEXT = 0x04,
    CC_TEXT_P = 0x05,
    CC_FILE = 0x06,
    CC_FILE_P = 0x07,
    CC_LINK = 0x08,
    CC_LINK_P = 0x09,
    CC_DOMAIN = 0x0a,
    CC_DOMAIN_P = 0x0b,
    CC_COMMENT = 0x0c,
    CC_COMMENT_P = 0x0d,
    CC_DELETE = 0x0e,
    CC_DELETE_P = 0x0f,
    CC_PAYMENT = 0x10,
    CC_PAYMENT_P = 0x11,
    CC_LANG = 0x12,
    CC_LANG_P = 0x13,
    CC_ENCRYPT = 0x14,
    CC_ENCRYPT_P = 0x15,
    /** Second Level Content Code * */
    // Tag
    CC_TAG_TEXT = 0x031004,
    CC_TAG_FILE = 0x031006,
    CC_TAG_LINK = 0x031008,
    CC_TAG_DOMAIN = 0x03100a,
    CC_TAG_COMMENT = 0x03100c,
    CC_TAG_NEWS = 0x03100e,
    CC_TAG_IMAGE = 0x031010,
    CC_TAG_AUDIO = 0x031012,
    CC_TAG_VIDEO = 0x031014,
    CC_TAG_MESSAGE = 0x031016,
    CC_TAG_ADVERTISMENT = 0x031018,
    CC_TAG_STREAM = 0x03101a,
    CC_TAG_CONTENT = 0x03101c,
    CC_TAG_COIN = 0x03101e,
    CC_TAG_ID = 0x031020,
    // Text
    CC_TEXT_ENCODINGSTRING = 0x0500,
    CC_TEXT_ENCODING_UTF8 = 0x0502,
    CC_TEXT_ENCODING_ASCII = 0x0504,
    CC_TEXT_ENCODING_UTF16 = 0x0506,
    CC_TEXT_ENCODING_UTF32 = 0x0508,
    CC_TEXT_ENCODING_ISO8859_1 = 0x050502,
    CC_TEXT_ENCODING_ISO8859_2 = 0x050504,
    CC_TEXT_ENCODING_ISO8859_3 = 0x050506,
    CC_TEXT_ENCODING_ISO8859_4 = 0x050508,
    CC_TEXT_ENCODING_ISO8859_5 = 0x05050a,
    CC_TEXT_ENCODING_ISO8859_6 = 0x05050c,
    CC_TEXT_ENCODING_ISO8859_7 = 0x05050e,
    CC_TEXT_ENCODING_ISO8859_8 = 0x050510,
    CC_TEXT_ENCODING_ISO8859_9 = 0x050512,
    CC_TEXT_ENCODING_ISO8859_10 = 0x050514,
    CC_TEXT_ENCODING_ISO8859_11 = 0x050516,
    CC_TEXT_ENCODING_ISO8859_12 = 0x050518,
    CC_TEXT_ENCODING_ISO8859_13 = 0x05051a,
    CC_TEXT_ENCODING_ISO8859_14 = 0x05051c,
    CC_TEXT_ENCODING_ISO8859_15 = 0x05051e,
    CC_TEXT_ENCODING_ISO8859_16 = 0x050520,
    CC_TEXT_ENCODING_GB2312 = 0x050522,
    CC_TEXT_ENCODING_GBK = 0x050524,
    CC_TEXT_ENCODING_GB18030 = 0x050526,
    CC_TEXT_ENCODING_BIG5 = 0x050528,
    CC_TEXT_ENCODING_HKSCS = 0x05052a,
    CC_TEXT_ENCODING_SHIFTJIS = 0x05052c,
    CC_TEXT_ENCODING_EUCJP = 0x05052e,
    CC_TEXT_ENCODING_ISO2022JP = 0x050530,
    CC_TEXT_ENCODING_EUCJIS2004 = 0x050532,
    CC_TEXT_ENCODING_ISO2022JP2004 = 0x050534,
    CC_TEXT_ENCODING_KSX1001 = 0x050536,
    CC_TEXT_ENCODING_EUCKR = 0x050538,
    CC_TEXT_ENCODING_ISO2022KR = 0x05053a,
    CC_TEXT_ENCODING_WINDOWS1250 = 0x05053c,
    CC_TEXT_ENCODING_WINDOWS1251 = 0x05053e,
    CC_TEXT_ENCODING_WINDOWS1252 = 0x050540,
    CC_TEXT_ENCODING_WINDOWS1253 = 0x050542,
    CC_TEXT_ENCODING_WINDOWS1254 = 0x050544,
    CC_TEXT_ENCODING_WINDOWS1255 = 0x050546,
    CC_TEXT_ENCODING_WINDOWS1256 = 0x050548,
    CC_TEXT_ENCODING_WINDOWS1257 = 0x05054a,
    CC_TEXT_ENCODING_WINDOWS1258 = 0x05054c,
    CC_TEXT_TYPESTRING = 0x0510,
    CC_TEXT_TYPE_TEXT = 0x0512,
    CC_TEXT_TYPE_HTML = 0x0514,
    CC_TEXT_TYPE_XML = 0x0516,
    CC_TEXT_TYPE_JSON = 0x0518,
    CC_TEXT_TYPE_YAML = 0x051a,
    CC_TEXT_TYPE_CSV = 0x051c,
    // File
    CC_FILE_CONTENT = 0x0700,
    CC_FILE_CONTENT_P = 0x0701,
    CC_FILE_NAME = 0x0702,
    CC_FILE_NAME_P = 0x0703,
    CC_FILE_TYPESTRING = 0x0710,
    // Link
    CC_LINK_TYPESTRING = 0x0900,
    CC_LINK_TYPE_TXIDOUT = 0x0910,
    CC_LINK_TYPE_TXID = 0x0912,
    CC_LINK_TYPE_COINTO = 0x091e,
    CC_LINK_TYPE_HTTP = 0x0920,
    CC_LINK_TYPE_HTTPS = 0x0922,
    CC_LINK_TYPE_MAILTO = 0x0924,
    CC_LINK_TYPE_FTP = 0x0926,
    CC_LINK_TYPE_FILE = 0x0928,
    CC_LINK_TYPE_CRID = 0x092a,
    CC_LINK_TYPE_ED2K = 0x0930,
    CC_LINK_TYPE_MAGNET = 0x0932,
    // Domain
    CC_DOMAIN_REG = 0x0b00,
    CC_DOMAIN_REG_P = 0x0b01,
    CC_DOMAIN_FORWARD = 0x0b02,
    CC_DOMAIN_FORWARD_P = 0x0b03,
    CC_DOMAIN_TRANSFER = 0x0b04,
    CC_DOMAIN_TRANSFER_P = 0x0b05,
    // Comment
    CC_COMMENT_CONTENT = 0x0d00,
    CC_COMMENT_CONTENT_P = 0x0d01,
    CC_COMMENT_LIKE = 0x0d02,
    CC_COMMENT_LIKE_P = 0x0d03,
    CC_COMMENT_DISLIKE = 0x0d04,
    CC_COMMENT_DISLIKE_P = 0x0d05,
    CC_COMMENT_SCORESTRING = 0x0d06,
    CC_COMMENT_SCORE_0 = 0x0d0600,
    CC_COMMENT_SCORE_1 = 0x0d0602,
    CC_COMMENT_SCORE_2 = 0x0d0604,
    CC_COMMENT_SCORE_3 = 0x0d0606,
    CC_COMMENT_SCORE_4 = 0x0d0608,
    CC_COMMENT_SCORE_5 = 0x0d060a,
    CC_COMMENT_SCORE_6 = 0x0d060c,
    CC_COMMENT_SCORE_7 = 0x0d060e,
    CC_COMMENT_SCORE_8 = 0x0d0610,
    CC_COMMENT_SCORE_9 = 0x0d0612,
    CC_COMMENT_SCORE_10 = 0x0d0614,
    CC_COMMENT_SCORE_P = 0x0d07,
    // Payment
    CC_PAYMENT_REQ = 0x1000,
    CC_PAYMENT_REQ_P = 0x1001,
    CC_PAYMENT_PRODID = 0x1002,
    CC_PAYMENT_PRODID_P = 0x1003,
    CC_PAYMENT_QUANTITY = 0x1004,
    CC_PAYMENT_UNITPRICE = 0x1006,
    CC_PAYMENT_DELIVERADD = 0x1008,
    CC_PAYMENT_CONTACTNUM = 0x100a,
    CC_PAYMENT_CONTACTEMAIL = 0x100c,
    CC_PAYMENT_REQ_ADDR = 0x100002,
    CC_PAYMENT_REQ_AMOUNT = 0x100004,
    CC_PAYMENT_REQ_VALIDTILL = 0x100006,
    CC_PAYMENT_REQ_VALIDTIME = 0x100008,
    // Lang
    CC_LANGSTRING = 0x12,
    CC_LANG_IETF = 0x1200,
    CC_LANG_ISO639_1 = 0x1202,
    CC_LANG_ISO639_2 = 0x1204,
    CC_LANG_ISO639_3 = 0x1206,
    CC_LANG_COUNTRY = 0x1208,
    CC_LANG_COUNTRYSTRING = 0x1208,
    CC_LANG_AB = 0x120002,
    CC_LANG_AA = 0x120004,
    CC_LANG_AF = 0x120006,
    CC_LANG_AK = 0x120008,
    CC_LANG_SQ = 0x12000a,
    CC_LANG_AM = 0x12000c,
    CC_LANG_AR = 0x12000e,
    CC_LANG_AN = 0x120010,
    CC_LANG_HY = 0x120012,
    CC_LANG_AS = 0x120014,
    CC_LANG_AV = 0x120016,
    CC_LANG_AE = 0x120018,
    CC_LANG_AY = 0x12001a,
    CC_LANG_AZ = 0x12001c,
    CC_LANG_BM = 0x12001e,
    CC_LANG_BA = 0x120020,
    CC_LANG_EU = 0x120022,
    CC_LANG_BE = 0x120024,
    CC_LANG_BN = 0x120026,
    CC_LANG_BH = 0x120028,
    CC_LANG_BI = 0x12002a,
    CC_LANG_BS = 0x12002c,
    CC_LANG_BR = 0x12002e,
    CC_LANG_BG = 0x120030,
    CC_LANG_MY = 0x120032,
    CC_LANG_CA = 0x120034,
    CC_LANG_CH = 0x120036,
    CC_LANG_CE = 0x120038,
    CC_LANG_NY = 0x12003a,
    CC_LANG_ZH = 0x12003c,
    CC_LANG_CV = 0x12003e,
    CC_LANG_KW = 0x120040,
    CC_LANG_CO = 0x120042,
    CC_LANG_CR = 0x120044,
    CC_LANG_HR = 0x120046,
    CC_LANG_CS = 0x120048,
    CC_LANG_DA = 0x12004a,
    CC_LANG_DV = 0x12004c,
    CC_LANG_NL = 0x12004e,
    CC_LANG_DZ = 0x120050,
    CC_LANG_EN = 0x120052,
    CC_LANG_EO = 0x120054,
    CC_LANG_ET = 0x120056,
    CC_LANG_EE = 0x120058,
    CC_LANG_FO = 0x12005a,
    CC_LANG_FJ = 0x12005c,
    CC_LANG_FI = 0x12005e,
    CC_LANG_FR = 0x120060,
    CC_LANG_FF = 0x120062,
    CC_LANG_GL = 0x120064,
    CC_LANG_KA = 0x120066,
    CC_LANG_DE = 0x120068,
    CC_LANG_EL = 0x12006a,
    CC_LANG_GN = 0x12006c,
    CC_LANG_GU = 0x12006e,
    CC_LANG_HT = 0x120070,
    CC_LANG_HA = 0x120072,
    CC_LANG_HE = 0x120074,
    CC_LANG_HZ = 0x120076,
    CC_LANG_HI = 0x120078,
    CC_LANG_HO = 0x12007a,
    CC_LANG_HU = 0x12007c,
    CC_LANG_IA = 0x12007e,
    CC_LANG_ID = 0x120080,
    CC_LANG_IE = 0x120082,
    CC_LANG_GA = 0x120084,
    CC_LANG_IG = 0x120086,
    CC_LANG_IK = 0x120088,
    CC_LANG_IO = 0x12008a,
    CC_LANG_IS = 0x12008c,
    CC_LANG_IT = 0x12008e,
    CC_LANG_IU = 0x120090,
    CC_LANG_JA = 0x120092,
    CC_LANG_JV = 0x120094,
    CC_LANG_KL = 0x120096,
    CC_LANG_KN = 0x120098,
    CC_LANG_KR = 0x12009a,
    CC_LANG_KS = 0x12009c,
    CC_LANG_KK = 0x12009e,
    CC_LANG_KM = 0x1200a0,
    CC_LANG_KI = 0x1200a2,
    CC_LANG_RW = 0x1200a4,
    CC_LANG_KY = 0x1200a6,
    CC_LANG_KV = 0x1200a8,
    CC_LANG_KG = 0x1200aa,
    CC_LANG_KO = 0x1200ac,
    CC_LANG_KU = 0x1200ae,
    CC_LANG_KJ = 0x1200b0,
    CC_LANG_LA = 0x1200b2,
    CC_LANG_LB = 0x1200b4,
    CC_LANG_LG = 0x1200b6,
    CC_LANG_LI = 0x1200b8,
    CC_LANG_LN = 0x1200ba,
    CC_LANG_LO = 0x1200bc,
    CC_LANG_LT = 0x1200be,
    CC_LANG_LU = 0x1200c0,
    CC_LANG_LV = 0x1200c2,
    CC_LANG_GV = 0x1200c4,
    CC_LANG_MK = 0x1200c6,
    CC_LANG_MG = 0x1200c8,
    CC_LANG_MS = 0x1200ca,
    CC_LANG_ML = 0x1200cc,
    CC_LANG_MT = 0x1200ce,
    CC_LANG_MI = 0x1200d0,
    CC_LANG_MR = 0x1200d2,
    CC_LANG_MH = 0x1200d4,
    CC_LANG_MN = 0x1200d6,
    CC_LANG_NA = 0x1200d8,
    CC_LANG_NV = 0x1200da,
    CC_LANG_ND = 0x1200dc,
    CC_LANG_NE = 0x1200de,
    CC_LANG_NG = 0x1200e0,
    CC_LANG_NB = 0x1200e2,
    CC_LANG_NN = 0x1200e4,
    CC_LANG_NO = 0x1200e6,
    CC_LANG_II = 0x1200e8,
    CC_LANG_NR = 0x1200ea,
    CC_LANG_OC = 0x1200ec,
    CC_LANG_OJ = 0x1200ee,
    CC_LANG_CU = 0x1200f0,
    CC_LANG_OM = 0x1200f2,
    CC_LANG_OR = 0x1200f4,
    CC_LANG_OS = 0x1200f6,
    CC_LANG_PA = 0x1200f8,
    CC_LANG_PI = 0x1200fa,
    CC_LANG_FA = 0x1200fc,
    CC_LANG_PL = 0x1200fe,
    CC_LANG_PS = 0x120100,
    CC_LANG_PT = 0x120102,
    CC_LANG_QU = 0x120104,
    CC_LANG_RM = 0x120106,
    CC_LANG_RN = 0x120108,
    CC_LANG_RO = 0x12010a,
    CC_LANG_RU = 0x12010c,
    CC_LANG_SA = 0x12010e,
    CC_LANG_SC = 0x120110,
    CC_LANG_SD = 0x120112,
    CC_LANG_SE = 0x120114,
    CC_LANG_SM = 0x120116,
    CC_LANG_SG = 0x120118,
    CC_LANG_SR = 0x12011a,
    CC_LANG_GD = 0x12011c,
    CC_LANG_SN = 0x12011e,
    CC_LANG_SI = 0x120120,
    CC_LANG_SK = 0x120122,
    CC_LANG_SL = 0x120124,
    CC_LANG_SO = 0x120126,
    CC_LANG_ST = 0x120128,
    CC_LANG_ES = 0x12012a,
    CC_LANG_SU = 0x12012c,
    CC_LANG_SW = 0x12012e,
    CC_LANG_SS = 0x120130,
    CC_LANG_SV = 0x120132,
    CC_LANG_TA = 0x120134,
    CC_LANG_TE = 0x120136,
    CC_LANG_TG = 0x120138,
    CC_LANG_TH = 0x12013a,
    CC_LANG_TI = 0x12013c,
    CC_LANG_BO = 0x12013e,
    CC_LANG_TK = 0x120140,
    CC_LANG_TL = 0x120142,
    CC_LANG_TN = 0x120144,
    CC_LANG_TO = 0x120146,
    CC_LANG_TR = 0x120148,
    CC_LANG_TS = 0x12014a,
    CC_LANG_TT = 0x12014c,
    CC_LANG_TW = 0x12014e,
    CC_LANG_TY = 0x120150,
    CC_LANG_UG = 0x120152,
    CC_LANG_UK = 0x120154,
    CC_LANG_UR = 0x120156,
    CC_LANG_UZ = 0x120158,
    CC_LANG_VE = 0x12015a,
    CC_LANG_VI = 0x12015c,
    CC_LANG_VO = 0x12015e,
    CC_LANG_WA = 0x120160,
    CC_LANG_CY = 0x120162,
    CC_LANG_WO = 0x120164,
    CC_LANG_FY = 0x120166,
    CC_LANG_XH = 0x120168,
    CC_LANG_YI = 0x12016a,
    CC_LANG_YO = 0x12016c,
    CC_LANG_ZA = 0x12016e,
    CC_LANG_ZU = 0x120170,
    
    
    CC_ENCRYPT_PARAMS = 0x1402,
CC_ENCRYPT_PARAMS_P = 0x1403,
CC_ENCRYPT_PARAMS_IV = 0x140002,
CC_ENCRYPT_PARAMS_SALT = 0x140004,
CC_ENCRYPT_PARAMS_SCRYPT_N = 0x140006,
CC_ENCRYPT_PARAMS_SCRYPT_P = 0x140008,
CC_ENCRYPT_PARAMS_SCRYPT_R = 0x14000a,
CC_ENCRYPT_PARAMS_AES_ITERATIONS = 0x14000c,
CC_ENCRYPT_PARAMS_ALGORITHM_STRING = 0x1404,
CC_ENCRYPT_PARAMS_ALGORITHM_SECRETCHAT = 0x140402,
CC_ENCRYPT_PARAMS_ALGORITHM_AES = 0x140404,
CC_ENCRYPT_PARAMS_ALGORITHM_SCRYPT = 0x140406,
CC_ENCRYPT_PARAMS_ALGORITHM_MHASH = 0x140408,
CC_ENCRYPT_PARAMS_MHASH_HEIGHT = 0x14000e,
};

std::string GetCcName(const cctype cc);
cctype GetCcValue(std::string ccName);
std::string GetCcHex(const cctype cc);

class CContent : public std::string
{
public:

    CContent()
    {
        SetEmpty();
    }

    CContent(const std::string& cttStr)
    {
        SetString(cttStr);
    }

    CContent(const vector<unsigned char>& cttVch)
    {
        SetString(cttVch);
    }

    CContent(const Array& cttJson)
    {
        SetJson(cttJson);
    }
    Array ToJson(stringformat fFormat = STR_FORMAT_BIN,bool fRecursive = true);
    bool ToJsonString(std::string& entry);
    bool SetEmpty();
    bool SetString(const std::string& cttStr);
    bool SetString(const vector<unsigned char>& cttVch);
    bool SetJson(const Array& cttJson);
    bool SetUnit(const cctype& cc,const std::string& cttStr);
    bool SetUnit(const std::string& ccname,const std::string& cttStr);
    std::string ToHumanString();
    bool GetCcUnit(iterator& pc, cctype& ccRet, std::string& content);
    bool ReadVarInt(iterator& pc,u_int64_t& n);
    bool ReadCompactSize(iterator& pc,u_int64_t& n);
    bool WriteVarInt(u_int64_t num);
    bool WriteCompactSize(u_int64_t num);
    bool ReadData(iterator & pc, int len, std::string& str);
    bool ReadDataReverse(iterator & pc, int len, std::string& str);
    std::string TrimToHumanString(const std::string& str);
    bool WriteData(const std::string str);
    bool WriteData(const std::string str, int len);
    bool IsCcParent(const cctype& cc);
    bool HasCc(const cctype& cc);
    bool FirstCc(const cctype& cc);
    bool IsStandard();
    bool EncodeP(const int cc,const std::vector<std::pair<int,string> >& vEncoding);
    bool EncodeUnit(const int cc,const string& content);
    bool Decode(std::vector<std::pair<int,string> >& vDecoded);
};
class CMessage
{
public:
    int nBlockHeight;
    uint256 txid;
    int nTx;
    int nVout;
    CScript IDFrom;
    CScript IDTo;
    CContent content;
    unsigned int nTime;
    //bool fIncoming=true;
    CMessage()
    {
    nBlockHeight=-1;
    txid=0;
    nTx=-1;
    nVout=0;
    CScript IDFrom;
    CScript IDTo;
    CContent content;
    nTime=GetTime();
    };
    Value ToJson(bool fLinkOnly=false);
    string ToJsonString(bool fLinkOnly=false);
    bool SetJson(const Object& json);
};
#endif
