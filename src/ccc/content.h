#ifndef CCC_CONTENT_H
#define CCC_CONTENT_H

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <boost/assign.hpp>
#include "utiltime.h"
#include "serialize.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "uint256.h"
//#include "script/script.h"
#include "ccc/link.h"
#include "amount.h"
using namespace json_spirit;
using namespace std;
using std::string;
class CScript;
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
    BI_DIRECTION = 0,
    INCOMING_ONLY = 1,
    OUTPUT_ONLY = 2,
};

/** Content codes */
enum cctype
{
/** Null * */
CC_NULL = 0x00,
CC_P = 0x01,
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
CC_PRODUCT = 0x16,
CC_PRODUCT_P = 0x17,
CC_MESSAGE = 0x18,
CC_MESSAGE_P = 0x19,
CC_CURRENCY = 0x1a,
CC_CURRENCY_P = 0x1b,
CC_AMOUNT = 0x1e,
CC_AMOUNT_P = 0x1f,
CC_QUANTITY = 0x20,
CC_QUANTITY_P = 0x21,
CC_TIME = 0x22,
CC_TIME_P = 0x23,
CC_BANK = 0x24,
CC_BANK_P = 0x25,
CC_HASH = 0x26,
CC_HASH_P = 0x27,
CC_CONTACT = 0x28,
CC_CONTACT_P = 0x29,
CC_REGION = 0x2a,
CC_REGION_P = 0x2b,
CC_UNIT = 0x2c,
CC_UNIT_P = 0x2d,
CC_ATTRIBUTE = 0x2e,
CC_ATTRIBUTE_P = 0x2f,
CC_ID = 0x30,
CC_ID_P = 0x31,
CC_SIGNATURE = 0x32,
CC_SIGNATURE_P = 0x33,
CC_COMPRESS = 0x34,
CC_COMPRESS_P = 0x35,
CC_AUTHOR = 0x36,
CC_VERSION = 0x38,
CC_NUMBER = 0x3a,
CC_ENCODING = 0x3c,
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
CC_TAG_SELLCOIN = 0x031022,
// Text
CC_TEXT_ENCODINGSTRING = 0x0500,
CC_TEXT_TYPESTRING = 0x0510,
CC_TEXT_TYPE_TEXT = 0x0512,
CC_TEXT_TYPE_HTML = 0x0514,
CC_TEXT_TYPE_XML = 0x0516,
CC_TEXT_TYPE_JSON = 0x0518,
CC_TEXT_TYPE_YAML = 0x051a,
CC_TEXT_TYPE_CSV = 0x051c,
CC_NUMBER_ENCODING_STRING = 0x3a00,
CC_NUMBER_ENCODING_DEC = 0x3a02,
CC_NUMBER_ENCODING_INT_LE = 0x3a04,
CC_NUMBER_ENCODING_FLOAT_LE = 0x3a06,
CC_NUMBER_ENCODING_DOUBLE_LE = 0x3a08,
CC_NUMBER_ENCODING_VARINT = 0x3a10,
CC_NUMBER_ENCODING_DEC_SCIENTIFIC = 0x3a12,
CC_NUMBER_ENCODING_BINARY_SCIENTIFIC = 0x3a14,
// File
CC_FILE_NAME = 0x0700,
CC_FILE_NAME_P = 0x0701,
CC_FILE_TYPESTRING = 0x0702,
CC_FILE_PART = 0x0704,
CC_FILE_COMBINE_P = 0x0707,
CC_FILE_PACKAGE_P = 0x0709,
CC_FILE_PACKAGE_MAINFILE = 0x070800,
CC_FILE_PACKAGE_NAME = 0x070802,
// Link
CC_LINK_TYPESTRING = 0x0900,
CC_LINK_TYPE_BLOCKCHAIN = 0x0910,
CC_LINK_TYPE_TXIDOUT = 0x0912,
CC_LINK_TYPE_COINTO = 0x091e,
CC_LINK_TYPE_HTTP = 0x0920,
CC_LINK_TYPE_HTTPS = 0x0922,
CC_LINK_TYPE_MAILTO = 0x0924,
CC_LINK_TYPE_FTP = 0x0926,
CC_LINK_TYPE_FILE = 0x0928,
CC_LINK_TYPE_CRID = 0x092a,
CC_LINK_TYPE_ED2K = 0x0930,
CC_LINK_TYPE_MAGNET = 0x0932,
CC_LINK_TYPE_SCRIPTPUBKEY = 0x0934,
CC_LINK_TYPE_DOMAIN = 0x0936,
// Domain
CC_DOMAIN_REG = 0x0b00,
CC_DOMAIN_REG_P = 0x0b01,
CC_DOMAIN_FORWARD = 0x0b02,
CC_DOMAIN_FORWARD_P = 0x0b03,
CC_DOMAIN_TRANSFER = 0x0b04,
CC_DOMAIN_TRANSFER_P = 0x0b05,

CC_DOMAIN_INFO = 0x0B06,
CC_DOMAIN_INFO_P = 0x0B07,
CC_DOMAIN_INFO_ALIAS = 0x0B0700,
CC_DOMAIN_INFO_INTRO = 0x0B0702,
CC_DOMAIN_INFO_ICON = 0x0B0704,
CC_DOMAIN_INFO_ICON_P = 0x0B0705,
// Comment
CC_COMMENT_CONTENT = 0x0d00,
CC_COMMENT_CONTENT_P = 0x0d01,
CC_COMMENT_LIKE = 0x0d02,
CC_COMMENT_LIKE_P = 0x0d03,
CC_COMMENT_DISLIKE = 0x0d04,
CC_COMMENT_DISLIKE_P = 0x0d05,
CC_COMMENT_SCORE = 0x0d06,
CC_COMMENT_SCORE_P = 0x0d07,
// Payment
CC_PAYMENT_REQ = 0x1000,
CC_PAYMENT_REQ_P = 0x1001,
CC_PAYMENT_REQ_VALIDTILL = 0x100002,
CC_PAYMENT_TYPE = 0x1002,
CC_PAYMENT_TYPE_P = 0x1003,
CC_PAYMENT_RECIPIENT = 0x1004,
CC_PAYMENT_MEMO = 0x1006,
CC_PAYMENT_DELIVERADD = 0x1008,
CC_PAYMENT_DELIVERADD_P = 0x1009,
CC_PAYMENT_REFUND_REQ = 0x100a,
CC_PAYMENT_REFUND_REQ_P = 0x100b,
CC_PAYMENT_ITEM_P = 0x100f,
CC_PAYMENT_LINK = 0x1010,
CC_PAYMENT_LINK_P = 0x1011,
CC_PAYMENT_TYPE_SHOPPING = 0x100200,
CC_PAYMENT_TYPE_PRODUCT = 0x100202,
CC_PAYMENT_TYPE_P2REQ = 0x100204,
CC_PAYMENT_TYPE_REFUND = 0x100206,
CC_PAYMENT_TYPE_SHIPMENTFEE = 0x100208,
CC_PAYMENT_TYPE_P2AGREEMENT = 0x100210,
CC_PAYMENT_TYPE_DUTCH = 0x100212,
CC_PAYMENT_TYPE_LEND = 0x100214,
CC_PAYMENT_TYPE_RETURN = 0x100216,
CC_PAYMENT_TYPE_RENT = 0x100218,
CC_PAYMENT_TYPE_INTEREST = 0x10021a,
// Lang
CC_LANGSTRING = 0x12,
CC_LANG_IETF = 0x1200,
CC_LANG_ISO639_1 = 0x1202,
CC_LANG_ISO639_2 = 0x1204,
CC_LANG_ISO639_3 = 0x1206,
CC_LANG_COUNTRY = 0x1208,
CC_LANG_COUNTRYSTRING = 0x1208,

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
//PRODUCT
CC_PRODUCT_PRICE = 0x1600,
CC_PRODUCT_PRICE_P = 0x1601,
CC_PRODUCT_ID = 0x1602,
CC_PRODUCT_NAME = 0x1604,
CC_PRODUCT_NAME_P = 0x1605,
CC_PRODUCT_PAYTO = 0x1606,
CC_PRODUCT_INTRO = 0x1608,
CC_PRODUCT_ICON = 0x160a,
CC_PRODUCT_ICON_P = 0x160b,
CC_PRODUCT_ATTIRBUTE = 0x160c,
CC_PRODUCT_ATTIRBUTE_P = 0x160d,
CC_PRODUCT_ATTIRBUTE_TYPE = 0x160c00,
CC_PRODUCT_UNIT_STRING = 0x160e,
CC_PRODUCT_UNIT_P = 0x160f,
CC_PRODUCT_UNIT_PIECE = 0x160e00,
CC_PRODUCT_SHIPMENTFEE = 0x1610,
CC_PRODUCT_SHIPMENTFEE_P = 0x1611,
CC_PRODUCT_EXPIRETIME = 0x1612,

CC_BANK_BANKCODE = 0x2400,


CC_CONTACT_PHONE = 0x2800,
CC_CONTACT_EMAIL = 0x2802,
CC_CONTACT_BLOCKCHAIN = 0x2804,
};
static std::map<int,std::string> mapCC=boost::assign::map_list_of
/** Null * */
(CC_NULL,"CC_NULL")
(CC_P,"CC_P")
/** Tag * */
(CC_TAG,"CC_TAG")
(CC_TAG_P,"CC_TAG_P")

