#include "shop.h"
#include "utilstrencodings.h"
#include <string.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <stdio.h>
#include "util.h"
#include "utiltime.h"
#include "base58.h"
#include "streams.h"


#include "json/json_spirit_writer_template.h"
using namespace boost;
using namespace std;


Value CProduct::ToJson(bool fLinkOnly)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("link",Value(link.ToString())));
    obj.push_back(Pair("id",Value(id)));
    obj.push_back(Pair("name",Value(name)));    
    obj.push_back(Pair("price",_ValueFromAmount(price)));
    if(shipmentFee!=-1)
        obj.push_back(Pair("shipmentfee",_ValueFromAmount(shipmentFee)));    
    if(recipient.size()>0)
    {
        string strID;
        ScriptPubKeyToString(recipient, strID);
        obj.push_back(Pair("recipient",Value(strID)));    
    }
    if(seller.size()>0)
    {
        Object objSeller;
        string strID;
        ScriptPubKeyToString(seller, strID);
        objSeller.push_back(Pair("id",Value(strID)));    
        if(sellerDomain.strDomain.size()>0)
        {
            
            objSeller.push_back(Pair("domain",sellerDomain.ToJson()));
        }
        obj.push_back(Pair("seller",objSeller));  
    }
    if(!icon.IsEmpty())
        obj.push_back(Pair("icon",Value(icon.ToString()))); 
    if(intro.size()>0)
        obj.push_back(Pair("intro",Value(intro)));
    if(nExpireTime>0)
        obj.push_back(Pair("expiretime",Value((uint64_t)nExpireTime)));
    Array arrTag;
    for(unsigned int j=0;j<vTag.size();j++)    
        arrTag.push_back(vTag[j]);    
    obj.push_back(Pair("tags",arrTag));
    return Value(obj);
}
string CProduct::ToJsonString(bool fLinkOnly )const
{
    return write_string(ToJson(fLinkOnly), false);
}
bool CProduct::SetJson(const Object& obj,string& strError)
{
    Value tmp = find_value(obj, "id");
    if (tmp.type() != str_type)
    {            
        strError="invalid product id";
        return false;
    }     
    id=tmp.get_str();
   // LogPrintf("CProduct::SetJson product id %s\n",id);
    tmp = find_value(obj, "name");
    if (tmp.type() != str_type)
    {            
        strError="invalid product name";
        return false;
    }     
    name=tmp.get_str();    
   // LogPrintf("CProduct::SetJson product name %s\n", name);
    tmp = find_value(obj, "price");    
    try{
        price=_AmountFromValue(tmp);
    }
    catch (Object& objError)
    {
        strError="price is not valid format";        
        return false;
    }
    if(price<0)
    {
        strError="negative price";        
        price=0;
        return false;
    }    
   // LogPrintf("CProduct::SetJson product price %i\n",price);
    tmp = find_value(obj, "shipmentfee");
    if (tmp.type() != null_type)
    { 
        try{
            shipmentFee=_AmountFromValue(tmp);
        }
        catch (Object& objError)
        {
            strError="shipment fee is not valid fromat";        
            return false;
        }        
  //      LogPrintf("CProduct::SetJson product shipment fee %i\n",shipmentFee);
    }            
    tmp = find_value(obj, "recipient");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=str_type)
        {
            strError="recipient is not string type";
            return false;
        }             
        if(!StringToScriptPubKey(tmp.get_str(),recipient)){
            strError="scriptPubKey is not valid format";
            return false;
        }        
   //     LogPrintf("CProduct::SetJson recipient %s\n", recipient.ToString());
    } 
    tmp = find_value(obj, "seller");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=str_type)
        {
            strError="seller is not string type";
            return false;
        }             
        if(!StringToScriptPubKey(tmp.get_str(),seller)){
            strError="scriptPubKey is not valid format";
            return false;
        }        
   //     LogPrintf("CProduct::SetJson seller %s\n", seller.ToString());
    } 
    tmp = find_value(obj, "icon");
    if (tmp.type() != null_type)
    {   
        if(tmp.type() != str_type||!icon.SetString(tmp.get_str()))
        {
            strError="invalid product icon";
            return false;
        }           
     //   LogPrintf("CProduct::SetJson icon %s\n", tmp.get_str());
    }     
    tmp = find_value(obj, "intro");
    if (tmp.type() != null_type)
    {            
        if(tmp.type() != str_type)
        {
            strError="intro is not str type";
            return false;
        }
        intro=tmp.get_str();        
      //  LogPrintf("CProduct::SetJson intro %s\n", intro);
    }     
    tmp = find_value(obj, "tags");
    if (tmp.type() != null_type)
    {            
        if(tmp.type() != array_type)
        {
            strError="tags is not array type";
            return false;
        }
        Array arrTags=tmp.get_array();
        for(unsigned int j=0;j<arrTags.size();j++)
        {
            if(arrTags[j].type()!=str_type)
            {
                strError="tag is not str type";
                return false;
            }
            if(arrTags[j].get_str()!="")
            {
                vTag.push_back(arrTags[j].get_str());
             //   LogPrintf("CProduct::SetJson tag %s\n", arrTags[j].get_str());
            }
        }
    }
    tmp = find_value(obj, "expiretime");
    if (tmp.type() != null_type)
    {  
        if(tmp.type()!=int_type)
        {
            strError="nExpireTime is not int type";
            return false;
        } 
        nExpireTime=tmp.get_int64();        
       // LogPrintf("CProduct::SetJson nExpireTime %i\n", nExpireTime);
    }
    return true;
}

