// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "crypter.h"

#include "script/script.h"
#include "script/standard.h"
#include "util.h"
#include "random.h"
#include "crypto/scrypt.h"
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <openssl/aes.h>
#include <openssl/evp.h>

CEncryptParameters::CEncryptParameters()
{

    RandAddSeedPerfmon();
 
    vchSalt.resize(WALLET_CRYPTO_KEY_SIZE);
    GetRandBytes(&vchSalt[0], WALLET_CRYPTO_KEY_SIZE);
    RandAddSeedPerfmon();
    chIV.resize(WALLET_CRYPTO_IV_SIZE);
    GetRandBytes(&chIV[0], WALLET_CRYPTO_IV_SIZE);
 
    N=16384;
    p=1;    
    r=8;    
}
bool CCrypter::SetKeyFromPassphrase(const SecureString& strKeyData, const CEncryptParameters &params)
{  
    //chIV=params.chIV;
    //chIV=params.chIV;
//    int t_size=params.chIV.size();
//     chIV=new unsigned char[t_size];
//     for (int i=0;i<t_size;++i)
//        chIV[i]=params.chIV[i];
    
     
    memcpy(&chIV[0], &params.chIV[0], sizeof chIV);    
    if(!scrypt_sp_generic(&strKeyData[0], strKeyData.size(), &params.vchSalt[0], params.vchSalt.size(),params.N,params.r,params.p, &chKey[0], sizeof(chKey)))           
    {
        LogPrintf("SetKeyFromPassphrase1\n");
        OPENSSL_cleanse(chKey, sizeof(chKey));
        OPENSSL_cleanse(chIV, sizeof(chIV));
        return false;
    }
    LogPrintf("SetKeyFromPassphrase2\n");
    fKeySet = true;
    return true;
}

bool CCrypter::SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV)
{
    if (chNewKey.size() != WALLET_CRYPTO_KEY_SIZE)// || chNewIV.size() != WALLET_CRYPTO_KEY_SIZE)
        return false;    
    //unsigned char chIV2[chNewIV.size()];
    //chIV=chIV2;  
//    int t_size=chNewIV.size();
//     chIV=new unsigned char[t_size];
//     for (int i=0;i<t_size;++i)
//        chIV[i]=chNewIV[i];
    
    memcpy(&chKey[0], &chNewKey[0], sizeof chKey);
    memcpy(&chIV[0], &chNewIV[0], sizeof chIV);
    fKeySet = true;
    return true;
}

bool CCrypter::Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char> &vchCiphertext)
{
    if (!fKeySet)
        return false;

    // max ciphertext len for a n bytes of plaintext is
    // n + AES_BLOCK_SIZE - 1 bytes
    int nLen = vchPlaintext.size();
    int nCLen = nLen + AES_BLOCK_SIZE, nFLen = 0;
    vchCiphertext = std::vector<unsigned char> (nCLen);

    EVP_CIPHER_CTX ctx;

    bool fOk = true;

    EVP_CIPHER_CTX_init(&ctx);
    if (fOk) fOk = EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, chKey, chIV) != 0;
    if (fOk) fOk = EVP_EncryptUpdate(&ctx, &vchCiphertext[0], &nCLen, &vchPlaintext[0], nLen) != 0;
    if (fOk) fOk = EVP_EncryptFinal_ex(&ctx, (&vchCiphertext[0]) + nCLen, &nFLen) != 0;
    LogPrintf("CCryptoKeyStore::iv:%s, \n",HexStr(&chIV[0], &chIV[0]+16));
    EVP_CIPHER_CTX_cleanup(&ctx);

    if (!fOk) return false;

    vchCiphertext.resize(nCLen + nFLen);
    return true;