/** First Level Content Code */
(CC_TEXT,"CC_TEXT")
(CC_TEXT_P,"CC_TEXT_P")
(CC_FILE,"CC_FILE")
(CC_FILE_P,"CC_FILE_P")
(CC_LINK,"CC_LINK")
(CC_LINK_P,"CC_LINK_P")
(CC_DOMAIN,"CC_DOMAIN")
(CC_DOMAIN_P,"CC_DOMAIN_P")
(CC_COMMENT,"CC_COMMENT")
(CC_COMMENT_P,"CC_COMMENT_P")
(CC_DELETE,"CC_DELETE")
(CC_DELETE_P,"CC_DELETE_P")
(CC_PAYMENT,"CC_PAYMENT")
(CC_PAYMENT_P,"CC_PAYMENT_P")
(CC_LANG,"CC_LANG")
(CC_LANG_P,"CC_LANG_P")
(CC_ENCRYPT,"CC_ENCRYPT")
(CC_ENCRYPT_P,"CC_ENCRYPT_P")
(CC_PRODUCT,"CC_PRODUCT")
(CC_PRODUCT_P,"CC_PRODUCT_P")
(CC_MESSAGE,"CC_MESSAGE")
(CC_MESSAGE_P,"CC_MESSAGE_P")
(CC_CURRENCY,"CC_CURRENCY")
(CC_CURRENCY_P,"CC_CURRENCY_P")
(CC_AMOUNT,"CC_AMOUNT")
(CC_AMOUNT_P,"CC_AMOUNT_P")
(CC_QUANTITY,"CC_QUANTITY")
(CC_QUANTITY_P,"CC_QUANTITY_P")
(CC_TIME,"CC_TIME")
(CC_TIME_P,"CC_TIME_P")
(CC_BANK,"CC_BANK")
(CC_BANK_P,"CC_BANK_P")
(CC_HASH,"CC_HASH")
(CC_HASH_P,"CC_HASH_P")
(CC_CONTACT,"CC_CONTACT")
(CC_CONTACT_P,"CC_CONTACT_P")
(CC_REGION,"CC_REGION")
(CC_REGION_P,"CC_REGION_P")
(CC_UNIT,"CC_UNIT")
(CC_UNIT_P,"CC_UNIT_P")
(CC_ATTRIBUTE,"CC_ATTRIBUTE")
(CC_ATTRIBUTE_P,"CC_ATTRIBUTE_P")
(CC_ID,"CC_ID")
(CC_ID_P,"CC_ID_P")
(CC_SIGNATURE,"CC_SIGNATURE")
(CC_SIGNATURE_P,"CC_SIGNATURE_P")
(CC_COMPRESS,"CC_COMPRESS")
(CC_COMPRESS_P,"CC_COMPRESS_P")
(CC_AUTHOR,"CC_AUTHOR")
(CC_VERSION,"CC_VERSION")
(CC_NUMBER,"CC_NUMBER")
(CC_ENCODING,"CC_ENCODING")
/** Second Level Content Code * */
// Tag
(CC_TAG_TEXT,"CC_TAG_TEXT")
(CC_TAG_FILE,"CC_TAG_FILE")
(CC_TAG_LINK,"CC_TAG_LINK")
(CC_TAG_DOMAIN,"CC_TAG_DOMAIN")
(CC_TAG_COMMENT,"CC_TAG_COMMENT")
(CC_TAG_NEWS,"CC_TAG_NEWS")
(CC_TAG_IMAGE,"CC_TAG_IMAGE")
(CC_TAG_AUDIO,"CC_TAG_AUDIO")
(CC_TAG_VIDEO,"CC_TAG_VIDEO")
(CC_TAG_MESSAGE,"CC_TAG_MESSAGE")
(CC_TAG_ADVERTISMENT,"CC_TAG_ADVERTISMENT")
(CC_TAG_STREAM,"CC_TAG_STREAM")
(CC_TAG_CONTENT,"CC_TAG_CONTENT")
(CC_TAG_COIN,"CC_TAG_COIN")
(CC_TAG_ID,"CC_TAG_ID")
(CC_TAG_SELLCOIN,"CC_TAG_SELLCOIN")
// Text
(CC_TEXT_ENCODINGSTRING,"CC_TEXT_ENCODINGSTRING")
(CC_TEXT_TYPESTRING,"CC_TEXT_TYPESTRING")
(CC_TEXT_TYPE_TEXT,"CC_TEXT_TYPE_TEXT")
(CC_TEXT_TYPE_HTML,"CC_TEXT_TYPE_HTML")
(CC_TEXT_TYPE_XML,"CC_TEXT_TYPE_XML")
(CC_TEXT_TYPE_JSON,"CC_TEXT_TYPE_JSON")
(CC_TEXT_TYPE_YAML,"CC_TEXT_TYPE_YAML")
(CC_TEXT_TYPE_CSV,"CC_TEXT_TYPE_CSV")
(CC_NUMBER_ENCODING_STRING,"CC_NUMBER_ENCODING_STRING")
(CC_NUMBER_ENCODING_DEC,"CC_NUMBER_ENCODING_DEC")
(CC_NUMBER_ENCODING_INT_LE,"CC_NUMBER_ENCODING_INT_LE")
(CC_NUMBER_ENCODING_FLOAT_LE,"CC_NUMBER_ENCODING_FLOAT_LE")
(CC_NUMBER_ENCODING_DOUBLE_LE,"CC_NUMBER_ENCODING_DOUBLE_LE")
(CC_NUMBER_ENCODING_VARINT,"CC_NUMBER_ENCODING_VARINT")
(CC_NUMBER_ENCODING_DEC_SCIENTIFIC,"CC_NUMBER_ENCODING_DEC_SCIENTIFIC")
(CC_NUMBER_ENCODING_BINARY_SCIENTIFIC,"CC_NUMBER_ENCODING_BINARY_SCIENTIFIC")
// File
(CC_FILE_NAME,"CC_FILE_NAME")
(CC_FILE_NAME_P,"CC_FILE_NAME_P")
(CC_FILE_TYPESTRING,"CC_FILE_TYPESTRING")
(CC_FILE_PART,"CC_FILE_PART")
(CC_FILE_COMBINE_P,"CC_FILE_COMBINE_P")
(CC_FILE_PACKAGE_P,"CC_FILE_PACKAGE_P")
(CC_FILE_PACKAGE_MAINFILE,"CC_FILE_PACKAGE_MAINFILE")
(CC_FILE_PACKAGE_NAME,"CC_FILE_PACKAGE_NAME")
// Link
(CC_LINK_TYPESTRING,"CC_LINK_TYPESTRING")
(CC_LINK_TYPE_BLOCKCHAIN,"CC_LINK_TYPE_BLOCKCHAIN")
(CC_LINK_TYPE_TXIDOUT,"CC_LINK_TYPE_TXIDOUT")
(CC_LINK_TYPE_COINTO,"CC_LINK_TYPE_COINTO")
(CC_LINK_TYPE_HTTP,"CC_LINK_TYPE_HTTP")
(CC_LINK_TYPE_HTTPS,"CC_LINK_TYPE_HTTPS")
(CC_LINK_TYPE_MAILTO,"CC_LINK_TYPE_MAILTO")
(CC_LINK_TYPE_FTP,"CC_LINK_TYPE_FTP")
(CC_LINK_TYPE_FILE,"CC_LINK_TYPE_FILE")
(CC_LINK_TYPE_CRID,"CC_LINK_TYPE_CRID")
(CC_LINK_TYPE_ED2K,"CC_LINK_TYPE_ED2K")
(CC_LINK_TYPE_MAGNET,"CC_LINK_TYPE_MAGNET")
(CC_LINK_TYPE_SCRIPTPUBKEY,"CC_LINK_TYPE_SCRIPTPUBKEY")
(CC_LINK_TYPE_DOMAIN,"CC_LINK_TYPE_DOMAIN")
// Domain
(CC_DOMAIN_REG,"CC_DOMAIN_REG")
(CC_DOMAIN_REG_P,"CC_DOMAIN_REG_P")
(CC_DOMAIN_FORWARD,"CC_DOMAIN_FORWARD")
(CC_DOMAIN_FORWARD_P,"CC_DOMAIN_FORWARD_P")
(CC_DOMAIN_TRANSFER,"CC_DOMAIN_TRANSFER")
(CC_DOMAIN_TRANSFER_P,"CC_DOMAIN_TRANSFER_P")

