
#include "main.h"
#include "txdb.h"
#include "fai/link.h"
#include "fai/content.h"
#include "fai/domain.h"
#include "fai/filepackage.h"
#include "fai/settings.h"
#include "timedata.h"
#include "base58.h"
//#include "utilstrencodings.h"
#include "fai/contentutil.h"
#include <string>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include "util.h"

bool GetContentByLink(const CLink clink,CContent& content)
{
    //LogPrintf("GetContentByLink1 \n");
    CBlockIndex* pblockindex;
    CBlock block;
    if (!GetBlockByHeight(clink.nHeight, block, pblockindex))
        return false;
    //LogPrintf("GetContentByLink2 \n");
    CTransaction tx;
    if (!GetTxFromBlock(block, clink.nTx, tx))
        return false;    
    //LogPrintf("GetContentByLink3 \n");
    if (!GetContentFromVout(tx, clink.nVout, content))
        return false;
    //LogPrintf("GetContentByLink 4\n");
    return true;
}
bool GetContentByLink(const CLinkUni clink,CContent& content)
{
    //LogPrintf("GetContentByLink1 \n");
    CBlockIndex* pblockindex;
    CBlock block;
    if (!GetBlockByHeight(clink.nHeight, block, pblockindex))
        return false;
    //LogPrintf("GetContentByLink2 \n");
    CTransaction tx;
    if (!GetTxFromBlock(block, clink.nTx, tx))
        return false;    
    //LogPrintf("GetContentByLink3 \n");
    if (!GetContentFromVout(tx, clink.nVout, content))
        return false;
    //LogPrintf("GetContentByLink 4\n");
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
    if (nTx<0||nTx < (int) block.vtx.size()) {
        txOut = block.vtx[nTx];
        return true;
    } else
        return false;
}
bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout)
{
    if ((nVout<0)||(nVout < (int) tx.vout.size())) {
        vout = tx.vout[nVout];
        return true;
    } else
        return false;
}
bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content)
{
    if ((nVout<0)||(nVout < (int) tx.vout.size())) {
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
            if(link.UnserializeConst(domain.redirectTo))
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
    LogPrintf("GetFileFromLinks \n");
    if(vlinks.size()==0)
        return false;
    if(vlinks.size()==1)
    {
        LogPrintf("GetFileFromLinks size 1\n");
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
    int64_t txIndex;
    psqliteDB->GetTxIndex(hashTx,txIndex);
    //LogPrintf("getntx,tx:%s,height:%i,ntx:%i \n",hashTx.GetHex(),txIndex>>16,txIndex&0xffff);
    return txIndex&0xffff;
//    CTransaction tx;
//    uint256 hashBlock = 0;
//    if (!GetTransaction(hashTx, tx, hashBlock, true))
//    {
//        error("No information available about transaction");
//        return -1;
//    }
//    CBlock block;
//    CBlockIndex* pblockindex = mapBlockIndex[hashBlock];
//    if (pblockindex==NULL||!ReadBlockFromDisk(block, pblockindex))
//    {
//        error("Can't read block from disk");
//        return -1;
//    }
//    return GetNTx(tx, block);
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
//    std::vector<CScript> vScript;
//    vScript.push_back(script);
//    std::vector<CDiskTxPos> vTxPos;
//     GetDiskTxPoses (vScript,vTxPos);
//     if(vTxPos.size()==0)
//         return false;
//     for(unsigned int i=0;i<vTxPos.size();i++)
//     {
//          CTransaction tx;
//        uint256 hashBlock;                 
//        if(GetTransaction(vTxPos[i], tx, hashBlock)){            
//            CTransaction prevTx;
//            uint256 tmphash;
//            if (!GetTransaction(tx.vin[0].prevout.hash, prevTx, tmphash, true))
//                continue;            
//            if(prevTx.vout[tx.vin[0].prevout.n].scriptPubKey==script)
//              return false;//RecoverPubKey(tx,0,pubKey);                
//        }
//     }
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
            lockBlocks=(int)(((int64_t)nLockTime-GetAdjustedTime()+1800)/Params().TargetSpacing());
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
            return (int) max((int) 0, (int)(nLockTime - chainActive.Tip()->nTime+1800));                
        }
    }
    return 0;
}
bool ParseUrl(const string urlIn,string& urlOut,int& nPageID)
{
    nPageID=255;
    urlOut=urlIn;
    CLinkUni link(urlIn);
     LogPrintf("ParseUrl  linktype %i,%s\n",link.linkType,GetCcName(link.linkType));
    switch ((int)link.linkType)
    {
        case CC_LINK_TYPE_NATIVE:
        {
            GetNativeLink(urlIn,urlOut,nPageID);
            LogPrintf("ParseUrl  urlIn:%s urlout:%s,pageid:%i\n",urlIn,urlOut,nPageID); 
            return true;
        }
        break;        
        case CC_LINK_TYPE_BLOCKCHAIN:        
        {
            CContent content;
            if(!GetContentByLink(link,content))
                return false;
            LogPrintf("ParseUrl linktype blockchain,link:%s",link.ToString());
            return _ParseContentUrl(link,content,urlOut,nPageID);
        }
        case CC_LINK_TYPE_DOMAIN:   
        {     
            //LogPrintf("ParseUrl  strDomain %s\n",link.strDomain);
            //LogPrintf("ParseUrl  strDomainExtension %s\n",link.strDomainExtension);
             return _ParseDomainUrl(link.strDomain,link.strDomainExtension,urlOut,nPageID,1);  
        }
        case CC_LINK_TYPE_TXIDOUT:       
            {       
                CContent content;
                if(!GetContentByTxidOut(link.txid,link.nVout, content))
                    return false;
                return _ParseContentUrl(link,content,urlOut,nPageID);
            }
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            {
                string id;
                if(!ScriptPubKeyToString(link.scriptPubKey,id))
                    return false;
                //urlIn="fai:"+id;
                GetNativeLink("fai:browser",urlOut,nPageID);                
                urlOut+="?id="+id;
                LogPrintf("ParseUrl  urlIn:%s urlout:%s,pageid:%i\n",urlIn,urlOut,nPageID);
                return true;
            }
        case CC_LINK_TYPE_HTTP:
            LogPrintf("ParseUrl  linktype http\n");
        case CC_LINK_TYPE_HTTPS:
        case CC_LINK_TYPE_FTP:
        case CC_LINK_TYPE_ED2K:
        case CC_LINK_TYPE_MAGNET:
        case CC_LINK_TYPE_FILE:
            LogPrintf("ParseUrl  linktype http,urlIn:%s urlout:%s,pageid:%i\n",urlIn,urlOut,nPageID);
            return true;
        default:
            urlOut="http://"+urlIn;
            LogPrintf("ParseUrl  urlIn:%s urlout:%s,pageid:%i\n",urlIn,urlOut,nPageID);        
    }  
    return true;
}
bool GetNativeLink(const string urlIn,string& urlOut,int& nPageID)
{
    std::size_t posColon = urlIn.find(URI_COLON);
    std::string str = urlIn.substr(posColon+1);
    LogPrintf("GetNativeLink  urlIN:%s\n",urlIn);
    string strExt;
    if(str.find("/")!=str.npos)
    {
        strExt=str.substr(str.find("/"));
        str=str.substr(0,str.find("/"));   
    }
    
    LogPrintf("GetNativeLink  ext:%s\n",str);
    for(int i=1;i<=HELPPAGE_ID;i++)
    {
        if (str==mapPageNames[i])
        {
            //temporary code
            boost::filesystem::path fullpath = boost::filesystem::initial_path().parent_path().parent_path().parent_path() / "cccpages" / "html" / (str+"_en.html");
            urlOut="file://"+fullpath.string()+(strExt.size()>0?strExt.substr(1):"");

            // final code
            nPageID=i;
            string strPath;
            if(GetFilePackageMain(mapPageNames[nPageID],strPath,true))
                urlOut=strPath+(strExt.size()>0?strExt.substr(1):"");
            LogPrintf("ParseUrl  urlIn:%s urlout:%s,pageid:%i\n",urlIn,urlOut,nPageID);
            return true;
        }
    }
    return false;
}
bool _ParseContentUrl(const CLinkUni link,const CContent content,string& urlOut,int& nPageID)
{
   switch ((int)content.GetFirstCc())
    {
        case CC_FILE_PACKAGE_P:
            if(!CFilePackage(link).InstallPackage(link.ToString(),true))
                return false;            
            return GetFilePackageMain(link.ToString(),urlOut,true);        
        default:
            GetNativeLink("fai:browser",urlOut,nPageID);    
            urlOut+=("?link="+link.ToString());
    }
   return true;
}
bool _ParseDomainUrl(const string& strDomain,const string& strDomainExt,string& urlOut,int& nPageID, int nIterations)
{
    if (nIterations>10)
        return false;
    //LogPrintf("_ParseDomainUrl  domain:%s \n",strDomain);
    CDomain domain;
    if(!pDomainDBView->GetDomainByName(strDomain,domain))
        return false;
    CLinkUni link;
    //LogPrintf("_ParseDomainUrl  domain:%s\n",domain.ToJsonString());
    switch((int)domain.redirectType)
    {
        case CC_LINK_TYPE_BLOCKCHAIN:        
        {
            //LogPrintf("_ParseDomainUrl  redirectTo %s\n",domain.redirectTo);                       
            if(!link.UnserializeConst(domain.redirectTo))
                return false;
            CContent content;
            if(!GetContentByLink(link,content))
                return false;
            return _ParseContentUrl(link,content,urlOut,nPageID);
        }
        case CC_LINK_TYPE_DOMAIN:        
             return _ParseDomainUrl(domain.redirectTo,strDomainExt,urlOut,nPageID,nIterations+1);   
        case CC_LINK_TYPE_TXIDOUT:       
            {       
                if(!link.SetStringTxidOut(domain.redirectTo))
                    return false;
                CContent content;
                if(!GetContentByTxidOut(link.txid,link.nVout,content))
                    return false;
                return _ParseContentUrl(link,content,urlOut,nPageID);
            }
        case CC_LINK_TYPE_SCRIPTPUBKEY:
            {                
                string id;
                CScript script;
                script.resize(domain.redirectTo.size());
                script.assign(domain.redirectTo.begin(),domain.redirectTo.end());
                if(!ScriptPubKeyToString(script,id))
                    return false;
                GetNativeLink("fai:browser",urlOut,nPageID);                
                urlOut+="?id="+id;
                LogPrintf("ParseUrl urlout:%s,pageid:%i\n",urlOut,nPageID);
                return true;
            }        
        case CC_LINK_TYPE_HTTP:
        case CC_LINK_TYPE_HTTPS:
        case CC_LINK_TYPE_FTP:
        case CC_LINK_TYPE_ED2K:
        case CC_LINK_TYPE_MAGNET:
        case CC_LINK_TYPE_FILE:
            urlOut=domain.redirectTo+strDomainExt;        
            return true;
        case -1:
            return false;
        default:
            urlOut="http://"+domain.redirectTo+strDomainExt;            
    }
    return true;
}


