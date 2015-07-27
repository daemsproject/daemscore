/* 
 * File:   contentutil.h
 * Author: 
 *
 * Created on July 27, 2015, 5:21 PM
 */

#ifndef CONTENTUTIL_H
#define	CONTENTUTIL_H
#include <string>
#include <vector>
using namespace std;
class CLink;
class CContent;
class CBlock;
class CTransaction;
class CTxOut;
class CBlockIndex;


bool GetContentByLink(const CLink clink,CContent& content);
bool GetDomainLink (const string strDomain,CLink& link);
bool GetFileFromLinks(const vector<CLink>& vlinks,string& strFile);
bool GetBlockByHeight(const int nHeight, CBlock& blockOut, CBlockIndex*& pblockindex);
bool GetTxFromBlock(const CBlock& block, const int nTx, CTransaction& txOut);
bool GetVoutFromTx(const CTransaction& tx, const int nVout, CTxOut& vout);
bool GetContentFromVout(const CTransaction& tx, const int nVout, CContent& content)
#endif	/* CONTENTUTIL_H */

