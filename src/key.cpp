// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "arith_uint256.h"
#include "crypto/common.h"
#include "crypto/hmac_sha512.h"
#include "eccryptoverify.h"
#include "pubkey.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include <secp256k1.h>
#include "ecwrapper.h"

static secp256k1_context_t* secp256k1_context = NULL;

bool CKey::Check(const unsigned char *vch) {
    return eccrypto::Check(vch);
}

void CKey::MakeNewKey(bool fCompressedIn) {
    RandAddSeedPerfmon();
    do {
        //GetRandBytes(vch, sizeof(vch));
        GetRandBytes(vch, 32);
    } while (!Check(vch));
    fValid = true;
    fEncrypted=false;
    fCompressed = fCompressedIn;
    GetPubKey(pubKey);
}

bool CKey::SetPrivKey(const CPrivKey &privkey, bool fCompressedIn) {
    if (!secp256k1_ec_privkey_import(secp256k1_context, (unsigned char*)begin(), &privkey[0], privkey.size()))
        return false;
    fCompressed = fCompressedIn;
    fValid = true;
    return true;
}

CPrivKey CKey::GetPrivKey() const {
    assert(fValid);
    CPrivKey privkey;
    int privkeylen, ret;
    privkey.resize(279);
    privkeylen = 279;
    ret = secp256k1_ec_privkey_export(secp256k1_context, begin(), (unsigned char*)&privkey[0], &privkeylen, fCompressed);
    assert(ret);
    privkey.resize(privkeylen);
    return privkey;
}

bool CKey::GetPubKey(CPubKey& result) const {
    assert(fValid);
    if(fEncrypted)
        return false;
    
    int clen = 65;
    int ret = secp256k1_ec_pubkey_create(secp256k1_context, (unsigned char*)result.begin(), &clen, begin(), fCompressed);
    assert((int)result.size() == clen);
    assert(ret);
    assert(result.IsValid());
    return true;
}

bool CKey::Sign(const uint256 &hash, std::vector<unsigned char>& vchSig, uint32_t test_case) const {
    if (!fValid)
        return false;
    if(fEncrypted)
        return false;
    vchSig.resize(72);
    int nSigLen = 72;
    unsigned char extra_entropy[32] = {0};
    WriteLE32(extra_entropy, test_case);
    int ret = secp256k1_ecdsa_sign(secp256k1_context, hash.begin(), (unsigned char*)&vchSig[0], &nSigLen, begin(), secp256k1_nonce_function_rfc6979, test_case ? extra_entropy : NULL);
    assert(ret);
    vchSig.resize(nSigLen);
    return true;
}

bool CKey::VerifyPubKey(const CPubKey& pubkey) const {
    if (pubkey.IsCompressed() != true) {
        return false;
    }
    unsigned char rnd[8];
    std::string str = "Bitcoin key verification\n";
    GetRandBytes(rnd, sizeof(rnd));
    uint256 hash;
    CHash256().Write((unsigned char*)str.data(), str.size()).Write(rnd, sizeof(rnd)).Finalize(hash.begin());
    std::vector<unsigned char> vchSig;
    Sign(hash, vchSig);
    return pubkey.Verify(hash, vchSig);
}

bool CKey::SignCompact(const uint256 &hash, std::vector<unsigned char>& vchSig) const {
    if (!fValid)
        return false;
    if(fEncrypted)
        return false;
    vchSig.resize(65);
    int rec = -1;
    int ret = secp256k1_ecdsa_sign_compact(secp256k1_context, hash.begin(), &vchSig[1], begin(), secp256k1_nonce_function_rfc6979, NULL, &rec);
    assert(ret);
    assert(rec != -1);
    vchSig[0] = 27 + rec + (fCompressed ? 4 : 0);
    return true;
}

bool CKey::Load(CPrivKey &privkey, CPubKey &vchPubKey, bool fSkipCheck=false) {
    if (!secp256k1_ec_privkey_import(secp256k1_context, (unsigned char*)begin(), &privkey[0], privkey.size()))
        return false;
    fCompressed = vchPubKey.IsCompressed();
    fValid = true;

    if (fSkipCheck)
        return true;

    return VerifyPubKey(vchPubKey);
}

