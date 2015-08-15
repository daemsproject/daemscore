/* 
 * File:   cc.h
 * Author: alan
 *
 * Created on August 2, 2015, 3:37 PM
 */

#ifndef CCC_CC_H
#define	CCC_CC_H

#include <boost/assign.hpp>
#include <map>

static const int STANDARD_CONTENT_MAX_CC = 32;

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
enum servicecode
{
    SERVICE_FULLNODE=0,
    SERVICE_NAT=1,
    SERVICE_STUN=2,
    SERVICE_FULLNODEPLUS=3,
    SERVICE_ICQ=4,
    SERVICE_RELAY=5,
    SERVICE_SEARCHENGINE=7,
    SERVICE_MININGPOOL=8,
    SERVICE_OFFCHAIN_MESSAGE=9,
    SERVICE_OFFCHAIN_PRIVATE=10,
    SERVICE_OFFCHAIN_PUBLIC=11,
    SERVICE_MALL=13,
    SERVICE_BANK=14,
    SERVICE_TORRENTFILE=15,
    SERVICE_EMAIL=16,
    SERVICE_NOBLOCKCHAINDATA=30,
    SERVICE_APP=31,
    

};
static std::map<int,std::string> mapServiceNames=boost::assign::map_list_of
(SERVICE_FULLNODE,"full_node_service")
    (SERVICE_NAT,"NAT_peer")
    (SERVICE_STUN,"STUN_service")
    (SERVICE_FULLNODEPLUS,"full_node_plus_service")
    (SERVICE_ICQ,"ICQ_service")
    (SERVICE_RELAY,"relay_service")
    (SERVICE_SEARCHENGINE,"search_engine_service")
    (SERVICE_MININGPOOL,"mining_pool_service")
    (SERVICE_OFFCHAIN_MESSAGE,"offchain_message_storage_service")
    (SERVICE_OFFCHAIN_PRIVATE,"offchain_private_storage_service")
    (SERVICE_OFFCHAIN_PUBLIC,"offchain_public_storage_service")
    (SERVICE_MALL,"offchain_mall_service")
    (SERVICE_BANK,"bank_service")
    (SERVICE_TORRENTFILE,"torrent_file_service")
    (SERVICE_EMAIL,"email_service")
    (SERVICE_NOBLOCKCHAINDATA,"no_block_chain_data")
    (SERVICE_APP,"app_service");
enum pageid
{
    WALLETPAGE_ID=1,
    BROWSERPAGE_ID=2,
    PUBLISHERPAGE_ID=3,
    MESSENGERPAGE_ID=4,
    MINERPAGE_ID=5,
    DOMAINPAGE_ID=6,
    SETTINGPAGE_ID=7,
    SERVICEPAGE_ID=8,
    SHOPPAGE_ID=9,
    APPPAGE_ID=10,
    TOOLSPAGE_ID=11,
    DOWNLOADERPAGE_ID=12,
    HELPPAGE_ID=13,
    CUSTOMPAGE_ID=255
};
static std::map<int,std::string> mapPageNames=boost::assign::map_list_of
(WALLETPAGE_ID,"wallet")
(BROWSERPAGE_ID,"browser")
(PUBLISHERPAGE_ID,"publisher")
(MESSENGERPAGE_ID,"messenger")
(MINERPAGE_ID,"miner")
(DOMAINPAGE_ID,"domain")
(SETTINGPAGE_ID,"settings")
(SERVICEPAGE_ID,"service")
(SHOPPAGE_ID,"shop")
(APPPAGE_ID,"app")
(TOOLSPAGE_ID,"tools")
(DOWNLOADERPAGE_ID,"downloader")
(HELPPAGE_ID,"help")
;
static std::map<int,std::string> mapDefaultServiceDomain=boost::assign::map_list_of
            (SERVICE_FULLNODE,"")
    (SERVICE_NAT,"")
    (SERVICE_STUN,"stun.f")
    (SERVICE_FULLNODEPLUS,"fullnodeplus.f")
    (SERVICE_ICQ,"icq.f")
    (SERVICE_RELAY,"relay.f")
    (SERVICE_SEARCHENGINE,"search.f")
    (SERVICE_MININGPOOL,"pool.f")
    (SERVICE_OFFCHAIN_MESSAGE,"offchainmessage.f")
    (SERVICE_OFFCHAIN_PRIVATE,"offchainprivate.f")
    (SERVICE_OFFCHAIN_PUBLIC,"offchain.f")
    (SERVICE_MALL,"mall.f")
    (SERVICE_BANK,"bank.f")
    (SERVICE_TORRENTFILE,"torrent.f")
    (SERVICE_EMAIL,"email.f")
    (SERVICE_NOBLOCKCHAINDATA,"")
    (SERVICE_APP,"app.f");

static std::map<int,std::string> mapDefaultPageDomain=boost::assign::map_list_of
            (WALLETPAGE_ID,"wallet.f")