(CC_DOMAIN_INFO,"CC_DOMAIN_INFO")
(CC_DOMAIN_INFO_P,"CC_DOMAIN_INFO_P")
(CC_DOMAIN_INFO_ALIAS,"CC_DOMAIN_INFO_ALIAS")
(CC_DOMAIN_INFO_INTRO,"CC_DOMAIN_INFO_INTRO")
(CC_DOMAIN_INFO_ICON,"CC_DOMAIN_INFO_ICON")
(CC_DOMAIN_INFO_ICON_P,"CC_DOMAIN_INFO_ICON_P")
// Comment
(CC_COMMENT_CONTENT,"CC_COMMENT_CONTENT")
(CC_COMMENT_CONTENT_P,"CC_COMMENT_CONTENT_P")
(CC_COMMENT_LIKE,"CC_COMMENT_LIKE")
(CC_COMMENT_LIKE_P,"CC_COMMENT_LIKE_P")
(CC_COMMENT_DISLIKE,"CC_COMMENT_DISLIKE")
(CC_COMMENT_DISLIKE_P,"CC_COMMENT_DISLIKE_P")
(CC_COMMENT_SCORE,"CC_COMMENT_SCORE")
(CC_COMMENT_SCORE_P,"CC_COMMENT_SCORE_P")
// Payment
(CC_PAYMENT_REQ,"CC_PAYMENT_REQ")
(CC_PAYMENT_REQ_P,"CC_PAYMENT_REQ_P")
(CC_PAYMENT_REQ_VALIDTILL,"CC_PAYMENT_REQ_VALIDTILL")
(CC_PAYMENT_TYPE,"CC_PAYMENT_TYPE")
(CC_PAYMENT_TYPE_P,"CC_PAYMENT_TYPE_P")
(CC_PAYMENT_RECIPIENT,"CC_PAYMENT_RECIPIENT")
(CC_PAYMENT_MEMO,"CC_PAYMENT_MEMO")
(CC_PAYMENT_DELIVERADD,"CC_PAYMENT_DELIVERADD")
(CC_PAYMENT_DELIVERADD_P,"CC_PAYMENT_DELIVERADD_P")
(CC_PAYMENT_REFUND_REQ,"CC_PAYMENT_REFUND_REQ")
(CC_PAYMENT_REFUND_REQ_P,"CC_PAYMENT_REFUND_REQ_P")
(CC_PAYMENT_ITEM_P,"CC_PAYMENT_ITEM_P")
(CC_PAYMENT_LINK,"CC_PAYMENT_LINK")
(CC_PAYMENT_LINK_P,"CC_PAYMENT_LINK_P")
(CC_PAYMENT_TYPE_SHOPPING,"CC_PAYMENT_TYPE_SHOPPING")
(CC_PAYMENT_TYPE_PRODUCT,"CC_PAYMENT_TYPE_PRODUCT")
(CC_PAYMENT_TYPE_P2REQ,"CC_PAYMENT_TYPE_P2REQ")
(CC_PAYMENT_TYPE_REFUND,"CC_PAYMENT_TYPE_REFUND")
(CC_PAYMENT_TYPE_SHIPMENTFEE,"CC_PAYMENT_TYPE_SHIPMENTFEE")
(CC_PAYMENT_TYPE_P2AGREEMENT,"CC_PAYMENT_TYPE_P2AGREEMENT")
(CC_PAYMENT_TYPE_DUTCH,"CC_PAYMENT_TYPE_DUTCH")
(CC_PAYMENT_TYPE_LEND,"CC_PAYMENT_TYPE_LEND")
(CC_PAYMENT_TYPE_RETURN,"CC_PAYMENT_TYPE_RETURN")
(CC_PAYMENT_TYPE_RENT,"CC_PAYMENT_TYPE_RENT")
(CC_PAYMENT_TYPE_INTEREST,"CC_PAYMENT_TYPE_INTEREST")
// Lang

