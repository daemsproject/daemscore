// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "rpcserver.h"
#include "pow.h"
#include "uint256.h"
#include "timedata.h"
#include "ccc/domain.h"
#include "ccc/contentutil.h"
#include <stdint.h>

#include <boost/thread.hpp>

using namespace std;

void static BatchWriteCoins(CLevelDBBatch &batch, const uint256 &hash, const CCoins &coins) {
    if (coins.IsPruned())
        batch.Erase(make_pair('c', hash));
    else
        batch.Write(make_pair('c', hash), coins);
}

void static BatchWriteHashBestChain(CLevelDBBatch &batch, const uint256 &hash) {
    batch.Write('B', hash);
}

CCoinsViewDB::CCoinsViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "chainstate", nCacheSize, fMemory, fWipe) {
}

bool CCoinsViewDB::GetCoins(const uint256 &txid, CCoins &coins) const {
    return db.Read(make_pair('c', txid), coins);
}

bool CCoinsViewDB::HaveCoins(const uint256 &txid) const {
    return db.Exists(make_pair('c', txid));
}

uint256 CCoinsViewDB::GetBestBlock() const {
    uint256 hashBestChain;
    if (!db.Read('B', hashBestChain))
        return uint256(0);
    return hashBestChain;
}

bool CCoinsViewDB::BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock) {
    CLevelDBBatch batch;
    size_t count = 0;
    size_t changed = 0;
    for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & CCoinsCacheEntry::DIRTY) {
            BatchWriteCoins(batch, it->first, it->second.coins);
            changed++;
        }
        count++;
        CCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }
    if (hashBlock != uint256(0))
        BatchWriteHashBestChain(batch, hashBlock);

    LogPrint("coindb", "Committing %u changed transactions (out of %u) to coin database...\n", (unsigned int)changed, (unsigned int)count);
    return db.WriteBatch(batch);
}

CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) : CLevelDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) {
}

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
    return Write(make_pair('b', blockindex.GetBlockHash()), blockindex);
}

bool CBlockTreeDB::WriteBlockFileInfo(int nFile, const CBlockFileInfo &info) {
    return Write(make_pair('f', nFile), info);
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &info) {
    return Read(make_pair('f', nFile), info);
}

bool CBlockTreeDB::WriteLastBlockFile(int nFile) {
    return Write('l', nFile);
}

bool CBlockTreeDB::WriteReindexing(bool fReindexing) {
    if (fReindexing)
        return Write('R', '1');
    else
        return Erase('R');
}

bool CBlockTreeDB::ReadReindexing(bool &fReindexing) {
    fReindexing = Exists('R');
    return true;
}

bool CBlockTreeDB::ReadLastBlockFile(int &nFile) {
    return Read('l', nFile);
}

bool CCoinsViewDB::GetStats(CCoinsStats &stats) const {
    /* It seems that there are no "const iterators" for LevelDB.  Since we
       only need read operations on it, use a const-cast to get around
       that restriction.  */
    boost::scoped_ptr<leveldb::Iterator> pcursor(const_cast<CLevelDBWrapper*>(&db)->NewIterator());
    pcursor->SeekToFirst();

    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    stats.hashBlock = GetBestBlock();
    ss << stats.hashBlock;
    CAmount nTotalAmount = 0;
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        try {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data()+slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'c') {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data()+slValue.size(), SER_DISK, CLIENT_VERSION);
                CCoins coins;
                ssValue >> coins;
                uint256 txhash;
                ssKey >> txhash;
                ss << txhash;
                ss << VARINT(coins.nVersion);
                ss << (coins.fCoinBase ? 'c' : 'n');
                ss << VARINT(coins.nHeight);
                stats.nTransactions++;
                for (unsigned int i=0; i<coins.vout.size(); i++) {
                    const CTxOut &out = coins.vout[i];
                    if (!out.IsNull()) {
                        stats.nTransactionOutputs++;
                        ss << VARINT(i+1);
                        ss << out;
                        nTotalAmount += out.nValue;
                    }
                }
                stats.nSerializedSize += 32 + slValue.size();
                ss << VARINT(0);
            }
            pcursor->Next();
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    stats.nHeight = mapBlockIndex.find(GetBestBlock())->second->nHeight;
    stats.hashSerialized = ss.GetHash();
    stats.nTotalAmount = nTotalAmount;
    return true;
}

bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos) {
    return Read(make_pair('t', txid), pos);
}