(BROWSERPAGE_ID,"browser.f")
(PUBLISHERPAGE_ID,"publisher.f")
(MESSENGERPAGE_ID,"messenger.f")
(DOMAINPAGE_ID,"domainpage.f")
(SETTINGPAGE_ID,"settings.f")
(SERVICEPAGE_ID,"servicepage.f")
(SHOPPAGE_ID,"shop.f")
(APPPAGE_ID,"app.f")
(TOOLSPAGE_ID,"tool.f")
(DOWNLOADERPAGE_ID,"downloader.f")
(HELPPAGE_ID,"help.f")
;
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
CC_NAME = 0x3e,
CC_NAME_P = 0x3f,
CC_SOURCE = 0x40,
CC_SOURCE_P = 0x41,
CC_RECIPIENT = 0x42,
CC_RECIPIENT_P = 0x43,
CC_POSITION = 0x46,
CC_POSITION_P = 0x47,
CC_COPYTO = 0x48,
CC_COPYTO_P = 0x49,
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
CC_ENCODINGSTRING = 0x3d00,
CC_ENCODING_UTF8 = 0x3d02,
CC_ENCODING_ASCII = 0x3d04,
CC_ENCODING_UTF16 = 0x3d06,
CC_ENCODING_UTF32 = 0x3d08,
CC_ENCODING_ISO8859_1 = 0x3d0b02,
CC_ENCODING_ISO8859_2 = 0x3d0b04,
CC_ENCODING_ISO8859_3 = 0x3d0b06,
CC_ENCODING_ISO8859_4 = 0x3d0b08,
CC_ENCODING_ISO8859_5 = 0x3d0b0a,
CC_ENCODING_ISO8859_6 = 0x3d0b0c,
CC_ENCODING_ISO8859_7 = 0x3d0b0e,
CC_ENCODING_ISO8859_8 = 0x3d0b10,
CC_ENCODING_ISO8859_9 = 0x3d0b12,
CC_ENCODING_ISO8859_10 = 0x3d0b14,
CC_ENCODING_ISO8859_11 = 0x3d0b16,
CC_ENCODING_ISO8859_12 = 0x3d0b18,
CC_ENCODING_ISO8859_13 = 0x3d0b1a,
CC_ENCODING_ISO8859_14 = 0x3d0b1c,
CC_ENCODING_ISO8859_15 = 0x3d0b1e,
CC_ENCODING_ISO8859_16 = 0x3d0b20,
CC_ENCODING_GB2312 = 0x3d0b22,
CC_ENCODING_GBK = 0x3d0b24,
CC_ENCODING_GB18030 = 0x3d0b26,
CC_ENCODING_BIG5 = 0x3d0b28,
CC_ENCODING_HKSCS = 0x3d0b2a,
CC_ENCODING_SHIFTJIS = 0x3d0b2c,
CC_ENCODING_EUCJP = 0x3d0b2e,
CC_ENCODING_ISO2022JP = 0x3d0b30,
CC_ENCODING_EUCJIS2004 = 0x3d0b32,
CC_ENCODING_ISO2022JP2004 = 0x3d0b34,
CC_ENCODING_KSX1001 = 0x3d0b36,
CC_ENCODING_EUCKR = 0x3d0b38,
CC_ENCODING_ISO2022KR = 0x3d0b3a,
CC_ENCODING_WINDOWS1250 = 0x3d0b3c,
CC_ENCODING_WINDOWS1251 = 0x3d0b3e,
CC_ENCODING_WINDOWS1252 = 0x3d0b40,
CC_ENCODING_WINDOWS1253 = 0x3d0b42,
CC_ENCODING_WINDOWS1254 = 0x3d0b44,
CC_ENCODING_WINDOWS1255 = 0x3d0b46,
CC_ENCODING_WINDOWS1256 = 0x3d0b48,
CC_ENCODING_WINDOWS1257 = 0x3d0b4a,
CC_ENCODING_WINDOWS1258 = 0x3d0b4c,
CC_TEXT_TYPESTRING = 0x0510,
CC_TEXT_TYPE_TEXT = 0x0512,
CC_TEXT_TYPE_HTML = 0x0514,
CC_TEXT_TYPE_XML = 0x0516,
CC_TEXT_TYPE_JSON = 0x0518,
CC_TEXT_TYPE_YAML = 0x051a,
CC_TEXT_TYPE_CSV = 0x051c,
CC_NUMBER_ENCODING_STRING = 0x3b00,
CC_NUMBER_ENCODING_DEC = 0x3b02,
CC_NUMBER_ENCODING_INT_LE = 0x3b04,
CC_NUMBER_ENCODING_FLOAT_LE = 0x3b06,
CC_NUMBER_ENCODING_DOUBLE_LE = 0x3b08,
CC_NUMBER_ENCODING_VARINT = 0x3b10,
CC_NUMBER_ENCODING_DEC_SCIENTIFIC = 0x3b12,
CC_NUMBER_ENCODING_BINARY_SCIENTIFIC = 0x3b14,
// File
CC_FILE_TYPESTRING = 0x0702,
CC_FILE_PART = 0x0704,
CC_FILE_PART_P = 0x0705,
CC_FILE_PART_NUMBER = 0x070502,
CC_FILE_COMBINE_P = 0x0707,
CC_FILE_PACKAGE_P = 0x0709,
CC_FILE_PACKAGE_MAINFILE = 0x070900,
// Link
CC_LINK_TYPESTRING = 0x0900,
CC_LINK_TYPE_NATIVE = 0x0902,
CC_LINK_TYPE_BLOCKCHAIN = 0x0904,
CC_LINK_TYPE_TXIDOUT = 0x0906,
CC_LINK_TYPE_DOMAIN = 0x0908,
CC_LINK_TYPE_SCRIPTPUBKEY = 0x0910,
CC_LINK_TYPE_FILEPACKAGE = 0x0912,
CC_LINK_TYPE_COINTO = 0x0914,
CC_LINK_TYPE_HTTP = 0x0916,
CC_LINK_TYPE_HTTPS = 0x0918,
CC_LINK_TYPE_MAILTO = 0x0920,
CC_LINK_TYPE_FTP = 0x0922,
CC_LINK_TYPE_FILE = 0x0924,
CC_LINK_TYPE_CRID = 0x0926,
CC_LINK_TYPE_ED2K = 0x0928,
CC_LINK_TYPE_MAGNET = 0x0930,
CC_LINK_TYPE_UNKNOWN = 0x0932,
CC_LINK_DISPLAY = 0x0980,
CC_LINK_GETN = 0x0982,
CC_LINK_GETN_TEXT = 0x0984,
CC_LINK_GETN_FILE = 0x0986,
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
CC_PAYMENT_REQ = 0x1100,
CC_PAYMENT_REQ_P = 0x1101,
CC_PAYMENT_REQ_VALIDTILL = 0x110102,
CC_PAYMENT_TYPE = 0x1102,
CC_PAYMENT_TYPE_P = 0x1103,
CC_PAYMENT_RECIPIENT = 0x1104,
CC_PAYMENT_MEMO = 0x1106,
CC_PAYMENT_MEMO_P = 0x1107,
CC_PAYMENT_DELIVERADD = 0x1108,
CC_PAYMENT_DELIVERADD_P = 0x1109,
CC_PAYMENT_REFUND_REQ = 0x110a,
CC_PAYMENT_REFUND_REQ_P = 0x110b,
CC_PAYMENT_ITEM_P = 0x110f,
CC_PAYMENT_LINK = 0x1110,
CC_PAYMENT_LINK_P = 0x1111,
CC_PAYMENT_TYPE_SHOPPING = 0x110300,
CC_PAYMENT_TYPE_PRODUCT = 0x110302,
CC_PAYMENT_TYPE_P2REQ = 0x110304,
CC_PAYMENT_TYPE_REFUND = 0x110306,
CC_PAYMENT_TYPE_SHIPMENTFEE = 0x110308,
CC_PAYMENT_TYPE_P2AGREEMENT = 0x110310,
CC_PAYMENT_TYPE_DUTCH = 0x110312,
CC_PAYMENT_TYPE_LEND = 0x110314,
CC_PAYMENT_TYPE_RETURN = 0x110316,
CC_PAYMENT_TYPE_RENT = 0x110318,
CC_PAYMENT_TYPE_INTEREST = 0x11031a,
// Lang
CC_LANGSTRING = 0x12,
CC_LANG_IETF = 0x1300,
CC_LANG_ISO639_1 = 0x1302,
CC_LANG_ISO639_2 = 0x1304,
CC_LANG_ISO639_3 = 0x1306,
CC_LANG_COUNTRY = 0x1308,
CC_LANG_COUNTRYSTRING = 0x1308,
CC_LANG_AB = 0x130002,
CC_LANG_AA = 0x130004,
CC_LANG_AF = 0x130006,
CC_LANG_AK = 0x130008,
CC_LANG_SQ = 0x13000a,
CC_LANG_AM = 0x13000c,
CC_LANG_AR = 0x13000e,
CC_LANG_AN = 0x130010,
CC_LANG_HY = 0x130012,
CC_LANG_AS = 0x130014,
CC_LANG_AV = 0x130016,
CC_LANG_AE = 0x130018,
CC_LANG_AY = 0x13001a,
CC_LANG_AZ = 0x13001c,
CC_LANG_BM = 0x13001e,
CC_LANG_BA = 0x130020,
CC_LANG_EU = 0x130022,
CC_LANG_BE = 0x130024,
CC_LANG_BN = 0x130026,
CC_LANG_BH = 0x130028,
CC_LANG_BI = 0x13002a,
CC_LANG_BS = 0x13002c,
CC_LANG_BR = 0x13002e,
CC_LANG_BG = 0x130030,
CC_LANG_MY = 0x130032,
CC_LANG_CA = 0x130034,
CC_LANG_CH = 0x130036,
CC_LANG_CE = 0x130038,
CC_LANG_NY = 0x13003a,
CC_LANG_ZH = 0x13003c,
CC_LANG_CV = 0x13003e,
CC_LANG_KW = 0x130040,
CC_LANG_CO = 0x130042,
CC_LANG_CR = 0x130044,
CC_LANG_HR = 0x130046,
CC_LANG_CS = 0x130048,
CC_LANG_DA = 0x13004a,
CC_LANG_DV = 0x13004c,
CC_LANG_NL = 0x13004e,
CC_LANG_DZ = 0x130050,
CC_LANG_EN = 0x130052,
CC_LANG_EO = 0x130054,
CC_LANG_ET = 0x130056,
CC_LANG_EE = 0x130058,
CC_LANG_FO = 0x13005a,
CC_LANG_FJ = 0x13005c,
CC_LANG_FI = 0x13005e,
CC_LANG_FR = 0x130060,
CC_LANG_FF = 0x130062,
CC_LANG_GL = 0x130064,
CC_LANG_KA = 0x130066,
CC_LANG_DE = 0x130068,
CC_LANG_EL = 0x13006a,
CC_LANG_GN = 0x13006c,
CC_LANG_GU = 0x13006e,
CC_LANG_HT = 0x130070,
CC_LANG_HA = 0x130072,
CC_LANG_HE = 0x130074,
CC_LANG_HZ = 0x130076,
CC_LANG_HI = 0x130078,
CC_LANG_HO = 0x13007a,
CC_LANG_HU = 0x13007c,
CC_LANG_IA = 0x13007e,
CC_LANG_ID = 0x130080,
CC_LANG_IE = 0x130082,
CC_LANG_GA = 0x130084,
CC_LANG_IG = 0x130086,
CC_LANG_IK = 0x130088,
CC_LANG_IO = 0x13008a,
CC_LANG_IS = 0x13008c,
CC_LANG_IT = 0x13008e,
CC_LANG_IU = 0x130090,
CC_LANG_JA = 0x130092,
CC_LANG_JV = 0x130094,
CC_LANG_KL = 0x130096,
CC_LANG_KN = 0x130098,
CC_LANG_KR = 0x13009a,
CC_LANG_KS = 0x13009c,
CC_LANG_KK = 0x13009e,
CC_LANG_KM = 0x1300a0,
CC_LANG_KI = 0x1300a2,
CC_LANG_RW = 0x1300a4,
CC_LANG_KY = 0x1300a6,
CC_LANG_KV = 0x1300a8,
CC_LANG_KG = 0x1300aa,
CC_LANG_KO = 0x1300ac,
CC_LANG_KU = 0x1300ae,
CC_LANG_KJ = 0x1300b0,
CC_LANG_LA = 0x1300b2,
CC_LANG_LB = 0x1300b4,
CC_LANG_LG = 0x1300b6,
CC_LANG_LI = 0x1300b8,
CC_LANG_LN = 0x1300ba,
CC_LANG_LO = 0x1300bc,
CC_LANG_LT = 0x1300be,
CC_LANG_LU = 0x1300c0,
CC_LANG_LV = 0x1300c2,
CC_LANG_GV = 0x1300c4,
CC_LANG_MK = 0x1300c6,
CC_LANG_MG = 0x1300c8,
CC_LANG_MS = 0x1300ca,
CC_LANG_ML = 0x1300cc,
CC_LANG_MT = 0x1300ce,
CC_LANG_MI = 0x1300d0,
CC_LANG_MR = 0x1300d2,
CC_LANG_MH = 0x1300d4,
CC_LANG_MN = 0x1300d6,
CC_LANG_NA = 0x1300d8,
CC_LANG_NV = 0x1300da,
CC_LANG_ND = 0x1300dc,
CC_LANG_NE = 0x1300de,
CC_LANG_NG = 0x1300e0,
CC_LANG_NB = 0x1300e2,
CC_LANG_NN = 0x1300e4,
CC_LANG_NO = 0x1300e6,
CC_LANG_II = 0x1300e8,
CC_LANG_NR = 0x1300ea,
CC_LANG_OC = 0x1300ec,
CC_LANG_OJ = 0x1300ee,
CC_LANG_CU = 0x1300f0,
CC_LANG_OM = 0x1300f2,
CC_LANG_OR = 0x1300f4,
CC_LANG_OS = 0x1300f6,
CC_LANG_PA = 0x1300f8,
CC_LANG_PI = 0x1300fa,
CC_LANG_FA = 0x1300fc,
CC_LANG_PL = 0x1300fe,
CC_LANG_PS = 0x130100,
CC_LANG_PT = 0x130102,
CC_LANG_QU = 0x130104,
CC_LANG_RM = 0x130106,
CC_LANG_RN = 0x130108,
CC_LANG_RO = 0x13010a,
CC_LANG_RU = 0x13010c,
CC_LANG_SA = 0x13010e,
CC_LANG_SC = 0x130110,
CC_LANG_SD = 0x130112,
CC_LANG_SE = 0x130114,
CC_LANG_SM = 0x130116,
CC_LANG_SG = 0x130118,
CC_LANG_SR = 0x13011a,
CC_LANG_GD = 0x13011c,
CC_LANG_SN = 0x13011e,
CC_LANG_SI = 0x130120,
CC_LANG_SK = 0x130122,
CC_LANG_SL = 0x130124,
CC_LANG_SO = 0x130126,
CC_LANG_ST = 0x130128,
CC_LANG_ES = 0x13012a,
CC_LANG_SU = 0x13012c,
CC_LANG_SW = 0x13012e,
CC_LANG_SS = 0x130130,
CC_LANG_SV = 0x130132,
CC_LANG_TA = 0x130134,
CC_LANG_TE = 0x130136,
CC_LANG_TG = 0x130138,
CC_LANG_TH = 0x13013a,
CC_LANG_TI = 0x13013c,
CC_LANG_BO = 0x13013e,
CC_LANG_TK = 0x130140,
CC_LANG_TL = 0x130142,
CC_LANG_TN = 0x130144,
CC_LANG_TO = 0x130146,
CC_LANG_TR = 0x130148,
CC_LANG_TS = 0x13014a,
CC_LANG_TT = 0x13014c,
CC_LANG_TW = 0x13014e,
CC_LANG_TY = 0x130150,
CC_LANG_UG = 0x130152,
CC_LANG_UK = 0x130154,
CC_LANG_UR = 0x130156,
CC_LANG_UZ = 0x130158,
CC_LANG_VE = 0x13015a,
CC_LANG_VI = 0x13015c,
CC_LANG_VO = 0x13015e,
CC_LANG_WA = 0x130160,
CC_LANG_CY = 0x130162,
CC_LANG_WO = 0x130164,
CC_LANG_FY = 0x130166,
CC_LANG_XH = 0x130168,
CC_LANG_YI = 0x13016a,
CC_LANG_YO = 0x13016c,
CC_LANG_ZA = 0x13016e,
CC_LANG_ZU = 0x130170,