(CC_LANG_IETF,"CC_LANG_IETF")
(CC_LANG_ISO639_1,"CC_LANG_ISO639_1")
(CC_LANG_ISO639_2,"CC_LANG_ISO639_2")
(CC_LANG_ISO639_3,"CC_LANG_ISO639_3")
(CC_LANG_COUNTRY,"CC_LANG_COUNTRY")

// Encrypt
(CC_ENCRYPT_PARAMS,"CC_ENCRYPT_PARAMS")
(CC_ENCRYPT_PARAMS_P,"CC_ENCRYPT_PARAMS_P")
(CC_ENCRYPT_PARAMS_IV,"CC_ENCRYPT_PARAMS_IV")
(CC_ENCRYPT_PARAMS_SALT,"CC_ENCRYPT_PARAMS_SALT")
(CC_ENCRYPT_PARAMS_SCRYPT_N,"CC_ENCRYPT_PARAMS_SCRYPT_N")
(CC_ENCRYPT_PARAMS_SCRYPT_P,"CC_ENCRYPT_PARAMS_SCRYPT_P")
(CC_ENCRYPT_PARAMS_SCRYPT_R,"CC_ENCRYPT_PARAMS_SCRYPT_R")
(CC_ENCRYPT_PARAMS_AES_ITERATIONS,"CC_ENCRYPT_PARAMS_AES_ITERATIONS")
(CC_ENCRYPT_PARAMS_ALGORITHM_STRING,"CC_ENCRYPT_PARAMS_ALGORITHM_STRING")
(CC_ENCRYPT_PARAMS_ALGORITHM_SECRETCHAT,"CC_ENCRYPT_PARAMS_ALGORITHM_SECRETCHAT")
(CC_ENCRYPT_PARAMS_ALGORITHM_AES,"CC_ENCRYPT_PARAMS_ALGORITHM_AES")
(CC_ENCRYPT_PARAMS_ALGORITHM_SCRYPT,"CC_ENCRYPT_PARAMS_ALGORITHM_SCRYPT")
(CC_ENCRYPT_PARAMS_ALGORITHM_MHASH,"CC_ENCRYPT_PARAMS_ALGORITHM_MHASH")
(CC_ENCRYPT_PARAMS_MHASH_HEIGHT,"CC_ENCRYPT_PARAMS_MHASH_HEIGHT")
//PRODUCT
(CC_PRODUCT_PRICE,"CC_PRODUCT_PRICE")
(CC_PRODUCT_PRICE_P,"CC_PRODUCT_PRICE_P")
(CC_PRODUCT_ID,"CC_PRODUCT_ID")
(CC_PRODUCT_NAME,"CC_PRODUCT_NAME")
(CC_PRODUCT_NAME_P,"CC_PRODUCT_NAME_P")
(CC_PRODUCT_PAYTO,"CC_PRODUCT_PAYTO")
(CC_PRODUCT_INTRO,"CC_PRODUCT_INTRO")
(CC_PRODUCT_ICON,"CC_PRODUCT_ICON")
(CC_PRODUCT_ICON_P,"CC_PRODUCT_ICON_P")
(CC_PRODUCT_ATTIRBUTE,"CC_PRODUCT_ATTIRBUTE")
(CC_PRODUCT_ATTIRBUTE_P,"CC_PRODUCT_ATTIRBUTE_P")
(CC_PRODUCT_ATTIRBUTE_TYPE,"CC_PRODUCT_ATTIRBUTE_TYPE")
(CC_PRODUCT_UNIT_STRING,"CC_PRODUCT_UNIT_STRING")
(CC_PRODUCT_UNIT_P,"CC_PRODUCT_UNIT_P")
(CC_PRODUCT_UNIT_PIECE,"CC_PRODUCT_UNIT_PIECE")
(CC_PRODUCT_SHIPMENTFEE,"CC_PRODUCT_SHIPMENTFEE")
(CC_PRODUCT_SHIPMENTFEE_P,"CC_PRODUCT_SHIPMENTFEE_P")
(CC_PRODUCT_EXPIRETIME,"CC_PRODUCT_EXPIRETIME")

