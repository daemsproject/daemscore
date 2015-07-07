// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

#include "base58.h"
#include "checkpoints.h"
#include "coincontrol.h"
#include "net.h"
#include "script/script.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "core_io.h"
#include <assert.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/thread.hpp>
#include "json/json_spirit_utils.h"
#include "ccc/content.h"
using namespace std;
using namespace json_spirit;

/**
 * Settings
 */
CFeeRate payTxFee(DEFAULT_TRANSACTION_FEE);
CAmount maxTxFee = DEFAULT_TRANSACTION_MAXFEE;
unsigned int nTxConfirmTarget = 1;
bool bSpendZeroConfChange = false;
bool fSendFreeTransactions = false;
bool fPayAtLeastCustomFee = true;

/** 
 * Fees smaller than this (in satoshi) are considered zero fee (for transaction creation) 
 * Override with -mintxfee
 */
CFeeRate CWallet::minTxFee = CFeeRate(1000);

/** @defgroup mapWallet
 *
 * @{
 */

struct CompareValueOnly
{
    bool operator()(const pair<CAmount, pair<const CWalletTx*, unsigned int> >& t1,
                    const pair<CAmount, pair<const CWalletTx*, unsigned int> >& t2) const
    {
        return t1.first < t2.first;
    }
};

std::string COutput::ToString() const
{
    return strprintf("COutput(%s, %d, %d) [%s]", tx->GetHash().ToString(), i, nDepth, FormatMoney(tx->vout[i].nValue));
}

const CWalletTx* CWallet::GetWalletTx(const uint256& hash) const
{
    LOCK(cs_wallet);
    std::map<uint256, CWalletTx>::const_iterator it = mapWallet.find(hash);
    if (it == mapWallet.end())
        return NULL;
    return &(it->second);
}
DBErrors CWallet::LoadWallet(bool& fFirstRunRet,bool fLoadTxs)
{
    //LogPrintf("wallet.cpp:loadwallet \n");
    fFirstRunRet = false;    
    
    CPubKey defaultID;
    if (!pwalletdb->GetDefaultWallet(defaultID)){
       
        LogPrintf("wallet.cpp:loadwallet firstrun \n");
        fFirstRunRet =true;        
    }
    else
    {       
        //LogPrintf("wallet.cpp:loadwallet3 \n");
        if(!pwalletdb->IsCurrentWallet(id)){
            if(id!=CPubKey())
                pwalletdb->SetCurrentWallet(id);
            else
                pwalletdb->SetCurrentWallet(defaultID);        
        }
        //LogPrintf("wallet.cpp:loadwallet4 \n");
        LoadKeyStore();
        //LogPrintf("wallet.cpp:loadwallet5 \n");
        LoadAddressBook();
        //LogPrintf("wallet.cpp:loadwallet6 \n");
        if (fLoadTxs)
            LoadTxs();
        //LogPrintf("wallet.cpp:loadwallet7 \n");
        LoadScripts();
        //LogPrintf("wallet.cpp:loadwallet8 \n");        
    }
    uiInterface.LoadWallet(this);
    LogPrintf("wallet.cpp:loadwallet done \n");
    return DB_LOAD_OK;        
}

bool CWallet::LoadKeyStore(){
    if (id==CPubKey()){
        if (!pwalletdb->GetDefaultWallet(id))
            return false;
        pwalletdb->SetCurrentWallet(id);
    }
    if (pwalletdb->ReadKeyStore(this))
        return true;
    
    //TODO remember this watch only address for ease of next run.
    return false;
}
bool CWallet::CreateNew(const SecureString& strKeyData,bool fWriteToDisk){
   AssertLockHeld(cs_wallet); 
    RandAddSeedPerfmon();    
    baseKey.MakeNewKey(true);
    baseKey.GetPubKey(baseKey.pubKey);
    id=baseKey.pubKey;
    mapKeys[id]=0;
    RandAddSeedPerfmon();    
    stepKey.MakeNewKey(true);
    stepKey.GetPubKey(stepKey.pubKey);
    fHasPriv=true;
    fHasPub=true;
    fHasStepPub=true;
    fHasStepPriv=true;
    if(!strKeyData.empty()){
        CCrypter crypter;
        crypter.SetKeyFromPassphrase(strKeyData,encParams);
        Encrypt(crypter);        
    }
    //int64_t nStartTime = GetTime();
    pwalletdb->SetCurrentWallet(id);    
    if(fWriteToDisk)
    {
        CPubKey tmp;
        if (!pwalletdb->GetDefaultWallet(tmp))
        pwalletdb->SetDefaultWallet(id);
        pwalletdb->WriteKeyStore(this); 
    }
    return true;
}
bool CWallet::Set(const CKey& baseKeyIn,const CKey& stepKeyIn,const SecureString& strKeyData,std::string prettyAddress,bool fWriteToDisk)
{
    AssertLockHeld(cs_wallet); 
    baseKey=baseKeyIn;
    baseKey.GetPubKey(id);
    stepKey=stepKeyIn;
    //TODO Checke fhaspriv
    if(!strKeyData.empty()){
        CCrypter crypter;
        crypter.SetKeyFromPassphrase(strKeyData,encParams);
        Encrypt(crypter);        
    }
     //int64_t nStartTime = GetTime();
     if(CompareBase32(CBitcoinAddress(id).ToString(),prettyAddress)==0)
         pwalletdb->SetCurrentWallet(prettyAddress);    
     else
        pwalletdb->SetCurrentWallet(id);    
    
    if(fWriteToDisk)
    {
        CPubKey tmp;
        if (!pwalletdb->GetDefaultWallet(tmp))
        pwalletdb->SetDefaultWallet(id);
        pwalletdb->WriteKeyStore(this); 
    }
    return true;
}
bool CWallet::SwitchToAccount(CPubKey idIn,bool fSetDefault){
    LogPrintf("wallet.cpp:SwitchToAccount id:%s \n",CBitcoinAddress(idIn).ToString()); 
    if(idIn==id)
        return false;
    //LogPrintf("wallet.cpp:SwitchToAccount 2 \n"); 
    if(pwalletdb->IsWalletExists(idIn))
        {        
      //  LogPrintf("wallet.cpp:SwitchToAccount 3 \n"); 
            id=idIn;
            ClearPassword();
            ClearSharedKey();
            bool fFirstRunRet;
            if (LoadWallet(fFirstRunRet,true)==DB_LOAD_OK)
            {
        //        LogPrintf("wallet.cpp:SwitchToAccount 4 \n"); 
                if(fSetDefault)
                {
          //        LogPrintf("wallet.cpp:SwitchToAccount 5 \n");   
                    pwalletdb->SetDefaultWallet(id);
                    
                }
                std::string strAddress;
               pwalletdb->GetCurrentWallet(strAddress);
                NotifyAccountSwitched(strAddress);
                return true;   
            }            
        }
        return false;
    };
    //generate a new extended key