//     AES_KEY enc_key, dec_key;
//    AES_set_encrypt_key(aes_key, keylength, &enc_key);
//    AES_cbc_encrypt(aes_input, enc_out, inputslength, &enc_key, iv_enc, AES_ENCRYPT);
//
//    AES_set_decrypt_key(aes_key, keylength, &dec_key);
//    AES_cbc_encrypt(enc_out, dec_out, encslength, &dec_key, iv_dec, AES_DECRYPT);
}

bool CCrypter::Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext)
{
    if (!fKeySet)
        return false;

    // plaintext will always be equal to or lesser than length of ciphertext
    int nLen = vchCiphertext.size();
    int nPLen = nLen, nFLen = 0;

    vchPlaintext = CKeyingMaterial(nPLen);

    EVP_CIPHER_CTX ctx;

    bool fOk = true;

    EVP_CIPHER_CTX_init(&ctx);
    if (fOk) fOk = EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, chKey, chIV) != 0;
    if (fOk) fOk = EVP_DecryptUpdate(&ctx, &vchPlaintext[0], &nPLen, &vchCiphertext[0], nLen) != 0;
    if (fOk) fOk = EVP_DecryptFinal_ex(&ctx, (&vchPlaintext[0]) + nPLen, &nFLen) != 0;
    EVP_CIPHER_CTX_cleanup(&ctx);

    if (!fOk) return false;

    vchPlaintext.resize(nPLen + nFLen);
    return true;
}




bool CCryptoKeyStore::SetCrypted()
{
    LOCK(cs_KeyStore);
    if (fUseCrypto)
        return true;
    if (!mapKeys.empty())
        return false;
    fUseCrypto = true;
    return true;
}


bool CCryptoKeyStore::Decrypt(CCrypter& crypter)
{
    {
        LOCK(cs_KeyStore);
        if (!IsCrypted())
            return true;

        
            
            const std::vector<unsigned char> vchCryptedSecret(baseKey.begin(), baseKey.end());
            CKeyingMaterial vchSecret;
            if(!crypter.Decrypt(vchCryptedSecret,vchSecret))
            {
                    return false;
               
            }
            const std::vector<unsigned char> vchCryptedStep(stepKey.begin(), stepKey.end());
            CKeyingMaterial vchStep;
            if(!crypter.Decrypt(vchCryptedStep,vchStep))
            {
                    return false;
               
            }
            if (vchSecret.size() != 32)
            {
                    return false;
            }
            CKey key;
            key.Set(vchSecret.begin(), vchSecret.end(), baseKey.IsCompressed());
            CPubKey pub;
            key.GetPubKey(pub);
            if ( pub!= baseKey.pubKey)
                return false;           
            
            CKey key1;
            key1.Set(vchStep.begin(), vchStep.end(), stepKey.IsCompressed());
            CPubKey pub1;
            key1.GetPubKey(pub1);
            if (pub1 != stepKey.pubKey)
                return false;           
            baseKey.Set(vchSecret.begin(), vchSecret.end(), baseKey.IsCompressed());
            stepKey.Set(vchSecret.begin(), vchSecret.end(), stepKey.IsCompressed());
            fUseCrypto=false;
        fDecryptionThoroughlyChecked = true;
    }
    NotifyStatusChanged(this);
    return true;
}
bool CCryptoKeyStore::VerifyPassword(SecureString strWalletPassphrase){  
    CCrypter crypter;
    if(!crypter.SetKeyFromPassphrase(strWalletPassphrase,encParams))  
            return false;
    if (!IsCrypted())
        return true;
    const std::vector<unsigned char> vchCryptedSecret(baseKey.begin(), baseKey.end());
    CKeyingMaterial vchSecret;
    if(!crypter.Decrypt(vchCryptedSecret,vchSecret))
    
            return false;
    if (vchSecret.size() != 32)
    
            return false;
    CKey key;
    key.Set(vchSecret.begin(), vchSecret.end(), baseKey.IsCompressed());
    CPubKey pub;
            key.GetPubKey(pub);
    if (pub!= baseKey.pubKey)
        return false; 
    const std::vector<unsigned char> vchCryptedStep(stepKey.begin(), stepKey.end());
    CKeyingMaterial vchStep;
    if(!crypter.Decrypt(vchCryptedStep,vchStep))
    
            return false;

    if (vchStep.size() != 32)
    
            return false;
    CKey key1;
    key1.Set(vchStep.begin(), vchStep.end(), stepKey.IsCompressed());
    CPubKey pub1;
            key1.GetPubKey(pub1);
    if (pub1!= stepKey.pubKey)
        return false; 
    
    return true;
}
bool CCryptoKeyStore::GetKey(const CPubKey &address, CKey &keyOut) const
    {
    LogPrintf("CCryptoKeyStore::GetKey");
    if (IsCrypted())
        return GetDecryptedKey(address, keyOut);
    return CBasicKeyStore::GetKey(address, keyOut);
//        {
//            LOCK(cs_KeyStore);
//            KeyMap::const_iterator mi = mapKeys.find(address);
//            if (mi != mapKeys.end())
//            {
//               
//                baseKey.AddSteps(stepKey,mi->second.first,keyOut);                
//                return true;
//            }
//        }
//        return false;
    }   