(CC_BANK_BANKCODE,"CC_BANK_BANKCODE")


(CC_CONTACT_PHONE,"CC_CONTACT_PHONE")
(CC_CONTACT_EMAIL,"CC_CONTACT_EMAIL")
(CC_CONTACT_BLOCKCHAIN,"CC_CONTACT_BLOCKCHAIN")
 ;
std::string GetCcName(const cctype cc);
cctype GetCcValue(const std::string ccName);
std::string GetCcHex(const cctype cc);
bool IsCcParent(const cctype& cc);

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
    Array ToJson(stringformat fFormat = STR_FORMAT_BIN, bool fRecursive = true)const;
    bool ToJsonString(std::string& entry)const;
    bool SetEmpty();
    bool IsEmpty()const;
    bool SetString(const std::string& cttStr);
    bool SetString(const vector<unsigned char>& cttVch);
    bool SetJson(const Array& cttJson);
    bool SetUnit(const cctype& cc, const std::string& cttStr);
    bool SetUnit(const std::string& ccname, const std::string& cttStr);
    std::string ToHumanString();
    bool GetCcUnit(const_iterator& pc, cctype& ccRet, std::string& content) const;
    bool ReadVarInt(const_iterator& pc, uint64_t& n)const;
    bool ReadCompactSize(const_iterator& pc, uint64_t& n)const;
    bool WriteVarInt(uint64_t num);
    bool WriteCompactSize(uint64_t num);
    bool ReadData(const_iterator & pc, int len, std::string& str)const;
    bool ReadDataReverse(const_iterator & pc, int len, std::string& str)const;
    std::string TrimToHumanString(const std::string& str)const;
    bool WriteData(const std::string str);
    bool WriteData(const std::string str, int len);

    bool HasCc(const cctype& cc)const;
    bool FirstCc(const cctype& cc)const;
    int GetFirstCc()const;
    bool IsStandard()const;
    bool EncodeP(const int cc, const std::vector<std::pair<int, string> >& vEncoding);
    bool EncodeUnit(const int cc, const string& content);
    bool Decode(std::vector<std::pair<int, string> >& vDecoded)const;
    bool DecodeDomainInfo(string& strAlias, string& strIntro, CLink& iconLink, std::vector<string>& vTags)const;
    bool DecodeLink(int& redirectType, string& redirectTo)const;
    bool DecodeFileString(std::string& strFile);
    bool GetTags(std::vector<std::pair<int, std::string> >& vTagList) const;
    bool _GetTags(std::vector<std::pair<int, std::string> >& vTagList, int ccp = -1) const;
    //bool DecodeTagP(std::vector<std::string> vTag)const;
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
        nBlockHeight = -1;
        txid = 0;
        nTx = -1;
        nVout = 0;
        CScript IDFrom;
        CScript IDTo;
        CContent content;
        nTime = GetTime();
    };
    Value ToJson(bool fLinkOnly = false)const;
    string ToJsonString(bool fLinkOnly = false)const;
    bool SetJson(const Object& json);
};