CC_ENCRYPT_PARAMS = 0x1502,
CC_ENCRYPT_PARAMS_P = 0x1503,
CC_ENCRYPT_PARAMS_IV = 0x150002,
CC_ENCRYPT_PARAMS_SALT = 0x150004,
CC_ENCRYPT_PARAMS_SCRYPT_N = 0x150006,
CC_ENCRYPT_PARAMS_SCRYPT_P = 0x150008,
CC_ENCRYPT_PARAMS_SCRYPT_R = 0x15000a,
CC_ENCRYPT_PARAMS_AES_ITERATIONS = 0x15000c,
CC_ENCRYPT_PARAMS_ALGORITHM_STRING = 0x1504,
CC_ENCRYPT_PARAMS_ALGORITHM_SECRETCHAT = 0x150402,
CC_ENCRYPT_PARAMS_ALGORITHM_AES = 0x150404,
CC_ENCRYPT_PARAMS_ALGORITHM_SCRYPT = 0x150406,
CC_ENCRYPT_PARAMS_ALGORITHM_MHASH = 0x150408,
CC_ENCRYPT_PARAMS_MHASH_HEIGHT = 0x15000e,
//PRODUCT
CC_PRODUCT_PRICE = 0x1700,
CC_PRODUCT_PRICE_P = 0x1701,
CC_PRODUCT_ID = 0x1702,
CC_PRODUCT_PAYTO = 0x1706,
CC_PRODUCT_INTRO = 0x1708,
CC_PRODUCT_ICON = 0x170a,
CC_PRODUCT_ICON_P = 0x170b,
CC_PRODUCT_ATTIRBUTE = 0x170c,
CC_PRODUCT_ATTIRBUTE_P = 0x170d,
CC_PRODUCT_ATTIRBUTE_TYPE = 0x170c00,
CC_PRODUCT_UNIT_STRING = 0x170e,
CC_PRODUCT_UNIT_P = 0x170f,
CC_PRODUCT_UNIT_PIECE = 0x170f00,
CC_PRODUCT_SHIPMENTFEE = 0x1710,
CC_PRODUCT_SHIPMENTFEE_P = 0x1711,
CC_PRODUCT_EXPIRETIME = 0x1712,
CC_TIME_START = 0x2302,
CC_TIME_END = 0x2304,
CC_TIME_CREATED = 0x2306,
CC_TIME_PUBLISHED = 0x2308,
CC_TIME_TIMEZONE = 0x2310,
CC_BANK_BANKCODE = 0x2500,