CPubKey CWallet::GenerateNewKey()
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY); // default to compressed public keys if we want 0.6.0 wallets
    nMaxSteps++;
    CPubKey extPub=baseKey.pubKey;
    extPub.AddSteps(stepKey.pubKey,nMaxSteps);
    //CPubKey extID=extPub.GetID();
    mapKeys[extPub]=nMaxSteps;
    pwalletdb->WriteKeyStore(this);     
    NotifyNewExtendedKey(CBitcoinAddress(extPub).ToString());
    return extPub;
}
bool CWallet::AddContacts(const std::map<string,json_spirit::Object>mapContact)
{
    return pwalletdb->AddContacts(mapContact); 
}
bool CWallet::GetContactPubKey(const std::string strContact,CPubKey& pubKey)
{
    json_spirit::Object objInfo;
    if(!pwalletdb->GetContactInfo(strContact,objInfo))
        return false;
    json_spirit::Value val;
    val=find_value(objInfo, "pubkey");
    if(val.type()!=json_spirit::str_type)
        return false;
    return CBitcoinAddress(val.get_str()).GetKey(pubKey);
    
}
//bool CWallet::GetSharedKeyFromAdb(const std::string strContact,CKey& sharedKey)
//{
//    json_spirit::Object objInfo;
//    if(!pwalletdb->GetContactInfo(strContact,objInfo))
//        return false;
//    json_spirit::Value val;
//    val=find_value(objInfo, "sharedKey");
//    if(val.type()!=json_spirit::str_type)
//        return false;
//    CBitcoinSecret keytmp;
//    keytmp.SetString(val.get_str());
//    sharedKey=keytmp.GetKey();
//    if(fUseCrypto)
//    {
//        CKey tmp=sharedKey;
//        return DecryptKey(tmp,sharedKey);
//    }
//    return true;
//}
bool CWallet::MakeSharedKey(const CPubKey& IDLocal,const CPubKey& IDForeign,CKey& sharedKey,bool fStore)
{
    CKey decryptedKey;    
    if(!CCryptoKeyStore::GetKey(IDLocal, decryptedKey))
        return false;
    if(!decryptedKey.MakeSharedKey(IDForeign,sharedKey))
        return false;
    if(fStore)
        StoreSharedKey(IDLocal,IDForeign,sharedKey);
    return true;
}
bool CWallet::EncryptMessages(const std::map<string,std::vector<string> >& mapMessagesIn, std::map<string,std::vector<string> >& mapMessagesOut,bool fEncrypt)
{
    for(std::map<string,std::vector<string> >  ::const_iterator it=mapMessagesIn.begin();it!=mapMessagesIn.end();it++)
    {
        CPubKey pub;
        CBitcoinAddress(it->first).GetKey(pub);
         //LogPrintf("CWallet::EncryptMessages pub\n");       
        CKey sharedKey;        
        if(!GetSharedKey(id,pub,sharedKey))
            MakeSharedKey(id,pub,sharedKey,true);
         //LogPrintf("CWallet::EncryptMessages sharedKey:%s\n",HexStr(sharedKey.begin(),sharedKey.end()));       
        std::vector<string> vMsg=it->second;
        std::vector<string> vMsgOut;
        for(unsigned int i=0;i<vMsg.size();i++){
            if(fEncrypt)
            {                
                string strMsg=vMsg[i];
                std::vector<unsigned char> vchIV(WALLET_CRYPTO_IV_SIZE);
                RandAddSeedPerfmon();                     
                GetRandBytes(&vchIV[0], WALLET_CRYPTO_IV_SIZE);  
                CKeyingMaterial simpleSig(32);
                uint256 hash;
                sharedKey.MakeSimpleSig(vchIV,hash);     
                
                copy(hash.begin(),hash.end(),simpleSig.begin());
                //LogPrintf("CWallet::EncryptMessages simplesig:%s\n",HexStr(simpleSig.begin(),simpleSig.end())); 
                CCrypter crypter;
                if (crypter.SetKey(simpleSig,vchIV))
                {
                    std::vector<unsigned char>  encrypted;
                    CKeyingMaterial origin(strMsg.size());
                    copy(strMsg.begin(),strMsg.end(),origin.begin()); 
                    //LogPrintf("CWallet::EncryptMessages origin:%s\n",HexStr(origin.begin(),origin.end())); 
                    if(crypter.Encrypt(origin, encrypted))
                    {
                         //LogPrintf("CWallet::EncryptMessages encrypted:%s\n",HexStr(encrypted.begin(),encrypted.end())); 
                        CContent ccmsg;
                        std::vector<std::pair<int,string> > vEncoding;
                        string str(vchIV.begin(),vchIV.end());
                        vEncoding.push_back(make_pair(0x140002,str));
                        string str2(encrypted.begin(),encrypted.end());  
                        vEncoding.push_back(make_pair(0x14,str2));
                        //LogPrintf("CWallet::EncryptMessages vstr\n"); 
                        ccmsg.EncodeP(0X15,vEncoding);
                        //LogPrintf("CWallet::EncryptMessages ccmsg %s\n",HexStr(ccmsg.begin(),ccmsg.end())); 
                        vMsgOut.push_back(ccmsg);
                        continue;
                    }
                }
                //}            
                string msgOut="";
                vMsgOut.push_back(msgOut);  
            }
            else
            {
                string msgOut="";
                std::vector<std::pair<int,string> > vContent;
                if(CContent(vMsg[i]).Decode(vContent))
                {
                    //LogPrintf("CWallet::decryptMessages pass1 \n"); 
                    for(unsigned int j=0;j<vContent.size();j++)
                    {
                        if(vContent[j].first==0x15)
                        {                        
                            //LogPrintf("CWallet::decryptMessages pass2 \n"); 
                            std::vector<std::pair<int,string> > vInnerContent;
                            if(CContent(vContent[j].second).Decode(vInnerContent))
                            {
                                //LogPrintf("CWallet::decryptMessages pass3 \n"); 
                                std::vector<unsigned char> vchIV(WALLET_CRYPTO_IV_SIZE);   
                                string strEncrypted;
                                bool fHasIV=false;
                                bool fHasContent=false;
                                for(unsigned int k=0;k<vInnerContent.size();k++)
                                {  
                                //LogPrintf("getmessagesFromtx:effective msg found:%s\n",vInnerContent[k].second);
                                    if(vInnerContent[k].first==0x140002)
                                    {
                                        
                                        string s= vInnerContent[k].second;                               
                                        //LogPrintf("CWallet::decryptMessages pass4 ,s%s size:%s\n",HexStr(s.begin(),s.end()),s.size()); 
                                        if(s.size()==WALLET_CRYPTO_IV_SIZE)
                                        {
                                             //LogPrintf("CWallet::decryptMessages pass41 \n");
                                            copy(s.begin(),s.end(),vchIV.begin());                                            
                                            fHasIV=true;
                                        }
                                    }
                                    else if (vInnerContent[k].first==0x14)
                                    {
                                        //LogPrintf("CWallet::decryptMessages pass5 \n"); 
                                        strEncrypted=vInnerContent[k].second; 
                                      fHasContent=true;
                                    }
                                                               
                                }
                                if(fHasIV&&fHasContent)
                                {
                                    //LogPrintf("decode message:effective msg found:iv:%s,content:%s\n",HexStr(vchIV.begin(),vchIV.end()),strEncrypted);                            
                                    CKeyingMaterial simpleSig(32);
                                    uint256 hash;
                                    sharedKey.MakeSimpleSig(vchIV,hash);                             
                                    copy(hash.begin(),hash.end(),simpleSig.begin());
                                    //LogPrintf("CWallet::EncryptMessages simplesig:%s\n",HexStr(simpleSig.begin(),simpleSig.end())); 
                                    CCrypter crypter;
                                    if (crypter.SetKey(simpleSig,vchIV))
                                    {
                                        CKeyingMaterial decrypted;
                                        std::vector<unsigned char> encrypted(strEncrypted.size());
                                        copy(strEncrypted.begin(),strEncrypted.end(),encrypted.begin()); 
                                        //LogPrintf("CWallet::EncryptMessages encrypted:%s\n",HexStr(encrypted.begin(),encrypted.end())); 
                                        if(crypter.Decrypt(encrypted, decrypted))
                                        {
                                            string strDecrypted(decrypted.begin(),decrypted.end());
                                            //LogPrintf("decode message:decoded msg %s\n",strDecrypted);  
                                            vMsgOut.push_back(CContent(strDecrypted));
                                            continue;
                                        }
                                    }
                                } 
                            }
                        }
                        vMsgOut.push_back(msgOut);
                    }
                }
                    
            }
        }
        mapMessagesOut[it->first]=vMsgOut;
        LogPrintf("CWallet::mapMessagesOut %i\n",mapMessagesOut.size()); 
    }    
    return true;
}
bool CWallet::LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &meta)
{
    AssertLockHeld(cs_wallet); // mapKeyMetadata
    if (meta.nCreateTime && (!nTimeFirstKey || meta.nCreateTime < nTimeFirstKey))
        nTimeFirstKey = meta.nCreateTime;

    mapKeyMetadata[pubkey] = meta;
    return true;
}


bool CWallet::AddCScript(const CScript& redeemScript)
{
    if (!CCryptoKeyStore::AddCScript(redeemScript))
        return false;
    
        return true;
    
}

bool CWallet::LoadCScript(const CScript& redeemScript)
{
    /* A sanity check was added in pull #3843 to avoid adding redeemScripts
     * that never can be redeemed. However, old wallets may still contain
     * these. Do not add them to the wallet and warn. */
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
    {
        std::string strAddr = CBitcoinAddress(CScriptID(redeemScript)).ToString();
        LogPrintf("%s: Warning: This wallet contains a redeemScript of size %i which exceeds maximum size %i thus can never be redeemed. Do not use address %s.\n",
            __func__, redeemScript.size(), MAX_SCRIPT_ELEMENT_SIZE, strAddr);
        return true;
    }

    return CCryptoKeyStore::AddCScript(redeemScript);
}


bool CWallet::DecryptWallet(const SecureString& strWalletPassphrase)
{
    CCrypter crypter;
    {
        LOCK(cs_wallet);
        
        if(!crypter.SetKeyFromPassphrase(strWalletPassphrase,encParams))  
            return false;
        if (!Decrypt(crypter))
            return false;
        pwalletdb->WriteKeyStore(this); 
        NotifyStatusChanged(this);
        return true;
    }
    return false;
}

bool CWallet::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase)
{
    //bool fWasLocked = IsLocked();

    {
        LOCK(cs_wallet);
        Lock();
        
        //int64_t nStartTime = GetTimeMillis();
        CCrypter crypter;   
        if(!crypter.SetKeyFromPassphrase(strOldWalletPassphrase,encParams))  
            return false;
        if (!Decrypt(crypter))
            return false;
        if(!crypter.SetKeyFromPassphrase(strNewWalletPassphrase,encParams))  
            return false;
        if(!Encrypt(crypter))
            return false;
        pwalletdb->WriteKeyStore(this);
        return true;
        
    }

    return false;
}
std::vector<std::string> CWallet::GetAccountList()
{
    std::vector<std::string> vstr;
//    std::vector<CPubKey> IDList;
   pwalletdb->GetWalletList(vstr);
  
//    for(unsigned i=0;i<IDList.size();i++)
//    {
//        vstr.push_back(CBitcoinAddress(IDList[i]).ToString());
//    }
    return vstr;
}
std::string CWallet::GetAddress()
{
  std::string add=  CBitcoinAddress(id).ToString();
  pwalletdb->GetWalletName(id,add);
  return add;
}

bool CWallet::SetMinVersion(enum WalletFeature nVersion, CWalletDB* pwalletdbIn, bool fExplicit)
{
    LOCK(cs_wallet); // nWalletVersion
    if (nWalletVersion >= nVersion)
        return true;
        
    // when doing an explicit upgrade, if we pass the max version permitted, upgrade all the way
    if (fExplicit && nVersion > nWalletMaxVersion)
            nVersion = FEATURE_LATEST;

    nWalletVersion = nVersion;

    if (nVersion > nWalletMaxVersion)
        nWalletMaxVersion = nVersion;
  
    pwalletdb->SetWalletConf("minversion",json_spirit::Value(nWalletVersion));
   
    return true;
}

bool CWallet::SetMaxVersion(int nVersion)
{
    LOCK(cs_wallet); // nWalletVersion, nWalletMaxVersion
    // cannot downgrade below current version
    if (nWalletVersion > nVersion)
        return false;

    nWalletMaxVersion = nVersion;
    pwalletdb->SetWalletConf("maxversion",json_spirit::Value(nWalletVersion));
    return true;
}

set<uint256> CWallet::GetConflicts(const uint256& txid) const
{
    set<uint256> result;
    AssertLockHeld(cs_wallet);

    std::map<uint256, CWalletTx>::const_iterator it = mapWallet.find(txid);
    if (it == mapWallet.end())
        return result;
    const CWalletTx& wtx = it->second;

    std::pair<TxSpends::const_iterator, TxSpends::const_iterator> range;

    BOOST_FOREACH(const CTxIn& txin, wtx.vin)
    {
        if (mapTxSpends.count(txin.prevout) <= 1)
            continue;  // No conflict if zero or one spends
        range = mapTxSpends.equal_range(txin.prevout);
        for (TxSpends::const_iterator it = range.first; it != range.second; ++it)
            result.insert(it->second);
    }
    return result;
}

void CWallet::SyncMetaData(pair<TxSpends::iterator, TxSpends::iterator> range)
{
    // We want all the wallet transactions in range to have the same metadata as
    // the oldest (smallest nOrderPos).
    // So: find smallest nOrderPos:

    int nMinOrderPos = std::numeric_limits<int>::max();
    const CWalletTx* copyFrom = NULL;
    for (TxSpends::iterator it = range.first; it != range.second; ++it)
    {
        const uint256& hash = it->second;
        int n = mapWallet[hash].nOrderPos;
        if (n < nMinOrderPos)
        {
            nMinOrderPos = n;
            copyFrom = &mapWallet[hash];
        }
    }
    // Now copy data from copyFrom to rest:
    for (TxSpends::iterator it = range.first; it != range.second; ++it)
    {
        const uint256& hash = it->second;
        CWalletTx* copyTo = &mapWallet[hash];
        if (copyFrom == copyTo) continue;
        copyTo->mapValue = copyFrom->mapValue;
        copyTo->vOrderForm = copyFrom->vOrderForm;
        // fTimeReceivedIsTxTime not copied on purpose
        // nTimeReceived not copied on purpose
        copyTo->nTimeSmart = copyFrom->nTimeSmart;
        copyTo->fFromMe = copyFrom->fFromMe;
        copyTo->strFromAccount = copyFrom->strFromAccount;
        // nOrderPos not copied on purpose
        // cached members not copied on purpose
    }
}

