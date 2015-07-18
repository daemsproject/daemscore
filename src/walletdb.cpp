// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletdb.h"
#include "base58.h"
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"
#include "wallet.h"
#include "utilstrencodings.h"
#include "crypter.h"
#include "keystore.h"
#include "hash.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <fstream> 
using namespace boost;
using namespace std;
using namespace json_spirit;
//static uint64_t nAccountingEntryNumber = 0;

//
// CWalletDB
//
bool CWalletDB::GetDefaultWallet(CPubKey& id){  
    std::string strAddress;
    if(GetDefaultWallet(strAddress)){
        //LogPrintf("walletdb.cpp:GetDefaultWallet6 %s \n",strAddress);         
            return CBitcoinAddress(strAddress).GetKey(id);                    
    }
        
    id=CPubKey();
    return false;
}
bool CWalletDB::GetDefaultWallet(std::string& strAddress){  
    //LogPrintf("walletdb.cpp:GetDefaultWallet1 %s\n",fpWalletPath.string());
    vector<std::string> vstrFileNames;    
    if(GetFileNames(fpWalletPath, vstrFileNames )<=0)
        return false;       
    //LogPrintf("walletdb.cpp:GetDefaultWallet1 \n");
    json_spirit::Value valDefaultId;    
    if (GetWalletConf("defaultId",valDefaultId)){
        //LogPrintf("walletdb.cpp:GetDefaultWallet2 \n");
        if (valDefaultId.type() == str_type){
            string strDefaultWallet = valDefaultId.get_str();                       
            CBitcoinAddress add;
            if(add.SetString(strDefaultWallet))   {
                
              //LogPrintf("walletdb.cpp:GetDefaultWallet3 %s \n",strDefaultWallet);   
              std::string str=strDefaultWallet;
              if(GetWalletName(strDefaultWallet,str))
              //LogPrintf("walletdb.cpp:GetDefaultWallet31 %s \n",str);  
                //if (filesystem::exists(fpWalletPath / str.append(".id")))                         
                {
                    // LogPrintf("walletdb.cpp:GetDefaultWallet4 %s \n",str);
                    strAddress= strDefaultWallet;
                    return true;
                }
                 //the default Id file can't be found
            }
            //defaultID invalid , TODO delete defaultId setting
        } 
        
    }
    //LogPrintf("walletdb.cpp:GetDefaultWallet4 \n");
    BOOST_FOREACH(const std::string fileName ,vstrFileNames){
        if(fileName.substr(fileName.length()-3)==".id"){
             
            std::vector<unsigned char> vchId;
            CBitcoinAddress add;
            strAddress=fileName.substr(0,fileName.length()-3);            
            //LogPrintf("walletdb.cpp:GetDefaultWallet5 %s\n",strAddress);
            if(!add.SetString(strAddress))
                continue;
            return true;
        }
    }     
    strAddress="";
    return false;
}
bool CWalletDB::SetCurrentWallet(const CPubKey& id){
    CBitcoinAddress add;
    add.Set(id);
    strCurrentWallet = add.ToString();
    //LogPrintf("walletdb.cpp:setcurrentwallet:%s \n",strCurrentWallet);
    return true;
}
bool CWalletDB::SetCurrentWallet(const std::string& strAddress){
    
    strCurrentWallet = strAddress;
    //LogPrintf("walletdb.cpp:setcurrentwallet:%s \n",strCurrentWallet);
    return true;
}
bool CWalletDB::GetCurrentWallet(CPubKey& id){
    CBitcoinAddress add;
    if(!add.SetString(strCurrentWallet))
        return false;
    return add.GetKey(id);    
}
bool CWalletDB::GetCurrentWallet(std::string& strAddress){
    CBitcoinAddress add;
    if(!add.SetString(strCurrentWallet))
        return false;
    strAddress=strCurrentWallet;
    return true;
}
bool CWalletDB::IsCurrentWallet(const CPubKey& id)
{
    CBitcoinAddress add;
    //LogPrintf("walletdb.cpp:IsCurrentWallet id size%i\n",id.size());
    add.Set(id);
    //LogPrintf("walletdb.cpp:IsCurrentWallet 2\n");
    return CompareBase32(CBitcoinAddress(id).ToString(),strCurrentWallet)==0;
}
bool CWalletDB::SetDefaultWallet(const CPubKey& id)
{
    json_spirit::Value valDefaultId;
    CBitcoinAddress add;
    add.Set(id);
    string strDefaultWallet = add.ToString();
    //LogPrintf("walletdb.cpp:setdefaultwallet id:%s\n",strDefaultWallet);
    return SetDefaultWallet(strDefaultWallet);    
}
bool CWalletDB::SetDefaultWallet(const std::string& strAddress){
    json_spirit::Value valDefaultId;        
    valDefaultId=json_spirit::Value(strAddress);
    if (SetWalletConf("defaultId",valDefaultId))
        return true;
    //LogPrintf("walletdb.cpp:setdefaultwallet fail\n");
    return false;
}
bool CWalletDB::GetWalletConf(const std::string& strConfName,json_spirit::Value& valConfValue){
    json_spirit::Object objConf;    
    if (GetWalletConfObj(objConf)){         
        valConfValue = find_value(objConf, strConfName);
        if (valConfValue.type()!=null_type)
            return true;
    }    
    return false;
}
bool CWalletDB::SetWalletConf(const std::string& strConfName,const json_spirit::Value& valConfValue){
    
    json_spirit::Object objConf;
    bool fFound=false;
    if (GetWalletConfObj(objConf)){             
        
        for(unsigned int i = 0; i != objConf.size(); ++i )
        {
            json_spirit::Pair& pair = objConf[i];
            const std::string name  = pair.name_;
            json_spirit::Value&  vvalue = pair.value_;
            if (name==strConfName){
                fFound=true;
                vvalue=valConfValue;                
                break;
            }            
        }
    }
    
    if (!fFound)
        objConf.push_back(Pair(strConfName, valConfValue));
    json_spirit::Value val=json_spirit::Value(objConf);
    
    return WriteJsonToFile(val,fpConfFile.string());
}
bool CWalletDB::WriteToAddressBook(const std::string& strCategory,const std::string& strKey,const json_spirit::Value& valInfo){
    json_spirit::Object objAddressBook;    
    //bool fFoundCategory=false;
    if (GetAddressBookObj(objAddressBook)){             
        for(unsigned int i = 0; i != objAddressBook.size(); ++i )
        {
            json_spirit::Pair& pair = objAddressBook[i];
            const std::string& name  = pair.name_;
            json_spirit::Value&  valCategory = pair.value_;
            if (name==strCategory){
      //          fFoundCategory=true;
                if (valCategory.type()!=obj_type)
                    return false;
                json_spirit::Object objCategory=valCategory.get_obj();
                for(unsigned int j = 0; j != objCategory.size(); ++j )
                {
                    json_spirit::Pair& pair1 = objCategory[j];
                    const std::string& name2  = pair1.name_;
                    json_spirit::Value&  valAdb = pair1.value_;
                    if (name2==strKey){
                        valAdb=valInfo;
                        valCategory=Value(objCategory);
                        json_spirit::Value val=json_spirit::Value(objAddressBook);
                        return WriteAddressBookToFile(val);
                    }
                }
               objCategory.push_back(Pair(strKey, valInfo)); 
               json_spirit::Value val=json_spirit::Value(objAddressBook);
               return WriteAddressBookToFile(val);
               
            }            
        }

    }
    json_spirit::Object objCategory;
    objCategory.push_back(Pair(strKey, valInfo)); 
    json_spirit::Value valCategory=json_spirit::Value(objAddressBook);
    objAddressBook.push_back(Pair(strCategory,valCategory)); 
    json_spirit::Value val=json_spirit::Value(objAddressBook);
    return WriteAddressBookToFile(val);
}
bool CWalletDB::EraseFromAddressBook(const std::string& strCategory,const std::string& strKey){
    json_spirit::Object objAddressBook;    
    //bool fFoundCategory=false;
    if (GetAddressBookObj(objAddressBook)){             
        for(unsigned int i = 0; i != objAddressBook.size(); ++i )
        {
            json_spirit::Pair& pair = objAddressBook[i];
            const std::string& name  = pair.name_;
            json_spirit::Value&  valCategory = pair.value_;
            if (name==strCategory){
                //fFoundCategory=true;
                if (valCategory.type()!=obj_type)
                    return false;
                json_spirit::Object objCategory=valCategory.get_obj();
                for(json_spirit::Object::iterator j = objCategory.begin(); j != objCategory.end(); ++j )
                {
                    json_spirit::Pair& pair2 = *j;
                    const std::string& name2  = pair2.name_;                    
                    if (name2==strKey){
                        objCategory.erase(j);   
                        valCategory=Value(objCategory);
                        json_spirit::Value val=json_spirit::Value(objAddressBook);
                        return WriteAddressBookToFile(val);
                    }
                }         
            }            
        }
    }
    return false;
}
bool CWalletDB::ReadFromAddressBook(const std::string& strCategory,const std::string& strKey,json_spirit::Value& valInfo)
{
    json_spirit::Object objAddressBook;    
    //bool fFoundCategory=false;
    if (GetAddressBookObj(objAddressBook)){             
        for(unsigned int i = 0; i != objAddressBook.size(); ++i )
        {
            json_spirit::Pair& pair = objAddressBook[i];
            const std::string& name  = pair.name_;
            json_spirit::Value&  valCategory = pair.value_;
            if (name==strCategory){
                //fFoundCategory=true;
                if (valCategory.type()!=obj_type)
                    return false;
                json_spirit::Object objCategory=valCategory.get_obj();
                for(json_spirit::Object::iterator j = objCategory.begin(); j != objCategory.end(); ++j )
                {
                    json_spirit::Pair& pair2 = *j;
                    const std::string& name2  = pair2.name_;                    
                    json_spirit::Value&  valInfo2 = pair2.value_;
                    if (name2==strKey){                        
                        valInfo=valInfo2;
                        return true;
                    }
                }         
            }            
        }
    }
    return false;
}
bool CWalletDB::WriteAddressBookToFile(json_spirit::Value& objAddressBook){
    std::string strFileName=strCurrentWallet;
    strFileName.append(".adb");    
    filesystem::path fpAddressBookFile=fpWalletPath / strFileName;
    return WriteJsonToFile(objAddressBook,fpAddressBookFile.string());
}
bool CWalletDB::GetAddressBookObj(json_spirit::Object& objAdb){    
    std::string strFileName=strCurrentWallet;
    strFileName.append(".adb");    
    filesystem::path fpAddressBookFile=fpWalletPath / strFileName;    
    json_spirit::Value valAdb;
    if(filesystem::exists(fpAddressBookFile)&&ReadFileToJson(strFileName,valAdb)){
            if (valAdb.type()==obj_type){
                objAdb = valAdb.get_obj();
                return true;
            }
        }
    return false;
}
bool CWalletDB::GetWalletConfObj(json_spirit::Object& objConf){    
   std::string strConfFileName=fpConfFile.string();    
    json_spirit::Value valConf;    
    if(filesystem::exists(fpConfFile)&&ReadFileToJson(strConfFileName,valConf)){
        
            if (valConf.type()==obj_type){
                objConf = valConf.get_obj();
                return true;
            }
        }    
    return false;
}