bool CProduct::SetContent(const CContent content)
{
    //LogPrintf("CProduct::SetContent\n");
    std::vector<std::pair<int, string> > vDecoded0;
    if(!content.Decode(vDecoded0))
        return false;
    if(vDecoded0[0].first!=CC_PRODUCT_P)
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    if(!CContent(vDecoded0[0].second).Decode(vDecoded))
        return false;
    int cc;
    string str;    
    for(unsigned int i=0;i<vDecoded.size();i++)
    {   
        cc=vDecoded[i].first;
        str=vDecoded[i].second;
        //LogPrintf("CProduct::SetContent cc:%i,str:%s \n",cc,str);
        if(cc==CC_PRODUCT_ID)
        {
            id=str; 
            //LogPrintf("CProduct::SetContent id:%s \n",id);
        }
        else if(cc==CC_NAME)
        {
            name=str;
            //LogPrintf("CProduct::SetContent name:%s \n",name);
        }
        else if(cc==CC_PRODUCT_PRICE)
        {
            CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong price format\n");
                return false;            
            }
            price=CAmount(u);
        }
        else if(cc==CC_PRODUCT_SHIPMENTFEE)
        {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong shipmentfee format\n");            
                return false;            
            }
            shipmentFee=CAmount(u);
        }
         else if(cc==CC_PRODUCT_PAYTO)
         {
             recipient=CScript((unsigned char*)str.c_str(),(unsigned char*)(str.c_str()+str.size()));             
        //     LogPrintf("CProduct::SetContent recipient str %s script %s \n",HexStr(str.begin(),str.end()),recipient.ToString());            
                 
         }
        else if(cc==CC_PRODUCT_EXPIRETIME)
        {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(cp.ReadVarInt(pc,u))     
                nExpireTime=uint32_t(u);
        }
        else if(cc==CC_PRODUCT_ICON&&str.size()>=3)       
            icon.Unserialize(str);
        else if(cc==CC_PRODUCT_INTRO&&str.size()>0)       
            intro=str;
        else if(cc==CC_TAG&&str.size()>0)
            vTag.push_back(str);
        else if(cc==CC_TAG_P)
        {
            std::vector<std::pair<int, string> > vDecodedTag;
            CContent(str).Decode(vDecodedTag);
            for(unsigned int i=0;i<vDecodedTag.size();i++)
            { 
                if(vDecodedTag[i].first==CC_TAG)
                vTag.push_back(vDecodedTag[i].second);
            }
        }
