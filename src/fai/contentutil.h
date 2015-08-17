/* 
 * File:   contentutil.h
 * Author: 
 *
 * Created on July 27, 2015, 5:21 PM
 */

#ifndef FAICOIN_CONTENTUTIL_H
#define	FAICOIN_CONTENTUTIL_H
#include <string>
#include <vector>
using namespace std;
class CLink;
class CLinkUni;
class CContent;
class CBlock;
class CTransaction;
class CTxOut;
class CBlockIndex;
class CTxIn;
class CScript;
class CPubKey;
class uint256;

extern bool GetContentByLink(const CLink clink,CContent& content);
extern bool GetContentByLink(const CLinkUni clink,CContent& content);
extern bool GetDomainLink (const string strDomain,CLink& link);
extern bool GetFileFromLinks(const vector<CLink>& vlinks,string& strFile,int timeOut=5000);
extern bool GetBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex);
extern bool GetTxFromBlock(const CBlock& block, const int nTx, CTransaction& txOut);
extern bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout);
extern bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content);
extern bool ParseUrl(const string urlIn,string& urlOut,int& nPageID);
extern bool _ParseDomainUrl(const string& strDomain,const string& strDomainExt,string& urlOut,int& nPageID,int nIterations);

bool GetNativeLink(const string urlIn,string& urlOut,int& nPageID);
bool _ParseContentUrl(const CLinkUni link,const CContent content,string& urlOut,int& nPageID);
bool GetContentByTxidOut(const uint256 txid,const int nVout,CContent& content);

int GetBlocksToMaturity(const unsigned int nLockTime);
int GetLockLasting(uint32_t nLockTime);

CScript GetTxInScriptPubKey(const CTxIn& txin);
bool GetPubKeyFromBlockChain(CScript script,CPubKey& pubKey);
bool TxidOutLink2BlockChainLink(const uint256 txid,const int nVout,CLink& linkOut);
/** Get nTx from block*/
int GetNTx(const uint256 &hashTx);
int GetNTx(const CTransaction &tx,const CBlock &block);
extern bool GetBalance(const vector<CScript>& vScriptPubKeys,CAmount& balance_available,CAmount& balance_unconfirmed,CAmount& balance_locked);
class COutPoint;
class CCheque;
bool IsSpentInMempool(const COutPoint op);
void GetUnspentCheques(const vector<CScript>& vScriptPubKeys,vector<CCheque>& vCheques,bool fSpendableOnly=true,int nMaxResults=1000,int nOffset=0);
void GetMempoolCheques(const vector<CScript>& vScriptPubKeys,vector<CCheque>& vCheques);
bool GetTxOutFromVoutPos(const int64_t pos,CTxOut& out);
#endif	/* FAICOIN_CONTENTUTIL_H */