bool CCryptoKeyStore::GetBaseKey(CKey& keyOut)const
{
    if (!IsCrypted())
    {
        keyOut= baseKey;
        return true;
    }
    CCrypter crypter;
    if(!crypter.SetKeyFromPassphrase(strPassword,encParams))  
            return false;
    const std::vector<unsigned char> vchCryptedSecret(baseKey.begin(), baseKey.end());
    CKeyingMaterial vchSecret;
    if(!crypter.Decrypt(vchCryptedSecret,vchSecret))
        return false;
    if (vchSecret.size() != 32)
        return false;    
    CKey key;
    key.Set(vchSecret.begin(), vchSecret.end(), baseKey.IsCompressed());
    CPubKey pub;
    key.GetPubKey(pub);
    if (pub != baseKey.pubKey)
        return false;
    keyOut=key;
    return true;    
}
bool CCryptoKeyStore::GetStepKey(CKey& keyOut) const
{
    if (!IsCrypted())
    {
        keyOut= stepKey;    
        return true;
    }
    CCrypter crypter;
    if(!crypter.SetKeyFromPassphrase(strPassword,encParams))  
            return false;
    const std::vector<unsigned char> vchCryptedStep(stepKey.begin(), stepKey.end());
    CKeyingMaterial vchStep;
    if(!crypter.Decrypt(vchCryptedStep,vchStep))
        return false;
    CKey key1;
    key1.Set(vchStep.begin(), vchStep.end(), stepKey.IsCompressed());
    CPubKey pub;
            key1.GetPubKey(pub);
    if (pub != stepKey.pubKey)
        return false; 
    keyOut=key1;
    return true;   
}
bool CCryptoKeyStore::GetDecryptedKey(const CPubKey &address, CKey &keyOut) const
{
    uint64_t nSteps;
    KeyMap::const_iterator mi = mapKeys.find(address);
    if (mi == mapKeys.end())
        return false;
    nSteps=mi->second;
    CCrypter crypter;
    if(!crypter.SetKeyFromPassphrase(strPassword,encParams))  
            return false;
    const std::vector<unsigned char> vchCryptedSecret(baseKey.begin(), baseKey.end());
    CKeyingMaterial vchSecret;
    if(!crypter.Decrypt(vchCryptedSecret,vchSecret))
        return false;
    if (vchSecret.size() != 32)
        return false;    
    CKey key;
    key.Set(vchSecret.begin(), vchSecret.end(), baseKey.IsCompressed());
    CPubKey pub;
            key.GetPubKey(pub);
    if (pub != baseKey.pubKey)
        return false; 
    if(nSteps==0){
        keyOut=key;
        return true;
    }
    const std::vector<unsigned char> vchCryptedStep(stepKey.begin(), stepKey.end());
    CKeyingMaterial vchStep;
    if(!crypter.Decrypt(vchCryptedStep,vchStep))
        return false;
    CKey key1;
    key1.Set(vchStep.begin(), vchStep.end(), stepKey.IsCompressed());
    CPubKey pub1;
            key1.GetPubKey(pub1);
    if (pub1 != stepKey.pubKey)
        return false; 
    key.AddSteps(key1,nSteps,keyOut);                
        return true; 
}
//bool CCryptoKeyStore::GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const
//{
//    {
//        LOCK(cs_KeyStore);
//        vchPubKeyOut=mapKeys.find(address)->second.second;
//        return true;
//    }
//    return false;
//}