bool CBlockTreeDB::WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> >&vect) {
    CLevelDBBatch batch;
    for (std::vector<std::pair<uint256,CDiskTxPos> >::const_iterator it=vect.begin(); it!=vect.end(); it++)
        batch.Write(make_pair('t', it->first), it->second);
    return WriteBatch(batch);
}

bool CBlockTreeDB::WriteFlag(const std::string &name, bool fValue) {
    return Write(std::make_pair('F', name), fValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const std::string &name, bool &fValue) {
    char ch;
    if (!Read(std::make_pair('F', name), ch))
        return false;
    fValue = ch == '1';
    return true;
}

bool CBlockTreeDB::LoadBlockIndexGuts()
{
    boost::scoped_ptr<leveldb::Iterator> pcursor(NewIterator());

    CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
    ssKeySet << make_pair('b', uint256(0));
    pcursor->Seek(ssKeySet.str());

    // Load mapBlockIndex
    while (pcursor->Valid()) {
        boost::this_thread::interruption_point();
        try {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data()+slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'b') {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data()+slValue.size(), SER_DISK, CLIENT_VERSION);
                CDiskBlockIndex diskindex;
                ssValue >> diskindex;

                // Construct block index object
                CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
                pindexNew->pprev          = InsertBlockIndex(diskindex.hashPrev);
                pindexNew->nHeight        = diskindex.nHeight;
                pindexNew->nFile          = diskindex.nFile;
                pindexNew->nDataPos       = diskindex.nDataPos;
                pindexNew->nUndoPos       = diskindex.nUndoPos;
                pindexNew->nVersion       = diskindex.nVersion;
                pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                pindexNew->nBlockHeight   = diskindex.nBlockHeight;
                pindexNew->nTime          = diskindex.nTime;
                pindexNew->nBits          = diskindex.nBits;
                pindexNew->nNonce         = diskindex.nNonce;
                pindexNew->nStatus        = diskindex.nStatus;
                pindexNew->nTx            = diskindex.nTx;

                // Cccoin: Disable PoW Sanity check while loading block index from disk.
                // We use the sha256 hash for the block index for performance reasons, which is recorded for later use.
                // CheckProofOfWork() uses the scrypt hash which is discarded after a block is accepted.
                // While it is technically feasible to verify the PoW, doing so takes several minutes as it
                // requires recomputing every PoW hash during every Cccoin startup.
                // We opt instead to simply trust the data that is on your local disk.
                //if (!CheckProofOfWork(pindexNew->GetBlockPoWHash(), pindexNew->nBits))
                //    return error("LoadBlockIndex() : CheckProofOfWork failed: %s", pindexNew->ToString());

                pcursor->Next();
            } else {
                break; // if shutdown requested or finished loading block index
            }
        } catch (std::exception &e) {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }

    return true;
}
CScript2TxPosViewDB::CScript2TxPosViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "script2txposdb", nCacheSize, fMemory, fWipe) {
}

bool CScript2TxPosViewDB::GetTxPosList(const CScript scriptPubKey,std::vector<CDiskTxPos> &vTxPos)  {     
    return db.Read(scriptPubKey, vTxPos);
}
bool CScript2TxPosViewDB::BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> > &mapScriptTxPosList) {
    
    CLevelDBBatch batch;
    size_t count = 0;
    //size_t changed = 1;
    for (std::map<CScript, std::vector<CDiskTxPos> >::const_iterator it = mapScriptTxPosList.begin(); it != mapScriptTxPosList.end();it++) {
        
            batch.Write(it->first, it->second);
           
        count++;       
    }    
    LogPrint("coindb", "Committing %u changed addresses to tam database...\n", (unsigned int)count);
    return db.WriteBatch(batch);
}

bool CScript2TxPosViewDB::Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos) {     
    return db.Write(scriptPubKey,vTxPos);
}