bool CKey::Derive(CKey& keyChild, unsigned char ccChild[32], unsigned int nChild, const unsigned char cc[32]) const {
    assert(IsValid());
    assert(IsCompressed());
    if(fEncrypted)
        return false;
    unsigned char out[64];
    LockObject(out);
    if ((nChild >> 31) == 0) {
        CPubKey pubkey;
        GetPubKey(pubkey);
        assert(pubkey.begin() + 33 == pubkey.end());
        BIP32Hash(cc, nChild, *pubkey.begin(), pubkey.begin()+1, out);
    } else {
        assert(begin() + 32 == end());
        BIP32Hash(cc, nChild, 0, begin(), out);
    }
    memcpy(ccChild, out+32, 32);
    memcpy((unsigned char*)keyChild.begin(), begin(), 32);
    bool ret = secp256k1_ec_privkey_tweak_add(secp256k1_context, (unsigned char*)keyChild.begin(), out);
    UnlockObject(out);
    keyChild.fCompressed = true;
    keyChild.fValid = ret;
    return ret;
}
bool CKey::AddSteps(const CKey& stepKey,const uint64_t& steps,CKey& resultKey)const {
    if(fEncrypted)
        return false;
    CKey temp=stepKey;
    resultKey=*this;
    if(steps==0)
        return true;    
    uint256 tweak(steps);
    bool ret=secp256k1_ec_privkey_tweak_mul((unsigned char*)temp.begin(),(unsigned char*)&tweak);
    //LogPrintf("ckey addstep aftermulti:%s",HexStr(temp.begin(),temp.end()));
    ret = secp256k1_ec_privkey_tweak_add((unsigned char*)resultKey.begin(), (unsigned char*)temp.begin());
    resultKey.GetPubKey(resultKey.pubKey);
    return ret;
}
bool CKey::AddSteps(const CKey& stepKey,const uint256& steps,CKey& resultKey)const {
    if(fEncrypted)
        return false;
    CKey temp=stepKey;
    resultKey=*this;
    uint256 tweak(steps);
    bool ret=secp256k1_ec_privkey_tweak_mul((unsigned char*)temp.begin(),(unsigned char*)&tweak);
    ret = secp256k1_ec_privkey_tweak_add((unsigned char*)resultKey.begin(), (unsigned char*)temp.begin());
    resultKey.GetPubKey(resultKey.pubKey);
    return ret;
}
 bool CKey::GetMultipliedTo(const uint64_t& steps,CKey& keyOut)
{
    if(fEncrypted)
        return false;
    keyOut=*this;
     //LogPrintf("key:getmultipliedto 2\n"); 
     uint256 tweak0=uint256(steps);
     unsigned char tweak[32];
     for(unsigned int i=0;i<32;i++){
         unsigned char* ch=tweak0.end()-i-1;
         tweak[i]=*ch;
     }
         
     //LogPrintf("key:getmultipliedto tweak:%s\n",HexStr(tweak,tweak+32)); 
     secp256k1_ec_privkey_tweak_mul((unsigned char*)keyOut.begin(),(unsigned char*)tweak);
     keyOut.GetPubKey(keyOut.pubKey);
     //LogPrintf("key:getmultipliedto 3\n"); 
     return true;
}
bool CKey::MakeSharedKey(const CPubKey& pubKey,CKey& sharedKey)
{
    if(fEncrypted)
        return false;
    unsigned char temp[pubKey.size()];
    memcpy(&temp[0],pubKey.begin(),pubKey.size());
    //LogPrintf("key:MakeSharedKey 1\n"); 
    if(secp256k1_ec_pubkey_tweak_mul(&temp[0],pubKey.size(),(unsigned char*)begin())){
      //  LogPrintf("key:MakeSharedKey 2\n"); 
        sharedKey.Set(&temp[0]+1,&temp[0]+33,true);
        sharedKey.GetPubKey(sharedKey.pubKey);
        return true;
    }
    return false;
}
bool CKey::MakeSimpleSig(const std::vector<unsigned char>& nounce,uint256& sig)
{
    if(fEncrypted)
        return false;
    sig= Hash(begin(),end(), nounce.begin(), nounce.end());
    return true;
}
bool CExtKey::Derive(CExtKey &out, unsigned int nChild) const {
    
    out.nDepth = nDepth + 1;
    CPubKey id ;
     key.GetPubKey(id);
    memcpy(&out.vchFingerprint[0], &id, 4);
    out.nChild = nChild;
    return key.Derive(out.key, out.vchChainCode, nChild, vchChainCode);
}

void CExtKey::SetMaster(const unsigned char *seed, unsigned int nSeedLen) {
    static const unsigned char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};
    unsigned char out[64];
    LockObject(out);
    CHMAC_SHA512(hashkey, sizeof(hashkey)).Write(seed, nSeedLen).Finalize(out);
    key.Set(&out[0], &out[32], true);
    memcpy(vchChainCode, &out[32], 32);
    UnlockObject(out);
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, sizeof(vchFingerprint));
}

CExtPubKey CExtKey::Neuter() const {
    CExtPubKey ret;
    ret.nDepth = nDepth;
    memcpy(&ret.vchFingerprint[0], &vchFingerprint[0], 4);
    ret.nChild = nChild;
     key.GetPubKey(ret.pubkey);
    memcpy(&ret.vchChainCode[0], &vchChainCode[0], 32);
    return ret;
}

void CExtKey::Encode(unsigned char code[74]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, vchChainCode, 32);
    code[41] = 0;
    assert(key.size() == 32);
    memcpy(code+42, key.begin(), 32);
}

void CExtKey::Decode(const unsigned char code[74]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(vchChainCode, code+9, 32);
    key.Set(code+42, code+74, true);
}

bool ECC_InitSanityCheck() {
    if (!CECKey::SanityCheck()) {
        return false;
    }
    CKey key;
    key.MakeNewKey(true);
    CPubKey pubkey;
    key.GetPubKey(pubkey);
    return key.VerifyPubKey(pubkey);
}


void ECC_Start() {
    assert(secp256k1_context == NULL);

    secp256k1_context_t *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    assert(ctx != NULL);

    {
        // Pass in a random blinding seed to the secp256k1 context.
        unsigned char seed[32];
        LockObject(seed);
        GetRandBytes(seed, 32);
        bool ret = secp256k1_context_randomize(ctx, seed);
        assert(ret);
        UnlockObject(seed);
    }

    secp256k1_context = ctx;
}

void ECC_Stop() {
    secp256k1_context_t *ctx = secp256k1_context;
    secp256k1_context = NULL;

    if (ctx) {
        secp256k1_context_destroy(ctx);
    }
}
