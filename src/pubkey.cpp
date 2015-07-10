// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pubkey.h"

#include "eccryptoverify.h"

//#ifdef USE_SECP256K1
#include <secp256k1.h>
//#include "secp256k1/src/scalar.h"
//#else
#include "ecwrapper.h"
//#endif
//! anonymous namespace
namespace {

class CSecp256k1Init {
public:
    CSecp256k1Init() {
        secp256k1_start(SECP256K1_START_SIGN||SECP256K1_START_VERIFY);
    }
    ~CSecp256k1Init() {
        secp256k1_stop();
    }
};
static CSecp256k1Init instance_of_csecp256k1;

} // anon namespace
bool CPubKey::Verify(const uint256 &hash, const std::vector<unsigned char>& vchSig) const {
    if (!IsValid())
        return false;
#ifdef USE_SECP256K1
    if (secp256k1_ecdsa_verify((const unsigned char*)&hash, 32, &vchSig[0], vchSig.size(), begin(), size()) != 1)
        return false;
#else
    CECKey key;
    if (!key.SetPubKey(begin(), size()))
        return false;
    if (!key.Verify(hash, vchSig))
        return false;
#endif
    return true;
}

bool CPubKey::RecoverCompact(const uint256 &hash, const std::vector<unsigned char>& vchSig) {
    if (vchSig.size() != 65)
        return false;
    int recid = (vchSig[0] - 27) & 3;
    bool fComp = ((vchSig[0] - 27) & 4) != 0;
#ifdef USE_SECP256K1
    int pubkeylen = 65;
    if (!secp256k1_ecdsa_recover_compact((const unsigned char*)&hash, 32, &vchSig[1], (unsigned char*)begin(), &pubkeylen, fComp, recid))
        return false;
    assert((int)size() == pubkeylen);
#else
    CECKey key;
    if (!key.Recover(hash, &vchSig[1], recid))
        return false;
    std::vector<unsigned char> pubkey;
    key.GetPubKey(pubkey, fComp);
    Set(pubkey.begin(), pubkey.end());
#endif
    return true;
}

bool CPubKey::IsFullyValid() const {
    if (!IsValid())
        return false;
#ifdef USE_SECP256K1
    if (!secp256k1_ecdsa_pubkey_verify(begin(), size()))
        return false;
#else
    CECKey key;
    if (!key.SetPubKey(begin(), size()))
        return false;
#endif
    return true;
}

bool CPubKey::Decompress() {
    if (!IsValid())
        return false;
#ifdef USE_SECP256K1
    int clen = size();
    int ret = secp256k1_ecdsa_pubkey_decompress((unsigned char*)begin(), &clen);
    assert(ret);
    assert(clen == (int)size());
#else
    CECKey key;
    if (!key.SetPubKey(begin(), size()))  // Bug here to fix : compressed pubkey can not set successfully
        return false;
    std::vector<unsigned char> pubkey;
    key.GetPubKey(pubkey, false);
    Set(pubkey.begin(), pubkey.end());
#endif
    return true;
}

bool CPubKey::Compress() {
    if (!IsValid())
        return false;
    if(IsCompressed())
        return true;
    CECKey key;
    if (!key.SetPubKey(begin(), size()))
        return false;
    std::vector<unsigned char> pubkey;
    key.GetPubKey(pubkey, true);
    Set(pubkey.begin(), pubkey.end());
    return true;
}
bool CPubKey::AddSteps(const CPubKey& stepPubKey,const long& nStep){
    
    //the hashing is a good secure procedure, but compromise is a much slower speed
    if (nStep==0)
        return true;    
    uint256 tweak=uint256((uint64_t)nStep);
   return secp256k1_ec_pubkey_tweak_addsteps((unsigned char*)begin(), size(),(unsigned char*)stepPubKey.begin(), stepPubKey.size(),(unsigned char*)&tweak);

}
bool CPubKey::AddSteps(const CPubKey& stepPubKey,const uint256& nStep,CPubKey& keyOut){
    
    //the hashing is a good secure procedure, but compromise is a much slower speed
    keyOut=*this;
    if (nStep==0)
    {
    
        return true;    
    }
    uint256 tweak=nStep;
    
   return secp256k1_ec_pubkey_tweak_addsteps((unsigned char*)keyOut.begin(), keyOut.size(),(unsigned char*)stepPubKey.begin(), stepPubKey.size(),(unsigned char*)&tweak);

}

bool CPubKey::Derive(CPubKey& pubkeyChild, unsigned char ccChild[32], unsigned int nChild, const unsigned char cc[32]) const {
    assert(IsValid());
    assert((nChild >> 31) == 0);
    assert(begin() + 33 == end());
    unsigned char out[64];
    BIP32Hash(cc, nChild, *begin(), begin()+1, out);
    memcpy(ccChild, out+32, 32);
#ifdef USE_SECP256K1
    pubkeyChild = *this;
    bool ret = secp256k1_ecdsa_pubkey_tweak_add((unsigned char*)pubkeyChild.begin(), pubkeyChild.size(), out);
#else
    CECKey key;
    bool ret = key.SetPubKey(begin(), size());
    ret &= key.TweakPublic(out);
    std::vector<unsigned char> pubkey;
    key.GetPubKey(pubkey, true);
    pubkeyChild.Set(pubkey.begin(), pubkey.end());
#endif
    return ret;
}

void CExtPubKey::Encode(unsigned char code[74]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, vchChainCode, 32);
    assert(pubkey.size() == 33);
    memcpy(code+41, pubkey.begin(), 33);
}

void CExtPubKey::Decode(const unsigned char code[74]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(vchChainCode, code+9, 32);
    pubkey.Set(code+41, code+74);
}

bool CExtPubKey::Derive(CExtPubKey &out, unsigned int nChild) const {
    out.nDepth = nDepth + 1;
    CPubKey id = pubkey;
    memcpy(&out.vchFingerprint[0], &id, 4);
    out.nChild = nChild;
    return pubkey.Derive(out.pubkey, out.vchChainCode, nChild, vchChainCode);
}

std::vector<unsigned char> CPubKey::GetChar() const
{
    return std::vector<unsigned char>(vch, vch + size());
}