struct CPricing
{
    CAmount price;
    string strUnit;
    string strCurrency;
    string strRegion;
    string strPaymentType;//e.g. full prepay, face2face, divided, credit card, cash, etc
    std::pair<int,int> volumeRange;
};

class CProduct
{
private:
    //bool fValid;
public:
    CLink link;
    std::string id;
    std::string name;
    CLink icon;
    std::string intro;
    CLink introLink;
    std::map<string,string> mapAttribute;
    std::vector<std::string> vTag;
    uint32_t nExpireTime;
    CAmount price;
    std::vector<CPricing>vPrice;
    CAmount shipmentFee;
    std::vector<CPricing>vShipmentFee;
    CScript seller;
    std::vector<std::string> vSellerDomain;
    CScript recipient;
    CProduct()
    {
        price=0;
        shipmentFee=-1;
        //fValid=false;
        id="";
        name="";
        nExpireTime=0;
    }
    bool IsValid(){return id!=""&&name!="";}
    Value ToJson(bool fLinkOnly = false)const;
    string ToJsonString(bool fLinkOnly = false)const;
    bool SetJson(const Object& obj,string& strError);
    bool SetContent(const CContent content);
    CContent ToContent()const;
};
class CPaymentItem
{
public:
    cctype ccPaymentType;
    CLink linkPayTo;
    std::string productID; //or hash
    CAmount price;
    int nQuantity;
    string strMemo;
    CPaymentItem()
    {
        price=0;
        nQuantity=0;
        ccPaymentType=CC_PAYMENT_TYPE_PRODUCT;
    }
     bool IsValid();
    Value ToJson(bool fLinkOnly = false)const;
    string ToJsonString(bool fLinkOnly = false)const;
    bool SetJson(const Object& obj,string& strError);
    bool SetContent(const CContent content);
    CContent ToContent()const;
};

class CPayment
{
private:
    //bool fValid;
public:
    cctype ccPaymentType;
    CScript recipient;
    vector<CPaymentItem> vItems;
    uint256 hash;//contract hash
    string strMemo;
    CPayment()
    { 
        hash=uint256(0); 
        ccPaymentType=CC_PAYMENT_TYPE_SHOPPING;
    }
    bool IsValid();
    Value ToJson(bool fLinkOnly = false)const;
    string ToJsonString(bool fLinkOnly = false)const;
    bool SetJson(const Object& obj,string& strError);
    bool SetContent(const CContent content);
    CContent ToContent()const;
    CAmount GetTotalValue() const;
};
class CPaymentRequest
{
    
};
#endif