//        else if(cc==CC_ATTRIBUTE_P)
//        {
//            
//        }
//        else if(cc==CC_PRICE_P)
//        {
//            
//        }
//        else if(cc==CC_SHIPMENTFEE_P)
//        {
//            
//        }
    }
    //LogPrintf("CProduct::SetContent done\n");
    return IsValid();
}
CContent CProduct::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(CC_PRODUCT_ID,id));
    vcc.push_back(make_pair(CC_NAME,name));
    CContent cPrice;
    cPrice.WriteVarInt(price);
    vcc.push_back(make_pair(CC_PRODUCT_PRICE,cPrice));
    if(shipmentFee!=-1)
    {
        CContent cshipmentfee;
        cshipmentfee.WriteVarInt(shipmentFee);
        vcc.push_back(make_pair(CC_PRODUCT_SHIPMENTFEE,cshipmentfee));
    }
    if(recipient.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_PAYTO,string(recipient.begin(),recipient.end())));
    if(!icon.IsEmpty())
        vcc.push_back(make_pair(CC_PRODUCT_ICON,icon.Serialize())); 
    if(intro.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_INTRO,intro));
    if(nExpireTime>0)
    {
        CContent cexpiretime;
        cexpiretime.WriteVarInt(nExpireTime);
        vcc.push_back(make_pair(CC_PRODUCT_EXPIRETIME,cexpiretime));
    }
    for(unsigned int j=0;j<vTag.size();j++)    
        vcc.push_back(make_pair(CC_TAG,vTag[j]));    
    CContent ctt;
    ctt.EncodeP(CC_PRODUCT_P,vcc); 
    return ctt;
}
Value CPayment::ToJson(bool fLinkOnly)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("type",Value(GetCcName(ccPaymentType))));
    //if(ccPaymentType==CC_PAYMENT_TYPE_SHOP)
    //{
    Array arr;
    for(unsigned int i=0;i<vItems.size();i++)        
        arr.push_back(vItems[i].ToJson(fLinkOnly));
    obj.push_back(Pair("paymentitems",Value(arr)));
    if(recipient.size()>0)
    {
        string strID;
        ScriptPubKeyToString(recipient, strID);
        obj.push_back(Pair("recipient",Value(strID)));    
    }    
    if(strMemo.size()>0)
        obj.push_back(Pair("memo",Value(strMemo)));    
    return Value(obj);
}
string CPayment::ToJsonString(bool fLinkOnly )const
{
    return write_string(ToJson(fLinkOnly), false);
}
bool CPayment::SetJson(const Object& obj,string& strError)
{
    Value tmp = find_value(obj, "type");
    if (tmp.type() != str_type)
    {            
        strError="invalid pyament type";
        return false;
    }     
    ccPaymentType=GetCcValue(tmp.get_str());
    //LogPrintf("CProduct::SetJson ccPaymentType %i\n", ccPaymentType);
    tmp = find_value(obj, "paymentitems");
    if (tmp.type() != array_type)
    {            
        strError="invalid payment items type";
        return false;
    }    
    Array arr=tmp.get_array();
    for (unsigned int i=0;i<arr.size();i++)
    {
        CPaymentItem item;
        if(arr[i].type()!=obj_type)
            {            
            strError="invalid pyament item type";
            return false;
        }
        if(!item.SetJson(arr[i].get_obj(),strError))
            return false;
        vItems.push_back(item);
    }   
    tmp = find_value(obj, "recipient");
    if (tmp.type()!=str_type)
    {
            strError="recipient type error";
            return false;
    }             
    if(!StringToScriptPubKey(tmp.get_str(),recipient)){
        strError="scriptPubKey is not valid format";
        return false;
    }     
   // LogPrintf("CProduct::SetJson recipient %s\n", recipient.ToString());
    tmp = find_value(obj, "memo");
    if (tmp.type() == str_type)
    {     
        strMemo=tmp.get_str();
     //   LogPrintf("CProduct::SetJson memo %s\n",strMemo);
    }
    
     
    return true;
}

