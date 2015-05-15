// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CCCOIN_TXADDRESSMAP_H
#define CCCOIN_TXADDRESSMAP_H

#include "compressor.h"
#include "serialize.h"
#include "uint256.h"
#include "undo.h"
#include "script/script.h"
#include "chain.h"
#include <assert.h>
#include <stdint.h>

#include <boost/foreach.hpp>
//#include <boost/unordered_map.hpp>
//typedef boost::unordered_map<CScript,std::vector<CDiskTxPos>> CTamMap;

/** Abstract view on the open txaddressmap dataset. */
class CTxAddressMapView
{
public:    
    virtual bool GetTxPosList(const CScript scriptPubKey,std::vector<CDiskTxPos> &vTxPos);    

    //! Do a bulk modification (multiple tam changes).
    //! The passed mapTam can be modified.
    virtual bool BatchWrite(const std::map<CScript,std::vector<CDiskTxPos> > &mapTamList);
    virtual bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
    //! As we use CTxAddressMapView polymorphically, have a virtual destructor
    virtual ~CTxAddressMapView() {}
    
};

class CTxAddressMap : public CTxAddressMapView
{
protected:
    CTxAddressMapView *base;
public:
    //CScript scriptPubKey;
    //std::vector<CDiskTxPos> vTxPos;
    //std::vector<CTransaction> vTx;
    //std::vector<uint256> vTxid;
    CTxAddressMap(CTxAddressMapView *viewIn);
    //CTxAddressMap(CTxAddressMapView *viewIn,CScript scriptPubKey);
    //CTxAddressMap(CTxAddressMapView *viewIn,CScript scriptPubKey,std::vector<CDiskTxPos> vTxPos);
    //bool getTxidList(Script &scriptPubKey,std::vector<uint256> &vTxid) const;
    //bool getTxidList(std::vector<uint256> &vTxid) const;    
    bool GetTxPosList(const CScript &scriptPubKey,std::vector<CDiskTxPos> &vTxPos);    
    //bool GetTxPosList(std::vector<CDiskTxPos> &vTxPos);        
    //bool BatchWrite(std::vector<CDiskTxPos> &vTxPos)const;    
    //bool BatchWrite(std::vector<std::pair<CDiskTxPos,CTransaction>>& vTxList) const;   
    bool BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> > &mapTamList);
    bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
    bool AddNewTxs(const std::map<CScript,CDiskTxPos> &mapTam);
    bool RemoveTxs(const std::map<CScript,CDiskTxPos> &mapTam);
    //bool getHistory(Script &scriptPubKey);
    //bool getHistory();    
    //bool getUnspent(Script &scriptPubKey);
    //bool getUnspent();        
};





#endif // CCCOIN_TXADDRESSMAP_H