/**
 * Outpoint is spent if any non-conflicted transaction
 * spends it:
 */
bool CWallet::IsSpent(const uint256& hash, unsigned int n,CAmount value) const
{
//    const COutPoint outpoint(hash, n,value);
//    pair<TxSpends::const_iterator, TxSpends::const_iterator> range;
//    range = mapTxSpends.equal_range(outpoint);
//
//    for (TxSpends::const_iterator it = range.first; it != range.second; ++it)
//    {
//        const uint256& wtxid = it->second;
//        std::map<uint256, CWalletTx>::const_iterator mit = mapWallet.find(wtxid);
//        if (mit != mapWallet.end() && mit->second.GetDepthInMainChain() >= 0)
//            return true; // Spent
//    }
//    return false;
    //browser :rewrite this function by enquering from the whole coin db
    CCoinsViewCache view(pcoinsTip);    
    const CCoins* coins = view.AccessCoins(hash);
            if (!coins || !coins->IsAvailable(n)||mempool.mapNextTx.count(COutPoint(hash,n,value))) {
                return true;
       }
    return false;
}

void CWallet::AddToSpends(const COutPoint& outpoint, const uint256& wtxid)
{
    mapTxSpends.insert(make_pair(outpoint, wtxid));
    //LogPrintf("wallet addtospends2\n");
    pair<TxSpends::iterator, TxSpends::iterator> range;
    range = mapTxSpends.equal_range(outpoint);
    SyncMetaData(range);
}


void CWallet::AddToSpends(const uint256& wtxid)
{
    assert(mapWallet.count(wtxid));
    CWalletTx& thisTx = mapWallet[wtxid];
    if (thisTx.IsCoinBase()) // Coinbases don't spend anything!
        return;
    //LogPrintf("wallet addtospends1\n");
    BOOST_FOREACH(const CTxIn& txin, thisTx.vin)
        AddToSpends(txin.prevout, wtxid);
}

bool CWallet::EncryptWallet(const SecureString& strWalletPassphrase)
{
    if (IsCrypted())
        return false;    
    LogPrintf("CWallet::EncryptWallet1,basepub size:%i,steppub size:%i\n",baseKey.pubKey.size(),stepKey.pubKey.size());
    CCrypter crypter;
    if (!crypter.SetKeyFromPassphrase(strWalletPassphrase,encParams))
        return false;  
    {
        LOCK(cs_wallet);
        if (!Encrypt(crypter))
            return false;
        //LogPrintf("CWallet::EncryptWallet2,basepub size:%i,steppub size:%i\n",baseKey.pubKey.size(),stepKey.pubKey.size());
        pwalletdb->WriteKeyStore(this);
        //LogPrintf("CWallet::EncryptWallet2\n");
    }
        
    //NotifyStatusChanged(this);
    return true;
}

int64_t CWallet::IncOrderPosNext(CWalletDB *pwalletdb)
{
    AssertLockHeld(cs_wallet); // nOrderPosNext
    int64_t nRet = nOrderPosNext++;    
    pwalletdb->WriteOrderPosNext(nOrderPosNext);    
    return nRet;
}

CWallet::TxItems CWallet::OrderedTxItems( bool fIncludeExtendedKeys)
{
    AssertLockHeld(cs_wallet); // mapWallet
    //CWalletDB walletdb();

    // First: get all CWalletTx and CAccountingEntry into a sorted-by-order multimap.
    TxItems txOrdered;

    // Note: maintaining indices in the database of (account,time) --> txid and (account, time) --> acentry
    // would make this much faster for applications that do this a lot.
    for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
        CWalletTx* wtx = &((*it).second);
        txOrdered.insert(make_pair(wtx->nOrderPos, TxPair(wtx, (CAccountingEntry*)0)));
        
    }
    LogPrintf("cwallet::orderedtxitems txs:%u \n",txOrdered.size());
//    acentries.clear();
//    pwalletdb->ListAccountCreditDebit(strAccount, acentries);
//    BOOST_FOREACH(CAccountingEntry& entry, acentries)
//    {
//        txOrdered.insert(make_pair(entry.nOrderPos, TxPair((CWalletTx*)0, &entry)));
//    }

    return txOrdered;
}

void CWallet::MarkDirty()
{
    {
        LOCK(cs_wallet);
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item, mapWallet)
            item.second.MarkDirty();
    }
}
void CWallet::SetBestChain(const CBlockLocator& loc){
    
}
bool CWallet::AddToWallet(const CWalletTx& wtxIn, bool fFromLoadWallet)
{
    uint256 hash = wtxIn.GetHash();
 //LogPrintf("wallet:AddToWallet1\n");   
    if (fFromLoadWallet)
    {
        mapWallet[hash] = wtxIn;
        mapWallet[hash].BindWallet(this);
        AddToSpends(hash);
        //LogPrintf("wallet:AddToWallet2\n");   
    }
    else
    {
        //LogPrintf("wallet:AddToWallet3\n");   
        LOCK(cs_wallet);
        // Inserts only if not already there, returns tx inserted or tx found
        pair<map<uint256, CWalletTx>::iterator, bool> ret = mapWallet.insert(make_pair(hash, wtxIn));
        CWalletTx& wtx = (*ret.first).second;
        wtx.BindWallet(this);
        //LogPrintf("wallet:AddToWallet4\n");   
        bool fInsertedNew = ret.second;
        if (fInsertedNew)
        {
            wtx.nTimeReceived = GetAdjustedTime();
            wtx.nOrderPos = nOrderPosNext++; 
            //LogPrintf("wallet:AddToWallet5\n");   
            wtx.nTimeSmart = wtx.nTimeReceived;
            if (wtxIn.hashBlock != 0)
            {
                if (mapBlockIndex.count(wtxIn.hashBlock))
                {
                    int64_t latestNow = wtx.nTimeReceived;
                    int64_t latestEntry = 0;
                    {
                        // Tolerate times up to the last timestamp in the wallet not more than 5 minutes into the future
                        int64_t latestTolerated = latestNow + 300;
                        //LogPrintf("wallet:AddToWallet6\n");   
                        //std::list<CAccountingEntry> acentries;
                        TxItems txOrdered = OrderedTxItems();
                        //LogPrintf("wallet:AddToWallet7\n");   
                        for (TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
                        {
                            CWalletTx *const pwtx = (*it).second.first;
                            if (pwtx == &wtx)
                                continue;
                            CAccountingEntry *const pacentry = (*it).second.second;
                            int64_t nSmartTime;
                            if (pwtx)
                            {
                                nSmartTime = pwtx->nTimeSmart;
                                if (!nSmartTime)
                                    nSmartTime = pwtx->nTimeReceived;
                            }
                            else
                                nSmartTime = pacentry->nTime;
                            if (nSmartTime <= latestTolerated)
                            {
                                latestEntry = nSmartTime;
                                if (nSmartTime > latestNow)
                                    latestNow = nSmartTime;
                                break;
                            }
                        }
                    }
                    //LogPrintf("wallet:AddToWallet8\n");   
                    int64_t blocktime = mapBlockIndex[wtxIn.hashBlock]->GetBlockTime();
                    wtx.nTimeSmart = std::max(latestEntry, std::min(blocktime, latestNow));
                }
                else
                    LogPrintf("AddToWallet() : found %s in block %s not in index\n",
                             wtxIn.GetHash().ToString(),
                             wtxIn.hashBlock.ToString());
            }
            //LogPrintf("wallet:AddToWallet81\n");
            AddToSpends(hash);
        }
        //LogPrintf("wallet:AddToWallet9\n");   
        bool fUpdated = false;
        if (!fInsertedNew)
        {
            // Merge
            if (wtxIn.hashBlock != 0 && wtxIn.hashBlock != wtx.hashBlock)
            {
                wtx.hashBlock = wtxIn.hashBlock;
                fUpdated = true;
            }
            if (wtxIn.nIndex != -1 && (wtxIn.vMerkleBranch != wtx.vMerkleBranch || wtxIn.nIndex != wtx.nIndex))
            {
                wtx.vMerkleBranch = wtxIn.vMerkleBranch;
                wtx.nIndex = wtxIn.nIndex;
                fUpdated = true;
            }
            if (wtxIn.fFromMe && wtxIn.fFromMe != wtx.fFromMe)
            {
                wtx.fFromMe = wtxIn.fFromMe;
                fUpdated = true;
            }
        }

        //// debug print
        LogPrintf("AddToWallet %s  %s%s\n", wtxIn.GetHash().ToString(), (fInsertedNew ? "new" : ""), (fUpdated ? "update" : ""));

        // Write to disk
        //if (fInsertedNew || fUpdated)
        //    if (!wtx.WriteToDisk())
        //        return false;

        // Break debit/credit balance caches:
        wtx.MarkDirty();

        // Notify UI of new or updated transaction
        //NotifyTransactionChanged(this, hash, fInsertedNew ? CT_NEW : CT_UPDATED);

        // notify an external script when a wallet transaction comes in or is updated
        std::string strCmd = GetArg("-walletnotify", "");

        if ( !strCmd.empty())
        {
            boost::replace_all(strCmd, "%s", wtxIn.GetHash().GetHex());
            boost::thread t(runCommand, strCmd); // thread runs free
        }

    }
    return true;
}

/**
 * Add a transaction to the wallet, or update it.
 * pblock is optional, but should be provided if the transaction is known to be in a block.
 * If fUpdate is true, existing transactions will be updated.
 */
bool CWallet::AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate)
{
    
    {
        AssertLockHeld(cs_wallet);
        
        bool fExisted = mapWallet.count(tx.GetHash()) != 0;
        if (fExisted && !fUpdate) return false;
        
        if (fExisted || IsMine(tx) || IsFromMe(tx))
        {
           
            CWalletTx wtx(this,tx);
            
            // Get merkle branch if transaction was found in a block
            if (pblock)
                wtx.SetMerkleBranch(*pblock);
            
            return AddToWallet(wtx);
            
        }
    }
    return false;
}

void CWallet::SyncTransaction(const CTransaction& tx, const CBlock* pblock)
{
    
    LOCK2(cs_main, cs_wallet);
    //uint256 hash = tx.GetHash();
    uint256 blockHash;
    if(pblock==NULL)
        blockHash=uint256(0);
    else
        blockHash=pblock->GetHash();
    //LogPrintf("notfiytransactionchanged\n");
    if(!fReindex){
        NotifyTransactionChanged(tx.GetHash(),blockHash);
        //LogPrintf("notfiytransactionchanged\n");
    }
    if (!AddToWalletIfInvolvingMe(tx, pblock, true))
        return; // Not one of ours

    // If a transaction changes 'conflicted' state, that changes the balance
    // available of the outputs it spends. So force those to be
    // recomputed, also:
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
        if (mapWallet.count(txin.prevout.hash))
            mapWallet[txin.prevout.hash].MarkDirty();
    }
    LogPrintf("CWallet::SyncTransaction new wallet tx recieved\n");
    //if(fReindex)
       // NotifyTransactionChanged(tx.GetHash(),blockHash);
}

