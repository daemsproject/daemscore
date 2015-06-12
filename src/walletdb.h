// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLETDB_H
#define BITCOIN_WALLETDB_H

#include "amount.h"
#include "key.h"
#include "util.h"
#include "keystore.h"
#include "pubkey.h"
#include "crypter.h"
#include <list>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include <boost/filesystem.hpp>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"
class CAccount;
class CAccountingEntry;
struct CBlockLocator;


class CScript;
class CWallet;

class uint160;
class uint256;

/** Error statuses for the wallet database */
enum DBErrors
{
    DB_LOAD_OK,
    DB_CORRUPT,
    DB_NONCRITICAL_ERROR,
    DB_TOO_NEW,
    DB_LOAD_FAIL,
    DB_NEED_REWRITE
};

class CKeyMetadata
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime; // 0 means unknown
    
    CKeyMetadata()
    {
        SetNull();
    }
    CKeyMetadata(int64_t nCreateTime_)
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = nCreateTime_;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
    }

    void SetNull()
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = 0;
    }
};

/** Access to the wallet database  */
class CWalletDB
{

public:
    std::string strCurrentWallet;
    int nWalletDBUpdated;
    boost::filesystem::path fpWalletPath= GetDataDir() / "ids" ;
    boost::filesystem::path fpConfFile= GetDataDir() / "ids" / "ids.conf";
    CWalletDB()
    {   
        LogPrintf("Cwalletdb() \n");
        //fpWalletPath = GetDataDir() / "ids" ; 
        boost::filesystem::create_directories(fpWalletPath);
        //fpConfFile = GetDataDir() / "ids" / "ids.conf"; 
        nWalletDBUpdated=0;
        LogPrintf("Cwalletdb()2 %s\n",fpWalletPath.string());
    }    
    DBErrors LoadWallet(CWallet* pwallet,CPubKey id);    
    bool GetDefaultWallet(CPubKey& id);
    bool GetDefaultWallet(std::string& strAddress);
    bool SetDefaultWallet(const std::string& strAddress);
    bool SetDefaultWallet(const CPubKey& id);
    bool SetCurrentWallet(const CPubKey& id);
    bool SetCurrentWallet(const std::string& strAddress);
    bool GetCurrentWallet(CPubKey& id);
    bool GetCurrentWallet(std::string& strAddress);
    bool IsCurrentWallet(const CPubKey& id);
    bool GetWalletList(std::vector<std::string>& vIds);    
    bool IsWalletExists(CPubKey& id);
    bool GetWalletName(const std::string& strNameIn,std::string& strNameOut);
    bool GetWalletName(const CPubKey& id,std::string& strName);
    bool SwitchToWallet(const CPubKey& id,CCryptoKeyStore* keyStore);
    //bool SwitchToWallet(std::string strWalletName);
    //bool ReadKeyStore(CCryptoKeyStore* keyStore);
    bool WriteKeyStore(CCryptoKeyStore* keyStore);
    bool SetWalletConf(const std::string& strConfName,const json_spirit::Value& valConfValue);
    bool GetWalletConf(const std::string& strConfName,json_spirit::Value& valConfValue);
    bool GetWalletConfObj(json_spirit::Object& objConf);
    bool ReadKeyStore(CCryptoKeyStore* keyStore);
    bool GetIdObj(const std::string& strId,json_spirit::Object& objId);  
    bool WriteToAddressBook(const std::string& strCategory,const std::string& strKey,const json_spirit::Value& valInfo);
    bool EraseFromAddressBook(const std::string& strCategory,const std::string& strKey);
    bool ReadFromAddressBook(const std::string& strCategory,const std::string& strKey,json_spirit::Value& valInfo);
    bool WriteAddressBookToFile(json_spirit::Value& objAddressBook);
    bool GetAddressBookObj(json_spirit::Object& objAdb);
    bool AddContacts(const std::map<std::string,json_spirit::Object>mapContact);
    bool GetContactInfo(const std::string strKey,json_spirit::Object objInfo);
    
    bool WriteName(const std::string& strAddress, const std::string& strName);
    bool EraseName(const std::string& strAddress);
    
    

    

    bool WriteCScript(const uint160& hash, const CScript& redeemScript);



    bool WriteOrderPosNext(int64_t nOrderPosNext);

        

    bool WriteMinVersion(int nVersion);

    

    /// Write destination data key,value tuple to database
    bool WriteDestData(const std::string &address, const std::string &key, const std::string &value);
    /// Erase destination data tuple from wallet database
    bool EraseDestData(const std::string &address, const std::string &key);

    bool WriteAccountingEntry(const CAccountingEntry& acentry);
    

private:
    CWalletDB(const CWalletDB&);
    //void operator=(const CWalletDB&);

    bool WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry);
};

bool BackupWallet(const CWallet& wallet, const std::string& strDest);

#endif // BITCOIN_WALLETDB_H
