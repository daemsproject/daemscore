/* 
 * File:   contentutil.h
 * Author: 
 *
 * Created on July 27, 2015, 5:21 PM
 */

#ifndef CCCOIN_CONTENTUTIL_H
#define	CCCOIN_CONTENTUTIL_H
#include <string>
#include <vector>
using namespace std;
class CLink;
class CContent;
class CBlock;
class CTransaction;
class CTxOut;
class CBlockIndex;


extern bool GetContentByLink(const CLink clink,CContent& content);
extern bool GetDomainLink (const string strDomain,CLink& link);
extern bool GetFileFromLinks(const vector<CLink>& vlinks,string& strFile);
extern bool GetBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex);
extern bool GetTxFromBlock(const CBlock& block, const int nTx, CTransaction& txOut);
extern bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout);
extern bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content);
#endif	/* CCCOIN_CONTENTUTIL_H */