void CWallet::EraseFromWallet(const uint256 &hash)
{
    
    {
        LOCK(cs_wallet);
        //if (mapWallet.erase(hash))
            //CWalletDB(strWalletFile).EraseTx(hash);
    }
    return;
}


isminetype CWallet::IsMine(const CTxIn &txin) const
{
    {
        LOCK(cs_wallet);
        map<uint256, CWalletTx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
        if (mi != mapWallet.end())
        {
            const CWalletTx& prev = (*mi).second;
            if (txin.prevout.n < prev.vout.size())
                return IsMine(prev.vout[txin.prevout.n]);
        }
    }
    return ISMINE_NO;
}

CAmount CWallet::GetDebit(const CTxIn &txin, const isminefilter& filter) const
{
    {
        LOCK(cs_wallet);
        //if(IsCoinBase())
        //    return 0;
        map<uint256, CWalletTx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
        if (mi != mapWallet.end())
        {
            const CWalletTx& prev = (*mi).second;
            if (txin.prevout.n < prev.vout.size())
                if (IsMine(prev.vout[txin.prevout.n]) & filter)
                    return prev.vout[txin.prevout.n].nValue;
        }
    }
    return 0;
}

bool CWallet::IsChange(const CTxOut& txout) const
{
    // TODO: fix handling of 'change' outputs. The assumption is that any
    // payment to a script that is ours, but is not in the address book
    // is change. That assumption is likely to break when we implement multisignature
    // wallets that return change back into a multi-signature-protected address;
    // a better way of identifying which outputs are 'the send' and which are
    // 'the change' will need to be implemented (maybe extend CWalletTx to remember
    // which output, if any, was change).
    if (::IsMine(*this, txout.scriptPubKey))
    {
        CTxDestination address;
        if (!ExtractDestination(txout.scriptPubKey, address))
            return true;

        LOCK(cs_wallet);
        if (!mapAddressBook.count(address))
            return true;
    }
    return false;
}

int64_t CWalletTx::GetTxTime() const
{
    int64_t n = nTimeSmart;
    return n ? n : nTimeReceived;
}

int CWalletTx::GetRequestCount() const
{
    // Returns -1 if it wasn't being tracked
    int nRequests = -1;
    {
        LOCK(pwallet->cs_wallet);
        if (IsCoinBase())
        {
            // Generated block
            if (hashBlock != 0)
            {
                map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
                if (mi != pwallet->mapRequestCount.end())
                    nRequests = (*mi).second;
            }
        }
        else
        {
            // Did anyone request this transaction?
            map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
            if (mi != pwallet->mapRequestCount.end())
            {
                nRequests = (*mi).second;

                // How about the block it's in?
                if (nRequests == 0 && hashBlock != 0)
                {
                    map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
                    if (mi != pwallet->mapRequestCount.end())
                        nRequests = (*mi).second;
                    else
                        nRequests = 1; // If it's in someone else's block it must have got out
                }
            }
        }
    }
    return nRequests;
}

void CWalletTx::GetAmounts(list<COutputEntry>& listReceived,
                           list<COutputEntry>& listSent, CAmount& nFee, string& strSentAccount, const isminefilter& filter) const
{
    nFee = 0;
    listReceived.clear();
    listSent.clear();
    strSentAccount = strFromAccount;

    // Compute fee:
    CAmount nDebit=0;
    if (!IsCoinBase())     
        nDebit= GetDebit(filter);
    if (nDebit > 0) // debit>0 means we signed/sent this transaction
    {
        CAmount nValueOut = GetValueOut();
        nFee = nDebit - nValueOut;
    }
    int nExtOuts=0;
    // Sent/received.
    for (unsigned int i = 0; i < vout.size(); ++i)
    {
        const CTxOut& txout = vout[i];
        isminetype fIsMine = pwallet->IsMine(txout);
        if(!fIsMine)
            nExtOuts++;
        // Only need to handle txouts if AT LEAST one of these is true:
        //   1) they debit from us (sent)
        //   2) the output is to us (received)
        if (nDebit > 0)
        {
            // Don't report 'change' txouts
            if (pwallet->IsChange(txout))
                continue;
        }
        else if (!(fIsMine & filter))
            continue;

        // In either case, we need to get the destination address
        CTxDestination address;
        if (!ExtractDestination(txout.scriptPubKey, address))
        {
            //LogPrintf("CWalletTx::GetAmounts: Unknown transaction type found, txid %s\n",
            //         this->GetHash().ToString());
            address = CNoDestination();
        }

        COutputEntry output = {address, txout.nValue, (int)i};

        // If we are debited by the transaction, add the output as a "sent" entry
        if (nDebit > 0)
            listSent.push_back(output);

        // If we are receiving the output, add it as a "received" entry
        if (fIsMine & filter)
            listReceived.push_back(output);
    }
    if(nExtOuts==0&&nDebit>0)//all vouts are mine,this is a in-wallet tx
    {
        
            COutputEntry output = {CNoDestination(), nFee, 0};
            listSent.push_back(output);
    }
        
}

void CWalletTx::GetAccountAmounts(const string& strAccount, CAmount& nReceived,
                                  CAmount& nSent, CAmount& nFee, const isminefilter& filter) const
{
    nReceived = nSent = nFee = 0;

    CAmount allFee;
    string strSentAccount;
    list<COutputEntry> listReceived;
    list<COutputEntry> listSent;
    GetAmounts(listReceived, listSent, allFee, strSentAccount, filter);

    if (strAccount == strSentAccount)
    {
        BOOST_FOREACH(const COutputEntry& s, listSent)
            nSent += s.amount;
        nFee = allFee;
    }
    {
        LOCK(pwallet->cs_wallet);
        BOOST_FOREACH(const COutputEntry& r, listReceived)
        {
            if(r.IsFrozen)
                continue;
            if (pwallet->mapAddressBook.count(r.destination))
            {
                map<CTxDestination, CAddressBookData>::const_iterator mi = pwallet->mapAddressBook.find(r.destination);
                if (mi != pwallet->mapAddressBook.end() && (*mi).second.name == strAccount)
                    nReceived += r.amount;
            }
            else if (strAccount.empty())
            {
                nReceived += r.amount;
            }
        }
    }
}


bool CWalletTx::WriteToDisk()
{
    return false;//CWalletDB(pwallet->strWalletFile).WriteTx(GetHash(), *this);
}

/**
 * Scan the block chain (starting in pindexStart) for transactions
 * from or to us. If fUpdate is true, found transactions that already
 * exist in the wallet will be updated.
 */
int CWallet::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate)
{
    int ret = 0;
    int64_t nNow = GetTime();

    CBlockIndex* pindex = pindexStart;
    {
        LOCK2(cs_main, cs_wallet);

        // no need to read and scan block, if block was created before
        // our wallet birthday (as adjusted for block time variability)
        while (pindex && nTimeFirstKey && (pindex->GetBlockTime() < (nTimeFirstKey - 7200)))
            pindex = chainActive.Next(pindex);

        ShowProgress(_("Rescanning..."), 0); // show rescan progress in GUI as dialog or on splashscreen, if -rescan on startup
        double dProgressStart = Checkpoints::GuessVerificationProgress(pindex, false);
        double dProgressTip = Checkpoints::GuessVerificationProgress(chainActive.Tip(), false);
        while (pindex)
        {
            if (pindex->nHeight % 100 == 0 && dProgressTip - dProgressStart > 0.0)
                ShowProgress(_("Rescanning..."), std::max(1, std::min(99, (int)((Checkpoints::GuessVerificationProgress(pindex, false) - dProgressStart) / (dProgressTip - dProgressStart) * 100))));

            CBlock block;
            ReadBlockFromDisk(block, pindex);
            BOOST_FOREACH(CTransaction& tx, block.vtx)
            {
                if (AddToWalletIfInvolvingMe(tx, &block, fUpdate))
                    ret++;
            }
            pindex = chainActive.Next(pindex);
            if (GetTime() >= nNow + 60) {
                nNow = GetTime();
                LogPrintf("Still rescanning. At block %d. Progress=%f\n", pindex->nHeight, Checkpoints::GuessVerificationProgress(pindex));
            }
        }
        ShowProgress(_("Rescanning..."), 100); // hide progress dialog in GUI
    }
    return ret;
}

void CWallet::ReacceptWalletTransactions()
{
    LOCK2(cs_main, cs_wallet);
    BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item, mapWallet)
    {
        const uint256& wtxid = item.first;
        CWalletTx& wtx = item.second;
        assert(wtx.GetHash() == wtxid);

        int nDepth = wtx.GetDepthInMainChain();

        if (!wtx.IsCoinBase() && nDepth < 0)
        {
            // Try to add to memory pool
            LOCK(mempool.cs);
            wtx.AcceptToMemoryPool(false);
        }
    }
}

void CWalletTx::RelayWalletTransaction()
{
    if (!IsCoinBase())
    {
        if (GetDepthInMainChain() == 0) {
            LogPrintf("Relaying wtx %s\n", GetHash().ToString());
            RelayTransaction((CTransaction)*this);
        }
    }
}

set<uint256> CWalletTx::GetConflicts() const
{
    set<uint256> result;
    if (pwallet != NULL)
    {
        uint256 myHash = GetHash();
        result = pwallet->GetConflicts(myHash);
        result.erase(myHash);
    }
    return result;
}

void CWallet::ResendWalletTransactions()
{
    // Do this infrequently and randomly to avoid giving away
    // that these are our transactions.
    if (GetTime() < nNextResend)
        return;
    bool fFirst = (nNextResend == 0);
    nNextResend = GetTime() + GetRand(30 * 60);
    if (fFirst)
        return;

    // Only do it if there's been a new block since last time
    if (nTimeBestReceived < nLastResend)
        return;
    nLastResend = GetTime();

    // Rebroadcast any of our txes that aren't in a block yet
    LogPrintf("ResendWalletTransactions()\n");
    {
        LOCK(cs_wallet);
        // Sort them in chronological order
        multimap<unsigned int, CWalletTx*> mapSorted;
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item, mapWallet)
        {
            CWalletTx& wtx = item.second;
            // Don't rebroadcast until it's had plenty of time that
            // it should have gotten in already by now.
            if (nTimeBestReceived - (int64_t)wtx.nTimeReceived > 5 * 60)
                mapSorted.insert(make_pair(wtx.nTimeReceived, &wtx));
        }
        BOOST_FOREACH(PAIRTYPE(const unsigned int, CWalletTx*)& item, mapSorted)
        {
            CWalletTx& wtx = *item.second;
            wtx.RelayWalletTransaction();
        }
    }
}

/** @} */ // end of mapWallet




/** @defgroup Actions
 *
 * @{
 */