bool GetContentByTxidOut(const uint256 txid,const int nVout,CContent& content)
{
    
    CTransaction tx;
    uint256 blockhash;
    if (!GetTransaction(txid, tx,blockhash,true))
        return false;    
    LogPrintf("GetContentByTxidOut \n");
    if (!GetContentFromVout(tx, nVout, content))
        return false;
    LogPrintf("GetContentByTxidOut 2\n");
    return true;
}
bool TxidOutLink2BlockChainLink(const uint256 txid,const int nVout,CLink& linkOut)
{
    int64_t txIndex;
    if(!psqliteDB->GetTxIndex(txid,txIndex))
        return false;
      linkOut.nHeight=txIndex>>16;
      linkOut.nTx=txIndex&0xffff;
//    CTransaction tx;    
//    uint256 hashBlock = 0;
//    if (!GetTransaction(txid, tx, hashBlock, true))
//    {
//        return false;
//    }
//    CBlock block;
//    CBlockIndex* pblockindex = mapBlockIndex[hashBlock];
//    
//    if (pblockindex==NULL||!ReadBlockFromDisk(block, pblockindex))
//    {
//        return false;
//    }
//    linkOut.nHeight=pblockindex->nHeight;
//    linkOut.nTx= GetNTx(tx, block);
    linkOut.nVout=nVout;
    return true;
}
bool GetBalance(const vector<CScript>& vScriptPubKeys,CAmount& balance_available,CAmount& balance_unconfirmed,CAmount& balance_locked)
{
    vector<CCheque> vCheques;
    GetUnspentCheques(vScriptPubKeys,vCheques,false,10000000);
    for(unsigned int i=0;i<vCheques.size();i++)
    {
        if(vCheques[i].txIndex<0)
            balance_unconfirmed+=vCheques[i].nValue;
        else if(GetBlocksToMaturity(vCheques[i].nLockTime)>0)
            balance_locked+=vCheques[i].nValue;
        else
            balance_available+=vCheques[i].nValue;
    }
    return true;
}
void GetUnspentCheques(const vector<CScript>& vScriptPubKeys,vector<CCheque>& vCheques,bool fSpendableOnly,int nMaxResults,int nOffset)
{
    if(!fSpendableOnly)
        GetMempoolCheques(vScriptPubKeys,vCheques);
    vector<CCheque> vCheques1;
    pScriptCoinDBView->Search(vScriptPubKeys,vCheques1,nMaxResults,nOffset);   
    for(unsigned int i=0;i<vCheques1.size();i++)
    {
        if(!IsSpentInMempool(COutPoint(vCheques1[i].txid,vCheques1[i].nOut,vCheques1[i].nValue))
                &&(!fSpendableOnly||GetBlocksToMaturity(vCheques1[i].nLockTime)<=0))
            vCheques.push_back(vCheques1[i]);
    }
}
void GetMempoolCheques(const vector<CScript>& vScriptPubKeys,vector<CCheque>& vCheques)
{
    std::vector<CTransaction> vutx; 
    mempool.GetUnconfirmedTransactions(vScriptPubKeys, vutx); 
    for(unsigned int i=0;i<vutx.size();i++)
    {
        for(unsigned int ii=0;ii<vutx[i].vout.size();ii++)
        if(find(vScriptPubKeys.begin(),vScriptPubKeys.end(),vutx[i].vout[ii].scriptPubKey)!=vScriptPubKeys.end()
                &&!IsSpentInMempool(COutPoint(vutx[i].GetHash(),ii,vutx[i].vout[ii].nValue)))
        {
            CCheque cheque;
            cheque.nLockTime=vutx[i].vout[ii].nLockTime;
            cheque.nValue=vutx[i].vout[ii].nValue;
            cheque.nOut=ii;
            cheque.scriptPubKey=vutx[i].vout[ii].scriptPubKey;
            cheque.txid=vutx[i].GetHash();
            cheque.txIndex=-1<<16;
            vCheques.push_back(cheque);
        }
    }
}
bool IsSpentInMempool(const COutPoint op)
{
    return(mempool.mapNextTx.find(op)!=mempool.mapNextTx.end());
}
//bool GetPromotedContents(const vector<CScript>& vSenders,const vector<string>& vCCs,const vector<string>& vTags,vector<CContentDBItem>& vContents,const int nMaxResults,const int nOffset)
//{
//    
//    psqliteDB->GetPromotedContents(const vector<CScript>& vSenders,const vector<string>& vCCs,const vector<string>& vTags,vector<CContentDBItem>& vContents,const int nMaxResults,const int nOffset)
//
//}
bool GetTxOutFromVoutPos(const int64_t pos,CTxOut& out)
{
    LogPrintf("Getransaction by pos %lld nfile %lld,pos%lld \n",pos,pos>>32,pos&0xffffffff);
    CDiskBlockPos filePos;
    filePos.nFile=pos>>32;
    filePos.nPos=pos&0xffffffff;
    CAutoFile file(OpenBlockFile(filePos, true), SER_DISK, CLIENT_VERSION);
    if (file.IsNull())
    {
        LogPrintf("Getransaction by pos, file not found \n");
        return error("%s: OpenBlockFile failed", __func__);
    }    
    try {        
        file >> out;
    } catch (std::exception &e) {
        LogPrintf("GetTxOutFromVoutPos by pos, error: %s\n",e.what());
        return error("%s : Deserialize or I/O error - %s", __func__, e.what());
    }
    return true;
}