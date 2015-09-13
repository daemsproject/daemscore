// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "rpcserver.h"
#include "pow.h"
#include "uint256.h"
#include "timedata.h"
#include "fai/domain.h"
#include "fai/contentutil.h"
#include <stdint.h>
#include <boost/algorithm/string.hpp>  
#include <boost/thread.hpp>
#include <bits/stl_vector.h>

using namespace std;

void static BatchWriteCoins(CLevelDBBatch &batch, const uint256 &hash, const CCoins &coins)
{
    if (coins.IsPruned())
        batch.Erase(make_pair('c', hash));
    else
        batch.Write(make_pair('c', hash), coins);
}

void static BatchWriteHashBestChain(CLevelDBBatch &batch, const uint256 &hash)
{
    batch.Write('B', hash);
}

CCoinsViewDB::CCoinsViewDB(size_t nCacheSize, bool fMemory, bool fWipe) : db(GetDataDir() / "chainstate", nCacheSize, fMemory, fWipe) { }

bool CCoinsViewDB::GetCoins(const uint256 &txid, CCoins &coins) const
{
    return db.Read(make_pair('c', txid), coins);
}

bool CCoinsViewDB::HaveCoins(const uint256 &txid) const
{
    return db.Exists(make_pair('c', txid));
}

uint256 CCoinsViewDB::GetBestBlock() const
{
    uint256 hashBestChain;
    if (!db.Read('B', hashBestChain))
        return uint256(0);
    return hashBestChain;
}

bool CCoinsViewDB::BatchWrite(CCoinsMap &mapCoins, const uint256 &hashBlock)
{
    CLevelDBBatch batch;
    size_t count = 0;
    size_t changed = 0;
    for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();)
    {
        if (it->second.flags & CCoinsCacheEntry::DIRTY)
        {
            BatchWriteCoins(batch, it->first, it->second.coins);
            changed++;
        }
        count++;
        CCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }
    if (hashBlock != uint256(0))
        BatchWriteHashBestChain(batch, hashBlock);

    LogPrint("coindb", "Committing %u changed transactions (out of %u) to coin database...\n", (unsigned int) changed, (unsigned int) count);
    return db.WriteBatch(batch);
}

CBlockTreeDB::CBlockTreeDB(size_t nCacheSize, bool fMemory, bool fWipe) : CLevelDBWrapper(GetDataDir() / "blocks" / "index", nCacheSize, fMemory, fWipe) { }

bool CBlockTreeDB::WriteBlockIndex(const CDiskBlockIndex& blockindex)
{
    return Write(make_pair('b', blockindex.GetBlockHash()), blockindex);
}

bool CBlockTreeDB::WriteBlockFileInfo(int nFile, const CBlockFileInfo &info)
{
    return Write(make_pair('f', nFile), info);
}

bool CBlockTreeDB::ReadBlockFileInfo(int nFile, CBlockFileInfo &info)
{
    return Read(make_pair('f', nFile), info);
}

bool CBlockTreeDB::WriteLastBlockFile(int nFile)
{
    return Write('l', nFile);
}

bool CBlockTreeDB::WriteReindexing(bool fReindexing)
{
    if (fReindexing)
        return Write('R', '1');
    else
        return Erase('R');
}

bool CBlockTreeDB::ReadReindexing(bool &fReindexing)
{
    fReindexing = Exists('R');
    return true;
}

bool CBlockTreeDB::ReadLastBlockFile(int &nFile)
{
    return Read('l', nFile);
}

bool CCoinsViewDB::GetStats(CCoinsStats &stats) const
{
    /* It seems that there are no "const iterators" for LevelDB.  Since we
       only need read operations on it, use a const-cast to get around
       that restriction.  */
    boost::scoped_ptr<leveldb::Iterator> pcursor(const_cast<CLevelDBWrapper*> (&db)->NewIterator());
    pcursor->SeekToFirst();

    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    stats.hashBlock = GetBestBlock();
    ss << stats.hashBlock;
    CAmount nTotalAmount = 0;
    while (pcursor->Valid())
    {
        boost::this_thread::interruption_point();
        try
        {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'c')
            {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
                CCoins coins;
                ssValue >> coins;
                uint256 txhash;
                ssKey >> txhash;
                ss << txhash;
                ss << VARINT(coins.nVersion);
                ss << (coins.fCoinBase ? 'c' : 'n');
                ss << VARINT(coins.nHeight);
                stats.nTransactions++;
                for (unsigned int i = 0; i < coins.vout.size(); i++)
                {
                    const CTxOut &out = coins.vout[i];
                    if (!out.IsNull())
                    {
                        stats.nTransactionOutputs++;
                        ss << VARINT(i + 1);
                        ss << out;
                        nTotalAmount += out.nValue;
                    }
                }
                stats.nSerializedSize += 32 + slValue.size();
                ss << VARINT(0);
            }
            pcursor->Next();
        } catch (std::exception &e)
        {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }
    stats.nHeight = mapBlockIndex.find(GetBestBlock())->second->nHeight;
    stats.hashSerialized = ss.GetHash();
    stats.nTotalAmount = nTotalAmount;
    return true;
}

bool CBlockTreeDB::ReadTxIndex(const uint256 &txid, CDiskTxPos &pos)
{
    return Read(make_pair('t', txid), pos);
}

bool CBlockTreeDB::WriteTxIndex(const std::vector<std::pair<uint256, CDiskTxPos> >&vect)
{
    CLevelDBBatch batch;
    for (std::vector<std::pair<uint256, CDiskTxPos> >::const_iterator it = vect.begin(); it != vect.end(); it++)
        batch.Write(make_pair('t', it->first), it->second);
    return WriteBatch(batch);
}

bool CBlockTreeDB::WriteFlag(const std::string &name, bool fValue)
{
    return Write(std::make_pair('F', name), fValue ? '1' : '0');
}

bool CBlockTreeDB::ReadFlag(const std::string &name, bool &fValue)
{
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
    while (pcursor->Valid())
    {
        boost::this_thread::interruption_point();
        try
        {
            leveldb::Slice slKey = pcursor->key();
            CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
            char chType;
            ssKey >> chType;
            if (chType == 'b')
            {
                leveldb::Slice slValue = pcursor->value();
                CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
                CDiskBlockIndex diskindex;
                ssValue >> diskindex;

                // Construct block index object
                CBlockIndex* pindexNew = InsertBlockIndex(diskindex.GetBlockHash());
                pindexNew->pprev = InsertBlockIndex(diskindex.hashPrev);
                pindexNew->nHeight = diskindex.nHeight;
                pindexNew->nFile = diskindex.nFile;
                pindexNew->nDataPos = diskindex.nDataPos;
                pindexNew->nUndoPos = diskindex.nUndoPos;
                pindexNew->nVersion = diskindex.nVersion;
                pindexNew->hashMerkleRoot = diskindex.hashMerkleRoot;
                pindexNew->nBlockHeight = diskindex.nBlockHeight;
                pindexNew->nTime = diskindex.nTime;
                pindexNew->nBits = diskindex.nBits;
                pindexNew->nNonce = diskindex.nNonce;
                pindexNew->nStatus = diskindex.nStatus;
                pindexNew->nTx = diskindex.nTx;

                // Faicoin: Disable PoW Sanity check while loading block index from disk.
                // We use the sha256 hash for the block index for performance reasons, which is recorded for later use.
                // CheckProofOfWork() uses the scrypt hash which is discarded after a block is accepted.
                // While it is technically feasible to verify the PoW, doing so takes several minutes as it
                // requires recomputing every PoW hash during every Faicoin startup.
                // We opt instead to simply trust the data that is on your local disk.
                //if (!CheckProofOfWork(pindexNew->GetBlockPoWHash(), pindexNew->nBits))
                //    return error("LoadBlockIndex() : CheckProofOfWork failed: %s", pindexNew->ToString());

                pcursor->Next();
            } else
            {
                break; // if shutdown requested or finished loading block index
            }
        } catch (std::exception &e)
        {
            return error("%s : Deserialize or I/O error - %s", __func__, e.what());
        }
    }

    return true;
}

CBlockPosDB::CBlockPosDB(CSqliteWrapper* dbIn, bool fWipe) : db(dbIn)
{
    if (fWipe)
        db->ClearTable("table_blockpos");
}