bool CWalletDB::GetWalletList(std::vector<std::string>& vIds){     
    //LogPrintf("CWalletDB::GetWalletList \n");
    vector<std::string> vstrFileNames;
    if(GetFileNames(fpWalletPath, vstrFileNames)<=0)
        return false;         
    //LogPrintf("CWalletDB::GetWalletList size:%i\n",vstrFileNames.size());
    BOOST_FOREACH(const std::string fileName ,vstrFileNames){
        if(fileName.substr(fileName.length()-3)==".id"){
      //      LogPrintf("CWalletDB::GetWalletList filename:%i\n",fileName);
            //std::vector<unsigned char> vchId;
            CBitcoinAddress add;
        //    LogPrintf("CWalletDB::GetWalletList substr:%s\n",fileName.substr(0,fileName.length()-3));
            if(!add.SetString(fileName.substr(0,fileName.length()-3)))
                continue;  
          //  LogPrintf("CWalletDB::GetWalletList add:%s\n",add.ToString());
            CPubKey pub;
            //LogPrintf("CWalletDB::GetWalletList addgetkey:%b\n",add.GetKey(pub));
            if(add.GetKey(pub))
                vIds.push_back(fileName.substr(0,fileName.length()-3));
        }
    }   
    return (vIds.size()>0);
        
}   
bool CWalletDB::IsWalletExists(CPubKey& id){
    std::string strName;
    return GetWalletName(id,strName);
}
bool CWalletDB::GetWalletName(const CPubKey& id,std::string& strName){
    std::vector<std::string> vIds;
    if(!GetWalletList(vIds))
        return false;
    CBitcoinAddress add;
    add.Set(id);
    //std::string strFileName=add.ToString().append(".id");
    for(unsigned int i=0;i<vIds.size();i++)
    {
        if (CompareBase32(vIds[i],add.ToString())==0){
            strName=vIds[i];
            return true;
        }
    }
    return false;
}
bool CWalletDB::GetWalletName(const std::string& strNameIn,std::string& strNameOut){    
    CPubKey pub;
    if(!CBitcoinAddress(strNameIn).GetKey(pub))
        return false;
    return GetWalletName(pub,strNameOut);
}
bool CWalletDB::SwitchToWallet(const CPubKey& id,CCryptoKeyStore* keyStore){
    if (GetWalletName(id,strCurrentWallet)){        
        //if(SetDefaultWallet(id)) TODO setdefault key here is wrong. because some once query will call this function
            return ReadKeyStore(keyStore);        
    }
    return false;
}
bool CWalletDB::ReadKeyStore(CCryptoKeyStore* keyStore){
    json_spirit::Object objId;
    Value valTemp;       
    CBitcoinAddress add(strCurrentWallet);
    CPubKey id;
    add.GetKey(id);
    keyStore->mapKeys.clear();
    keyStore->mapKeys[id]=0;
     //LogPrintf("walletdb.cpp:readkeystore strCurrentWallet:%s \n",strCurrentWallet);
    if (GetIdObj(strCurrentWallet,objId)){        
        
        //base key priv
        valTemp = find_value(objId, "basepriv");
        if(valTemp.type()==json_spirit::str_type){
            CBitcoinSecret addtmp;
            addtmp.SetString(valTemp.get_str());
            keyStore->baseKey=addtmp.GetKey();
            LogPrintf("privekey to pubkey:%s \n",CBitcoinAddress(keyStore->baseKey.pubKey).ToString());
            keyStore->fHasPriv=true;
        }
        //step key priv
        valTemp = find_value(objId, "steppriv");
        if(valTemp.type()==json_spirit::str_type){
            CBitcoinSecret addtmp;
            addtmp.SetString(valTemp.get_str());
            keyStore->stepKey=addtmp.GetKey();
            LogPrintf("privekey to pubkey:%s \n",CBitcoinAddress(keyStore->stepKey.pubKey).ToString());
            keyStore->fHasStepPriv=true;
        }
        valTemp = find_value(objId, "basepub");
        if(valTemp.type()==json_spirit::str_type){
                //std::vector<unsigned char> vchTmp;
                std::string str=valTemp.get_str();
                 
                if(CBitcoinAddress(str).GetKey(keyStore->baseKey.pubKey))  {
                    //LogPrintf("walletdb.cpp:readkeysotre basepub:%s,size:%i \n",HexStr(keyStore->baseKey.pubKey.begin(),keyStore->baseKey.pubKey.end()),keyStore->baseKey.pubKey.size());
                    keyStore->fHasPub=true;                    
                }              
                    
        }
        valTemp = find_value(objId, "steppub");
        if(valTemp.type()==json_spirit::str_type){
                //std::vector<unsigned char> vchTmp;
                std::string str=valTemp.get_str();               
                if(CBitcoinAddress(str).GetKey(keyStore->stepKey.pubKey)){
                    //LogPrintf("walletdb.cpp:readkeysotre steppub:%s,size:%i \n",HexStr(keyStore->stepKey.pubKey.begin(),keyStore->stepKey.pubKey.end()),keyStore->stepKey.pubKey.size());
                    keyStore->fHasStepPub=true;                
                }
        }
        valTemp = find_value(objId, "maxsteps");
        if(valTemp.type()==json_spirit::int_type){
            keyStore->nMaxSteps=(uint64_t)valTemp.get_int64();
        }
        valTemp = find_value(objId, "isencrypted");
        if(valTemp.type()==json_spirit::bool_type){
                keyStore->fUseCrypto=valTemp.get_bool();                      
        }        
        valTemp = find_value(objId, "starttime");
        if(valTemp.type()==json_spirit::int_type){
                keyStore->nStartTime=(uint32_t)valTemp.get_int();                      
        }
        // if is crypted, load cryption params
        if (keyStore->fUseCrypto){            
            valTemp = find_value(objId, "iv");
            if(valTemp.type()==json_spirit::str_type){
                std::string str=valTemp.get_str();
                 keyStore->encParams.chIV= ParseHex(str);
                // keyStore->encParams.chIV.resize(str.size());
                //keyStore->encParams.chIV.assign(str.begin(),str.end());                               
            }
            valTemp = find_value(objId, "salt");
            if(valTemp.type()==json_spirit::str_type){
                std::string str=valTemp.get_str();
                //keyStore->encParams.chIV.resize(str.size()/2);
                keyStore->encParams.vchSalt= ParseHex(str);
                //keyStore->encParams.chIV.assign(str.begin(),str.end());                                               
            }
            valTemp = find_value(objId, "n");
            if(valTemp.type()==json_spirit::int_type)
                keyStore->encParams.N=(uint64_t)valTemp.get_int64();  
            valTemp = find_value(objId, "p");
            if(valTemp.type()==json_spirit::int_type)
                keyStore->encParams.p=(unsigned int)valTemp.get_int64();  
            valTemp = find_value(objId, "r");
            if(valTemp.type()==json_spirit::int_type)
                keyStore->encParams.r=(unsigned int)valTemp.get_int64();  
        }
        else
            keyStore->encParams=CEncryptParameters();
        //create id map
        
        //LogPrintf("walletdb.cpp:readkeystore:%u \n",keyStore->mapKeys.size());
        if (keyStore->CanExtendKeys()&&keyStore->nMaxSteps>0)
        {
            //LogPrintf("walletdb.cpp:nmaxsteps:%u \n",keyStore->nMaxSteps);
            CPubKey extPub=keyStore->baseKey.pubKey;
            extPub.Decompress();
            CPubKey decompressedStepPub=keyStore->stepKey.pubKey;
            decompressedStepPub.Decompress();
            for (uint64_t i=1;i<=keyStore->nMaxSteps;i++){            
                //extPub.AddSteps(keyStore->stepKey.pubKey,1);
                //CPubKey compressedPub=extPub;
                CPubKey compressedPub;
                extPub.AddSteps(keyStore->stepKey.pubKey,Hash(&i,&i+1),compressedPub);                   
                compressedPub.Compress();
                //LogPrintf("walletdb.cpp:readkeystore pubkey %i:%s \n",i,));
                keyStore->mapKeys[compressedPub]=i;
            }
        }
        return true;
    }
    LogPrintf("walletdb.cpp:readkeystore:wallet file not found \n");
    return false;
}
bool CWalletDB::WriteKeyStore(CCryptoKeyStore* keyStore){
    json_spirit::Object objId;    
    if(keyStore->fHasPriv)    {
        //LogPrintf("walletdb.cpp:WriteKeyStore basekey:%s,size:%i\n",HexStr(keyStore->baseKey.begin(),keyStore->baseKey.end()),keyStore->baseKey.size());
        objId.push_back(Pair("basepriv", json_spirit::Value(CBitcoinSecret(keyStore->baseKey).ToString())));
    }
    
    //LogPrintf("walletdb.cpp:WriteKeyStore%2\n");
    if(keyStore->fHasStepPriv){
       //LogPrintf("walletdb.cpp:WriteKeyStore stepKey:%s,size:%i\n",HexStr(keyStore->stepKey.begin(),keyStore->stepKey.end()),keyStore->stepKey.size());
        objId.push_back(Pair("steppriv", json_spirit::Value(CBitcoinSecret(keyStore->stepKey).ToString())));
    }
    //LogPrintf("walletdb.cpp:WriteKeyStore%3\n");
    
    if(keyStore->fHasPub){
        //LogPrintf("walletdb.cpp:WriteKeyStore basepub:%s,size:%i\n",HexStr(keyStore->baseKey.pubKey.begin(),keyStore->baseKey.pubKey.end()),keyStore->baseKey.pubKey.size());
        objId.push_back(Pair("basepub", json_spirit::Value(CBitcoinAddress(keyStore->baseKey.pubKey).ToString())));
    }    
    if(keyStore->fHasStepPub){
        //LogPrintf("walletdb.cpp:WriteKeyStore steppub:%s,size:%i\n",HexStr(keyStore->stepKey.pubKey.begin(),keyStore->stepKey.pubKey.end()),keyStore->stepKey.pubKey.size());
        objId.push_back(Pair("steppub", json_spirit::Value(CBitcoinAddress(keyStore->stepKey.pubKey).ToString())));
    }
    //LogPrintf("walletdb.cpp:WriteKeyStore%4\n");
    objId.push_back(Pair("maxsteps", json_spirit::Value(keyStore->nMaxSteps)));
    //LogPrintf("walletdb.cpp:WriteKeyStore%5\n");
    objId.push_back(Pair("isencrypted",json_spirit::Value(keyStore->fUseCrypto)));
    //LogPrintf("walletdb.cpp:WriteKeyStore%6\n");
    objId.push_back(Pair("starttime", json_spirit::Value((uint64_t)keyStore->nStartTime)));
    //LogPrintf("walletdb.cpp:WriteKeyStore%7\n");
    if (keyStore->fUseCrypto){
        std::string str=HexStr(keyStore->encParams.vchSalt.begin(),keyStore->encParams.vchSalt.end());
        //LogPrintf("walletdb.cpp:WriteKeyStore salt %s,length%i\n",str,keyStore->encParams.vchSalt.size());
        //for(vector<unsigned char>::iterator iter = keyStore->encParams.vchSalt.begin(); iter != keyStore->encParams.vchSalt.end(); ++iter)    
//            str += *iter;         
        objId.push_back(Pair("salt", json_spirit::Value(str)));
        //LogPrintf("walletdb.cpp:WriteKeyStore%9\n");
        std::string strTemp=HexStr(keyStore->encParams.chIV.begin(),keyStore->encParams.chIV.end());
        //LogPrintf("walletdb.cpp:WriteKeyStore iv %s,length%i\n",strTemp,keyStore->encParams.chIV.size());
        //for(vector<unsigned char>::iterator iter = keyStore->encParams.chIV.begin(); iter != keyStore->encParams.chIV.end(); ++iter)    
        //    strTemp += *iter;    
        objId.push_back(Pair("iv", json_spirit::Value(strTemp)));
        
        objId.push_back(Pair("n", json_spirit::Value((uint64_t)keyStore->encParams.N)));
        
        objId.push_back(Pair("p", json_spirit::Value((uint64_t)keyStore->encParams.p)));
        
        objId.push_back(Pair("r", json_spirit::Value((uint64_t)keyStore->encParams.r)));
        
    }
    //LogPrintf("walletdb.cpp:WriteKeyStore currentwallet %s\n",strCurrentWallet);
    string filename=strCurrentWallet;
    GetWalletName(strCurrentWallet,filename);
    filename.append(".id");
    filesystem::path fpFile=fpWalletPath / filename; 
    //LogPrintf("walletdb.cpp:WriteKeyStore currentwallet after%s\n",strCurrentWallet);
    return WriteJsonToFile(json_spirit::Value(objId),fpFile.string());
}
bool CWalletDB::GetIdObj(const std::string& strId,json_spirit::Object& objId){    
    //LogPrintf("walletdb.cpp:strId %s\n",strId);
    std::string strFileName;
    if(!GetWalletName(strId,strFileName))
        return false;
    strFileName.append(".id");   
    json_spirit::Value valId;
    filesystem::path fpFile=fpWalletPath / strFileName;
    //LogPrintf("walletdb.cpp:getIdObj:%b %s\n",filesystem::exists(fpFile),fpFile.string());    
    if(ReadFileToJson(fpFile.string(),valId)){
            if (valId.type()==obj_type){
                objId = valId.get_obj();
                return true;
            }
        }    
    return false;
}

