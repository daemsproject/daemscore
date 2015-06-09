// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CCCOIN_KEYSTORE_H
#define CCCOIN_KEYSTORE_H
#include "util.h"
#include "key.h"
#include "pubkey.h"
#include "sync.h"
#include <boost/signals2/signal.hpp>
#include <boost/variant.hpp>

class CScript;
class CScriptID;

/** A virtual base class for key stores */
class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;

public:
    virtual ~CKeyStore() {}

   
    virtual bool AddKey(const CKey &key){return false;};

    //! Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CPubKey &address) const =0;
    virtual bool GetKey(const CPubKey &address, CKey& keyOut) const 
    {
        LogPrintf("CKeyStore::virtual GetKey called\n");
        return false;
    }
    virtual void GetKeys(std::set<CPubKey> &setAddress) const =0;
    //virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;

    //! Support for BIP 0013 : see https://github.com/bitcoin/bips/blob/master/bip-0013.mediawiki
    virtual bool AddCScript(const CScript& redeemScript) =0;
    virtual bool HaveCScript(const CScriptID &hash) const =0;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;
    
};

typedef std::map<CPubKey, uint64_t>  KeyMap;//uint645 is step
typedef std::map<CScriptID, CScript > ScriptMap;

/** Basic key store, that keeps keys in an address->secret map */
class CBasicKeyStore : public CKeyStore
{
protected:
    
    
public:
    CKey baseKey;
    //keystep for extended keys,can be encrypted
    CKey stepKey;
    uint32_t nStartTime;
    uint64_t nMaxSteps;        
    KeyMap mapKeys; 
    bool fHasPriv;
    bool fHasPub;
    bool fHasStepPub;
    bool fHasStepPriv;
    ScriptMap mapScripts;
    CBasicKeyStore()
    {
        nMaxSteps=0;
        nStartTime=0;
        fHasPriv=false;
        fHasPub=false;
        fHasStepPub=false;
        fHasStepPriv=false;
    }
    bool HavePriv(){return fHasPriv;};
    bool HavePub(){return fHasPub;};
    bool CanExtendKeys(){return fHasPub&&fHasStepPub;};
    bool HaveKey(const CPubKey &address) const
    {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }
    void GetKeys(std::set<CPubKey> &setAddress) const
    {
        setAddress.clear();
        {
            LOCK(cs_KeyStore);
            KeyMap::const_iterator mi = mapKeys.begin();
            while (mi != mapKeys.end())
            {
                setAddress.insert((*mi).first);
                mi++;
            }
        }
    }
    bool GetKey(const CPubKey &address, CKey& keyOut) const
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
                baseKey.AddSteps(stepKey,mi->second,keyOut);                
                return true;
            }
        }
         //LogPrintf("CBasicKeyStore::GetKey key not found\n");
        return false;
    }   
    virtual bool AddCScript(const CScript& redeemScript);
    virtual bool HaveCScript(const CScriptID &hash) const;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;

};
#endif // BITCOIN_KEYSTORE_H