bool CWallet::LoadTxs(){
    //TODO load from disk file     
    std::vector<CPubKey> vIds;
        for (KeyMap::iterator it = mapKeys.begin(); it != mapKeys.end(); ++it)
            vIds.push_back(it->first);    
    mapWallet=GetWalletTxs(vIds);
    nOrderPosNext=mapWallet.size();
    return true;
}
std::map<uint256, CWalletTx> CWallet::GetWalletTxs(std::vector<CPubKey> vIds)const {
     LogPrintf("wallet.cpp getwallettxs ids:%u \n",vIds.size());
    std::vector<CScript> vScriptPubkeys;
    for (std::vector<CPubKey>::iterator it = vIds.begin(); it != vIds.end(); ++it){        
        CBitcoinAddress address;
        address.Set(*it);
        vScriptPubkeys.push_back(GetScriptForDestination(address.Get()));
        LogPrintf("wallet.cpp getwallettxs script:%s \n",GetScriptForDestination(address.Get()).ToString());
    }
    std::vector<std::pair<CTransaction, uint256> > vTxs;
    GetTransactions(vScriptPubkeys,vTxs);
    std::map<uint256, CWalletTx> mapWalletTx;
    LogPrintf("wallet.cpp getwallettxs txs:%u \n",vTxs.size());
    int64_t nOrderPos=0;
    for (std::vector<std::pair<CTransaction, uint256> >::reverse_iterator it = vTxs.rbegin(); it != vTxs.rend(); ++it){                
        CWalletTx wtx(this,it->first);
        wtx.nOrderPos=nOrderPos;
        nOrderPos++;
        wtx.hashBlock=it->second;
        wtx.SetMerkleBranch();
        mapWalletTx.insert(make_pair(it->first.GetHash(),wtx));        
    }
    
    LogPrintf("wallet.cpp mapwallettxs:%u \n",mapWalletTx.size());
    return mapWalletTx;
}
CAmount CWallet::GetBalance(std::vector<CPubKey> vIds) const
{
    map<uint256, CWalletTx> mapWTx=GetWalletTxs(vIds);
    CAmount nTotal = 0;
    {        
        for (map<uint256, CWalletTx>::const_iterator it = mapWTx.begin(); it != mapWTx.end(); ++it)
        {
            const CWalletTx* pcoin = &(*it).second;
            if (pcoin->IsTrusted())
                nTotal += pcoin->GetAvailableCredit();
        }
    }

    return nTotal;
}
CAmount CWallet::GetBalance(bool fRefresh) 
{
    if (fRefresh)
        LoadTxs();
    CAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx* pcoin = &(*it).second;
            if (pcoin->IsTrusted())
                nTotal += pcoin->GetAvailableCredit();
        }
    }
    return nTotal;
}

CAmount CWallet::GetUnconfirmedBalance(bool fRefresh) 
{
    if (fRefresh)
        LoadTxs();
    CAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx* pcoin = &(*it).second;
            //if (!IsFinalTx(*pcoin) || (!pcoin->IsTrusted() && pcoin->GetDepthInMainChain() == 0))
            //LogPrintf("CWallet::GetUnconfirmedBalance depth:%i \n",pcoin->GetDepthInMainChain());
            if (pcoin->GetDepthInMainChain() == 0)
                nTotal += pcoin->GetCredit(ISMINE_SPENDABLE|ISMINE_WATCH_ONLY);
            //LogPrintf("CWallet::GetUnconfirmedBalance:total %u \n",nTotal);
        }
    }
    return nTotal;
}

CAmount CWallet::GetImmatureBalance(bool fRefresh) 
{
    if (fRefresh)
        LoadTxs();
    CAmount nTotal = 0;
    {
        LOCK2(cs_main, cs_wallet);
        for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const CWalletTx* pcoin = &(*it).second;
            nTotal += pcoin->GetImmatureCredit();
        }
    }
    return nTotal;
}

/**
 * populate vCoins with vector of available COutputs.
 */
void CWallet::AvailableCoins(vector<COutput>& vCoins, bool fOnlyConfirmed, const CCoinControl *coinControl) const
{
    vCoins.clear();

    {
        LOCK2(cs_main, cs_wallet);
        for (map<uint256, CWalletTx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
        {
            const uint256& wtxid = it->first;
            const CWalletTx* pcoin = &(*it).second;

            if (!IsFinalTx(*pcoin))
                continue;

            if (fOnlyConfirmed && !pcoin->IsTrusted())
                continue;
            int nDepth = pcoin->GetDepthInMainChain();
            
            if (nDepth <= 0)
                continue;

            for (unsigned int i = 0; i < pcoin->vout.size(); i++) {
                if (pcoin->GetBlocksToMaturity(i) > 0)
                    continue;
                isminetype mine = IsMine(pcoin->vout[i]);
                if (!(IsSpent(wtxid, i,pcoin->vout[i].nValue)) && mine != ISMINE_NO &&
                    !IsLockedCoin((*it).first, i,pcoin->vout[i].nValue) && pcoin->vout[i].nValue > 0 &&
                    (!coinControl || !coinControl->HasSelected() || coinControl->IsSelected((*it).first, i,pcoin->vout[i].nValue)))
                        vCoins.push_back(COutput(pcoin, i, nDepth, (mine & ISMINE_SPENDABLE) != ISMINE_NO));
            }
        }
    }
}

static void ApproximateBestSubset(vector<pair<CAmount, pair<const CWalletTx*,unsigned int> > >vValue, const CAmount& nTotalLower, const CAmount& nTargetValue,
                                  vector<char>& vfBest, CAmount& nBest, int iterations = 1000)
{
    vector<char> vfIncluded;

    vfBest.assign(vValue.size(), true);
    nBest = nTotalLower;

    seed_insecure_rand();

    for (int nRep = 0; nRep < iterations && nBest != nTargetValue; nRep++)
    {
        vfIncluded.assign(vValue.size(), false);
        CAmount nTotal = 0;
        bool fReachedTarget = false;
        for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
        {
            for (unsigned int i = 0; i < vValue.size(); i++)
            {
                //The solver here uses a randomized algorithm,
                //the randomness serves no real security purpose but is just
                //needed to prevent degenerate behavior and it is important
                //that the rng is fast. We do not use a constant random sequence,
                //because there may be some privacy improvement by making
                //the selection random.
                if (nPass == 0 ? insecure_rand()&1 : !vfIncluded[i])
                {
                    nTotal += vValue[i].first;
                    vfIncluded[i] = true;
                    if (nTotal >= nTargetValue)
                    {
                        fReachedTarget = true;
                        if (nTotal < nBest)
                        {
                            nBest = nTotal;
                            vfBest = vfIncluded;
                        }
                        nTotal -= vValue[i].first;
                        vfIncluded[i] = false;
                    }
                }
            }
        }
    }
}

bool CWallet::SelectCoinsMinConf(const CAmount& nTargetValue, int nConfMine, int nConfTheirs, vector<COutput> vCoins,
                                 set<pair<const CWalletTx*,unsigned int> >& setCoinsRet, CAmount& nValueRet) const
{
    setCoinsRet.clear();
    nValueRet = 0;

    // List of values less than target
    pair<CAmount, pair<const CWalletTx*,unsigned int> > coinLowestLarger;
    coinLowestLarger.first = std::numeric_limits<CAmount>::max();
    coinLowestLarger.second.first = NULL;
    vector<pair<CAmount, pair<const CWalletTx*,unsigned int> > > vValue;
    CAmount nTotalLower = 0;

    random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);
    //TODO sort cheques
    BOOST_FOREACH(const COutput &output, vCoins)
    {
        if (!output.fSpendable)
            continue;
        
        const CWalletTx *pcoin = output.tx;

        if (output.nDepth < (pcoin->IsFromMe(ISMINE_ALL) ? nConfMine : nConfTheirs))
            continue;
        
        int i = output.i;
        CAmount n = pcoin->vout[i].nValue;
        
        pair<CAmount,pair<const CWalletTx*,unsigned int> > coin = make_pair(n,make_pair(pcoin, i));

        if (n == nTargetValue)
        {
            setCoinsRet.insert(coin.second);
            nValueRet += coin.first;
            return true;
        }
        else if (n < nTargetValue + CENT)
        {
            vValue.push_back(coin);
            nTotalLower += n;
        }
        else if (n < coinLowestLarger.first)
        {
            coinLowestLarger = coin;
        }
    }
    if (coinLowestLarger.second.first != NULL){
        setCoinsRet.insert(coinLowestLarger.second);
        nValueRet += coinLowestLarger.first;
        return true;
    }
    if (nTotalLower < nTargetValue)
        return false;
    if (nTotalLower == nTargetValue)
    {
        for (unsigned int i = 0; i < vValue.size(); ++i)
        {
            setCoinsRet.insert(vValue[i].second);
            nValueRet += vValue[i].first;
        }
        return true;
    }
    // Solve subset sum by stochastic approximation
    sort(vValue.rbegin(), vValue.rend(), CompareValueOnly());
    vector<char> vfBest;
    CAmount nBest;

    ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest, 1000);
    if (nBest != nTargetValue && nTotalLower >= nTargetValue + CENT)
        ApproximateBestSubset(vValue, nTotalLower, nTargetValue + CENT, vfBest, nBest, 1000);

    // If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
    //                                   or the next bigger coin is closer), return the bigger coin
    if (coinLowestLarger.second.first &&
        ((nBest != nTargetValue && nBest < nTargetValue + CENT) || coinLowestLarger.first <= nBest))
    {
        setCoinsRet.insert(coinLowestLarger.second);
        nValueRet += coinLowestLarger.first;
    }
    else {
        for (unsigned int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
            {
                setCoinsRet.insert(vValue[i].second);
                nValueRet += vValue[i].first;
            }

        LogPrint("selectcoins", "SelectCoins() best subset: ");
        for (unsigned int i = 0; i < vValue.size(); i++)
            if (vfBest[i])
                LogPrint("selectcoins", "%s ", FormatMoney(vValue[i].first));
        LogPrint("selectcoins", "total %s\n", FormatMoney(nBest));
    }

    return true;
}