bool CWalletDB::WriteName(const std::string& strAddress, const std::string& strName)
{
    nWalletDBUpdated++;
    std::string str="name";
    return WriteToAddressBook(str, strAddress, Value(strName));
}

bool CWalletDB::EraseName(const std::string& strAddress)
{
    // This should only be used for sending addresses, never for receiving addresses,
    // receiving addresses must always have an address book entry if they're not change return.
    nWalletDBUpdated++;
    std::string str="name";
    return EraseFromAddressBook(str, strAddress);
}
bool CWalletDB::WriteDestData(const std::string &address, const std::string &key, const std::string &value)
{
    nWalletDBUpdated++;
    std::string str="dest";
    return WriteToAddressBook(str, key, Value(value));
}
bool CWalletDB::EraseDestData(const std::string &address, const std::string &key){
    nWalletDBUpdated++;
    std::string str="dest";
    return EraseFromAddressBook(str, address);
}



bool CWalletDB::WriteCScript(const uint160& hash, const CScript& redeemScript)
{
    nWalletDBUpdated++;
    std::string str="cscript";
    return WriteToAddressBook(str, hash.ToString(),redeemScript.ToString());
}


bool CWalletDB::WriteOrderPosNext(int64_t nOrderPosNext)
{
    nWalletDBUpdated++;
    return false;//Write(std::string("orderposnext"), nOrderPosNext);
}