CDomainViewDB::CDomainViewDB(bool fWipe) : db(GetDataDir() / "sqlitedb", fWipe) {
}
bool CDomainViewDB::Update(const CScript ownerIn,const string& strDomainContent,const uint64_t lockedValue,const uint32_t nLockTimeIn,const CLink link)
{
    LogPrintf("txdb CDomainViewDB Update %s %s\n", HexStr(strDomainContent.begin(),strDomainContent.end()),link.ToString());
    CDomain domain;
    bool fRegister=false;
    bool fForward=false;
    bool fHasRecord=false;
    if(!domain.SetContent(CContent(strDomainContent),ownerIn,fRegister,fForward))
        return false;
    LogPrintf("update domain name %s \n", domain.strDomain);    
    if(domain.strDomain=="")
            return false;
    CDomain existingDomain;
    if(domain.IsLevel2())
    {
        LogPrintf("update domain level2 \n");    
        CDomain level1Domain;               
        if(!GetDomainByName(GetLevel1Domain(domain.strDomain),level1Domain)||GetLockLasting(level1Domain.nExpireTime)==0)
            return false;
        LogPrintf("update domain level2 level1 found\n");    
        if(ownerIn!=level1Domain.owner)
            return false;
        LogPrintf("update domain level2 owner:%S\n",ownerIn.ToString());    
        //this is extremely important:skip the checking of ownership of level2 domains, so as to save huge work when there's a fallback
        domain.owner=level1Domain.owner;
    }    
    fHasRecord=GetDomainByName(domain.strDomain,existingDomain);
    if(fHasRecord)
    LogPrintf("update domain expiretime:%i timeleft:%i \n",existingDomain.nExpireTime,GetLockLasting(existingDomain.nExpireTime));
    if(fHasRecord&&(GetLockLasting(existingDomain.nExpireTime)>0))
    {
        LogPrintf("update domain exists \n");
        if(existingDomain.owner!=ownerIn&&!domain.IsLevel2())//for level2, don't check owner here,it's already checked above
            return false;
        
        if(fRegister)
        {
            if(nLockTimeIn==0||LockTimeToTime(nLockTimeIn)<LockTimeToTime(existingDomain.nExpireTime))//renew time earlier than current time
                return false;
            LogPrintf("update domain renew\n");
            if(lockedValue<(domain.nDomainGroup==DOMAIN_10000?(domain.IsLevel2()?100*COIN:10000*COIN):100*COIN))
                return false;
            existingDomain=domain;
            existingDomain.nExpireTime=nLockTimeIn;
        }
        else
        {
        
            existingDomain.SetContent(CContent(strDomainContent),ownerIn,fRegister,fForward);
            LogPrintf("update domain exists content set \n");
        }
            
    }
    else if(fRegister)//new registration
    {        
        LogPrintf("update domain register value%i,time:%i\n",lockedValue,nLockTimeIn); 
        if(lockedValue<(domain.nDomainGroup==DOMAIN_10000?(domain.IsLevel2()?100*COIN:10000*COIN):100*COIN))
            return false;
        if(GetLockLasting(nLockTimeIn)==0)
        {
            LogPrintf("update domain register locktime too short\n"); 
            return false;
        }
        LogPrintf("update domain register value passed\n"); 
        existingDomain=domain;
        existingDomain.owner=ownerIn;
        //if(existingDomain.nExpireTime<nLockTimeIn)//there's possiblilty that renew time is closer to previous lock time
            existingDomain.nExpireTime=nLockTimeIn;
        LogPrintf("update domain register done\n"); 
        //
        
    }
    else
        return false;
    if (fForward)
    {
        LogPrintf("update domain forward\n"); 
        if(existingDomain.redirectType==CC_LINK_TYPE_SCRIPTPUBKEY)
        {
            string id;
            
            existingDomain.redirectID.assign(existingDomain.redirectTo.begin(),existingDomain.redirectTo.end());
            ScriptPubKeyToString(existingDomain.redirectID,id);
            string strMessage=existingDomain.strDomain+"->"+id;
            LogPrintf("update domain forward type:scriptpubkey msg4sig:%s,sig:%s\n",strMessage,HexStr(existingDomain.forwardsig.begin(),existingDomain.forwardsig.end())); 
            CHashWriter ss(SER_GETHASH, 0);
            ss << strMessage;
            CPubKey pubkey;
            
            if (!pubkey.RecoverCompact(ss.GetHash(), existingDomain.forwardsig)||CBitcoinAddress(pubkey).ToString()!=id)
            {
                LogPrintf("update domain forward verify sig failed,recovered id:%s,original id:%s\n",CBitcoinAddress(pubkey).ToString(),id); 
                return false;
            }
        }
        existingDomain.vDirectHistory.push_back(link);
        while(existingDomain.vDirectHistory.size()>8)
            existingDomain.vDirectHistory.erase(existingDomain.vDirectHistory.begin());
    }
    LogPrintf("update domain to write\n"); 
    if(fHasRecord)
    {
       //char* sql=existingDomain.GetUpdateSql();
       //return db.Write(sql);
        return db.Update(existingDomain);
    }
    return db.Insert(existingDomain);
    
}
//TODO reverse: only reverses REG, transfer and enlong.
                    //if this action is reg or transfer, reversely find the last reg/tranfer action. if none, this domain is diabled.
                    //this must be done to avoid fallback attack to get domainnames for free.
                    //list all txs related to the owner, find txs has this domain,
