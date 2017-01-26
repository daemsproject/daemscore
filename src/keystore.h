// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KEYSTORE_H
#define BITCOIN_KEYSTORE_H

#include "key.h"
#include "pubkey.h"
#include "script/script.h"
#include "script/standard.h"
#include "sync.h"
#include "hash.h"
#include "zcash/Address.hpp"
#include "zcash/NoteEncryption.hpp"

#include <boost/signals2/signal.hpp>
#include <boost/variant.hpp>

class CScript;
class CScriptID;

/** A virtual base class for key stores */
class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;
    mutable CCriticalSection cs_SpendingKeyStore;

public:
    virtual ~CKeyStore() {}

    //! Add a key to the store.
    virtual bool AddKeyPubKey(const CKey &key, const CPubKey &pubkey) =0;    
    
    virtual bool AddKey(const CKey &key){return false;};

    //! Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CKeyID &address) const =0;
    virtual bool GetKey(const CKeyID &address, CKey& keyOut) const =0;
    virtual void GetKeys(std::set<CKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;

    //! Support for BIP 0013 : see https://github.com/bitcoin/bips/blob/master/bip-0013.mediawiki
    virtual bool AddCScript(const CScript& redeemScript) =0;
    virtual bool HaveCScript(const CScriptID &hash) const =0;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;

};

typedef std::map<CKeyID, CKey> KeyMap;
typedef std::map<CPubKey,std::map<CPubKey, CKey> > SharedKeyMap;
typedef std::map<CScriptID, CScript > ScriptMap;
typedef std::set<CScript> WatchOnlySet;
typedef std::map<libzcash::PaymentAddress, libzcash::SpendingKey> SpendingKeyMap;
typedef std::map<libzcash::PaymentAddress, ZCNoteDecryption> NoteDecryptorMap;

/** Basic key store, that keeps keys in an address->secret map */
class CBasicKeyStore : public CKeyStore
{
protected:
    
    ScriptMap mapScripts;
    WatchOnlySet setWatchOnly;
    SpendingKeyMap mapSpendingKeys;
    NoteDecryptorMap mapNoteDecryptors;

public:
    CKey baseKey;
    //keystep for extended keys,can be encrypted
    CKey stepKey;
    uint32_t nStartTime;
    uint64_t nMaxSteps;        
    KeyMap mapKeys;
    SharedKeyMap mapSharedKeys;
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
    bool GetSharedKey(const CPubKey IDLocal,const CPubKey IDForeign,CKey& sharedKey);
    bool HasSharedKey(const CPubKey IDLocal,const CPubKey IDForeign)const;
    
    void ClearSharedKey(const CPubKey IDLocal=CPubKey(),const CPubKey IDForeign=CPubKey());

    
    void StoreSharedKey(const CPubKey IDLocal,const CPubKey IDForeign,const CKey& sharedKey);
    
    bool HavePriv()const{return fHasPriv;};
    bool HavePub()const{return fHasPub;};
    bool CanExtendKeys()const{return fHasPub&&fHasStepPub;};
    bool HaveKey(const CPubKey &address) const
    {
        bool result;
        {
            LOCK(cs_KeyStore);
            result = (mapKeys.count(address) > 0);
        }
        return result;
    }
    void GetKeys(std::set<CKeyID> &setAddress) const
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
    bool GetKey(const CPubKey &address, CKey& keyOut) const;
    bool GetExtendPubKey(const uint64_t nStep,CPubKey& pub)const;
    virtual bool AddCScript(const CScript& redeemScript);
    virtual bool HaveCScript(const CScriptID &hash) const;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;

    
    bool GetNoteDecryptor(const libzcash::PaymentAddress &address, ZCNoteDecryption &decOut) const
    {
        {
            LOCK(cs_SpendingKeyStore);
            NoteDecryptorMap::const_iterator mi = mapNoteDecryptors.find(address);
            if (mi != mapNoteDecryptors.end())
            {
                decOut = mi->second;
                return true;
            }
        }
        return false;
    }
    void GetPaymentAddresses(std::set<libzcash::PaymentAddress> &setAddress) const
    {
        setAddress.clear();
        {
            LOCK(cs_SpendingKeyStore);
            SpendingKeyMap::const_iterator mi = mapSpendingKeys.begin();
            while (mi != mapSpendingKeys.end())
            {
                setAddress.insert((*mi).first);
                mi++;
            }
        }
    }
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;
typedef std::map<CKeyID, std::pair<CPubKey, std::vector<unsigned char> > > CryptedKeyMap;
typedef std::map<libzcash::PaymentAddress, std::vector<unsigned char> > CryptedSpendingKeyMap;

#endif // BITCOIN_KEYSTORE_H