bool CWalletDB::WriteMinVersion(int nVersion)
{
    return false;//Write(std::string("minversion"), nVersion);
}


bool CWalletDB::WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry)
{
    return false;//Write(std::make_pair(std::string("acentry"), std::make_pair(acentry.strAccount, nAccEntryNum)), acentry);
}

bool CWalletDB::WriteAccountingEntry(const CAccountingEntry& acentry)
{
    return false;//WriteAccountingEntry(++nAccountingEntryNumber, acentry);
}
bool CWalletDB::AddContacts(const std::map<string,json_spirit::Object>mapContact){    
    std::string str="contact";
    for(std::map<string,Object>::const_iterator it=mapContact.begin();it!=mapContact.end();it++)
    {
        if(!WriteToAddressBook(str, it->first, Value(it->second)))
            return false;
    }
    return true;
}
bool CWalletDB::GetContactInfo(const std::string strKey,json_spirit::Object objInfo)
{
    std::string str="contact";
    json_spirit::Value val;
    if(!ReadFromAddressBook(str,strKey,val))
        return false;
    if(val.type()!=json_spirit::obj_type)
        return false;
    objInfo=val.get_obj();
    return true;
}
class CWalletScanState {
public:
    unsigned int nKeys;
    unsigned int nCKeys;
    unsigned int nKeyMeta;
    bool fIsEncrypted;
    bool fAnyUnordered;
    int nFileVersion;
    vector<uint256> vWalletUpgrade;

    CWalletScanState() {
        nKeys = nCKeys = nKeyMeta = 0;
        fIsEncrypted = false;
        fAnyUnordered = false;
        nFileVersion = 0;
    }
};