CC_CONTACT_PHONE = 0x2900,
CC_CONTACT_EMAIL = 0x2902,
CC_CONTACT_BLOCKCHAIN = 0x2904,

CC_POSITION_LONGITUDE = 0x4702,
CC_POSITION_LATITUDE = 0x4704,
CC_POSITION_ALTITUDE = 0x4706,
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
(CC_NAME,"CC_NAME")
(CC_NAME_P,"CC_NAME_P")
(CC_SOURCE,"CC_SOURCE")
(CC_SOURCE_P,"CC_SOURCE_P")
(CC_RECIPIENT,"CC_RECIPIENT")
(CC_RECIPIENT_P,"CC_RECIPIENT_P")
(CC_POSITION,"CC_POSITION")
(CC_POSITION_P,"CC_POSITION_P")
(CC_COPYTO,"CC_COPYTO")
(CC_COPYTO_P,"CC_COPYTO_P")
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
(CC_ENCODINGSTRING,"CC_ENCODINGSTRING")
(CC_ENCODING_UTF8,"CC_ENCODING_UTF8")
(CC_ENCODING_ASCII,"CC_ENCODING_ASCII")
(CC_ENCODING_UTF16,"CC_ENCODING_UTF16")
(CC_ENCODING_UTF32,"CC_ENCODING_UTF32")
(CC_ENCODING_ISO8859_1,"CC_ENCODING_ISO8859_1")
(CC_ENCODING_ISO8859_2,"CC_ENCODING_ISO8859_2")
(CC_ENCODING_ISO8859_3,"CC_ENCODING_ISO8859_3")
(CC_ENCODING_ISO8859_4,"CC_ENCODING_ISO8859_4")
(CC_ENCODING_ISO8859_5,"CC_ENCODING_ISO8859_5")
(CC_ENCODING_ISO8859_6,"CC_ENCODING_ISO8859_6")
(CC_ENCODING_ISO8859_7,"CC_ENCODING_ISO8859_7")
(CC_ENCODING_ISO8859_8,"CC_ENCODING_ISO8859_8")
(CC_ENCODING_ISO8859_9,"CC_ENCODING_ISO8859_9")
(CC_ENCODING_ISO8859_10,"CC_ENCODING_ISO8859_10")
(CC_ENCODING_ISO8859_11,"CC_ENCODING_ISO8859_11")
(CC_ENCODING_ISO8859_12,"CC_ENCODING_ISO8859_12")
(CC_ENCODING_ISO8859_13,"CC_ENCODING_ISO8859_13")
(CC_ENCODING_ISO8859_14,"CC_ENCODING_ISO8859_14")
(CC_ENCODING_ISO8859_15,"CC_ENCODING_ISO8859_15")
(CC_ENCODING_ISO8859_16,"CC_ENCODING_ISO8859_16")
(CC_ENCODING_GB2312,"CC_ENCODING_GB2312")
(CC_ENCODING_GBK,"CC_ENCODING_GBK")
(CC_ENCODING_GB18030,"CC_ENCODING_GB18030")
(CC_ENCODING_BIG5,"CC_ENCODING_BIG5")
(CC_ENCODING_HKSCS,"CC_ENCODING_HKSCS")
(CC_ENCODING_SHIFTJIS,"CC_ENCODING_SHIFTJIS")
(CC_ENCODING_EUCJP,"CC_ENCODING_EUCJP")
(CC_ENCODING_ISO2022JP,"CC_ENCODING_ISO2022JP")
(CC_ENCODING_EUCJIS2004,"CC_ENCODING_EUCJIS2004")
(CC_ENCODING_ISO2022JP2004,"CC_ENCODING_ISO2022JP2004")
(CC_ENCODING_KSX1001,"CC_ENCODING_KSX1001")
(CC_ENCODING_EUCKR,"CC_ENCODING_EUCKR")
(CC_ENCODING_ISO2022KR,"CC_ENCODING_ISO2022KR")
(CC_ENCODING_WINDOWS1250,"CC_ENCODING_WINDOWS1250")
(CC_ENCODING_WINDOWS1251,"CC_ENCODING_WINDOWS1251")
(CC_ENCODING_WINDOWS1252,"CC_ENCODING_WINDOWS1252")
(CC_ENCODING_WINDOWS1253,"CC_ENCODING_WINDOWS1253")
(CC_ENCODING_WINDOWS1254,"CC_ENCODING_WINDOWS1254")
(CC_ENCODING_WINDOWS1255,"CC_ENCODING_WINDOWS1255")
(CC_ENCODING_WINDOWS1256,"CC_ENCODING_WINDOWS1256")
(CC_ENCODING_WINDOWS1257,"CC_ENCODING_WINDOWS1257")
(CC_ENCODING_WINDOWS1258,"CC_ENCODING_WINDOWS1258")
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
(CC_FILE_TYPESTRING,"CC_FILE_TYPESTRING")
(CC_FILE_PART,"CC_FILE_PART")
(CC_FILE_PART_P,"CC_FILE_PART_P")
(CC_FILE_PART_NUMBER,"CC_FILE_PART_NUMBER")
(CC_FILE_COMBINE_P,"CC_FILE_COMBINE_P")
(CC_FILE_PACKAGE_P,"CC_FILE_PACKAGE_P")
(CC_FILE_PACKAGE_MAINFILE,"CC_FILE_PACKAGE_MAINFILE")
// Link
(CC_LINK_TYPESTRING,"CC_LINK_TYPESTRING")
(CC_LINK_TYPE_NATIVE,"CC_LINK_TYPE_NATIVE")
(CC_LINK_TYPE_BLOCKCHAIN,"CC_LINK_TYPE_BLOCKCHAIN")
(CC_LINK_TYPE_TXIDOUT,"CC_LINK_TYPE_TXIDOUT")
(CC_LINK_TYPE_DOMAIN,"CC_LINK_TYPE_DOMAIN")
(CC_LINK_TYPE_SCRIPTPUBKEY,"CC_LINK_TYPE_SCRIPTPUBKEY")
(CC_LINK_TYPE_FILEPACKAGE,"CC_LINK_TYPE_FILEPACKAGE")
(CC_LINK_TYPE_COINTO,"CC_LINK_TYPE_COINTO")
(CC_LINK_TYPE_HTTP,"CC_LINK_TYPE_HTTP")
(CC_LINK_TYPE_HTTPS,"CC_LINK_TYPE_HTTPS")
(CC_LINK_TYPE_MAILTO,"CC_LINK_TYPE_MAILTO")
(CC_LINK_TYPE_FTP,"CC_LINK_TYPE_FTP")
(CC_LINK_TYPE_FILE,"CC_LINK_TYPE_FILE")
(CC_LINK_TYPE_CRID,"CC_LINK_TYPE_CRID")
(CC_LINK_TYPE_ED2K,"CC_LINK_TYPE_ED2K")
(CC_LINK_TYPE_MAGNET,"CC_LINK_TYPE_MAGNET")
(CC_LINK_TYPE_UNKNOWN,"CC_LINK_TYPE_UNKNOWN")
(CC_LINK_DISPLAY,"CC_LINK_DISPLAY")
(CC_LINK_GETN,"CC_LINK_GETN")
(CC_LINK_GETN_TEXT,"CC_LINK_GETN_TEXT")
(CC_LINK_GETN_FILE,"CC_LINK_GETN_FILE")
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
(CC_COMMENT_SCORESTRING,"CC_COMMENT_SCORESTRING")
(CC_COMMENT_SCORE_0,"CC_COMMENT_SCORE_0")
(CC_COMMENT_SCORE_1,"CC_COMMENT_SCORE_1")
(CC_COMMENT_SCORE_2,"CC_COMMENT_SCORE_2")
(CC_COMMENT_SCORE_3,"CC_COMMENT_SCORE_3")
(CC_COMMENT_SCORE_4,"CC_COMMENT_SCORE_4")
(CC_COMMENT_SCORE_5,"CC_COMMENT_SCORE_5")
(CC_COMMENT_SCORE_6,"CC_COMMENT_SCORE_6")
(CC_COMMENT_SCORE_7,"CC_COMMENT_SCORE_7")
(CC_COMMENT_SCORE_8,"CC_COMMENT_SCORE_8")
(CC_COMMENT_SCORE_9,"CC_COMMENT_SCORE_9")
(CC_COMMENT_SCORE_10,"CC_COMMENT_SCORE_10")
(CC_COMMENT_SCORE_P,"CC_COMMENT_SCORE_P")
// Payment
(CC_PAYMENT_REQ,"CC_PAYMENT_REQ")
(CC_PAYMENT_REQ_P,"CC_PAYMENT_REQ_P")
(CC_PAYMENT_REQ_VALIDTILL,"CC_PAYMENT_REQ_VALIDTILL")
(CC_PAYMENT_TYPE,"CC_PAYMENT_TYPE")
(CC_PAYMENT_TYPE_P,"CC_PAYMENT_TYPE_P")
(CC_PAYMENT_RECIPIENT,"CC_PAYMENT_RECIPIENT")
(CC_PAYMENT_MEMO,"CC_PAYMENT_MEMO")
(CC_PAYMENT_MEMO_P,"CC_PAYMENT_MEMO_P")
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