bool CBlockPosDB::Write(const int nFile, const int nPos, const uint256 hashBlock, const int nHeight)
{
    int64_t nPosDB = (((int64_t) nFile) << 32) | nPos;

    return db->WriteBlockPos(nPosDB, hashBlock, nHeight);
    //return db->Insert("table_blockpos","blockheight",SQLITEDATATYPE_INT,nHeight,"blockhash",SQLITEDATATYPE_BLOB,vch,"pos",SQLITEDATATYPE_INT64,nPosDB,true);

}

bool CBlockPosDB::GetByPos(const int nFile, const int nPos, uint256& hashBlock, int& nHeight)
{
    int64_t nPosDB = (((int64_t) nFile) << 32) | nPos;
    //LogPrintf("CBlockPosDB::GetByPos :%lld \n",nPosDB);
    return db->GetBlockPosItem(nPosDB, hashBlock, nHeight);
}

CScript2TxPosDB::CScript2TxPosDB(CSqliteWrapper* dbIn, bool fWipe) : db(dbIn)
{
    if (fWipe)
        db->ClearTable("table_script2txpos");
}

bool CScript2TxPosDB::GetTxPosList(const CScript scriptPubKey, std::vector<CTxPosItem> &vTxPos)
{
    string strVtxPos;
    if(scriptPubKey.size()>3000)
        return false;
    string strScriptPubKey = "x'" + HexStr(scriptPubKey.begin(), scriptPubKey.end()) + "'";

    if (!db->SearchStr("table_script2txpos", "script", strScriptPubKey.c_str(), "vtxpos", SQLITEDATATYPE_BLOB, strVtxPos))
    {
        //LogPrintf("CScript2TxPosDB::GetTxPosList script %s,vtxpos not found \n",scriptPubKey.ToString());
        //std::vector<CTxPosItem> vTxPos1;
        //Write(scriptPubKey,vTxPos1);
        return false;
    }
    //LogPrintf("CScript2TxPosDB::GetTxPosList script %s,vtxpos %s \n",strScriptPubKey,strVtxPos.size());
    try
    {
        std::vector<CTxPosItem> vTxPos1;
        CDataStream ssValue(strVtxPos.data(), strVtxPos.data() + strVtxPos.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> vTxPos1;
        vTxPos.insert(vTxPos.end(), vTxPos1.begin(), vTxPos1.end());
    } catch (const std::exception&)
    {
        LogPrintf("CScript2TxPosDB::GetTxPosList unserialize error %s\n", HexStr(strVtxPos.begin(), strVtxPos.end()));
        return false;
    }
    return true;
}

bool CScript2TxPosDB::BatchInsert(const std::map<CScript, std::vector<CTxPosItem> > &mapScriptTxPosList)
{
    vector<pair<string, string> > vListNew;
    for (std::map<CScript, std::vector<CTxPosItem> >::const_iterator it = mapScriptTxPosList.begin(); it != mapScriptTxPosList.end(); it++)
    {
        string strScriptPubKey(it->first.begin(), it->first.end());
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(it->second));
        ssKey << it->second;
        string strVTxPos;
        strVTxPos.assign(ssKey.begin(), ssKey.end());
        vListNew.push_back(make_pair(strScriptPubKey, strVTxPos));
        //LogPrintf("CScript2TxPosDB::BatchInsert script %s,vtxpos %i \n", it->first.ToString(), strVTxPos.size());
    }
    return db->InsertBatch("table_script2txpos", "script", SQLITEDATATYPE_BLOB, "vtxpos", SQLITEDATATYPE_BLOB, vListNew, true);
}

bool CScript2TxPosDB::BatchUpdate(const std::map<CScript, std::vector<CTxPosItem> > &mapScriptTxPosList)
{
    vector<pair<string, string> > vList;
    for (std::map<CScript, std::vector<CTxPosItem> >::const_iterator it = mapScriptTxPosList.begin(); it != mapScriptTxPosList.end(); it++)
    {
        string strScriptPubKey(it->first.begin(), it->first.end());
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(it->second));
        ssKey << it->second;
        string strVTxPos;
        strVTxPos.assign(ssKey.begin(), ssKey.end());
        vList.push_back(make_pair(strScriptPubKey, strVTxPos));
        //LogPrintf("CScript2TxPosDB::BatchUpdate script %s,vtxpos %i \n",it->first.ToString(),strVTxPos.size());
    }
    return db->BatchUpdate("table_script2txpos", "script", SQLITEDATATYPE_BLOB, "vtxpos", SQLITEDATATYPE_BLOB, vList);
}

bool CScript2TxPosDB::Write(const CScript &scriptPubKey, const std::vector<CTxPosItem> &vTxPos)
{
    string strScriptPubKey(scriptPubKey.begin(), scriptPubKey.end());
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(ssKey.GetSerializeSize(vTxPos));
    ssKey << vTxPos;
    string strVTxPos;
    strVTxPos.assign(ssKey.begin(), ssKey.end());
    //string strVTxPos="x'"+HexStr(ssKey.begin(),ssKey.end())+"'";    
    return db->Insert("table_script2txpos", "script", SQLITEDATATYPE_BLOB, strScriptPubKey, "vtxpos", SQLITEDATATYPE_BLOB, strVTxPos, false);
}

bool CScript2TxPosDB::AddNewTxs(const std::map<CScript, vector<CTxPosItem> >&mapScriptTxPos)
{
 //LogPrintf("AddNewTxs\n");
    std::map<CScript, std::vector<CTxPosItem> > mapScriptTxPosListUpdate;
    // LogPrintf("AddNewTxs1\n");
    std::map<CScript, std::vector<CTxPosItem> > mapScriptTxPosListInsert;
   //  LogPrintf("AddNewTxs2\n");
    std::vector<CTxPosItem> vTxPos;
// LogPrintf("AddNewTxs4\n");
    for (std::map<CScript, vector<CTxPosItem> >::const_iterator it = mapScriptTxPos.begin(); it != mapScriptTxPos.end(); it++)
    {
        //Script scriptPubKey=it->first;     
        vTxPos.clear();
        if (!GetTxPosList(it->first, vTxPos))
        {
          //   LogPrintf("AddNewTxs5\n");
            mapScriptTxPosListInsert.insert(*it);
            continue;
        }
        bool fChanged = false;
        for (unsigned int ii = 0; ii < it->second.size(); ii++)
        {
            bool found = false;
            for (unsigned int i = 0; i < vTxPos.size(); i++)
            {
                if (vTxPos[i] == it->second[ii])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                vTxPos.push_back(it->second[ii]);
                fChanged = true;
            }
        }
        // LogPrintf("AddNewTxs6\n");
        if (fChanged)
            mapScriptTxPosListUpdate.insert(make_pair(it->first, vTxPos));
    }
    //LogPrintf("AddNewTxs7\n");
    return BatchUpdate(mapScriptTxPosListUpdate) & BatchInsert(mapScriptTxPosListInsert);
}

bool CScript2TxPosDB::RemoveTxs(const std::map<CScript, vector<CTxPosItem> >&mapScriptTxPos)
{
    std::map<CScript, std::vector<CTxPosItem> > mapScriptTxPosList;
    std::vector<CTxPosItem> vTxPos;
    for (std::map<CScript, vector<CTxPosItem> >::const_iterator it = mapScriptTxPos.begin(); it != mapScriptTxPos.end(); it++)
    {
        vTxPos.clear();
        GetTxPosList(it->first, vTxPos);
        bool fChanged = false;
        for (unsigned int ii = 0; ii < it->second.size(); ii++)
        {
            std::vector<CTxPosItem>::iterator it2 = find(vTxPos.begin(), vTxPos.end(), it->second[ii]);
            if (it2 != vTxPos.end())
            {
                vTxPos.erase(it2);
                fChanged = true;
            }
        }
        if (fChanged)
            mapScriptTxPosList.insert(make_pair(it->first, vTxPos));
    }
    return BatchUpdate(mapScriptTxPosList);
}

CDomainViewDB::CDomainViewDB(CSqliteWrapper* dbIn, bool fWipe) : db(dbIn)
{
    if (fWipe)
        ClearTables();
}