bool CCryptoKeyStore::Encrypt(CCrypter& crypter)
{
    {
        //LogPrintf("CCryptoKeyStore::Encrypt1\n");
        LOCK(cs_KeyStore);
        if (IsCrypted())
            return false;             
        CKeyingMaterial vchSecret(baseKey.begin(), baseKey.end());
        //LogPrintf("CCryptoKeyStore::Encrypt basekey:%s, len:%i\n",HexStr(baseKey.begin(), baseKey.end()),baseKey.end()-baseKey.begin());
        std::vector<unsigned char> vchCryptedSecret;        
            if (!crypter.Encrypt(vchSecret , vchCryptedSecret))
                return false;
        //LogPrintf("CCryptoKeyStore::Encrypted key:%s, len:%i\n",HexStr(vchCryptedSecret.begin(), vchCryptedSecret.end()),vchCryptedSecret.size());
        //LogPrintf("CCryptoKeyStore::Encrypt3\n");
        //check if encryption succeeded
        CKeyingMaterial vchSecretRecovered;
        if(!crypter.Decrypt(vchCryptedSecret,vchSecretRecovered)||vchSecretRecovered!=vchSecret)
            return false;       
        //LogPrintf("CCryptoKeyStore::recovered key:%s, len:%i\n",HexStr(vchSecretRecovered.begin(), vchSecretRecovered.end()),vchSecretRecovered.size());
        //LogPrintf("CCryptoKeyStore::Encrypt4\n");
        CKeyingMaterial vchSecret2(stepKey.begin(), stepKey.end());
        std::vector<unsigned char> vchCryptedStep;
           if (!crypter.Encrypt(vchSecret2 , vchCryptedStep))
                return false;
        //LogPrintf("CCryptoKeyStore::Encrypt5\n");
        //check if encryption succeeded        
        if(!crypter.Decrypt(vchCryptedStep,vchSecretRecovered)||vchSecretRecovered!=vchSecret2)
            return false;
        LogPrintf("CCryptoKeyStore::basepub:%i\n",baseKey.pubKey.size());
        baseKey.Set(vchCryptedSecret.begin(), vchCryptedSecret.end(), true); 
        LogPrintf("CCryptoKeyStore::basepub after set:%i\n",baseKey.pubKey.size());
        stepKey.Set(vchCryptedStep.begin(), vchCryptedStep.end(), true); 
        //LogPrintf("CCryptoKeyStore::Encrypt7\n");
        fUseCrypto = true; 
    }
    NotifyStatusChanged(this);
    //LogPrintf("CCryptoKeyStore::Encrypt8\n");
    return true;
}
bool CCryptoKeyStore::DecryptKey(const CKey &keyIn, CKey &keyOut) const
{    
    CCrypter crypter;
    if(!crypter.SetKeyFromPassphrase(strPassword,encParams))  
            return false;
    const std::vector<unsigned char> vchCryptedSecret(keyIn.begin(), keyIn.end());
    CKeyingMaterial vchSecret;
    if(!crypter.Decrypt(vchCryptedSecret,vchSecret))
        return false;
    if (vchSecret.size() != 32)
        return false;        
    keyOut.Set(vchSecret.begin(), vchSecret.end(), keyIn.IsCompressed());    
    return true;
}
