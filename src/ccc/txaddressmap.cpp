// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "txaddressmap.h"
#include "random.h"
#include <assert.h>
#include <util.h>
bool CTxAddressMapView::GetTxPosList(const CScript scriptPubkey, std::vector<CDiskTxPos> &vTxPos) {
    LogPrintf("virtual gettxposlist called\n");
    return false;};

bool CTxAddressMapView::BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> > &mapTamList) {
    LogPrintf("virtual BatchWrite called\n");
    return false;
};

bool CTxAddressMapView::Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos) {return false;};

CTxAddressMap::CTxAddressMap(CTxAddressMapView *viewIn):base(viewIn) { }
//CTxAddressMap::CTxAddressMap(CTxAddressMapView *viewIn,Script scriptPubKey) : base(viewIn) ,scriptPubKey(scriptPubKey){
//        base->GetTxPosList(&scriptPubKey,&vTxPos);
//}
//CTxAddressMap::CTxAddressMap(CTxAddressMapView *viewIn,Script scriptPubKey,std::vector<CDiskTxPos> vTxPos) :base(viewIn) , scriptPubKey(scriptPubKey),vTxPos(vTxPos){ }
//    
//    bool CTxAddressMap::getTxidList(std::vector<uint256> &vTxid){return false;};    
//    }   
bool CTxAddressMap::GetTxPosList(const CScript &scriptPubKey,std::vector<CDiskTxPos> &vTxPos) {  
    LogPrintf("CTxAddressMap::GetTxPosList\n");
        return base->GetTxPosList(scriptPubKey,vTxPos);
}        
//    bool CTxAddressMap::GetTxPosList(std::vector<CDiskTxPos> &vTxPos){
//        return base->GetTxPosList(&scriptPubKey,&vTxPos);
//    };        
    //bool BatchWrite(std::vector<CDiskTxPos> &vTxPos)const{return false;};    
//    bool CTxAddressMap::BatchWrite(std::vector<std::pair<CDiskTxPos,CTransaction>> &vTxList)const
//    {
//        for (std::vector<std::pair<CDiskTxPos,CTransaction> >::const_iterator it=vTxList.begin(); it!=vTxList.end(); it++)
//        {
//            CDiskTxPos pos=it->first;
//            CTransaction tx=it->second;
//        }
//        return false;
//    }      
bool CTxAddressMap::BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> >& mapTamList) {return false;}
bool CTxAddressMap::Write(const CScript &scriptPubKey, const std::vector<CDiskTxPos>& vTxPos) {return false;}  
bool CTxAddressMap::AddNewTxs(const  std::map<CScript,CDiskTxPos> &mapTam){
    
    std::map<CScript, std::vector<CDiskTxPos> >  mapTamList;
   
    std::vector<CDiskTxPos> vTxPos;
    
    for (std::map<CScript,CDiskTxPos>::const_iterator it=mapTam.begin(); it!=mapTam.end(); it++){
        //Script scriptPubKey=it->first;     
        
        base->GetTxPosList(it->first,vTxPos);
        LogPrintf("txaddressmap.cpp:addnewtxs vtxpos length:%u \n",vTxPos.size());
        
        if (find(vTxPos.begin(),vTxPos.end(),it->second)==vTxPos.end()){
           
            vTxPos.push_back(it->second);
            
            mapTamList.insert(make_pair(it->first,vTxPos));
            
        }
    }
    return base->BatchWrite(mapTamList);           
}
bool CTxAddressMap::RemoveTxs(const std::map<CScript,CDiskTxPos> &mapTam){
    std::map<CScript, std::vector<CDiskTxPos> >  mapTamList;
    std::vector<CDiskTxPos> vTxPos;
    for (std::map<CScript,CDiskTxPos>::const_iterator it=mapTam.begin(); it!=mapTam.end(); it++){          
        base->GetTxPosList(it->first,vTxPos);
         LogPrintf("txaddressmap.cpp:removetxs vtxpos length:%u \n",vTxPos.size());
        std::vector<CDiskTxPos>::iterator it2=find(vTxPos.begin(),vTxPos.end(),it->second);
        if (it2!=vTxPos.end()){
                vTxPos.erase(it2);
                mapTamList.insert(make_pair(it->first,vTxPos));               
        }        
    }
    return base->BatchWrite(mapTamList);           
}
//    bool getHistory(Script &scriptPubKey);
//    bool getHistory();    
//    bool getUnspent(Script &scriptPubKey);
//    bool getUnspent();