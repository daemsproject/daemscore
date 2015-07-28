
#include "main.h"
#include "txdb.h"
#include "ccc/link.h"
#include "ccc/content.h"
#include "ccc/domain.h"
#include "timedata.h"
//#include "utilstrencodings.h"
#include "ccc/contentutil.h"
#include <string>
#include <boost/foreach.hpp>
#include "util.h"

bool GetContentByLink(const CLink clink,CContent& content)
{
    CBlockIndex* pblockindex;
    CBlock block;
    if (!GetBlockByHeight(clink.nHeight, block, pblockindex))
        return false;
    CTransaction tx;
    if (!GetTxFromBlock(block, clink.nTx, tx))
        return false;    
    if (!GetContentFromVout(tx, clink.nVout, content))
        return false;
    return true;
}
bool GetBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex)
{
    if (nHeight < 0 || nHeight > chainActive.Height())
    {
        LogPrintf("GetBlockByHeight,Block height out of range");
        return false;
    }
    pblockindex = chainActive[nHeight];
    
    if (!ReadBlockFromDisk(blockOut, pblockindex))
    {
        LogPrintf("GetBlockByHeight,Can't read block from disk");
        return false;
    }
    return true;
}
bool GetTxFromBlock(const CBlock& block, const int nTx, CTransaction& txOut)
{
    if (nTx <= (int) block.vtx.size()) {
        txOut = block.vtx[nTx];
        return true;
    } else
        return false;
}
bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout)
{
    if (nVout <= (int) tx.vout.size()) {
        vout = tx.vout[nVout];
        return true;
    } else
        return false;
}
bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content)
{
    if (nVout <= (int) tx.vout.size()) {
        CTxOut vout = tx.vout[nVout];
        content.SetString(vout.strContent);
        return true;
    } else
        return false;
}
bool GetDomainLink (const string strDomain,CLink& link)
{    
    CDomain domain;
    if(pDomainDBView->GetDomainByName(strDomain,domain))
    {
        if(domain.redirectType==CC_LINK_TYPE_BLOCKCHAIN)
        {
            if(link.SetString(domain.redirectTo))
            {
                LogPrintf("GetDomainLink %s %s\n",HexStr(domain.redirectTo.begin(),domain.redirectTo.end()),link.ToString(LINK_FORMAT_DEC));
                return true;
            }
        }
    }
    return false;
}
bool GetFileFromLinks(const vector<CLink>& vlinks,string& strFile)
{
    if(vlinks.size()==0)
        return false;
    if(vlinks.size()==1)
    {
        CContent content;
        if(!GetContentByLink(vlinks[0],content))
            return false;
        return content.DecodeFileString(strFile);   
    }    
    for(unsigned int i=0;i<vlinks.size();i++)
    {
        CContent content;
        if(!GetContentByLink(vlinks[i],content))
            return false;
        std::vector<std::pair<int, string> > vDecoded;
        if(!content.Decode(vDecoded))
            return false;
        if(vDecoded[0].first==CC_FILE_PART)            
            strFile+=vDecoded[0].second;
        else if(vDecoded[0].first==CC_FILE_PART_P)
        {
            std::vector<std::pair<int, string> > vDecoded1;
            if(!CContent(vDecoded[0].second).Decode(vDecoded1))
                return false;
            bool fFound=false;
            for(unsigned int i=0;i<vDecoded1.size();i++)
                if(vDecoded1[i].first==CC_FILE_PART)  
                {
                    fFound=true;
                    strFile+=vDecoded[0].second;
                    break;
                }
            if (!fFound)
                return false;            
        }
        else
            return false;
    }
    return true;
}
int GetNTx(const uint256 &hashTx) 
{
    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hashTx, tx, hashBlock, true))
    {
        error("No information available about transaction");
        return -1;
    }
    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBlock];
    if (pblockindex==NULL||!ReadBlockFromDisk(block, pblockindex))
    {
        error("Can't read block from disk");
        return -1;
    }
    return GetNTx(tx, block);
}

int GetNTx(const CTransaction &tx, const CBlock &block)
{
    int nTx = 0;

    BOOST_FOREACH(const CTransaction&bTx, block.vtx)
    {
        if (tx == bTx)
            return nTx;
        nTx++;
    }
    return -1;
}
bool GetPubKeyFromBlockChain(CScript script,CPubKey& pubKey)
{
    std::vector<CScript> vScript;
    vScript.push_back(script);
    std::vector<CDiskTxPos> vTxPos;
     GetDiskTxPoses (vScript,vTxPos);
     if(vTxPos.size()==0)
         return false;
     for(unsigned int i=0;i<vTxPos.size();i++)
     {
          CTransaction tx;
        uint256 hashBlock;                 
        if(GetTransaction(vTxPos[i], tx, hashBlock)){            
            CTransaction prevTx;
            uint256 tmphash;
            if (!GetTransaction(tx.vin[0].prevout.hash, prevTx, tmphash, true))
                continue;            
            if(prevTx.vout[tx.vin[0].prevout.n].scriptPubKey==script)
              return false;//RecoverPubKey(tx,0,pubKey);                
        }
     }
     return false;
}
CScript GetTxInScriptPubKey(const CTxIn& txin)
{
    CTransaction prevTx;
    uint256 tmphash;
    if (!GetTransaction(txin.prevout.hash, prevTx, tmphash, true)) {
        LogPrintf("GetTxInScriptPubKey: null vin prevout\n");
        return CScript();
    }    
    return  prevTx.vout[txin.prevout.n].scriptPubKey;    
}
int GetBlocksToMaturity(const unsigned int nLockTime)
{
    if (nLockTime!=0){        
        if ((int64_t)nLockTime < LOCKTIME_THRESHOLD )
            return max(0, (int)((int)nLockTime+1 - (int)chainActive.Height()));  
        else{
            int lockBlocks;
            lockBlocks=(int)(((int64_t)nLockTime-GetAdjustedTime())/Params().TargetSpacing());
            return max(0, lockBlocks);
        }
    }
        return 0;
}
//this function is relative time to chainactive.tip
int GetLockLasting(uint32_t nLockTime)
{
    int64_t blocks = 0;
    if (nLockTime != 0) {
        if (nLockTime < LOCKTIME_THRESHOLD) { 
            blocks = max(0, (int) ((int) nLockTime + 1 - (int) chainActive.Height()));
            return blocks * Params().TargetSpacing();
        } else {
            return (int) max((int) 0, (int)(nLockTime - chainActive.Tip()->nTime));                
        }
    }
    return 0;
}