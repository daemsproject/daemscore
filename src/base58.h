// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Why base-58 instead of standard base-64 encoding?
 * - Don't want 0OIl characters that look the same in some fonts and
 *      could be used to create visually identical looking account numbers.
 * - A string with non-alphanumeric characters is not as easily accepted as an account number.
 * - E-mail usually won't line-break if there's no punctuation to break at.
 * - Double-clicking selects the whole number as one word if it's all alphanumeric.
 */
#ifndef BITCOIN_BASE58_H
#define BITCOIN_BASE58_H

#include "chainparams.h"
#include "key.h"
#include "pubkey.h"
#include "script/script.h"
#include "script/standard.h"

#include <string>
#include <vector>

/**
 * Encode a byte sequence as a base32-encoded string.
 * pbegin and pend cannot be NULL, unless both are.
 */
std::string EncodeBase32(const unsigned char* pbegin, const unsigned char* pend);

/**
 * Encode a byte vector as a base32-encoded string
 */
std::string EncodeBase32(const std::vector<unsigned char>& vch);
std::string EncodeBase32(const int i);

/**
 * Decode a base32-encoded string (psz) into a byte vector (vchRet).
 * return true if decoding is successful.
 * psz cannot be NULL.
 */
bool DecodeBase32(const char* psz, std::vector<unsigned char>& vchRet);

/**
 * Decode a base32-encoded string (str) into a byte vector (vchRet).
 * return true if decoding is successful.
 */
bool DecodeBase32(const std::string& str, std::vector<unsigned char>& vchRet);

/**
 * Decode a base32-encoded string (str) into integer.
 * return -1 if overflow.
 */
int DecodeBase32ToInt(const char* psz);
int DecodeBase32ToInt(const std::string& str);

/**
 * Encode a byte vector into a base32-encoded string, including checksum
 */
std::string EncodeBase32Check(const std::vector<unsigned char>& vchIn);

/**
 * Decode a base32-encoded string (psz) that includes a checksum into a byte
 * vector (vchRet), return true if decoding is successful
 */
bool DecodeBase32Check(const char* psz, std::vector<unsigned char>& vchRet);

/**
 * Decode a base32-encoded string (str) that includes a checksum into a byte
 * vector (vchRet), return true if decoding is successful
 */
bool DecodeBase32Check(const std::string& str, std::vector<unsigned char>& vchRet);

int CompareBase32(const std::string& s1, const std::string& s2);

/**
 * Base class for all base32-encoded data
 */
class CBase32Data
{
protected:
    //! the version byte(s)
    std::vector<unsigned char> vchVersion;

    //! the actually encoded data
    typedef std::vector<unsigned char, zero_after_free_allocator<unsigned char> > vector_uchar;
    vector_uchar vchData;

    CBase32Data();
    void SetData(const std::vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize);
    void SetData(const std::vector<unsigned char> &vchVersionIn, const unsigned char *pbegin, const unsigned char *pend);

public:
    bool SetString(const char* psz, unsigned int nVersionBytes = 1);
    bool SetString(const std::string& str);
    std::string ToString() const;
    std::string GetHeader(unsigned int nHeaderLen) const;
    int CompareTo(const CBase32Data& b32) const;

    bool operator==(const CBase32Data& b32) const { return CompareTo(b32) == 0; }
    bool operator<=(const CBase32Data& b32) const { return CompareTo(b32) <= 0; }
    bool operator>=(const CBase32Data& b32) const { return CompareTo(b32) >= 0; }
    bool operator< (const CBase32Data& b32) const { return CompareTo(b32) <  0; }
    bool operator> (const CBase32Data& b32) const { return CompareTo(b32) >  0; }
};

/** base32-encoded Bitcoin addresses.
 * Public-key-hash-addresses have flag 0 (or 111 testnet).
 * The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
 * Script-hash-addresses have flag 0x90 (or 196 testnet).
 * The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
 */
class CBitcoinAddress : public CBase32Data {
public:
    bool Set(const CPubKey& id);
    bool Set(const CScriptID &id);
    bool Set(const CTxDestination &dest);
    bool Set(const CScript &script);
    bool IsValid() const;
    bool IsValid(const CChainParams &params) const;

    CBitcoinAddress() {}
    CBitcoinAddress(const CTxDestination &dest) { Set(dest); }
    CBitcoinAddress(const CPubKey& id) { Set(id); }
    CBitcoinAddress(const CScript &script) { Set(script); }
    CBitcoinAddress(const std::string& strAddress) { SetString(strAddress); }
    CBitcoinAddress(const char* pszAddress) { SetString(pszAddress); }

    CTxDestination Get() const;
    bool GetKey(CPubKey& keyID) const;
    bool IsScript() const;
    std::string ToString() const;
};

/**
 * A base32-encoded secret key
 */
class CBitcoinSecret : public CBase32Data
{
public:
    void SetKey(const CKey& vchSecret);
    CKey GetKey();
    bool IsValid() const;
    bool SetString(const char* pszSecret);
    bool SetString(const std::string& strSecret);

    CBitcoinSecret(const CKey& vchSecret) { SetKey(vchSecret); }
    CBitcoinSecret() {}
};

template<typename K, int Size, CChainParams::Base32Type Type> class CBitcoinExtKeyBase : public CBase32Data
{
public:
    void SetKey(const K &key) {
        unsigned char vch[Size];
        key.Encode(vch);
        SetData(Params().Base32Prefix(Type), vch, vch+Size);
    }

    K GetKey() {
        K ret;
        ret.Decode(&vchData[0], &vchData[Size]);
        return ret;
    }

    CBitcoinExtKeyBase(const K &key) {
        SetKey(key);
    }

    CBitcoinExtKeyBase() {}
};

typedef CBitcoinExtKeyBase<CExtKey, 74, CChainParams::EXT_SECRET_KEY> CBitcoinExtKey;
typedef CBitcoinExtKeyBase<CExtPubKey, 74, CChainParams::EXT_PUBLIC_KEY> CBitcoinExtPubKey;
bool StringToScriptPubKey(const string& str,CScript& script);
bool ScriptPubKeyToString(const CScript& script,string& str);
std::string ToStandardB32String(const std::string str);
 bool B32Equal(const std::string str1,const std::string str2);
#endif // BITCOIN_BASE58_H
