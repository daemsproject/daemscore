// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FAICOIN_SCRIPT2TXPOSDB_H
#define FAICOIN_SCRIPT2TXPOSDB_H

#include "compressor.h"
#include "serialize.h"
#include "uint256.h"
#include "undo.h"
#include "script/script.h"
#include "chain.h"
#include <assert.h>
#include <stdint.h>

#include <boost/foreach.hpp>


/** Abstract view on the open SCRIPT2TXPOSDB dataset. */
class CScript2TxPosDBView
{
public:    
    virtual bool GetTxPosList(const CScript scriptPubKey,std::vector<CDiskTxPos> &vTxPos);    

    //! Do a bulk modification (multiple tam changes).
    //! The passed mapScriptTxPosList can be modified.
    virtual bool BatchWrite(const std::map<CScript,std::vector<CDiskTxPos> > &mapScriptTxPosList);
    virtual bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
    //! As we use CScript2TxPosDBView polymorphically, have a virtual destructor
    virtual ~CScript2TxPosDBView() {}
    
};

//class CScript2TxPosDB : public CScript2TxPosDBView
//{
//protected:
//    CScript2TxPosDBView *base;
//public:    
//    CScript2TxPosDB(CScript2TxPosDBView *viewIn);     
//    bool GetTxPosList(const CScript &scriptPubKey,std::vector<CDiskTxPos> &vTxPos);   
//    
//    bool BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> > &mapScriptTxPosList);
//    bool Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos);
//    bool AddNewTxs(const std::map<CScript,CDiskTxPos> &mapScriptTxPos);
//    bool RemoveTxs(const std::map<CScript,CDiskTxPos> &mapScriptTxPos);
//         
//};





#endif // FAICOIN_SCRIPT2TXPOSDB_H