bool CDomainViewDB::Reverse(const string& strDomainContent)
{
//    LogPrintf("Reverse %s \n", HexStr(strDomainContent));
//    CDomain domain;
//    if(!domain.SetContent(CContent(strDomainContent)))
//        return false;
//    LogPrintf("Reverse domain name %s \n", domain.strDomain);
//    if(domain.strDomain=="")
//            return false;
//    //NO owner change, no need to reverse
//    if(domain.owner==CScript())
//        return true;
//    std::vector<CDiskTxPos> vTxPos;
//    pScript2TxPosDB->GetTxPosList(domain.owner,vTxPos);
//    CDomain existingDomain;
//    if(GetDomainByName(domain.nDomainGroup,domain.strDomain,existingDomain))
//    {
//        LogPrintf("Reverse domain exists \n");
//        existingDomain.SetContent(strDomainContent);
//        LogPrintf("Reverse domain exists content set \n");
//           return Write(domain);
//    }
//    else    
        return false;
 
}
bool CDomainViewDB::Write(const CDomain &domain)
{
    //char* sql=domain.GetInsertSql();
    //    return db.Write(sql);
    return db.Insert(domain);
}
bool CDomainViewDB::_GetDomainByForward(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const 
{
    if(scriptPubKey.size()==0)
        return false;
    char* searchColumn="redirrectto";
    const char* searchValue;//NOte: for varchar, need to add'' arround value
    const char* tableName=(nExtension==DOMAIN_10000?"domainf":"domainfai");
//    char** result;
//    int nRow;
//    int nColumn;
    //string str;
    
    string str2="x'";
    str2.append(HexStr(scriptPubKey.begin(),scriptPubKey.end())).append("'");
    searchValue=str2.c_str();
    
    
    
    return db.GetDomain(tableName,searchColumn, searchValue,vDomain);
    
    
    
//    if(db.Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
//    {
//        for(int i=0;i<nRow;i++)
//        {
//            vDomain.push_back(CDomain(result,i*nColumn));
//        }
//        sqlite3_free_table(result);
//        return true;
//    }
//    else
//        return false;
}
bool CDomainViewDB::_GetDomainByOwner(const int nExtension,const CScript scriptPubKey,std::vector<CDomain> &vDomain)const 
{
    if(scriptPubKey.size()==0)
        return false;
    char* searchColumn="owner";
    const char* searchValue;//NOte: for varchar, need to add'' arround value
    const char* tableName=(nExtension==DOMAIN_10000?"domainf":"domainfai");
//    char** result;
//    int nRow;
//    int nColumn;
    //string str;
    
    string str2="x'";
    str2.append(HexStr(scriptPubKey.begin(),scriptPubKey.end())).append("'");
    searchValue=str2.c_str();
    
    
    return db.GetDomain(tableName,searchColumn, searchValue,vDomain);
    
    
//    
//    if(db.Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
//    {
//        for(int i=0;i<nRow;i++)
//        {
//            vDomain.push_back(CDomain(result,i*nColumn));
//        }
//        sqlite3_free_table(result);
//        return true;
//    }
//    else
//        return false;
}
bool CDomainViewDB::GetDomainByName(const string strDomainName,CDomain& domain)const 
{
    //char* searchColumn="domainname";
    string searchColumn="domainname";
    const char* searchValue;//NOte: for varchar, need to add'' arround value
    
    //const char* tableName=(GetDomainGroup(strDomainName)==DOMAIN_10000?"domainf":"domainfai");
    string tableName=(GetDomainGroup(strDomainName)==DOMAIN_10000?"domainf":"domainfai");
//    char** result;
//    int nRow;
//    int nColumn;    
    string str2="'";
    str2.append(strDomainName).append("'");
    searchValue=str2.c_str();
    std::vector<CDomain> vDomain;
    
    
    if(db.GetDomain(tableName.c_str(),searchColumn.c_str(), searchValue,vDomain)&&vDomain.size()>0)
    {
        domain=vDomain[0];
        return true;
    }
    return false;
    
//    
//    
//    if(db.Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
//    {
//        if(nRow==0){
//            return false;
//            sqlite3_free_table(result);
//        }            
//        domain=CDomain(result,0); 
//        
//        return true;
//    }
//    else
//        return false;
}
bool CDomainViewDB::GetDomainByForward(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI)const 
{
    std::vector<CDomain> vDomain1;
    int nExtension=DOMAIN_10000;
    bool ret=_GetDomainByForward(nExtension,scriptPubKey, vDomain1);
    if(FSupportFAI)
    {
        nExtension=DOMAIN_100;
        ret&=_GetDomainByForward(nExtension,scriptPubKey, vDomain1);
    }
    for(unsigned int i=0;i<vDomain1.size();i++)
    {
        if(LockTimeToTime(vDomain1[i].nExpireTime)>=GetAdjustedTime())
            vDomain.push_back(vDomain1[i]);
    }
    return ret;
}
bool CDomainViewDB::GetDomainByForward(const CScript scriptPubKey,CDomain& domain,bool FSupportFAI)const
{
    std::vector<CDomain> vDomain;
    if(!GetDomainByForward(scriptPubKey,vDomain,FSupportFAI))
        return false;
    if(vDomain.size()==0)
        return false;
    domain=vDomain[0];
    for(unsigned int i=1;i<vDomain.size();i++)
    {        
        if(domain.GetLastRedirectLink()<vDomain[i].GetLastRedirectLink())
            domain=vDomain[i];
    }
    return true;
}
bool CDomainViewDB::GetDomainByOwner(const CScript scriptPubKey,std::vector<CDomain> &vDomain,bool FSupportFAI)const 
{
    int nExtension=DOMAIN_10000;
    bool ret=_GetDomainByOwner(nExtension,scriptPubKey, vDomain);
    if(FSupportFAI)
    {
        nExtension=DOMAIN_100;
        ret&=_GetDomainByOwner(nExtension,scriptPubKey, vDomain);
    }
    return ret;
}
bool CDomainViewDB::WriteBlockDomains(const uint256 blockHash,const map<CScript,string>& mapBlockDomains)
{
    CDataStream sBlockDomains(SER_DISK, CLIENT_VERSION);
    sBlockDomains<<mapBlockDomains;
    return db.Insert(blockHash,sBlockDomains);
}
bool  CDomainViewDB::GetBlockDomains(const uint256 blockHash,CDataStream& sBlockDomains)
{    
    return db.GetBlockDomains(blockHash, sBlockDomains);
}


CTagViewDB::CTagViewDB( bool fWipe): db(GetDataDir() / "sqlitedb", fWipe){}
bool CTagViewDB::HasLink(const CLink link)const{
    vector<CLink> vLink;
    vector<string> vTag;
    return (db.GetLinks(vTag,0,link,vLink)&&vLink.size()>0);
}
bool CTagViewDB::Search(vector<CLink>& vLink,const std::vector<string> &vTag,const int cc,const int nMaxItems,const int nOffset)const
{
    return db.GetLinks(vTag,cc,CLink(),vLink,nMaxItems, nOffset);

}           
bool CTagViewDB::Insert(const int cc,const string tag,const CLink link,const int nExpireTime)
{
    if(tag.size()>32)
        return false;
    int tagID;
    db.InsertTagID(tag,tagID);
        LogPrintf("txdb insert tag %s \n", tag);
    return db.InsertTag(cc,tagID,link, nExpireTime);
}
bool CTagViewDB::ClearExpired()
{
    return db.ClearExpiredTags(GetAdjustedTime());
}
CScriptCoinDB::CScriptCoinDB( bool fWipe): db(GetDataDir() / "sqlitedb", fWipe){}
bool CScriptCoinDB::Insert(const CCheque cheque)
{
    int scriptPubKeyIndex;
    db.InsertScriptIndex(cheque.scriptPubKey,scriptPubKeyIndex);
    LogPrintf("txdb insert scriptPubKeyID %i \n", scriptPubKeyIndex);
    int txIndex;
    db.InsertTxIndex(cheque.txid,txIndex);    
    return db.InsertCheque(scriptPubKeyIndex,txIndex,cheque.nOut, cheque.nValue,cheque.nLockTime);
}
bool CScriptCoinDB::Search(const vector<CScript>& vScriptPubKey,vector<CCheque> & vCheques)const 
{
    return db.GetCheques(vScriptPubKey, vCheques);

} 
bool CScriptCoinDB::Erase(const uint256 txid, const uint32_t nOut)
{
    int txIndex;
    db.InsertTxIndex(txid,txIndex); 
    return db.EraseCheque(txIndex,nOut);
}