bool CWallet::SelectCoins(const CAmount& nTargetValue, set<pair<const CWalletTx*,unsigned int> >& setCoinsRet, CAmount& nValueRet, const CCoinControl* coinControl) const
{
    LogPrintf("wallet.cpp %s targetvalue:%ul\n",__func__,nTargetValue);
    vector<COutput> vCoins;
    AvailableCoins(vCoins, true, coinControl);
    LogPrintf("wallet.cpp %s available chenques:%u\n",__func__,vCoins.size());
    // coin control -> return all selected outputs (we want all selected to go into the transaction for sure)
    if (coinControl && coinControl->HasSelected())
    {
        BOOST_FOREACH(const COutput& out, vCoins)
        {
            if(!out.fSpendable)
                continue;
            nValueRet += out.tx->vout[out.i].nValue;
            setCoinsRet.insert(make_pair(out.tx, out.i));
        }
        return (nValueRet >= nTargetValue);
    }

    return (SelectCoinsMinConf(nTargetValue, 1, 6, vCoins, setCoinsRet, nValueRet) ||
            SelectCoinsMinConf(nTargetValue, 1, 1, vCoins, setCoinsRet, nValueRet));
            //(bSpendZeroConfChange && SelectCoinsMinConf(nTargetValue, 0, 1, vCoins, setCoinsRet, nValueRet)));
}
bool CWallet::CreateTransactionUnsigned(const CPaymentOrder& pr,
                                CWalletTx& wtxNew,std::string& strFailReason)
{
    CAmount nValue = 0;
    BOOST_FOREACH (const CTxOut& s, pr.vout)
    {
        if (s.nValue < 0)
        {
            strFailReason = _("Transaction amounts must be positive");
            return false;
        }
        nValue += s.nValue;
    }
    if (pr.vout.empty() || nValue < 0)
    {
        strFailReason = _("Transaction amounts must be positive");
        return false;
    }

    wtxNew.fTimeReceivedIsTxTime = true;
    wtxNew.BindWallet(this);
    CMutableTransaction txNew;    
    //txNew.nLockTime=pr.nLockTime;
    CAmount nFeeRet;
    {
        LOCK2(cs_main, cs_wallet);
        {
            nFeeRet = 0;
            while (true)
            {
                txNew.vin.clear();
                txNew.vout.clear();
                wtxNew.fFromMe = true;

                CAmount nTotalValue = nValue + nFeeRet;
                // vouts to the payees
                BOOST_FOREACH (const CTxOut& s, pr.vout)
                {
                    
                    if (s.IsDust(::minRelayTxFee))
                    {
                        strFailReason = _("output amount too small");
                        return false;
                    }
                    txNew.vout.push_back(s);
                }

                // Choose coins to use
                set<pair<const CWalletTx*,unsigned int> > setCoins;
                CAmount nValueIn = 0;
                if (!SelectCoins(nTotalValue, setCoins, nValueIn))
                {
                    strFailReason = _("Insufficient funds");
                    return false;
                }
                CAmount nChange = nValueIn - nValue - nFeeRet;

                if (nChange > 0)
                {
                    // Fill a vout to ourself
                    // TODO: pass in scriptChange instead of reservekey so
                    // change transaction isn't always pay-to-bitcoin-address
                    if((pr.nRequestType==PR_DOMAIN_REGISTER||pr.nRequestType==PR_DOMAIN_RENEW)&&txNew.vout.size()==1)
                    {
                        // note: for domain register, the recepiente is ourslefves, so just add change to that out, no need an indepent change out.
                        txNew.vout[0].nValue+=nChange;
                    }
                    else
                    {
                    CScript scriptChange;
                    
                    // coin control: send change to custom address
                    if (pr.changeAddress.size()>0)
                        scriptChange = pr.changeAddress;
                    // no coin control: send change to newly generated address
                    else
                        scriptChange = pr.vFrom[0];
                    CTxOut newTxOut(nChange, scriptChange);
                    if(!newTxOut.IsDust(::minRelayTxFee))
                        txNew.vout.push_back(newTxOut);
                }                
                }                
                // Fill vin
                BOOST_FOREACH(const PAIRTYPE(const CWalletTx*,unsigned int)& coin, setCoins)
                    txNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second,coin.first->vout[coin.second].nValue));

                // Sign
//                int nIn = 0;
//                BOOST_FOREACH(const PAIRTYPE(const CWalletTx*,unsigned int)& coin, setCoins)
//                    if (!SignSignature(*this, *coin.first, txNew, nIn++))
//                    {
//                        strFailReason = _("Signing transaction failed");
//                        return false;
//                    }

                // Embed the constructed transaction data in wtxNew.
                *static_cast<CTransaction*>(&wtxNew) = CTransaction(txNew);

                // Limit size,TODO calculate multisig and script size 
                unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION)+txNew.vin.size()*67;
                if (nBytes >= MAX_STANDARD_TX_SIZE)
                {
                    strFailReason = _("Transaction too large");
                    return false;
                }
                CAmount nFeeNeeded = int64_t(pr.dFeeRate*nBytes);

                if (nFeeRet >= nFeeNeeded)
                    break; // Done, enough fee included.

                // Include more fee and try again.
                nFeeRet = nFeeNeeded;
                continue;
            }
        }
    }
    return true;
}
bool CWallet::SignTransaction(const CWalletTx& wtxIn,CWalletTx& wtxSigned,int nSigType)
{
                
    CMutableTransaction txSigned=CMutableTransaction(wtxIn);    
                unsigned int nIn = 0;
                BOOST_FOREACH(CTxIn in, wtxIn.vin){
                    CWalletTx wtx=mapWallet[in.prevout.hash];                    
                    if (!SignSignature(*this, wtx, txSigned, nIn++,nSigType)){
                     LogPrintf("wallet.cpp:signtransaction signsignature failed\n");   
                        return false;
                    }
                }                
                // Embed the constructed transaction data in wtxNew.
                *static_cast<CTransaction*>(&wtxSigned) = CTransaction(txSigned);
                return true;
}
bool SignAndSendTx(CWallet* pwallet,const CWalletTx& tx,const int nSigType, const int nOP,const SecureString& ssInput,const bool fDelete,CWalletTx& wtxSigned,std::string& result)
{      
    if(nOP==1)
        if(!pwallet->SetPassword(ssInput)){
            if(fDelete)
            delete pwallet;
            result= ("{\"error\":\"wrong password\"}");
            return false;
        }    
    if(nOP==2){
        //TODO signer funcs
        std::vector<CScript> sigs;
//        if (!DecodeSigs(string(ssInput.begin(),ssInput.end()),sigs)){
//            if(fDelete)
//            delete pwallet;
//            result=("{\"error\":\"invalid signatures\"}");
//            return false;
//        }        
        CMutableTransaction mtx=CMutableTransaction(tx);        
        for(unsigned int i=0;i<mtx.vin.size();i++)
            mtx.vin[i].scriptSig=sigs[i];
        *static_cast<CTransaction*>(&wtxSigned) = CTransaction(mtx);
    }
    else if(!pwallet->SignTransaction(tx, wtxSigned,nSigType)){
        if(fDelete)
        delete pwallet;
        result= ("{\"error\":\"sign transaction failed\"}");            
        return false;
    }
     LogPrintf("SignAndSendTx:signOK\n");
    if (!wtxSigned.AcceptToMemoryPool(false))
        {            
            LogPrintf("SignAndSendTx:sendtx : Error: Transaction not valid\n");
           if(fDelete)
            delete pwallet;
            result =("{\"error\":\"tx rejected\"}");
            return false;            
        }
     LogPrintf("SignAndSendTx:acceptedto mempool\n");
     RelayTransaction(wtxSigned);
     LogPrintf("SignAndSendTx:sendtx :%s\n",EncodeHexTx(CTransaction(wtxSigned)));
     if(fDelete)
        delete pwallet;
    result= ("{\"success\":\"tx sent\"}");
    return true;            
}

bool CWallet::CreateTransaction(const vector<pair<CScript, CAmount> >& vecSend,
                                CWalletTx& wtxNew, CReserveKey& reservekey, CAmount& nFeeRet, std::string& strFailReason, const CCoinControl* coinControl)
{
    CAmount nValue = 0;
    BOOST_FOREACH (const PAIRTYPE(CScript, CAmount)& s, vecSend)
    {
        if (nValue < 0)
        {
            strFailReason = _("Transaction amounts must be positive");
            return false;
        }
        nValue += s.second;
    }
    if (vecSend.empty() || nValue < 0)
    {
        strFailReason = _("Transaction amounts must be positive");
        return false;
    }

    wtxNew.fTimeReceivedIsTxTime = true;
    wtxNew.BindWallet(this);
    CMutableTransaction txNew;
    //cccoin:for test
//    txNew.nLockTime=chainActive.Height()+10;
    {
        LOCK2(cs_main, cs_wallet);
        {
            nFeeRet = 0;
            while (true)
            {
                txNew.vin.clear();
                txNew.vout.clear();
                wtxNew.fFromMe = true;

                CAmount nTotalValue = nValue + nFeeRet;
//                double dPriority = 0;
                // vouts to the payees
                BOOST_FOREACH (const PAIRTYPE(CScript, CAmount)& s, vecSend)
                {
                    CTxOut txout(s.second, s.first);
                    if (txout.IsDust(::minRelayTxFee))
                    {
                        strFailReason = _("Transaction amount too small");
                        return false;
                    }
                    txNew.vout.push_back(txout);
                }

                // Choose coins to use
                set<pair<const CWalletTx*,unsigned int> > setCoins;
                CAmount nValueIn = 0;
                if (!SelectCoins(nTotalValue, setCoins, nValueIn, coinControl))
                {
                    strFailReason = _("Insufficient funds");
                    return false;
                }
//                BOOST_FOREACH(/(const CWalletTx*, unsigned int) pcoin, setCoins)
//                {
//                    CAmount nCredit = pcoin.first->vout[pcoin.second].nValue;
//                    //The coin age after the next block (depth+1) is used instead of the current,
//                    //reflecting an assumption the user would accept a bit more delay for
//                    //a chance at a free transaction.
//                    //But mempool inputs might still be in the mempool, so their age stays 0
//                    int age = pcoin.first->GetDepthInMainChain();
//                    if (age != 0)
//                        age += 1;
//                    dPriority += (double)nCredit * age;
                //}

                CAmount nChange = nValueIn - nValue - nFeeRet;

                if (nChange > 0)
                {
                    // Fill a vout to ourself
                    // TODO: pass in scriptChange instead of reservekey so
                    // change transaction isn't always pay-to-bitcoin-address
                    CScript scriptChange;

                    // coin control: send change to custom address
                    if (coinControl && !boost::get<CNoDestination>(&coinControl->destChange))
                        scriptChange = GetScriptForDestination(coinControl->destChange);

                    // no coin control: send change to newly generated address
                    else
                    {
                        // Note: We use a new key here to keep it from being obvious which side is the change.
                        //  The drawback is that by not reusing a previous key, the change may be lost if a
                        //  backup is restored, if the backup doesn't have the new private key for the change.
                        //  If we reused the old key, it would be possible to add code to look for and
                        //  rediscover unknown transactions that were written with keys of ours to recover
                        //  post-backup change.

                        // Reserve a new key pair from key pool
                        CPubKey vchPubKey;
                        bool ret;
                        ret = reservekey.GetReservedKey(vchPubKey);
                        assert(ret); // should never fail, as we just unlocked

                        scriptChange = GetScriptForDestination(vchPubKey);
                    }

                    CTxOut newTxOut(nChange, scriptChange);

                    // Never create dust outputs; if we would, just
                    // add the dust to the fee.
//                    if (newTxOut.nValue < DUST_THRESHOLD)
//                    {
//                        nFeeRet += nChange;
//                        reservekey.ReturnKey();
//                    }
//                    else
//                    {
                        // Insert change txn at random position:
                        vector<CTxOut>::iterator position = txNew.vout.begin()+GetRandInt(txNew.vout.size()+1);
                        txNew.vout.insert(position, newTxOut);
//                    }
                }
                else
                    reservekey.ReturnKey();

                // Fill vin
                BOOST_FOREACH(const PAIRTYPE(const CWalletTx*,unsigned int)& coin, setCoins)
                    txNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second,coin.first->vout[coin.second].nValue));

                // Sign
                int nIn = 0;
                BOOST_FOREACH(const PAIRTYPE(const CWalletTx*,unsigned int)& coin, setCoins)
                    if (!SignSignature(*this, *coin.first, txNew, nIn++))
                    {
                        strFailReason = _("Signing transaction failed");
                        return false;
                    }

                // Embed the constructed transaction data in wtxNew.
                *static_cast<CTransaction*>(&wtxNew) = CTransaction(txNew);

                // Limit size
                unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
                if (nBytes >= MAX_STANDARD_TX_SIZE)
                {
                    strFailReason = _("Transaction too large");
                    return false;
                }
                //dPriority = wtxNew.ComputePriority(dPriority, nBytes);

                // Can we complete this as a free transaction?
//                if (fSendFreeTransactions && nBytes <= MAX_FREE_TRANSACTION_CREATE_SIZE)
//                {
//                    // Not enough fee: enough priority?
//                    double dPriorityNeeded = mempool.estimatePriority(nTxConfirmTarget);
//                    // Not enough mempool history to estimate: use hard-coded AllowFree.
//                    if (dPriorityNeeded <= 0 && AllowFree())
//                        break;
//
//                    // Small enough, and priority high enough, to send for free
//                    if (dPriorityNeeded > 0 && dPriority >= dPriorityNeeded)
//                        break;
//                }

                CAmount nFeeNeeded = GetMinimumFee(nBytes, nTxConfirmTarget, mempool);

                // If we made it here and we aren't even able to meet the relay fee on the next pass, give up
                // because we must be at the maximum allowed fee.
                if (nFeeNeeded < ::minRelayTxFee.GetFee(nBytes))
                {
                    strFailReason = _("Transaction too large for fee policy");
                    return false;
                }

                if (nFeeRet >= nFeeNeeded)
                    break; // Done, enough fee included.

                // Include more fee and try again.
                nFeeRet = nFeeNeeded;
                continue;
            }
        }
    }
    return true;
}

