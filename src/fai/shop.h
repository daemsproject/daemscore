#ifndef FAI_SHOP_H
#define FAI_SHOP_H

#include "fai/content.h"
#include "fai/domain.h"
using namespace json_spirit;
using namespace std;
using std::string;
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
    CDomain sellerDomain;
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
    //CAmount nPaid;
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
