// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include "allocators.h"
#include "serialize.h"
#include "uint256.h"
#include "pubkey.h"
#include <stdexcept>
#include <vector>
#include "util.h"
#include "utilstrencodings.h"

//class CPubKey;

struct CExtPubKey;

/** 
 * secp256k1:
 * const unsigned int PRIVATE_KEY_SIZE = 279;
 * const unsigned int PUBLIC_KEY_SIZE  = 65;
 * const unsigned int SIGNATURE_SIZE   = 72;
 *
 * see www.keylength.com
 * script supports up to 75 for single byte push
 */

/**
 * secure_allocator is defined in allocators.h
 * CPrivKey is a serialized private key, with all parameters included (279 bytes)
 */
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

/** An encapsulated private key. */
class CKey
{
private:
    //! Whether this private key is valid. We check for correctness when modifying the key
    //! data, so fValid should always correspond to the actual state.
    bool fValid;

    //! Whether the public key corresponding to this private key is (to be) compressed.
    bool fCompressed;
    bool fEncrypted;
    //! The privkey byte data
    unsigned char vch[48];    
    
    //! Check whether the 32-byte array pointed to be vch is valid keydata.
    bool static Check(const unsigned char* vch);

public:
    CPubKey pubKey;
    //! Construct an invalid private key.
    CKey() : fValid(false), fCompressed(false),fEncrypted(false)
    {
        LockObject(vch);
    }

    //! Copy constructor. This is necessary because of memlocking.
    CKey(const CKey& secret) : fValid(secret.fValid), fCompressed(secret.fCompressed),fEncrypted(secret.fEncrypted)
    {
        LockObject(vch);
        memcpy(vch, secret.vch, sizeof(secret.vch));
    }

    //! Destructor (again necessary because of memlocking).
    ~CKey()
    {
        UnlockObject(vch);
    }
    bool IsEncrypted(){return fEncrypted;};
    friend bool operator==(const CKey& a, const CKey& b)
    {
        return a.fCompressed == b.fCompressed && a.size() == b.size() &&a.fEncrypted==b.fEncrypted&&
               memcmp(&a.vch[0], &b.vch[0], a.size()) == 0;
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn)    { 
        int len=pend - pbegin;
        if (len != 32&&len != 48)
        {//TODO changed hardcoded encryption length
            LogPrintf("CKey::Set:length is not 32 or 48\n");
            fValid = false;
            return;
        }
        if(len== 32)   
        {
            if (!Check(&pbegin[0])) {            
                LogPrintf("CKey::Set:check error\n");
                fValid = false;
                return;
            }
        
            fEncrypted=false;
        }        
        memcpy(vch, (unsigned char*)&pbegin[0], len);
        fValid = true;
        if(len == 48)
            fEncrypted=true;
        else
                GetPubKey(pubKey);
        fCompressed = fCompressedIn;
        
    }
//    template <typename T>
//    void SetPrivKey(const T pbegin, const T pend, bool fCompressedIn)    { 
//        int len=pend - pbegin;
//        if (len != 32&&len != 48)
//        {//TODO changed hardcoded encryption length
//            LogPrintf("CKey::Set:length is not 32 or 48\n");
//            fValid = false;
//            return;
//        }
//        if(len== 32)   
//        {
//            if (!Check(&pbegin[0])) {            
//                LogPrintf("CKey::Set:check error\n");
//                fValid = false;
//                return;
//            }
//            fEncrypted=false;
//        }
//        vch.resize(len);
//            
//        memcpy(vch, (unsigned char*)&pbegin[0], len);
//        fValid = true;
//        if(len == 48)
//            fEncrypted=true;
//        fCompressed = fCompressedIn;
//        
//    }
    //! Simple read-only vector-like interface.
    unsigned int size() const { return (fValid ?(fEncrypted?48:32) : 0); }
    const unsigned char* begin() const { return vch; }
    const unsigned char* end() const { return vch + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return fValid; }

    //! Check whether the public key corresponding to this private key is (to be) compressed.
    bool IsCompressed() const { return fCompressed; }

    //! Initialize from a CPrivKey (serialized OpenSSL private key data).
    bool SetPrivKey(const CPrivKey& vchPrivKey, bool fCompressed);

    //! Generate a new private key using a cryptographic PRNG.
    void MakeNewKey(bool fCompressed=true);
    bool MakeSharedKey(const CPubKey& pubKey,CKey& sharedKey);
    bool GetMultipliedTo(const uint64_t& steps,CKey& keyOut);
    bool MakeSimpleSig(const std::vector<unsigned char> nounce,uint256& sig);
    /**
     * Convert the private key to a CPrivKey (serialized OpenSSL private key data).
     * This is expensive. 
     */
    CPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key.
     * This is expensive.
     */
     bool GetPubKey(CPubKey& result) const;

    /**
     * Create a DER-serialized signature.
     * The test_case parameter tweaks the deterministic nonce, and is only for
     * testing. It should be zero for normal use.
     */
    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, uint32_t test_case = 0) const;

    /**
     * Create a compact signature (65 bytes), which allows reconstructing the used public key.
     * The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
     * The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
     *                  0x1D = second key with even y, 0x1E = second key with odd y,
     *                  add 0x04 for compressed keys.
     */
    bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;

    //! Derive BIP32 child key.
    bool Derive(CKey& keyChild, unsigned char ccChild[32], unsigned int nChild, const unsigned char cc[32]) const;
    bool AddSteps(const CKey& stepKey,const uint64_t& steps,CKey& resultKey)const;
    /**
     * Verify thoroughly whether a private key and a public key match.
     * This is done using a different mechanism than just regenerating it.
     */
    bool VerifyPubKey(const CPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(CPrivKey& privkey, CPubKey& vchPubKey, bool fSkipCheck);

    //! Check whether an element of a signature (r or s) is valid.
    static bool CheckSignatureElement(const unsigned char* vch, int len, bool half);
};

struct CExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    unsigned char vchChainCode[32];
    CKey key;

    friend bool operator==(const CExtKey& a, const CExtKey& b)
    {
        return a.nDepth == b.nDepth && memcmp(&a.vchFingerprint[0], &b.vchFingerprint[0], 4) == 0 && a.nChild == b.nChild &&
               memcmp(&a.vchChainCode[0], &b.vchChainCode[0], 32) == 0 && a.key == b.key;
    }

    void Encode(unsigned char code[74]) const;
    void Decode(const unsigned char code[74]);
    bool Derive(CExtKey& out, unsigned int nChild) const;
    CExtPubKey Neuter() const;
    void SetMaster(const unsigned char* seed, unsigned int nSeedLen);
};

/** Check that required EC support is available at runtime */
bool ECC_InitSanityCheck(void);

#endif // BITCOIN_KEY_H