bool CDomainViewDB::GetUpdateDomain(const CScript ownerIn, const string& strDomainContent, const uint64_t lockedValue, const uint32_t nLockTimeIn, const CLink link, CDomain& domainOut, bool&fHasRecord)
{
    //LogPrintf("txdb CDomainViewDB Update %s %s\n",ownerIn.ToString(),link.ToString());
    CDomain domain;
    bool fRegister = false;
    bool fForward = false;
    //bool fHasRecord=false;
    if (!domain.SetContent(CContent(strDomainContent), ownerIn, fRegister, fForward))
        return false;
   // LogPrintf("update domain name %s ,owner:%s,fRegister:%b,fForward:%b\n", domain.strDomain, domain.owner.ToString(), fRegister, fForward);
    if (domain.strDomain == "")
        return false;
    CDomain existingDomain;
    if (domain.IsLevel2())
    {
        //LogPrintf("update domain level2 \n");    
        CDomain level1Domain;
        if (!GetDomainByName(GetLevel1Domain(domain.strDomain), level1Domain) || GetLockLasting(level1Domain.nExpireTime) == 0)
            return false;
        // LogPrintf("update domain level2 level1 found\n");    
        if (ownerIn != level1Domain.owner)
            return false;
        //LogPrintf("update domain level2 owner:%S\n",ownerIn.ToString());    
        //this is extremely important:skip the checking of ownership of level2 domains, so as to save huge work when there's a fallback
        domain.owner = level1Domain.owner;
    }
    fHasRecord = GetDomainByName(domain.strDomain, existingDomain);
    if (fHasRecord && (GetLockLasting(existingDomain.nExpireTime) > 0))
    {
        // LogPrintf("update domain exists \n");
        if (existingDomain.owner != ownerIn&&!domain.IsLevel2())//for level2, don't check owner here,it's already checked above
            return false;

        if (fRegister)
        {
            if (nLockTimeIn == 0 || LockTimeToTime(nLockTimeIn) < LockTimeToTime(existingDomain.nExpireTime))//renew time earlier than current time
                return false;
           // LogPrintf("update domain renew %s\n", domain.strDomain);
            if (lockedValue < (domain.nDomainGroup == DOMAIN_10000 ? (domain.IsLevel2() ? 100 * COIN : 10000 * COIN) : 100 * COIN))
                return false;
            //existingDomain = domain;
            existingDomain.nExpireTime = LockTimeToTime(nLockTimeIn);
            existingDomain.nLockValue = lockedValue;
        } else
        {

            existingDomain.SetContent(CContent(strDomainContent), ownerIn, fRegister, fForward);
            //   LogPrintf("update domain exists content set,fForward:%b \n",fForward);
        }

    } else if (fRegister)//new registration
    {
        //LogPrintf("update domain register value%i,time:%i\n",lockedValue,nLockTimeIn); 
        if (lockedValue < (domain.nDomainGroup == DOMAIN_10000 ? (domain.IsLevel2() ? 100 * COIN : 10000 * COIN) : 100 * COIN))
            return false;
        if (GetLockLasting(nLockTimeIn) == 0)
        {
            //     LogPrintf("update domain register locktime too short\n"); 
            return false;
        }
        //  LogPrintf("update domain register value passed\n"); 
        existingDomain = domain;
        existingDomain.owner = ownerIn;
        //if(existingDomain.nExpireTime<nLockTimeIn)//there's possiblilty that renew time is closer to previous lock time
        existingDomain.nExpireTime = LockTimeToTime(nLockTimeIn);
        existingDomain.nLockValue = lockedValue;
        //LogPrintf("update domain register done\n");
        //

    } else
        return false;
    if (fForward)
    {
        //  LogPrintf("update domain forward\n"); 
        if (existingDomain.redirectType == CC_LINK_TYPE_SCRIPTPUBKEY)
        {
            string id;
            existingDomain.redirectID.assign(existingDomain.redirectTo.begin(), existingDomain.redirectTo.end());
            ScriptPubKeyToString(existingDomain.redirectID, id);
            string strMessage = existingDomain.strDomain + "->" + id;
            //LogPrintf("update domain forward type:scriptpubkey msg4sig:%s,sig:%s\n", strMessage, HexStr(existingDomain.forwardsig.begin(), existingDomain.forwardsig.end()));
            CHashWriter ss(SER_GETHASH, 0);
            ss << strMessage;
            CPubKey pubkey;
            if (!pubkey.RecoverCompact(ss.GetHash(), existingDomain.forwardsig) || CBitcoinAddress(pubkey).ToString() != id)
            {
               // LogPrintf("update domain forward verify sig failed,recovered id:%s,original id:%s\n", CBitcoinAddress(pubkey).ToString(), id);
                return false;
            }

        }
        if (find(existingDomain.vDirectHistory.begin(), existingDomain.vDirectHistory.end(), link) == existingDomain.vDirectHistory.end())
        {
            existingDomain.vDirectHistory.push_back(link);
            while (existingDomain.vDirectHistory.size() > 8)
                existingDomain.vDirectHistory.erase(existingDomain.vDirectHistory.begin());
        }
    }
    domainOut = existingDomain;
    //LogPrintf("update domain name %s ,owner:%s\n", domainOut.strDomain,domainOut.owner.ToString());    
    //    LogPrintf("update domain to write\n"); 
    //    if(fHasRecord)
    //    {
    //       //char* sql=existingDomain.GetUpdateSql();
    //       //return db->Write(sql);
    //        return db->Update(existingDomain);
    //    }
    //    return db->Insert(existingDomain);
    return true;
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

bool CDomainViewDB::Write(const CDomain &domain, const bool fExists)
{
    int64_t ownerID;
    if (!db->GetScriptIndex(domain.owner, ownerID))
    {
        LogPrintf("GetScriptIndex failed %s \n", domain.owner.ToString());
        return false;
    }
    if (fExists)
        return db->Update(domain, ownerID);
    return db->Insert(domain, ownerID);
}

bool CDomainViewDB::WriteTags(const CDomain &domain, const map<string, int64_t>& mapTags)
{
    int64_t domainID;
    const char* tableName = (domain.nDomainGroup == DOMAIN_10000 ? "domain10000" : "domain100");
    string str = "'" + domain.strDomain + "'";
    if (!db->SearchInt(tableName, "domainname", str.c_str(), "rowid", domainID))
    {
        LogPrintf("GetDomainID failed %s \n", domain.strDomain);
        return false;
    }
    vector<int64_t> vTagIDs;
    for (unsigned int i = 0; i < domain.vTags.size(); i++)
    {
        vTagIDs.push_back(mapTags.find(domain.vTags[i])->second);
    }
    char chDomainID[20];
    sprintf(chDomainID, "%lld", domainID);
    const char* tableName2 = (domain.nDomainGroup == DOMAIN_10000 ? "domaintag10000" : "domaintag100");
    db->Delete(tableName2, "domainid", chDomainID, "=");
    return db->InsertTags(tableName2, domainID, vTagIDs);
}

bool CDomainViewDB::_GetDomainByForward(const int nExtension, const CScript scriptPubKey, std::vector<CDomain> &vDomain, bool fGetTags)const
{
    if (scriptPubKey.size() == 0)
        return false;
    string searchColumn = "redirrectto";
    const char* searchValue; //NOte: for varchar, need to add'' arround value
    const string tableName = (nExtension == DOMAIN_10000 ? "domain10000" : "domain100");
    //    char** result;
    //    int nRow;
    //    int nColumn;
    //string str;

    string str2 = "x'";
    str2.append(HexStr(scriptPubKey.begin(), scriptPubKey.end())).append("'");
    searchValue = str2.c_str();
    //LogPrintf("_GetDomainByForward  \n");


    return db->GetDomain(tableName.c_str(), searchColumn.c_str(), "=", searchValue, vDomain, 30, fGetTags);



    //    if(db->Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
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

bool CDomainViewDB::_GetDomainByOwner(const int nExtension, const CScript scriptPubKey, std::vector<CDomain> &vDomain, bool fGetTags)const
{
    if (scriptPubKey.size() == 0)
        return false;
    string searchColumn = "owner";
    //const char* searchValue;//NOte: for varchar, need to add'' arround value
    const string tableName = (nExtension == DOMAIN_10000 ? "domain10000" : "domain100");
    //    char** result;
    //    int nRow;
    //    int nColumn;
    //string str;

    // string strowner="x'";
    // strowner.append(HexStr(scriptPubKey.begin(),scriptPubKey.end())).append("'");    
    int64_t ownerID;
    if (!db->GetScriptIndex(scriptPubKey, ownerID))
        return false;
    char chOwnerID[20];
    sprintf(chOwnerID, "%lld", ownerID);
    return db->GetDomain(tableName.c_str(), searchColumn.c_str(), "=", chOwnerID, vDomain, 100000, fGetTags);


    //    
    //    if(db->Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
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

bool CDomainViewDB::GetDomainByName(const string strDomainName, CDomain& domain, bool fGetTags)const
{
    //char* searchColumn="domainname";
    string searchColumn = "domainname";
    const char* searchValue; //NOte: for varchar, need to add'' arround value

    //const char* tableName=(GetDomainGroup(strDomainName)==DOMAIN_10000?"domainf":"domainfai");
    string strDomaintmp = strDomainName;
    boost::algorithm::to_lower(strDomaintmp);
    string tableName = (GetDomainGroup(strDomaintmp) == DOMAIN_10000 ? "domain10000" : "domain100");
    //    char** result;
    //    int nRow;
    //    int nColumn;    
    string str2 = "'";
    str2.append(strDomaintmp).append("'");
    searchValue = str2.c_str();
    std::vector<CDomain> vDomain;


    if (db->GetDomain(tableName.c_str(), searchColumn.c_str(), "=", searchValue, vDomain, 1, fGetTags) && vDomain.size() > 0)
    {
        domain = vDomain[0];
        return true;
    }
    //LogPrintf("CDomainViewDB::GetDomainByName:domain not found \n");
    return false;

    //    
    //    
    //    if(db->Get(tableName,searchColumn,searchValue,result,nRow,nColumn))
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

bool CDomainViewDB::GetDomainByForward(const CScript scriptPubKey, std::vector<CDomain> &vDomain, bool FSupport100, bool fGetTags)const
{
    std::vector<CDomain> vDomain1;
    int nExtension = DOMAIN_10000;
    bool ret = _GetDomainByForward(nExtension, scriptPubKey, vDomain1, fGetTags);
    if (FSupport100)
    {
        nExtension = DOMAIN_100;
        ret &= _GetDomainByForward(nExtension, scriptPubKey, vDomain1, fGetTags);
    }
    for (unsigned int i = 0; i < vDomain1.size(); i++)
    {
        if (LockTimeToTime(vDomain1[i].nExpireTime) >= GetAdjustedTime())
            vDomain.push_back(vDomain1[i]);
    }
    return ret;
}

bool CDomainViewDB::GetDomainByForward(const CScript scriptPubKey, CDomain& domain, bool FSupport100, bool fGetTags)const
{
    std::vector<CDomain> vDomain;
    if (!GetDomainByForward(scriptPubKey, vDomain, FSupport100, fGetTags))
        return false;
    if (vDomain.size() == 0)
        return false;
    domain = vDomain[0];
    for (unsigned int i = 1; i < vDomain.size(); i++)
    {
        if (domain.GetLastRedirectLink() < vDomain[i].GetLastRedirectLink())
            domain = vDomain[i];
    }
    return true;
}

bool CDomainViewDB::GetDomainByOwner(const CScript scriptPubKey, std::vector<CDomain> &vDomain, bool FSupport100, bool fGetTags)const
{
    int nExtension = DOMAIN_10000;
    bool ret = _GetDomainByOwner(nExtension, scriptPubKey, vDomain, fGetTags);
    if (FSupport100)
    {
        nExtension = DOMAIN_100;
        ret &= _GetDomainByOwner(nExtension, scriptPubKey, vDomain, fGetTags);
    }
    return ret;
}

bool CDomainViewDB::GetDomainByTags(const vector<string>& vTag, vector<CDomain>& vDomain, bool FSupport100, const int nMax, bool fGetTags)const
{
    //LogPrintf("CDomainViewDB::GetDomainByTags1\n");
    std::vector<CDomain> vDomain1;
    vector<int64_t>vTagIDs;
    string searchColumn = "rowid";
    //char searchValue[2000];//NOte: for varchar, need to add'' arround value
    string tableName = "domaintag10000";
    char chTag[1000];
    bool ret = false;    
    if (vTag.size() == 0)
    {
        char chTime[1000];
        sprintf(chTime, "%lld", GetAdjustedTime());
        tableName = "domain10000";
        return db->GetDomain(tableName.c_str(), "expiredate", ">", chTime, vDomain, nMax, fGetTags);
    }
    if (vTag.size() > 10)
        return false;
   //LogPrintf("CDomainViewDB::GetDomainByTags2\n");
    const char* tagselectstatement = "SELECT domainid FROM %s WHERE tagid =%lld ";
    int64_t tagid;
    if (!db->GetTagID(vTag[0], tagid))
        return false;
    vTagIDs.push_back(tagid);
    sprintf(chTag, tagselectstatement, tableName.c_str(), vTagIDs[0]);
   // LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
    const char* tagselectstatement2 = "SELECT domainid FROM %s WHERE domainid IN(%s) AND tagid=%lld ";
    string strTgstmt;
    for (unsigned int i = 1; i < vTag.size(); i++)
    {
        if (!db->GetTagID(vTag[i], tagid))
            return false;
        vTagIDs.push_back(tagid);
        strTgstmt.assign(chTag);
        sprintf(chTag, tagselectstatement2, tableName.c_str(), strTgstmt.c_str(), vTagIDs[i]);
    //    LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
    }
    strTgstmt.assign(chTag);
    sprintf(chTag, "(%s) ", strTgstmt.c_str());
   // LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
    tableName = "domain10000";
    ret = db->GetDomain(tableName.c_str(), searchColumn.c_str(), "IN", chTag, vDomain1, nMax, fGetTags);
    if (FSupport100)
    {
        char chTag1[1000];
        tableName = "domaintag100";
        sprintf(chTag1, tagselectstatement, tableName.c_str(), vTagIDs[0]);
     //   LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
        for (unsigned int i = 1; i < vTag.size(); i++)
        {
            strTgstmt.assign(chTag1);
            sprintf(chTag1, tagselectstatement2, tableName.c_str(), strTgstmt.c_str(), vTagIDs[i]);
      //      LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
        }
        strTgstmt.assign(chTag1);
        sprintf(chTag1, "(%s) ", strTgstmt.c_str());
     //   LogPrintf("CDomainViewDB::GetDomainByTags stmt:%s\n", chTag);
        tableName = "domain100";
        ret &= db->GetDomain(tableName.c_str(), searchColumn.c_str(), "IN", chTag1, vDomain1, nMax, fGetTags);
    }

    for (unsigned int i = 0; i < vDomain1.size(); i++)
    {
        if (LockTimeToTime(vDomain1[i].nExpireTime) >= GetAdjustedTime())
            vDomain.push_back(vDomain1[i]);
        if ((int)vDomain.size() >= nMax)
            return ret;
    }
    return ret;
}

bool CDomainViewDB::GetDomainsByAlias(const std::string strAlias, std::vector<CDomain> &vDomain, const bool FSupport100, const int nMax, bool fGetTags) const
{
    std::vector<CDomain> vDomain1;

    string searchColumn = "alias";
    char searchValue[1000];
    sprintf(searchValue, "'%s' LIMIT %i", strAlias.c_str(), nMax); //NOte: for varchar, need to add'' arround value

    string tableName = "domain10000";
    bool ret = db->GetDomain(tableName.c_str(), searchColumn.c_str(), "=", searchValue, vDomain1, nMax, fGetTags);
    if (FSupport100)
    {
        tableName = "domain100";
        ret &= db->GetDomain(tableName.c_str(), searchColumn.c_str(), "=", searchValue, vDomain1, nMax, fGetTags);
    }
    for (unsigned int i = 0; i < vDomain1.size(); i++)
    {
        if (LockTimeToTime(vDomain1[i].nExpireTime) >= GetAdjustedTime())
            vDomain.push_back(vDomain1[i]);
    }
    return ret;
}

bool CDomainViewDB::GetDomainNamesToExpire(std::vector<string> &vDomainNames, const uint32_t nExpireIn, bool FSupport100, const int nMax)const
{
    string searchColumn = "expiredate";
    char chTime[1000];
    sprintf(chTime, "%lld", nExpireIn + GetAdjustedTime());
    string tableName = "domain10000";
    bool ret = db->SearchStrs(tableName.c_str(), "expiredate", chTime, "domainname", SQLITEDATATYPE_TEXT, vDomainNames, "<", nMax);

    if (FSupport100)
    {
        tableName = "domain100";
        ret &= db->SearchStrs(tableName.c_str(), "expiredate", chTime, "domainname", SQLITEDATATYPE_TEXT, vDomainNames, "<", nMax);

    }
    return ret;
}

bool CDomainViewDB::WriteBlockDomains(const uint256 blockHash, const map<CScript, string>& mapBlockDomains)
{
    CDataStream sBlockDomains(SER_DISK, CLIENT_VERSION);
    sBlockDomains << mapBlockDomains;
    return db->Insert(blockHash, sBlockDomains);
}

bool CDomainViewDB::GetBlockDomains(const uint256 blockHash, CDataStream& sBlockDomains)
{
    return db->GetBlockDomains(blockHash, sBlockDomains);
}

bool CDomainViewDB::ClearExpired(const uint32_t time)
{
    vector<int64_t>vDomainIDs10000;
    vector<int64_t>vDomainIDs100;
    //db->GetExpiredDomainIDs("domain10000",vDomainIDs10000,time);
    //db->GetExpiredDomainIDs("domain100",vDomainIDs100,time);
    char chTime[20];
    sprintf(chTime, "%u", time);
    db->SearchInts("domain10000", "expiredate", chTime, "rowid", vDomainIDs10000, "<");
    db->SearchInts("domain100", "expiredate", chTime, "rowid", vDomainIDs100, "<");
    db->Delete("domain10000", "expiredate", chTime, "<");
    db->Delete("domain100", "expiredate", chTime, "<");
    char chDomainID[20];
    for (unsigned int i = 0; i < vDomainIDs10000.size(); i++)
    {
        sprintf(chDomainID, "%lld", vDomainIDs10000[i]);
        db->Delete("domaintag10000", "domainid", chDomainID, "=");
    }
    for (unsigned int i = 0; i < vDomainIDs100.size(); i++)
    {
        sprintf(chDomainID, "%lld", vDomainIDs100[i]);
        db->Delete("domaintag100", "domainid", chDomainID, "=");
    }
    return true;
}

bool CDomainViewDB::ClearTables()
{
    db->ClearTable("domain10000");
    db->ClearTable("domain100");
    db->ClearTable("blockdomaintable");
    db->ClearTable("domaintag10000");
    db->ClearTable("domaintag100");
    return true;
}

CTagViewDB::CTagViewDB(CSqliteWrapper* dbIn, bool fWipe) : db(dbIn)
{
    if (fWipe)
        ClearTables();
}
//bool CTagViewDB::HasLink(const CLink link)const{
//    vector<CLink> vLink;
//    vector<string> vTag;
//    return (db->GetLinks(vTag,0,link,vLink)&&vLink.size()>0);
//}
//bool CTagViewDB::Search(vector<CLink>& vLink,const std::vector<string> &vTag,const int cc,const int nMaxItems,const int nOffset)const
//{
//    return db->GetLinks(vTag,cc,CLink(),vLink,nMaxItems, nOffset);
//
//}           

bool CTagViewDB::Insert(const CContentDBItem& item)
{
    int64_t scriptIndex;
    db->GetScriptIndex(item.sender, scriptIndex);
    int64_t nLink = item.link.SerializeInt();
    db->InsertContent(nLink, item.pos, scriptIndex, item.cc, item.lockValue, item.lockTime);
    for (unsigned int i = 0; i > item.vTags.size(); i++)
    {
        if (item.vTags[i].size() > 32)
            continue;
        int64_t tagID;
        db->InsertTagID(item.vTags[i], tagID);
        LogPrintf("txdb insert tag %s \n", item.vTags[i]);
        db->InsertTag("tag", (int64_t) tagID, nLink);
    }
    return true;
}

bool CTagViewDB::InsertTags(const vector<string>& vTags)
{
    return db->InsertBatch("tagid", "tag", SQLITEDATATYPE_TEXT, vTags, false);
}

bool CTagViewDB::InsertContentTags(const vector<CContentDBItem>& vContents, const map<string, int64_t>& mapTags)
{
    bool result = true;
    const char* tableName = "tag";
    for (unsigned int ii = 0; ii < vContents.size(); ii++)
    {
        vector<int64_t> vTagIDs;
        for (unsigned int i = 0; i < vContents[ii].vTags.size(); i++)
        {
            vTagIDs.push_back(mapTags.find(vContents[ii].vTags[i])->second);
        }
        result &= db->InsertTags(tableName, vContents[ii].link.SerializeInt(), vTagIDs);
    }
    return result;
}
bool CTagViewDB::DeleteContentTags(const vector<CContentDBItem>& vContents)
{
    bool result = true;
    const char* tableName = "tag";
    for (unsigned int ii = 0; ii < vContents.size(); ii++)
    {
        char chInt[100];
        sprintf(chInt,"%lld",vContents[ii].link.SerializeInt());
        result &= db->Delete(tableName,"link",chInt ,"=" );
    }
    return result;
}
bool CTagViewDB::ClearExpired(uint32_t nTime)
{
    vector<int64_t>vLink;
    char chTime[20];
    sprintf(chTime, "%u", nTime);
    db->SearchInts("table_content", "locktime", chTime, "link", vLink, "<");
    db->Delete("table_content", "locktime", chTime, "<");

    char chLink[20];
    for (unsigned int i = 0; i < vLink.size(); i++)
    {
        sprintf(chLink, "%lld", vLink[i]);
        db->Delete("tag", "link", chLink, "=");
    }

    return true;
    //return db->,ClearExpiredTags(nTime);
}

bool CTagViewDB::ClearTables()
{
    return db->ClearTable("tag") && db->ClearTable("table_content");

}

CScriptCoinDB::CScriptCoinDB(CSqliteWrapper* dbIn, bool fWipe) : db(dbIn)
{
    if (fWipe)
        ClearTables();
}

bool CScriptCoinDB::Insert(const CCheque cheque)
{
    int64_t scriptPubKeyIndex;
    if (!db->GetScriptIndex(cheque.scriptPubKey, scriptPubKeyIndex))
    {
        LogPrintf("CScriptCoinDB insertscriptindex failed \n");
    }
    //    int txIndex=0;
    //    if(!db->InsertTxIndex(cheque.txid,txIndex))
    //    {
    //        LogPrintf("CScriptCoinDB inserttxindex failed \n");
    //    }
    //LogPrintf("txdb  CScriptCoinDB insert scriptPubKeyID %i,txIndex %i \n", scriptPubKeyIndex,txIndex);
    return db->InsertCheque(scriptPubKeyIndex, (cheque.txIndex << 16) + cheque.nOut, cheque.nValue, cheque.nLockTime);
}

bool CScriptCoinDB::Search(const vector<CScript>& vScriptPubKey, vector<CCheque> & vCheques, int nMaxResults, int nOffset)const
{
    if (vScriptPubKey.size() <= 100)
        return db->GetCheques(vScriptPubKey, vCheques, nMaxResults, nOffset);
    int ii = 0;
    bool rs = true;
    while (ii < (int) vScriptPubKey.size())
    {
        int nBatch = min((int) vScriptPubKey.size() - ii, 100);
        vector<CScript> vScriptPubKey1;
        vScriptPubKey1.assign(vScriptPubKey.begin() + ii, vScriptPubKey.begin() + ii + nBatch);
        rs &= db->GetCheques(vScriptPubKey1, vCheques, nMaxResults, nOffset);
        ii += nBatch;
    }
    return rs;
}

bool CScriptCoinDB::Erase(const uint256 txid, const uint32_t nOut)
{
    int64_t txIndex = 0;
    db->InsertTxIndex(txid, txIndex);
    int64_t nLink = (txIndex << 16) + nOut;
    //return db->EraseCheque((txIndex<<16)+nOut);
    char chLink[20];
    sprintf(chLink, "%lld", nLink);
    return db->Delete("table_unspent", "link", chLink, "=");
}

bool CScriptCoinDB::ClearTables()
{
    db->ClearTable("txindextable");
    db->ClearTable("table_unspent");
    return true;
}

bool UpdateSqliteDB(const CBlock& block, const vector<pair<uint256, CDiskTxPos> >& vPos, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, bool fErase)
{
    // LogPrintf("UpdateSqliteDB blockheight:%i,txs:%i \n",block.nBlockHeight,block.vtx.size());
    map<CScript, vector<CTxPosItem> > mapScript2TxPos;
    map<CScript, int64_t> mapScriptIndex;
    //vector<CScript> vScriptNew;
    vector<string> vTags;
    map<string, int64_t> mapTags;
    vector<string> vTagNew;
    vector<CContentDBItem>vContents;
    map<CScript, string> mapBlockDomains;
    map<uint256, int64_t>mapTxIndex;
    vector < pair<CDomain, bool> > vDomains;
    vector<CCheque> vChequeAdd;
    vector<int64_t> vChequeErase;
    //step2:get script2txpos list,content list,taglist,domain tags
    //LogPrintf("UpdateSqliteDB1 \n");
    if (!fErase)
    {
        //LogPrintf("UpdateSqliteDB2 \n");
        GetBlockDomainUpdateList(block, vPrevouts, vDomains, fErase);
        GetBlockDomainTags(vDomains, vTags);
        //LogPrintf("UpdateSqliteDB3 \n");

        //LogPrintf("UpdateSqliteDB4 \n");
        GetBlockContentAndTagList(block, vPos, vPrevouts, vTags, vContents);
        //LogPrintf("UpdateSqliteDB5 \n");
        FindBlockTagIDAndNewTags(vTags, mapTags, vTagNew);
        //LogPrintf("UpdateSqliteDB6 \n");
        //LogPrintf("UpdateSqliteDB7 \n");
        PrePareBlockTxIndex(block, mapTxIndex);
    }
    else{
        GetBlockContentAndTagList(block, vPos, vPrevouts, vTags, vContents);
    }
    //LogPrintf("UpdateSqliteDB8 \n");
    GetBlockScript2TxPosList(block, vPos, vPrevouts, mapScript2TxPos, fErase);
    //LogPrintf("UpdateSqliteDB9 \n");
    //step3:flush script2txpos list,taglist,txindex
    psqliteDB->BeginBatch();
    //LogPrintf("UpdateSqliteDB10 \n");
    if (fErase)
    {
        //LogPrintf("UpdateSqliteDB11 \n");
        pScript2TxPosDB->RemoveTxs(mapScript2TxPos);
        //LogPrintf("UpdateSqliteDB12 \n");
    } else
    {
       // LogPrintf("UpdateSqliteDB13 \n");
        pScript2TxPosDB->AddNewTxs(mapScript2TxPos);
        //LogPrintf("UpdateSqliteDB14 \n");
        pTagDBView->InsertTags(vTagNew);
        //LogPrintf("UpdateSqliteDB15 \n");
        psqliteDB->InsertTxIndice(mapTxIndex);
        //  LogPrintf("UpdateSqliteDB16 \n");
    }
    // LogPrintf("UpdateSqliteDB17 \n");
    psqliteDB->EndBatch();
    // LogPrintf("UpdateSqliteDB18 \n");
    //step4 get script index,tagindex
    if (!fErase)
    {
        //  LogPrintf("UpdateSqliteDB19 \n");
        vector<string> vTagTmp;
        //  LogPrintf("UpdateSqliteDB20 \n");
        FindBlockTagIDAndNewTags(vTagNew, mapTags, vTagTmp);
        //  LogPrintf("UpdateSqliteDB21 \n");
    }
    // LogPrintf("UpdateSqliteDB22 \n");
    GetBlockScriptIndice(mapScript2TxPos, mapScriptIndex);
    // LogPrintf("UpdateSqliteDB23 \n");

    //step5 get domains4block,cheques
    if (!fErase)
    {
        // LogPrintf("UpdateSqliteDB24 \n");
        GetBlockSenderDomains(block, vPrevouts, mapBlockDomains);
        // LogPrintf("UpdateSqliteDB GetBlockSenderDomains %i \n",mapBlockDomains.size());
    }
    ///LogPrintf("UpdateSqliteDB26 \n");
    GetBlockChequeUpdates(block, vPrevouts, vChequeAdd, vChequeErase, fErase);
    //LogPrintf("UpdateSqliteDB27 \n");
    //step6 flush domains4block,domains,cheques,contents,content tags, domain tags
    // clearexpired domains per 120blocks(aprox.6 hours)
    psqliteDB->BeginBatch();
    //LogPrintf("UpdateSqliteDB28 \n");
    if (!fErase)
    {
        // LogPrintf("UpdateSqliteDB29 \n");
        pBlockPosDB->Write(vPos[0].second.nFile, vPos[0].second.nPos, block.GetHash(), block.nBlockHeight);
        //  LogPrintf("UpdateSqliteDB30 \n");
        pDomainDBView->WriteBlockDomains(block.GetHash(), mapBlockDomains);
        //  LogPrintf("UpdateSqliteDB31 \n");
        for (unsigned int i = 0; i < vDomains.size(); i++)
        {
            //  LogPrintf("UpdateSqliteDB32 \n");
            pDomainDBView->Write(vDomains[i].first, vDomains[i].second);
            //  LogPrintf("UpdateSqliteDB33 \n");
            pDomainDBView->WriteTags(vDomains[i].first, mapTags);
            //  LogPrintf("UpdateSqliteDB34 \n");
        }
        // LogPrintf("UpdateSqliteDB35 \n");
        psqliteDB->InsertContents(vContents, mapScriptIndex);
        // LogPrintf("UpdateSqliteDB36 \n");
        pTagDBView->InsertContentTags(vContents, mapTags);
        // LogPrintf("UpdateSqliteDB37 \n");
    }else
    {
        pTagDBView->DeleteContentTags(vContents);
        psqliteDB->DeleteContents(vContents);
    }
    // LogPrintf("UpdateSqliteDB38 \n");
    psqliteDB->BatchInsertCheque(vChequeAdd, mapScriptIndex);
    // LogPrintf("UpdateSqliteDB39 \n");
    int ii = 0;
    while (ii < (int) vChequeErase.size())
    {
        // LogPrintf("eraseCheques %i \n",ii);
        char strList[2000];
        string tmp;
        sprintf(strList, "(%lld", vChequeErase[ii]);
        tmp.assign(strList);
        int nBatch = min((int) vChequeErase.size() - ii, 100);
        for (int i = 1; i < nBatch; i++)
        {
            sprintf(strList, "%s,%lld", tmp.c_str(), vChequeErase[ii + i]);
            tmp.assign(strList);
        }
        sprintf(strList, "%s)", tmp.c_str());
        // LogPrintf("eraseCheque sql:%s \n",&strList[0]);
        psqliteDB->Delete("table_unspent", "link", strList, "IN");
        ii += nBatch;
    }
    //LogPrintf("UpdateSqliteDB40 \n");
    if (block.nBlockHeight % 120 == 0)
    {
        pDomainDBView->ClearExpired(GetAdjustedTime());
        pTagDBView->ClearExpired(GetAdjustedTime());
    }
    // LogPrintf("UpdateSqliteDB41 \n");
    psqliteDB->EndBatch();
    //LogPrintf("UpdateSqliteDB42 \n");
    //sleep(60);
    return true;
}

void GetBlockScript2TxPosList(const CBlock& block, const vector<pair<uint256, CDiskTxPos> >& vPos, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, map<CScript, vector<CTxPosItem> >& mapScript2TxPos, bool fErase)
{
    //LogPrintf("GetBlockScript2TxPosList1 blockheight:%i\n",block.nBlockHeight);
    int nHeaderLen = 88;
    for (unsigned int i = 0; i < block.vtx.size(); i++)
    {
        // LogPrintf("GetBlockScript2TxPosList2 ntx:%i\n",i);
        const CTransaction &tx = block.vtx[i];
        CTxPosItem posItem;
        posItem.nFile = vPos[i].second.nFile;
        posItem.nPos = vPos[i].second.nPos + nHeaderLen + vPos[i].second.nTxOffset;
        posItem.nTx = i;
        //LogPrintf("GetBlockScript2TxPosList3 vprevouts size %i,vin size %i\n",vPrevouts[i].size(),tx.vin.size());
        map<CScript, CTxPosItem> mapSender;

        if (!tx.IsCoinBase())
        {
            //LogPrintf("GetBlockScript2TxPosList4 \n");
            posItem.nFlags = TXITEMFLAG_SENDER;
            for (unsigned int j = 0; j < tx.vin.size(); j++)
                mapSender[vPrevouts[i][j].first] = posItem;

            //            if(mapSender.size()>1)
            //            {
            //                posItem.nFlags|=1<<TXITEMFLAG_MULTIPLESENDER;
            //                for(map<CScript,CTxPosItem>::iterator it=mapSender.begin();it!=mapSender.end();it++)
            //                    it->second.nFlags |=1<<TXITEMFLAG_MULTIPLESENDER;
            //            }
            //LogPrintf("GetBlockScript2TxPosList5 \n");
        }
        //        else    
        //            posItem.nFlags|=1<<TXITEMFLAG_COINBASE;
        //posItem.nFlags&=~(1<<TXITEMFLAG_SENDER);
        //posItem.nFlags |=1<<TXITEMFLAG_RECEIVER;
        map<CScript, CTxPosItem> mapReceiver;
        //bool fHasLockTime=false;
        //LogPrintf("GetBlockScript2TxPosList6 \n");

        BOOST_FOREACH(const CTxOut &txout, tx.vout)
        {
            posItem.nFlags = 0;
            if (txout.strContent.size() > 0)
            {
                if (!tx.IsCoinBase())
                    mapSender.begin()->second.nFlags |= TXITEMFLAG_SENDCONTENT;
                int cc = CContent(txout.strContent).GetFirstCc();
                switch (cc)
                {
                    case CC_MESSAGE_P:
                        posItem.nFlags |= TXITEMFLAG_MESSAGE;
                        if (!tx.IsCoinBase())
                            mapSender.begin()->second.nFlags |= TXITEMFLAG_MESSAGE;
                        break;
                    case CC_PRODUCT_P:
                        //posItem.nFlags|=(1<<TXITEMFLAG_PRODUCT);
                        if (!tx.IsCoinBase())
                            mapSender.begin()->second.nFlags |= TXITEMFLAG_PRODUCT;
                        break;
                    case CC_PAYMENT_P:
                        posItem.nFlags |= TXITEMFLAG_PURCHASE;
                        if (!tx.IsCoinBase())
                            mapSender.begin()->second.nFlags |= TXITEMFLAG_PURCHASE;
                        break;
                    case CC_DOMAIN_P:
                        //posItem.nFlags|=(1<<TXITEMFLAG_DOMAIN);
                        if (!tx.IsCoinBase())
                            mapSender.begin()->second.nFlags |= TXITEMFLAG_DOMAIN;
                        break;
                    default:
                        break;
                }
            }
            //if address is empty don't record it
            if (txout.scriptPubKey.size() == 0)
                continue;
            if (mapSender.find(txout.scriptPubKey) != mapSender.end())
            {
                //mapSender[txout.scriptPubKey].nFlags |=1<<TXITEMFLAG_RECEIVER;
                if (txout.strContent.size() > 0)
                    mapSender[txout.scriptPubKey].nFlags |= TXITEMFLAG_RECEIVECONTENT;
                if (txout.nValue > 0)
                    //{
                    mapSender[txout.scriptPubKey].nFlags |= TXITEMFLAG_RECEIVEMONEY;
                //                    if(txout.nLockTime>0)
                //                        mapSender[txout.scriptPubKey].nFlags |=1<<TXITEMFLAG_HASLOCKTIME;
                //}
            } else
            {
                if (mapReceiver.find(txout.scriptPubKey) == mapReceiver.end())
                    mapReceiver[txout.scriptPubKey] = posItem;
                if (txout.strContent.size() > 0)
                    mapReceiver[txout.scriptPubKey].nFlags |= TXITEMFLAG_RECEIVECONTENT;
                if (txout.nValue > 0)
                    //{
                    mapReceiver[txout.scriptPubKey].nFlags |= TXITEMFLAG_RECEIVEMONEY;
                //                    if(txout.nLockTime>0)
                //                        mapReceiver[txout.scriptPubKey].nFlags |=1<<TXITEMFLAG_HASLOCKTIME;
                //}
            }
        }

        //LogPrintf("GetBlockScript2TxPosList7 \n");
        MergeScript2TxPosList(mapScript2TxPos, mapSender);
        MergeScript2TxPosList(mapScript2TxPos, mapReceiver);
    }
}

void MergeScript2TxPosList(map<CScript, vector<CTxPosItem> >& parent, const map<CScript, CTxPosItem>& child)
{
    for (map<CScript, CTxPosItem>::const_iterator it = child.begin(); it != child.end(); it++)
    {
        map<CScript, vector<CTxPosItem> >::iterator itparent = parent.find(it->first);
        if (itparent != parent.end())
        {
            bool found = false;
            for (unsigned int i = 0; i < itparent->second.size(); i++)
            {
                if (itparent->second[i] == it->second)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                itparent->second.push_back(it->second);
        } else
        {
            vector<CTxPosItem> v;
            v.push_back(it->second);
            parent[it->first] = v;
        }
    }
}

void GetBlockContentAndTagList(const CBlock& block, const vector<pair<uint256, CDiskTxPos> >& vPos, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, vector<string>& vTags, vector<CContentDBItem>& vContents)
{
    int nHeaderLen = 88;
    for (unsigned int ii = 0; ii < block.vtx.size(); ii++)
    {
        const CTransaction &tx = block.vtx[ii];
        //LogPrintf("main:updatetagdb ntags %i \n",nTags);
        for (unsigned int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];
            if (out.nValue < 1000000 || out.strContent.size() == 0)
                continue;
            uint32_t nExpireTime = LockTimeToTime(tx.vout[i].nLockTime);
            //LogPrintf("main:updatetagdb expiretime %i,now %i \n",nExpireTime,GetAdjustedTime());
            if (nExpireTime == 0 || nExpireTime < chainActive.Tip()->nTime + 3600 * 24)
                continue;
            CContent str = out.strContent;
            //LogPrintf("main:updatetagdb str isstandard: %b \n",str.IsStandard());
            if (!str.IsStandard())
                continue;
            if (str.GetFirstCc() == CC_DOMAIN_P)
                continue;
            std::vector<std::pair<int, std::string> >vTagList;
            str.GetTags(vTagList);
            CLink link(block.nBlockHeight, ii, i);
            //            if(pTagDBView->HasLink(link))
            //            continue;
            //int txSize=tx.GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
            int64_t nOutPos = (((int64_t) (vPos[ii].second.nFile)) << 32) + vPos[ii].second.nPos + nHeaderLen + vPos[ii].second.nTxOffset + tx.GetOutPos(i);
            //LogPrintf("GetBlockContentAndTagList: nfile:%i,npos:%i,voutoffset%i,outpos:%lld \n", vPos[ii].second.nFile, vPos[ii].second.nPos, nHeaderLen + vPos[ii].second.nTxOffset + tx.GetOutPos(i), nOutPos);
            CScript sender = vPrevouts[ii][i].first;
            vector<string>vTagsTx;
            for (int64_t i = 0; i < min((int64_t) 10, (int64_t) min((int64_t) (out.nValue / 1000000), (int64_t) vTagList.size())); i++)
            {
                vTagsTx.push_back(vTagList[i].second);
                if (find(vTags.begin(), vTags.end(), vTagList[i].second) == vTags.end())
                    vTags.push_back(vTagList[i].second);
            }
            CContentDBItem item(link, nOutPos, sender, str.GetFirstCc(), out.nValue, nExpireTime, vTagsTx);
            vContents.push_back(item);
        }
    }
}

void FindBlockTagIDAndNewTags(const vector<string>& vTags, map<string, int64_t>& mapTags, vector<string>& vTagNew)
{
    int64_t tagID;
    for (unsigned int i = 0; i < vTags.size(); i++)
    {
        if (psqliteDB->GetTagID(vTags[i], tagID))
        {
            //LogPrintf("FindBlockTagIDAndNewTags:tag %s,id%i \n",vTags[i],tagID);
            mapTags[vTags[i]] = tagID;
        } else
            if (find(vTagNew.begin(), vTagNew.end(), vTags[i]) == vTagNew.end())
            vTagNew.push_back(vTags[i]);
    }
}

void PrePareBlockTxIndex(const CBlock& block, map<uint256, int64_t>& mapTxIndex)
{
    // LogPrintf("PrePareBlockTxIndex %i:txs:%i \n",block.nBlockHeight,block.vtx.size());
    for (unsigned int i = 0; i < block.vtx.size(); i++)
    {
        const CTransaction &tx = block.vtx[i];
        uint256 txid = tx.GetHash();

        int64_t txIndex = block.nBlockHeight;
        ///if(chainActive.Tip()->GetBlockHash()==block.hashPrevBlock)
        //     txIndex=chainActive.Tip()->nBlockHeight+1;        
        txIndex <<= 16;
        txIndex |= i;
        mapTxIndex[txid] = txIndex;
        //   LogPrintf("PrePareBlockTxIndex %i:txid:%s,ixIndex:%i\n",block.nBlockHeight,txid.GetHex(),txIndex);
    }
}

void GetBlockDomainUpdateList(const CBlock& block, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, vector<pair<CDomain, bool> >& vDomains, bool fReverse)
{
    //LogPrintf("GetBlockDomainUpdateList vtx:%i\n",block.vtx.size()); 
    for (unsigned int ii = 0; ii < block.vtx.size(); ii++)
    {
        const CTransaction &tx = block.vtx[ii];
        //LogPrintf("GetBlockDomainUpdateList ntx:%i\n",ii); 
        if (tx.IsCoinBase())//coinbase is not allowed to register domain
            continue;
        for (unsigned int i = 0; i < tx.vout.size(); i++)
        {
            if (tx.vout[i].strContent.size() == 0)
                continue;
            int cc = CContent(tx.vout[i].strContent).GetFirstCc();
            //LogPrintf("GetBlockDomainUpdateList cc:%s \n",GetCcName((cctype)cc)); 
            if (cc == CC_DOMAIN_P)
            {
                int nMaxCC = STANDARD_CONTENT_MAX_CC;
                //LogPrintf("GetBlockDomainUpdateList  content:%s \n", CContent(tx.vout[i].strContent).ToHumanString(nMaxCC));
                std::vector<std::pair<int, std::string> >vContent;
                if (CContent(tx.vout[i].strContent).Decode(vContent))
                {
                    //LogPrintf("GetBlockDomainUpdateList decoded \n"); 
                    if (fReverse)
                    {
                        //bool ret=pDomainDBView->Reverse(coins->vout[prevout.n].scriptPubKey,vContent[0].second);
                    } else
                    {
                        CLink link(block.nBlockHeight, ii, i);
                        CDomain domain;
                        bool fExists;
                        if (pDomainDBView->GetUpdateDomain(vPrevouts[ii][0].first, vContent[0].second, (uint64_t) tx.vout[i].nValue, IsFrozen(tx, i, block.nBlockHeight, block.nTime) ? tx.vout[i].nLockTime : 0, link, domain, fExists))
                        {
                            //LogPrintf("domain owner:%s",domain.owner.ToString());
                            vDomains.push_back(make_pair(domain, fExists));
                        } else
                            LogPrintf("domain update failed \n");
                    }
                }
            }
        }
    }
}

void GetBlockScriptIndice(const map<CScript, vector<CTxPosItem> >& mapScript2TxPos, map<CScript, int64_t>& mapScriptIndex)
{
    for (map<CScript, vector<CTxPosItem> >::const_iterator it = mapScript2TxPos.begin(); it != mapScript2TxPos.end(); it++)
    {
        int64_t scriptIndex;
        if (psqliteDB->GetScriptIndex(it->first, scriptIndex))
            mapScriptIndex[it->first] = scriptIndex;
    }
}

void GetBlockDomainTags(const vector<pair<CDomain, bool> >& vDomains, vector<string>& vTags)
{
    for (unsigned int i = 0; i < vDomains.size(); i++)
        for (unsigned int j = 0; j < vDomains[i].first.vTags.size(); j++)
            if (find(vTags.begin(), vTags.end(), vDomains[i].first.vTags[j]) == vTags.end())
                vTags.push_back(vDomains[i].first.vTags[j]);
}

void GetBlockSenderDomains(const CBlock& block, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, map<CScript, string>& mapBlockDomains)
{
    for (unsigned int ii = 0; ii < block.vtx.size(); ii++)
    {
        const CTransaction &tx = block.vtx[ii];
        if (tx.IsCoinBase())//coinbase is not allowed to register domain
            continue;
        for (unsigned int i = 0; i < tx.vin.size(); i++)
        {
            CScript scriptPubKey = vPrevouts[ii][i].first;
            if (mapBlockDomains.find(scriptPubKey) == mapBlockDomains.end())
            {
                CDomain domain;
                if (pDomainDBView->GetDomainByForward(scriptPubKey, domain, false))
                    mapBlockDomains[scriptPubKey] = domain.strDomain;
            }
        }
    }
}

void GetBlockChequeUpdates(const CBlock& block, const vector<vector<pair<CScript, uint32_t> > >& vPrevouts, vector<CCheque>& vChequeAdd, vector<int64_t>& vChequeErase, bool fReverse)
{
    if (fReverse)
    {
        for (int ii = (int) block.vtx.size() - 1; ii >= 0; ii--)
        {
            const CTransaction &tx = block.vtx[ii];
            if (!tx.IsCoinBase())
            {
                for (unsigned int i = 0; i < tx.vin.size(); i++)
                {
                    const COutPoint &prevout = tx.vin[i].prevout;
                    {
                        CCheque cheque;
                        cheque.nLockTime = vPrevouts[ii][i].second;
                        cheque.nOut = prevout.n;
                        cheque.nValue = tx.vin[i].prevout.nValue;
                        cheque.scriptPubKey = vPrevouts[ii][i].first;
                        cheque.txid = prevout.hash;
                        if (psqliteDB->GetTxIndex(cheque.txid, cheque.txIndex))
                            vChequeAdd.push_back(cheque);
                        else
                            LogPrintf("GetBlockChequeUpdates reverse txindex not found, block %i,ntx %i,childtxid %s,nvin %i", block.nBlockHeight, ii, tx.GetHash().GetHex(), i);
                    }
                }
            }
            for (unsigned int i = 0; i < tx.vout.size(); i++)
            {
                //if address is empty don't record it
                if (tx.vout[i].scriptPubKey.size() == 0 || tx.vout[i].nValue == 0)
                    continue;
                {
                    uint64_t link = ((int64_t) block.nBlockHeight << 32)+(ii << 16) + i;
                    vChequeErase.push_back(link);
                }
            }
        }
        return;
    }
    for (unsigned int ii = 0; ii < block.vtx.size(); ii++)
    {
        const CTransaction &tx = block.vtx[ii];
        uint256 txid = tx.GetHash();
        //LogPrintf("UpdateScriptCoinDB txid:%s\n",txid.GetHex());
        if (!tx.IsCoinBase())
        {
            for (unsigned int i = 0; i < tx.vin.size(); i++)
            {
                const COutPoint &prevout = tx.vin[i].prevout;
                {
                    uint64_t link;
                    int64_t txIndex;
                    if (psqliteDB->GetTxIndex(prevout.hash, txIndex))
                    {
                        link = (txIndex << 16) + prevout.n;
                        //LogPrintf("GetBlockChequeUpdates link to erase %lld %lld \n",txIndex,link);
                        vChequeErase.push_back(link);
                    } else
                        LogPrintf("GetBlockChequeUpdates txindex not found, block %i,ntx %i,childtxid %s,nvin %i", block.nBlockHeight, ii, tx.GetHash().GetHex(), i);
                }
            }
        }
        for (unsigned int i = 0; i < tx.vout.size(); i++)
        {
            //if address is empty don't record it
            if (tx.vout[i].scriptPubKey.size() == 0 || tx.vout[i].nValue == 0)
                continue;
            {
                CCheque cheque;
                cheque.nLockTime = tx.vout[i].nLockTime;
                cheque.nOut = i;
                cheque.nValue = tx.vout[i].nValue;
                cheque.scriptPubKey = tx.vout[i].scriptPubKey;
                cheque.txid = txid;
                cheque.txIndex = ((int64_t) block.nBlockHeight << 16) + ii;
                vChequeAdd.push_back(cheque);
            }
        }
    }

}

bool SearchPromotedContents(const vector<CScript>& vSenders, const vector<int>& vCCs, const vector<string>& vTags, vector<CContentDBItem>& vContents, const int nMaxResults, const int nOffset)
{
    vector<int64_t> vSenderIDs;
    if (vSenders.size() > 100)
        return false;
    for (unsigned int i = 0; i < vSenders.size(); i++)
    {
        int64_t id;
        if (psqliteDB->GetScriptIndex(vSenders[i], id))
            vSenderIDs.push_back(id);
    }
    if (vCCs.size() > 10)
        return false;
    vector<int64_t> vTagIDs;
    //LogPrintf("SearchPromotedContents vtags:%i \n", vTags.size());
    if (vTags.size() > 9)
        return false;
    for (unsigned int i = 0; i < vTags.size(); i++)
    {
        int64_t tagid;
        if (!psqliteDB->GetTagID(vTags[i], tagid))
            return false;
        vTagIDs.push_back(tagid);
    }
    // LogPrintf("SearchPromotedContents vtagids:%i \n",vTagIDs.size());
    return psqliteDB->SearchContents(vSenderIDs, vCCs, vTagIDs, vContents, nMaxResults, nOffset);
}