bool CWallet::CreateTransaction(CScript scriptPubKey, const CAmount& nValue,
                                CWalletTx& wtxNew, CReserveKey& reservekey, CAmount& nFeeRet, std::string& strFailReason, const CCoinControl* coinControl)
{
    vector< pair<CScript, CAmount> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, nValue));
    return CreateTransaction(vecSend, wtxNew, reservekey, nFeeRet, strFailReason, coinControl);
}

/**
 * Call after CreateTransaction unless you want to abort
 */
bool CWallet::CommitTransaction(CWalletTx& wtxNew, CReserveKey& reservekey)
{
    {
        LOCK2(cs_main, cs_wallet);
        LogPrintf("CommitTransaction:\n%s", wtxNew.ToString());
        {
            
            // Take key pair from key pool so it won't be used again
            reservekey.KeepKey();

            // Add tx to wallet, because if it has change it's also ours,
            // otherwise just for transaction history.
            AddToWallet(wtxNew);

            // Notify that old coins are spent
            set<CWalletTx*> setCoins;
            BOOST_FOREACH(const CTxIn& txin, wtxNew.vin)
            {
                CWalletTx &coin = mapWallet[txin.prevout.hash];
                coin.BindWallet(this);
                //NotifyTransactionChanged(this, coin.GetHash(), CT_UPDATED);
            }

            
        }

        // Track how many getdata requests our transaction gets
        mapRequestCount[wtxNew.GetHash()] = 0;

        // Broadcast
        if (!wtxNew.AcceptToMemoryPool(false))
        {
            // This must not fail. The transaction has already been signed and recorded.
            LogPrintf("CommitTransaction() : Error: Transaction not valid");
            return false;
        }
        wtxNew.RelayWalletTransaction();
    }
    return true;
}

CAmount CWallet::GetMinimumFee(unsigned int nTxBytes, unsigned int nConfirmTarget, const CTxMemPool& pool)
{
    // payTxFee is user-set "I want to pay this much"
    CAmount nFeeNeeded = payTxFee.GetFee(nTxBytes);
    // user selected total at least (default=true)
    if (fPayAtLeastCustomFee && nFeeNeeded > 0 && nFeeNeeded < payTxFee.GetFeePerK())
        nFeeNeeded = payTxFee.GetFeePerK();
    // User didn't set: use -txconfirmtarget to estimate...
    if (nFeeNeeded == 0)
        nFeeNeeded = pool.estimateFee(nConfirmTarget).GetFee(nTxBytes);
    // ... unless we don't have enough mempool data, in which case fall
    // back to a hard-coded fee
    if (nFeeNeeded == 0)
        nFeeNeeded = minTxFee.GetFee(nTxBytes);
    // prevent user from paying a non-sense fee (like 1 satoshi): 0 < fee < minRelayFee
    if (nFeeNeeded < ::minRelayTxFee.GetFee(nTxBytes))
        nFeeNeeded = ::minRelayTxFee.GetFee(nTxBytes);
    // But always obey the maximum
    if (nFeeNeeded > maxTxFee)
        nFeeNeeded = maxTxFee;
    return nFeeNeeded;
}










bool CWallet::SetAddressBook(const CTxDestination& address, const string& strName, const string& strPurpose)
{
    bool fUpdated = false;
    {
        LOCK(cs_wallet); // mapAddressBook
        std::map<CTxDestination, CAddressBookData>::iterator mi = mapAddressBook.find(address);
        fUpdated = mi != mapAddressBook.end();
        mapAddressBook[address].name = strName;
        if (!strPurpose.empty()) /* update purpose only if requested */
            mapAddressBook[address].purpose = strPurpose;
    }
    NotifyAddressBookChanged(this, address, strName, ::IsMine(*this, address) != ISMINE_NO,
                             strPurpose, (fUpdated ? CT_UPDATED : CT_NEW) );
    
//    if (!strPurpose.empty() && !CWalletDB().WritePurpose(CBitcoinAddress(address).ToString(), strPurpose))
//        return false;
    return CWalletDB().WriteName(CBitcoinAddress(address).ToString(), strName);
}

bool CWallet::DelAddressBook(const CTxDestination& address)
{
//    {
//        LOCK(cs_wallet); // mapAddressBook
//
//        if(fFileBacked)
//        {
//            // Delete destdata tuples associated with address
//            std::string strAddress = CBitcoinAddress(address).ToString();
//            BOOST_FOREACH(const PAIRTYPE(string, string) &item, mapAddressBook[address].destdata)
//            {
//                CWalletDB(strWalletFile).EraseDestData(strAddress, item.first);
//            }
//        }
//        mapAddressBook.erase(address);
//    }
//
//    NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address) != ISMINE_NO, "", CT_DELETED);
//
//    if (!fFileBacked)
//        return false;
//    CWalletDB(strWalletFile).ErasePurpose(CBitcoinAddress(address).ToString());
//    return CWalletDB(strWalletFile).EraseName(CBitcoinAddress(address).ToString());
    return false;
}






std::map<CTxDestination, CAmount> CWallet::GetAddressBalances()
{
    map<CTxDestination, CAmount> balances;

    {
        LOCK(cs_wallet);
        BOOST_FOREACH(PAIRTYPE(uint256, CWalletTx) walletEntry, mapWallet)
        {
            CWalletTx *pcoin = &walletEntry.second;

            if (!IsFinalTx(*pcoin) || !pcoin->IsTrusted())
                continue;
            int nDepth = pcoin->GetDepthInMainChain();
            if (nDepth < (pcoin->IsFromMe(ISMINE_ALL) ? 0 : 1))
                continue;

            for (unsigned int i = 0; i < pcoin->vout.size(); i++)
            {
                if (pcoin->GetBlocksToMaturity(i) > 0)
                    continue;
                CTxDestination addr;
                if (!IsMine(pcoin->vout[i]))
                    continue;
                if(!ExtractDestination(pcoin->vout[i].scriptPubKey, addr))
                    continue;

                CAmount n = IsSpent(walletEntry.first, i,pcoin->vout[i].nValue) ? 0 : pcoin->vout[i].nValue;

                if (!balances.count(addr))
                    balances[addr] = 0;
                balances[addr] += n;
            }
        }
    }

    return balances;
}

set< set<CTxDestination> > CWallet::GetAddressGroupings()
{
    AssertLockHeld(cs_wallet); // mapWallet
    set< set<CTxDestination> > groupings;
    set<CTxDestination> grouping;

    BOOST_FOREACH(PAIRTYPE(uint256, CWalletTx) walletEntry, mapWallet)
    {
        CWalletTx *pcoin = &walletEntry.second;

        if (pcoin->vin.size() > 0)
        {
            bool any_mine = false;
            // group all input addresses with each other
            BOOST_FOREACH(CTxIn txin, pcoin->vin)
            {
                CTxDestination address;
                if(!IsMine(txin)) /* If this input isn't mine, ignore it */
                    continue;
                if(!ExtractDestination(mapWallet[txin.prevout.hash].vout[txin.prevout.n].scriptPubKey, address))
                    continue;
                grouping.insert(address);
                any_mine = true;
            }

            // group change with input addresses
            if (any_mine)
            {
               BOOST_FOREACH(CTxOut txout, pcoin->vout)
                   if (IsChange(txout))
                   {
                       CTxDestination txoutAddr;
                       if(!ExtractDestination(txout.scriptPubKey, txoutAddr))
                           continue;
                       grouping.insert(txoutAddr);
                   }
            }
            if (grouping.size() > 0)
            {
                groupings.insert(grouping);
                grouping.clear();
            }
        }

        // group lone addrs by themselves
        for (unsigned int i = 0; i < pcoin->vout.size(); i++)
            if (IsMine(pcoin->vout[i]))
            {
                CTxDestination address;
                if(!ExtractDestination(pcoin->vout[i].scriptPubKey, address))
                    continue;
                grouping.insert(address);
                groupings.insert(grouping);
                grouping.clear();
            }
    }

    set< set<CTxDestination>* > uniqueGroupings; // a set of pointers to groups of addresses
    map< CTxDestination, set<CTxDestination>* > setmap;  // map addresses to the unique group containing it
    BOOST_FOREACH(set<CTxDestination> grouping, groupings)
    {
        // make a set of all the groups hit by this new group
        set< set<CTxDestination>* > hits;
        map< CTxDestination, set<CTxDestination>* >::iterator it;
        BOOST_FOREACH(CTxDestination address, grouping)
            if ((it = setmap.find(address)) != setmap.end())
                hits.insert((*it).second);

        // merge all hit groups into a new single group and delete old groups
        set<CTxDestination>* merged = new set<CTxDestination>(grouping);
        BOOST_FOREACH(set<CTxDestination>* hit, hits)
        {
            merged->insert(hit->begin(), hit->end());
            uniqueGroupings.erase(hit);
            delete hit;
        }
        uniqueGroupings.insert(merged);

        // update setmap
        BOOST_FOREACH(CTxDestination element, *merged)
            setmap[element] = merged;
    }

    set< set<CTxDestination> > ret;
    BOOST_FOREACH(set<CTxDestination>* uniqueGrouping, uniqueGroupings)
    {
        ret.insert(*uniqueGrouping);
        delete uniqueGrouping;
    }

    return ret;
}