(CC_LANG_AB,"CC_LANG_AB")
(CC_LANG_AA,"CC_LANG_AA")
(CC_LANG_AF,"CC_LANG_AF")
(CC_LANG_AK,"CC_LANG_AK")
(CC_LANG_SQ,"CC_LANG_SQ")
(CC_LANG_AM,"CC_LANG_AM")
(CC_LANG_AR,"CC_LANG_AR")
(CC_LANG_AN,"CC_LANG_AN")
(CC_LANG_HY,"CC_LANG_HY")
(CC_LANG_AS,"CC_LANG_AS")
(CC_LANG_AV,"CC_LANG_AV")
(CC_LANG_AE,"CC_LANG_AE")
(CC_LANG_AY,"CC_LANG_AY")
(CC_LANG_AZ,"CC_LANG_AZ")
(CC_LANG_BM,"CC_LANG_BM")
(CC_LANG_BA,"CC_LANG_BA")
(CC_LANG_EU,"CC_LANG_EU")
(CC_LANG_BE,"CC_LANG_BE")
(CC_LANG_BN,"CC_LANG_BN")
(CC_LANG_BH,"CC_LANG_BH")
(CC_LANG_BI,"CC_LANG_BI")
(CC_LANG_BS,"CC_LANG_BS")
(CC_LANG_BR,"CC_LANG_BR")
(CC_LANG_BG,"CC_LANG_BG")
(CC_LANG_MY,"CC_LANG_MY")
(CC_LANG_CA,"CC_LANG_CA")
(CC_LANG_CH,"CC_LANG_CH")
(CC_LANG_CE,"CC_LANG_CE")
(CC_LANG_NY,"CC_LANG_NY")
(CC_LANG_ZH,"CC_LANG_ZH")
(CC_LANG_CV,"CC_LANG_CV")
(CC_LANG_KW,"CC_LANG_KW")
(CC_LANG_CO,"CC_LANG_CO")
(CC_LANG_CR,"CC_LANG_CR")
(CC_LANG_HR,"CC_LANG_HR")
(CC_LANG_CS,"CC_LANG_CS")
(CC_LANG_DA,"CC_LANG_DA")
(CC_LANG_DV,"CC_LANG_DV")
(CC_LANG_NL,"CC_LANG_NL")
(CC_LANG_DZ,"CC_LANG_DZ")
(CC_LANG_EN,"CC_LANG_EN")
(CC_LANG_EO,"CC_LANG_EO")
(CC_LANG_ET,"CC_LANG_ET")
(CC_LANG_EE,"CC_LANG_EE")
(CC_LANG_FO,"CC_LANG_FO")
(CC_LANG_FJ,"CC_LANG_FJ")
(CC_LANG_FI,"CC_LANG_FI")
(CC_LANG_FR,"CC_LANG_FR")
(CC_LANG_FF,"CC_LANG_FF")
(CC_LANG_GL,"CC_LANG_GL")
(CC_LANG_KA,"CC_LANG_KA")
(CC_LANG_DE,"CC_LANG_DE")
(CC_LANG_EL,"CC_LANG_EL")
(CC_LANG_GN,"CC_LANG_GN")
(CC_LANG_GU,"CC_LANG_GU")
(CC_LANG_HT,"CC_LANG_HT")
(CC_LANG_HA,"CC_LANG_HA")
(CC_LANG_HE,"CC_LANG_HE")
(CC_LANG_HZ,"CC_LANG_HZ")
(CC_LANG_HI,"CC_LANG_HI")
(CC_LANG_HO,"CC_LANG_HO")
(CC_LANG_HU,"CC_LANG_HU")
(CC_LANG_IA,"CC_LANG_IA")
(CC_LANG_ID,"CC_LANG_ID")
(CC_LANG_IE,"CC_LANG_IE")
(CC_LANG_GA,"CC_LANG_GA")
(CC_LANG_IG,"CC_LANG_IG")
(CC_LANG_IK,"CC_LANG_IK")
(CC_LANG_IO,"CC_LANG_IO")
(CC_LANG_IS,"CC_LANG_IS")
(CC_LANG_IT,"CC_LANG_IT")
(CC_LANG_IU,"CC_LANG_IU")
(CC_LANG_JA,"CC_LANG_JA")
(CC_LANG_JV,"CC_LANG_JV")
(CC_LANG_KL,"CC_LANG_KL")
(CC_LANG_KN,"CC_LANG_KN")
(CC_LANG_KR,"CC_LANG_KR")
(CC_LANG_KS,"CC_LANG_KS")
(CC_LANG_KK,"CC_LANG_KK")
(CC_LANG_KM,"CC_LANG_KM")
(CC_LANG_KI,"CC_LANG_KI")
(CC_LANG_RW,"CC_LANG_RW")
(CC_LANG_KY,"CC_LANG_KY")
(CC_LANG_KV,"CC_LANG_KV")
(CC_LANG_KG,"CC_LANG_KG")
(CC_LANG_KO,"CC_LANG_KO")
(CC_LANG_KU,"CC_LANG_KU")
(CC_LANG_KJ,"CC_LANG_KJ")
(CC_LANG_LA,"CC_LANG_LA")
(CC_LANG_LB,"CC_LANG_LB")
(CC_LANG_LG,"CC_LANG_LG")
(CC_LANG_LI,"CC_LANG_LI")
(CC_LANG_LN,"CC_LANG_LN")
(CC_LANG_LO,"CC_LANG_LO")
(CC_LANG_LT,"CC_LANG_LT")
(CC_LANG_LU,"CC_LANG_LU")
(CC_LANG_LV,"CC_LANG_LV")
(CC_LANG_GV,"CC_LANG_GV")
(CC_LANG_MK,"CC_LANG_MK")
(CC_LANG_MG,"CC_LANG_MG")
(CC_LANG_MS,"CC_LANG_MS")
(CC_LANG_ML,"CC_LANG_ML")
(CC_LANG_MT,"CC_LANG_MT")
(CC_LANG_MI,"CC_LANG_MI")
(CC_LANG_MR,"CC_LANG_MR")
(CC_LANG_MH,"CC_LANG_MH")
(CC_LANG_MN,"CC_LANG_MN")
(CC_LANG_NA,"CC_LANG_NA")
(CC_LANG_NV,"CC_LANG_NV")
(CC_LANG_ND,"CC_LANG_ND")
(CC_LANG_NE,"CC_LANG_NE")
(CC_LANG_NG,"CC_LANG_NG")
(CC_LANG_NB,"CC_LANG_NB")
(CC_LANG_NN,"CC_LANG_NN")
(CC_LANG_NO,"CC_LANG_NO")
(CC_LANG_II,"CC_LANG_II")
(CC_LANG_NR,"CC_LANG_NR")
(CC_LANG_OC,"CC_LANG_OC")
(CC_LANG_OJ,"CC_LANG_OJ")
(CC_LANG_CU,"CC_LANG_CU")
(CC_LANG_OM,"CC_LANG_OM")
(CC_LANG_OR,"CC_LANG_OR")
(CC_LANG_OS,"CC_LANG_OS")
(CC_LANG_PA,"CC_LANG_PA")
(CC_LANG_PI,"CC_LANG_PI")
(CC_LANG_FA,"CC_LANG_FA")
(CC_LANG_PL,"CC_LANG_PL")
(CC_LANG_PS,"CC_LANG_PS")
(CC_LANG_PT,"CC_LANG_PT")
(CC_LANG_QU,"CC_LANG_QU")
(CC_LANG_RM,"CC_LANG_RM")
(CC_LANG_RN,"CC_LANG_RN")
(CC_LANG_RO,"CC_LANG_RO")
(CC_LANG_RU,"CC_LANG_RU")
(CC_LANG_SA,"CC_LANG_SA")
(CC_LANG_SC,"CC_LANG_SC")
(CC_LANG_SD,"CC_LANG_SD")
(CC_LANG_SE,"CC_LANG_SE")
(CC_LANG_SM,"CC_LANG_SM")
(CC_LANG_SG,"CC_LANG_SG")
(CC_LANG_SR,"CC_LANG_SR")
(CC_LANG_GD,"CC_LANG_GD")
(CC_LANG_SN,"CC_LANG_SN")
(CC_LANG_SI,"CC_LANG_SI")
(CC_LANG_SK,"CC_LANG_SK")
(CC_LANG_SL,"CC_LANG_SL")
(CC_LANG_SO,"CC_LANG_SO")
(CC_LANG_ST,"CC_LANG_ST")
(CC_LANG_ES,"CC_LANG_ES")
(CC_LANG_SU,"CC_LANG_SU")
(CC_LANG_SW,"CC_LANG_SW")
(CC_LANG_SS,"CC_LANG_SS")
(CC_LANG_SV,"CC_LANG_SV")
(CC_LANG_TA,"CC_LANG_TA")
(CC_LANG_TE,"CC_LANG_TE")
(CC_LANG_TG,"CC_LANG_TG")
(CC_LANG_TH,"CC_LANG_TH")
(CC_LANG_TI,"CC_LANG_TI")
(CC_LANG_BO,"CC_LANG_BO")
(CC_LANG_TK,"CC_LANG_TK")
(CC_LANG_TL,"CC_LANG_TL")
(CC_LANG_TN,"CC_LANG_TN")
(CC_LANG_TO,"CC_LANG_TO")
(CC_LANG_TR,"CC_LANG_TR")
(CC_LANG_TS,"CC_LANG_TS")
(CC_LANG_TT,"CC_LANG_TT")
(CC_LANG_TW,"CC_LANG_TW")
(CC_LANG_TY,"CC_LANG_TY")
(CC_LANG_UG,"CC_LANG_UG")
(CC_LANG_UK,"CC_LANG_UK")
(CC_LANG_UR,"CC_LANG_UR")
(CC_LANG_UZ,"CC_LANG_UZ")
(CC_LANG_VE,"CC_LANG_VE")
(CC_LANG_VI,"CC_LANG_VI")
(CC_LANG_VO,"CC_LANG_VO")
(CC_LANG_WA,"CC_LANG_WA")
(CC_LANG_CY,"CC_LANG_CY")
(CC_LANG_WO,"CC_LANG_WO")
(CC_LANG_FY,"CC_LANG_FY")
(CC_LANG_XH,"CC_LANG_XH")
(CC_LANG_YI,"CC_LANG_YI")
(CC_LANG_YO,"CC_LANG_YO")
(CC_LANG_ZA,"CC_LANG_ZA")
(CC_LANG_ZU,"CC_LANG_ZU")
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
(CC_TIME_START,"CC_TIME_START")
(CC_TIME_END,"CC_TIME_END")
(CC_TIME_CREATED,"CC_TIME_CREATED")
(CC_TIME_PUBLISHED,"CC_TIME_PUBLISHED")
(CC_TIME_TIMEZONE,"CC_TIME_TIMEZONE")
(CC_BANK_BANKCODE,"CC_BANK_BANKCODE")



(CC_CONTACT_PHONE,"CC_CONTACT_PHONE")
(CC_CONTACT_EMAIL,"CC_CONTACT_EMAIL")
(CC_CONTACT_BLOCKCHAIN,"CC_CONTACT_BLOCKCHAIN")

(CC_POSITION_LONGITUDE,"CC_POSITION_LONGITUDE")
(CC_POSITION_LATITUDE,"CC_POSITION_LATITUDE")
(CC_POSITION_ALTITUDE,"CC_POSITION_ALTITUDE")
;
#endif	/* CC_H */

