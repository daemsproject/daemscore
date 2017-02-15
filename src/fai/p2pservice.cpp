#include "main.h"

#include "addrman.h"
#include "alert.h"
#include "chainparams.h"
#include "fai/settings.h"
#include "fai/link.h"
#include "fai/content.h"
#include "p2pservice.h"
#include "checkpoints.h"
#include "checkqueue.h"
#include "init.h"
#include "merkleblock.h"
#include "net.h"
#include "pow.h"
#include "txdb.h"
#include "txmempool.h"
#include "ui_interface.h"
#include "util.h"
#include "utilmoneystr.h"
#include "fai/contentutil.h"
#include "fai/domain.h"

#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>
bool ProcessP2PServiceRequest(CNode* pfrom,CDataStream& vRecv, int64_t nTimeReceived)
{
    string strCommand="service";
    int sc;
    int msgID;//an id for client to distingish requests
    vRecv>>sc>>msgID;
    LogPrintf("ProcessP2PServiceRequest sc:%s,msgID:%i \n",sc,msgID);
    switch (sc)
    {
        case SC_FULLNODEPLUS_C_GETTX_BYTXID:
        {                
            CTransaction txOut;
            uint256 hashBlock;
            uint256 hash;
            vRecv>>hash;
            if(!GetTransaction(hash, txOut, hashBlock, true))
            {
                 pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("txid not found"));
                 Misbehaving(pfrom->GetId(), 1);
                 return false;
            }
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_TX;
            ss<<msgID;
            ss<<txOut;
            pfrom->PushMessage(ss,"service");
            break;
        }
        case SC_FULLNODEPLUS_C_GETTX_BYHEIGHTNTX:
        {
            int nHeight;
            int nTx;
            vRecv>>nHeight>>nTx;
            LogPrintf("SC_FULLNODEPLUS_C_GETTX_BYHEIGHTNTX \n");
            CBlockIndex* pblockindex;
            CBlock block;
            if (!GetBlockByHeight(nHeight, block, pblockindex))
            {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("block height too big"));
                Misbehaving(pfrom->GetId(), 5);
                return false;
            }
            LogPrintf("SC_FULLNODEPLUS_C_GETTX_BYHEIGHTNTX2 \n");
            CTransaction tx;
            if (!GetTxFromBlock(block, nTx, tx))
            {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("ntx too big"));
                Misbehaving(pfrom->GetId(), 1);
                return false;
            }
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_TX;
            ss<<msgID;
            ss<<tx;
            pfrom->PushMessage(ss,"service");
            break;
        }
        case SC_FULLNODEPLUS_C_GETTXOUT_BYTXIDNOUT:
        {
            CTransaction txOut;
            uint256 hashBlock;
            uint256 hash;
            unsigned short nOut;
            vRecv>>hash>>nOut;
            if(!GetTransaction(hash, txOut, hashBlock, true))
            {
                 pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("txid not found"));
                 Misbehaving(pfrom->GetId(), 1);
                 return false;
            }
            if(nOut>=txOut.vout.size())
                {
                 pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("nout too big"));
                 Misbehaving(pfrom->GetId(), 1);
                 return false;
            }            
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_TXOUT;
            ss<<msgID;
            ss<<txOut.vout[nOut];
             pfrom->PushMessage(ss,"service");
            break;
        }
        case SC_FULLNODEPLUS_C_GETTXOUT_BYHEIGHTNTXNOUT:
        {
            CTransaction txOut;
            int nHeight;
            int nTx;                
            unsigned short nOut;
            vRecv>>nHeight>>nTx>>nOut;
            CBlockIndex* pblockindex;
            CBlock block;
            if (!GetBlockByHeight(nHeight, block, pblockindex))
            {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("block height too big"));
                Misbehaving(pfrom->GetId(), 5);
                return false;
            }
            LogPrintf("SC_FULLNODEPLUS_C_GETTX_BYHEIGHTNTX2 \n");
            CTransaction tx;
            if (!GetTxFromBlock(block, nTx, tx))
            {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("ntx too big"));
                Misbehaving(pfrom->GetId(), 1);
                return false;
            }
            if(nOut>=tx.vout.size())
                {
                 pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("nout too big"));
                 Misbehaving(pfrom->GetId(), 1);
                 return false;
            }                
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_TXOUT;
            ss<<msgID;
            ss<<tx.vout[nOut];
             pfrom->PushMessage(ss,"service");
            break;
        }
        case SC_FULLNODEPLUS_C_GETDOMAININFO:
        {
            string strDomain;               
            vRecv>>strDomain;
             if(!IsValidDomainFormat(strDomain))
             {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("invalid domain format"));
                Misbehaving(pfrom->GetId(), 10);
                return false;
             }
            CDomain domain;
           if(!pDomainDBView->GetDomainByName(strDomain,domain))
           {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("domain not found"));                    
                return false;
           }
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_DOMAININFO;
            ss<<msgID;
            ss<<domain;
             pfrom->PushMessage(ss,"service");
             break;
        }
        case SC_FULLNODEPLUS_C_GETUNSPENTTXOUT:
        {                
            CScript id;
            vRecv>>id;
            if(id.size()==0)
                {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("invalid ScriptPubKey format"));
                Misbehaving(pfrom->GetId(), 10);
                return false;
             }
            vector<CScript> vScriptPubKey;
            vScriptPubKey.push_back(id);
            vector<CCheque> vCheques;
            GetUnspentCheques(vScriptPubKey,vCheques,false,1000);            
           CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_UNSPENTTXOUT;
            ss<<msgID;
            ss<<vCheques;
             pfrom->PushMessage(ss,"service");
             break;
        }
        case SC_FULLNODEPLUS_C_SEARCHPROMOTEDCONTENT:
        {
            vector<string>vTags;
            vector<int> vCCs;
            vRecv>>vCCs>>vTags;
            if(vTags.size()>9||vCCs.size()>1)
            {
                pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("invalid search parameters"));
                Misbehaving(pfrom->GetId(), 10);
                return false;
             }
            vector<CContentDBItem> vContents;
            vector<CScript> vSenders;//set empty 
            SearchPromotedContents(vSenders,vCCs,vTags,vContents,30,0);            
            for(unsigned int i=0;i<vContents.size();i++)
            {
                CDomain domain;
                pDomainDBView->GetDomainByForward(vContents[i].sender,domain,false);
                vContents[i].senderDomain=domain.strDomain;
            }
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_PROMOTEDCONTENTS;
            ss<<msgID;
            ss<<vContents;
             pfrom->PushMessage(ss,"service");
             break;
        }
        case SC_FULLNODEPLUS_C_GETFEERATE:
        {                
            unsigned int threshould;
            vRecv>>threshould;
            if(threshould>MEMPOOL_ENTRANCE_THRESHOLD)
                threshould=MEMPOOL_ENTRANCE_THRESHOLD;
            double dFeeRate=mempool.getEntranceFeeRate(threshould);
            if (dFeeRate==0)
                dFeeRate=1000;
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss<<(int)SC_FULLNODEPLUS_S_FEERATE;
            ss<<msgID;
            ss<<dFeeRate;
            pfrom->PushMessage(ss,"service");
            break;
        }
        default:
        {
            pfrom->PushMessage("reject", strCommand, REJECT_NONSTANDARD, string("invalid sc"));
            //    Misbehaving(pfrom->GetId(), 10);
            //    return false;
            LogPrintf("service result: service code:%s,msgID:%i,data:%s",mapSCNames[sc],msgID,HexStr(vRecv.begin(),vRecv.end()));
            
        }
    }
    
    return true;
}