set<CTxDestination> CWallet::GetAccountAddresses(string strAccount) const
{
    LOCK(cs_wallet);
    set<CTxDestination> result;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, CAddressBookData)& item, mapAddressBook)
    {
        const CTxDestination& address = item.first;
        const string& strName = item.second.name;
        if (strName == strAccount)
            result.insert(address);
    }
    return result;
}
void CWallet::UpdatedTransaction(const uint256 &hashTx)
{
    {
        LOCK(cs_wallet);
        // Only notify UI if this transaction is in this wallet
        map<uint256, CWalletTx>::const_iterator mi = mapWallet.find(hashTx);
        //if (mi != mapWallet.end())
            //NotifyTransactionChanged(this, hashTx, CT_UPDATED);
    }
}

void CWallet::LockCoin(COutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.insert(output);
}

void CWallet::UnlockCoin(COutPoint& output)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.erase(output);
}

void CWallet::UnlockAllCoins()
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    setLockedCoins.clear();
}

bool CWallet::IsLockedCoin(uint256 hash, unsigned int n,CAmount value) const
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    COutPoint outpt(hash, n,value);

    return (setLockedCoins.count(outpt) > 0);
}

void CWallet::ListLockedCoins(std::vector<COutPoint>& vOutpts)
{
    AssertLockHeld(cs_wallet); // setLockedCoins
    for (std::set<COutPoint>::iterator it = setLockedCoins.begin();
         it != setLockedCoins.end(); it++) {
        COutPoint outpt = (*it);
        vOutpts.push_back(outpt);
    }
}

/** @} */ // end of Actions

class CAffectedKeysVisitor : public boost::static_visitor<void> {
private:
    const CKeyStore &keystore;
    std::vector<CPubKey> &vKeys;

public:
    CAffectedKeysVisitor(const CKeyStore &keystoreIn, std::vector<CPubKey> &vKeysIn) : keystore(keystoreIn), vKeys(vKeysIn) {}

    void Process(const CScript &script) {
        txnouttype type;
        std::vector<CTxDestination> vDest;
        int nRequired;
        if (ExtractDestinations(script, type, vDest, nRequired)) {
            BOOST_FOREACH(const CTxDestination &dest, vDest)
                boost::apply_visitor(*this, dest);
        }
    }

    void operator()(const CPubKey &keyId) {
        if (keystore.HaveKey(keyId))
            vKeys.push_back(keyId);
    }
    void operator()(const CScript &script) {     
    }
    
    void operator()(const CScriptID &scriptId) {
        CScript script;
        if (keystore.GetCScript(scriptId, script))
            Process(script);
    }

    void operator()(const CNoDestination &none) {}
};

bool CWallet::AddDestData(const CTxDestination &dest, const std::string &key, const std::string &value)
{
    if (boost::get<CNoDestination>(&dest))
        return false;

    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));    
    return pwalletdb->WriteDestData(CBitcoinAddress(dest).ToString(), key, value);
}

bool CWallet::EraseDestData(const CTxDestination &dest, const std::string &key)
{
    if (!mapAddressBook[dest].destdata.erase(key))
        return false;    
    return pwalletdb->EraseDestData(CBitcoinAddress(dest).ToString(), key);
}

bool CWallet::LoadDestData(const CTxDestination &dest, const std::string &key, const std::string &value)
{
    mapAddressBook[dest].destdata.insert(std::make_pair(key, value));
    return true;
}

bool CWallet::GetDestData(const CTxDestination &dest, const std::string &key, std::string *value) const
{
    std::map<CTxDestination, CAddressBookData>::const_iterator i = mapAddressBook.find(dest);
    if(i != mapAddressBook.end())
    {
        CAddressBookData::StringMap::const_iterator j = i->second.destdata.find(key);
        if(j != i->second.destdata.end())
        {
            if(value)
                *value = j->second;
            return true;
        }
    }
    return false;
}
CAmount CWalletTx::GetAvailableCredit(bool fUseCache) const
    {
        if (pwallet == 0)
            return 0;
        
        // Must wait until coinbase is safely deep enough in the chain before valuing it
        //if (GetBlocksToMaturity() > 0)
        //    return 0;
        
        //if (fUseCache && fAvailableCreditCached)
       //     return nAvailableCreditCached;
        //LogPrintf("wallet.h GetAvailableCredit:3 \n");
        CAmount nCredit = 0;
        uint256 hashTx = GetHash();
        CCoinsViewCache view(pcoinsTip);    
        const CCoins* coins = view.AccessCoins(hashTx);
        if (!coins)
                return 0;        
        //LogPrintf("wallet.cpp GetAvailableCredit vout size:%u \n",coins->vout.size());
        for (unsigned int i = 0; i < vout.size(); i++)
        {
                //LogPrintf("wallet.cpp vout%u value:%u \n",i,vout[i].nValue);
                //LogPrintf("wallet.cpp coinavailable:%b \n",coins->IsAvailable(i));
                //LogPrintf("wallet.cpp in mempool:%b \n",mempool.mapNextTx.count(COutPoint(hashTx,i,vout[i].nValue)));
            if (coins->IsAvailable(i)&&!mempool.mapNextTx.count(COutPoint(hashTx,i,vout[i].nValue))&&GetBlocksToMaturity(i) == 0)
            {
                //LogPrintf("wallet.h GetAvailableCredit:5 \n");
                const CTxOut &txout = vout[i];
                nCredit += pwallet->GetCredit(txout, ISMINE_SPENDABLE);
                //LogPrintf("wallet.cpp coinavailable:%b \n",pwallet->IsMine(txout));
                //LogPrintf("wallet.cpp GetAvailableCredit: %u \n",nCredit);
                if (!MoneyRange(nCredit))
                    throw std::runtime_error("CWalletTx::GetAvailableCredit() : value out of range");
            }
            
        }

        //nAvailableCreditCached = nCredit;
        //fAvailableCreditCached = true;
        return nCredit;
    }

CWalletKey::CWalletKey(int64_t nExpires)
{
    nTimeCreated = (nExpires ? GetTime() : 0);
    nTimeExpires = nExpires;
}

int CMerkleTx::SetMerkleBranch(const CBlock& block)
{
    AssertLockHeld(cs_main);
    CBlock blockTmp;

    // Update the tx's hashBlock
    hashBlock = block.GetHash();

    // Locate the transaction
    for (nIndex = 0; nIndex < (int)block.vtx.size(); nIndex++)
        if (block.vtx[nIndex] == *(CTransaction*)this)
            break;
    if (nIndex == (int)block.vtx.size())
    {
        vMerkleBranch.clear();
        nIndex = -1;
        LogPrintf("ERROR: SetMerkleBranch() : couldn't find tx in block\n");
        return 0;
    }

    // Fill in merkle branch
    vMerkleBranch = block.GetMerkleBranch(nIndex);

    // Is the tx in a block that's in the main chain
    BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi == mapBlockIndex.end())
        return 0;
    const CBlockIndex* pindex = (*mi).second;
    if (!pindex || !chainActive.Contains(pindex))
        return 0;
    nBlockHeight=pindex->nHeight;
    return chainActive.Height() - pindex->nHeight + 1;
}

int CMerkleTx::GetDepthInMainChainINTERNAL(const CBlockIndex* &pindexRet) const
{
    //LogPrintf("CMerkleTx::GetDepthInMainChainINTERNAL %s\n",hashBlock.ToString());
    if (hashBlock == 0)//||hashBlock==uint256(0))
        return 0;
    AssertLockHeld(cs_main);

    // Find the block it claims to be in
    BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi == mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex = (*mi).second;
    if (!pindex || !chainActive.Contains(pindex))
        return 0;
    //temporarily enable merkle check functions
//    if (nIndex==-1)
//        SetMerkleBranch();
    // Make sure the merkle branch connects to this block
    if (!fMerkleVerified)
    {
        if (CBlock::CheckMerkleBranch(GetHash(), vMerkleBranch, nIndex) != pindex->hashMerkleRoot)
            return 0;
        fMerkleVerified = true;
    }

    pindexRet = pindex;
    return chainActive.Height() - pindex->nHeight + 1;
}
bool CMerkleTx::SetMerkleBranch(){
    CBlock block;
        BlockMap::iterator it2 = mapBlockIndex.find(hashBlock);
        if (it2 != mapBlockIndex.end()){
            CBlockIndex* pindex =it2->second;
            if (!ReadBlockFromDisk(block, pindex))
                return false;//state.Abort("Failed to read block");
            SetMerkleBranch(block);
            return true;
        }
        return false;
}
int CMerkleTx::GetDepthInMainChain(const CBlockIndex* &pindexRet) const
{
    AssertLockHeld(cs_main);
    int nResult = GetDepthInMainChainINTERNAL(pindexRet);
    //if (nResult == 0 && !mempool.exists(GetHash()))
    //    return -1; // Not in chain, not in mempool

    return nResult;
}

int CMerkleTx::GetBlocksToMaturity(int nPos) const
{
    
    if(nPos>=(int)vout.size())
        nPos=-1;
    if(nPos==-1)
    {
        int maxBlocks=0;
        for(unsigned int i=0;i<vout.size();i++)
            maxBlocks=max(maxBlocks,GetBlocksToMaturity(i));
        return maxBlocks;
    }
    if (vout[nPos].nLockTime!=0){        
        if ((int64_t)vout[nPos].nLockTime < LOCKTIME_THRESHOLD )
            return max(0, (int)((int)vout[nPos].nLockTime+1 - (int)chainActive.Height()));  
        else{
            int lockBlocks;
            lockBlocks=(int)(((int64_t)vout[nPos].nLockTime-GetAdjustedTime())/Params().TargetSpacing());
            return max(0, lockBlocks);
        }
    }
        return 0;
}


bool CMerkleTx::AcceptToMemoryPool(bool fLimitFree, bool fRejectInsaneFee)
{
    CValidationState state;
    LogPrintf("wallet:accept to mempool\n");
    return ::AcceptToMemoryPool(mempool, state, *this, fLimitFree, NULL, fRejectInsaneFee);
}