bool CPayment::SetContent(const CContent content)
{
    //LogPrintf("CPayment::SetContent\n");
    std::vector<std::pair<int, string> > vDecoded0;
    if(!content.Decode(vDecoded0))
        return false;
    if(vDecoded0[0].first!=CC_PAYMENT_P)
        return false;
    std::vector<std::pair<int, string> > vDecoded;
    CContent(vDecoded0[0].second).Decode(vDecoded);   
    int cc;
    string str;  
    //find payment type first
    bool fFound=false;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
        cc=vDecoded[i].first;
        if(cc>=CC_PAYMENT_TYPE_SHOPPING&&cc<=CC_PAYMENT_TYPE_P2AGREEMENT)
        {
            fFound=true;
            ccPaymentType=(cctype)cc;
            break;
        }
    }
    if (!fFound)
    {
        LogPrintf("CPayment::SetContent ccPaymentType not found \n");
        return false;
    }
    //temporarily only support shopping
    if(ccPaymentType!=CC_PAYMENT_TYPE_SHOPPING)
    {
        LogPrintf("CPayment::SetContent ccPaymentType is not shopping (%s) \n",GetCcName((cctype)cc));
        return false;
    }
    for(unsigned int i=0;i<vDecoded.size();i++)
    {   
        cc=vDecoded[i].first;
        str=vDecoded[i].second;
        //LogPrintf("CProduct::SetContent cc:%i,str:%s \n",cc,str);
        if (cc==CC_PAYMENT_ITEM_P)
        {
            CPaymentItem item;
            if(!item.SetContent(str))
                return false;
            vItems.push_back(item);
        }
        
         else if(cc==CC_PAYMENT_RECIPIENT)
         {
             recipient=CScript((unsigned char*)str.c_str(),(unsigned char*)(str.c_str()+str.size()));             
           //  LogPrintf("CProduct::SetContent recipient str %s script %s \n",HexStr(str.begin(),str.end()),recipient.ToString());            
                 
         } 
        else if (cc==CC_PAYMENT_MEMO)
         {
             strMemo=str;
         }
         
    }
    //LogPrintf("CProduct::SetContent done\n");
    return IsValid();
}
CContent CPayment::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(ccPaymentType,""));
    for(unsigned int j=0;j<vItems.size();j++)  
    {
       CContent item= vItems[j].ToContent();
        string::const_iterator pc = item.begin();
        cctype cc;
        CContent contentStr;
        item.GetCcUnit(pc, cc, contentStr);
        vcc.push_back(make_pair(CC_PAYMENT_ITEM_P,contentStr));  
    }
    if(strMemo.size()>0)
        vcc.push_back(make_pair(CC_PAYMENT_MEMO,strMemo));
    //Recipient is not serialized to content, it will be put into vout address
    CContent ctt;
    ctt.EncodeP(CC_PAYMENT_P,vcc); 
    return ctt;
}
CAmount CPayment::GetTotalValue() const
{
    CAmount nTotal=0;
    for(unsigned int j=0;j<vItems.size();j++)
        nTotal+=vItems[j].nQuantity*vItems[j].price;
    return nTotal;
}
bool CPayment::IsValid(){
    for(unsigned int j=0;j<vItems.size();j++)
        if(!vItems[j].IsValid())
            return false;    
    return (vItems.size()>0&&recipient.size()>0&&ccPaymentType>0);
}
bool CPaymentItem::IsValid(){
    switch (ccPaymentType)
    {
        case CC_PAYMENT_TYPE_SHIPMENTFEE:
            return true;            
        case CC_PAYMENT_TYPE_PRODUCT:
            return (productID.size()>0&&nQuantity!=0);
        default:
            return false;
    }    
}
bool CPaymentItem::SetContent(const CContent content)
{
    std::vector<std::pair<int, string> > vDecoded;
    if(!content.Decode(vDecoded))
        return false;
    int cc;
    string str; 
    //find payment type first
    bool fFound=false;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
         cc=vDecoded[i].first;
        str=vDecoded[i].second;
    
        if(cc>=CC_PAYMENT_TYPE_SHOPPING&&cc<=CC_PAYMENT_TYPE_P2AGREEMENT)
        {
            fFound=true;
            ccPaymentType=(cctype)cc;
            break;
        }
    }
    if (!fFound)
        return false;
    for(unsigned int i=0;i<vDecoded.size();i++)
    {
         cc=vDecoded[i].first;
        str=vDecoded[i].second;
    
        if(cc==CC_PRODUCT_ID)
        {
            productID=str; 
           // LogPrintf("CProduct::SetContent id:%s \n",str);
        }        
        else if(cc==CC_PRODUCT_PRICE)
        {
            CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong price format\n");
                return false;            
            }
            price=CAmount(u);
        }        
         else if (cc==CC_QUANTITY)
         {
             CContent cp(str);
            string::const_iterator pc = cp.begin();
            uint64_t u;
            if(!cp.ReadVarInt(pc,u))
            {
                LogPrintf("CProduct::SetContent wrong quantity format\n");
                return false;            
            }
            nQuantity=CAmount(u);
         }
        else if (cc==CC_PAYMENT_LINK)
         {
             linkPayTo.Unserialize(str);
         }
        else if (cc==CC_PAYMENT_MEMO)
         {
             strMemo=str;
         }
    }
    return true;
}
Value CPaymentItem::ToJson(bool fLinkOnly)const
{
    json_spirit::Object obj;
    obj.push_back(Pair("type",Value(GetCcName(ccPaymentType))));
    if(ccPaymentType==CC_PAYMENT_TYPE_PRODUCT||ccPaymentType==CC_PAYMENT_TYPE_SHIPMENTFEE)
    {
        if(productID.size()>0)
            obj.push_back(Pair("productID",Value(productID)));
        if(!linkPayTo.IsEmpty())
            obj.push_back(Pair("paytolink",Value(linkPayTo.ToString())));
        obj.push_back(Pair("price",_ValueFromAmount(price)));
        obj.push_back(Pair("quantity",Value(nQuantity)));
    }       
    if(strMemo.size()>0)
        obj.push_back(Pair("memo",Value(strMemo)));    
    return Value(obj);
}
bool CPaymentItem::SetJson(const Object& obj,string& strError)
{
    Value tmp = find_value(obj, "type");
    if (tmp.type() != str_type)
    {            
        strError="invalid payment type";
        return false;
    }     
    ccPaymentType=GetCcValue(tmp.get_str());
    //LogPrintf("CPaymentItem::SetJson ccPaymentType %s,%i\n",tmp.get_str(),ccPaymentType);
    tmp = find_value(obj, "productid");
    if (tmp.type() == str_type)
    {
        productID=tmp.get_str();
        //LogPrintf("CPaymentItem::SetJson product id %s\n",productID);
    }
    tmp = find_value(obj, "paytolink");
    if (tmp.type() == str_type)
    {     
        string str=tmp.get_str();
        linkPayTo.SetString(str);
       // LogPrintf("CPaymentItem::SetJson productlink %s\n",linkPayTo.ToString());
    }
           
    
    tmp = find_value(obj, "price");    
    try{
        price=_AmountFromValue(tmp);
    }
    catch (Object& objError)
    {
        strError="price is not valid format";        
        return false;
    }
    if(price<0)
    {
        strError="negative price";        
        price=0;
        return false;
    }    
   // LogPrintf("CPaymentItem::SetJson product price %i\n",price);
    tmp = find_value(obj, "quantity");
    if (tmp.type() != null_type)
    { 
        try{
            nQuantity=_AmountFromValue(tmp)/COIN;
        }
        catch (Object& objError)
        {
            strError="nQuantity is not valid format";        
            return false;
        }        
        //LogPrintf("CPaymentItem::SetJson product nQuantity %i\n",nQuantity);
    }            
           
    tmp = find_value(obj, "memo");
    if (tmp.type() == str_type)
    {     
        strMemo=tmp.get_str();
      //  LogPrintf("CPaymentItem::SetJson memo %s\n",strMemo);
    }
   
     
    return true;
}
CContent CPaymentItem::ToContent()const
{
    std::vector<std::pair<int,string> > vcc;
    vcc.push_back(make_pair(ccPaymentType,""));
    if(productID.size()>0)
        vcc.push_back(make_pair(CC_PRODUCT_ID,productID));
    if(!linkPayTo.IsEmpty())
        vcc.push_back(make_pair(CC_PAYMENT_LINK,linkPayTo.Serialize()));
    CContent cPrice;
    cPrice.WriteVarInt(price);
    vcc.push_back(make_pair(CC_PRODUCT_PRICE,cPrice));
     CContent cQuantity;
    cQuantity.WriteVarInt(nQuantity);
    vcc.push_back(make_pair(CC_QUANTITY,cQuantity));
    if(strMemo.size()>0)
        vcc.push_back(make_pair(CC_PAYMENT_MEMO,strMemo));
    CContent ctt;
    ctt.EncodeP(CC_PAYMENT_ITEM_P,vcc); 
    return ctt;
}