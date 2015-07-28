
#include "main.h"
#include "txdb.h"
#include "ccc/link.h"
#include "ccc/content.h"
#include "ccc/domain.h"
//#include "utilstrencodings.h"
#include "ccc/content.h"
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
        if(vDecoded[0].first!=CC_FILE_PART)
            return false;
        strFile+=vDecoded[0].second;
    }
    return true;
}