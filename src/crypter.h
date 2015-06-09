// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTER_H
#define BITCOIN_CRYPTER_H

#include "allocators.h"
#include "keystore.h"
#include "serialize.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"
#include <vector>
class uint256;

const unsigned int WALLET_CRYPTO_KEY_SIZE = 32;
const unsigned int WALLET_CRYPTO_IV_SIZE = 16;
const unsigned int WALLET_CRYPTO_SALT_SIZE = 8;

/**
 * Private key encryption is done based on a CMasterKey,
 * which holds a salt and random encryption key.
 * 
 * CMasterKeys are encrypted using AES-256-CBC using a key
 * derived using derivation method nDerivationMethod
 * (0 == EVP_sha512()) and derivation iterations nDeriveIterations.
 * vchOtherDerivationParameters is provided for alternative algorithms
 * which may require more parameters (such as scrypt).
 * 
 * Wallet Private Keys are then encrypted using AES-256-CBC
 * with the double-sha256 of the public key as the IV, and the
 * master key's key as the encryption key (see keystore.[ch]).
 */

/** Encrypt params for wallet encryption */
class CEncryptParameters
{
public:
    std::vector<unsigned char> vchSalt;
    std::vector<unsigned char> chIV;//unsigned char chIV[WALLET_CRYPTO_IV_SIZE];
    unsigned long N;
    unsigned int p;
    unsigned int r;    

    //ADD_SERIALIZE_METHODS;

//    template <typename Stream, typename Operation>
//    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {        
//        READWRITE(vchSalt);
//        READWRITE(chIV);
//        READWRITE(N);
//        READWRITE(p);
//        READWRITE(r);        
//    }

    CEncryptParameters(std::vector<unsigned char> vchSaltIn,std::vector<unsigned char> chIVIn
            ,unsigned long NIn,unsigned int pIn,unsigned int rIn):
            vchSalt(vchSaltIn),chIV(chIVIn),N(NIn),p(pIn),r(rIn){};
    CEncryptParameters();
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;

/** Encryption/decryption context with key information */
class CCrypter
{
protected:
    unsigned char chKey[WALLET_CRYPTO_KEY_SIZE];
    unsigned char chIV[WALLET_CRYPTO_IV_SIZE];
    bool fKeySet;

public:
    bool SetKeyFromPassphrase(const SecureString& strKeyData, const CEncryptParameters &params);
    bool Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext);
    bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext);
    bool SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

    void CleanKey()
    {
        OPENSSL_cleanse(chKey, sizeof(chKey));
        OPENSSL_cleanse(chIV, sizeof(chIV));
        fKeySet = false;
    }
    //CCrypter(unsigned char chKey,unsigned char chIV);
    CCrypter()
    {
        fKeySet = false;

        // Try to keep the key data out of swap (and be a bit over-careful to keep the IV that we don't even use out of swap)
        // Note that this does nothing about suspend-to-disk (which will put all our key data on disk)
        // Note as well that at no point in this program is any attempt made to prevent stealing of keys by reading the memory of the running process.
        LockedPageManager::Instance().LockRange(&chKey[0], sizeof chKey);
        LockedPageManager::Instance().LockRange(&chIV[0], sizeof chIV);
    }

    ~CCrypter()
    {
        CleanKey();

        LockedPageManager::Instance().UnlockRange(&chKey[0], sizeof chKey);
        LockedPageManager::Instance().UnlockRange(&chIV[0], sizeof chIV);
    }
};

bool EncryptSecret(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vchPlaintext, const uint256& nIV, std::vector<unsigned char> &vchCiphertext);
bool DecryptSecret(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext, const uint256& nIV, CKeyingMaterial& vchPlaintext);

/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CCryptoKeyStore : public CBasicKeyStore
{
private:
    //This is extremely private, remove it immediately after signing
    SecureString strPassword;
    
    
    //! keeps track of whether Unlock has run a thorough check before
    bool fDecryptionThoroughlyChecked;
    bool GetDecryptedKey(const CPubKey &address, CKey &keyOut) const;
protected:
    bool SetCrypted();
    
    //! will encrypt previously unencrypted keys


public:
    CEncryptParameters encParams;
        bool Encrypt(CCrypter& crypter);

    bool Decrypt(CCrypter& crypter);
    bool fUseCrypto;
    CCryptoKeyStore() 
    {
        fUseCrypto=false;
        fDecryptionThoroughlyChecked=false;
        encParams=CEncryptParameters();
    }
    CCryptoKeyStore(json_spirit::Value keyStoreJson);
    bool IsCrypted() const
    {
        return fUseCrypto;
    }
    bool SetPassword(SecureString pwd){        
        if(!VerifyPassword(pwd))
            return false;
        strPassword=pwd;
        NotifyStatusChanged(this);
        return true;
    }
    bool VerifyPassword(SecureString pwd);
    void ClearPassword(){
        strPassword="1234567890";
        strPassword.clear();
    }
    bool IsLocked() const
    {
        return IsCrypted()&&strPassword.size()==0;
    }

    bool Lock(){
        ClearPassword();
        NotifyStatusChanged(this);
        return true;
    };
    bool Unlock(const SecureString& strWalletPassphrase)
{
   
        
        if(SetPassword(strWalletPassphrase))
        {
            return true;
            NotifyStatusChanged(this);
        }
        return false;
    
}
    
    
    bool HaveKey(const CPubKey &address) const
    {
        {
            LOCK(cs_KeyStore);            
            return CBasicKeyStore::HaveKey(address);
            
        }
        return false;
    }
    bool GetKey(const CPubKey &address, CKey& keyOut) const;
    bool GetBaseKey(CKey& keyOut)const;
    bool GetStepKey(CKey& keyOut)const;
        
    //bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;
    void GetKeys(std::set<CPubKey> &setAddress) const
    {        
            CBasicKeyStore::GetKeys(setAddress);
            return;        
    }
    //decrypt sharedkey
    bool DecryptKey(const CKey &keyIn, CKey &keyOut)const;
    /**
     * Wallet status (encrypted, locked) changed.
     * Note: Called without locks held.
     */
    boost::signals2::signal<void (CCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif // BITCOIN_CRYPTER_H
