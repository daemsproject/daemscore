// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "keystore.h"

#include "crypter.h"
#include "key.h"
#include "script/script.h"
#include "script/standard.h"
#include "util.h"

#include <boost/foreach.hpp>

//bool CKeyStore::GetPubKey(const CKeyID &address, CPubKey &vchPubKeyOut) const
//{
//    CKey key;
//    if (!GetKey(address, key))
//        return false;
//    vchPubKeyOut = key.GetPubKey();
//    return true;
//}



bool CBasicKeyStore::AddCScript(const CScript& redeemScript)
{
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
        return error("CBasicKeyStore::AddCScript() : redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);

    LOCK(cs_KeyStore);
    mapScripts[CScriptID(redeemScript)] = redeemScript;
    return true;
}

bool CBasicKeyStore::HaveCScript(const CScriptID& hash) const
{
    LOCK(cs_KeyStore);
    return mapScripts.count(hash) > 0;
}

bool CBasicKeyStore::GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const
{
    LOCK(cs_KeyStore);
    ScriptMap::const_iterator mi = mapScripts.find(hash);
    if (mi != mapScripts.end())
    {
        redeemScriptOut = (*mi).second;
        return true;
    }
    return false;
}
bool CBasicKeyStore::GetSharedKey(const CPubKey IDLocal,const CPubKey IDForeign,CKey& sharedKey)
{
    if(!HasSharedKey(IDLocal,IDForeign))
        return false;        
    sharedKey=mapSharedKeys[IDLocal][IDForeign];
    return true;
}
bool CBasicKeyStore::HasSharedKey(const CPubKey IDLocal,const CPubKey IDForeign)
{
    SharedKeyMap::iterator it=mapSharedKeys.find(IDLocal);
    if(it==mapSharedKeys.end())
        return false;
    std::map<CPubKey, CKey>::iterator it2=it->second.find(IDForeign);
    if(it2==it->second.end())
        return false;    
    return true;
}
void CBasicKeyStore::ClearSharedKey(const CPubKey IDLocal,const CPubKey IDForeign)
{
    if (IDLocal==CPubKey())
        mapSharedKeys.clear();
    else if(IDForeign==CPubKey())
        mapSharedKeys.erase(IDLocal);
    else        
        if(mapSharedKeys.find(IDLocal)!=mapSharedKeys.end())
              mapSharedKeys[IDLocal].erase(IDForeign);
}
void CBasicKeyStore::StoreSharedKey(const CPubKey IDLocal,const CPubKey IDForeign,const CKey& sharedKey)
{
    if(mapSharedKeys.find(IDLocal)!=mapSharedKeys.end())
        mapSharedKeys[IDLocal][IDForeign]=sharedKey;
    else
    {
         std::map<CPubKey, CKey> mapkey;
         mapkey[IDForeign]=sharedKey;
         mapSharedKeys[IDLocal]=mapkey;
    }
}
bool CBasicKeyStore::GetKey(const CPubKey &address, CKey& keyOut) const
{
    //LogPrintf("CBasicKeyStore::GetKey \n");
   {
        LOCK(cs_KeyStore);
        KeyMap::const_iterator mi = mapKeys.find(address);
        if (mi != mapKeys.end())
        {
            if(mi->second==0){
                keyOut=baseKey;
                return true;
            }                    
            //baseKey.AddSteps(stepKey,mi->second,keyOut);                
            baseKey.AddSteps(stepKey,Hash((char*)&(mi->second),(char*)&(mi->second)+sizeof(mi->second)),keyOut);                
            return true;
        }
    }
     //LogPrintf("CBasicKeyStore::GetKey key not found\n");
    return false;
}  