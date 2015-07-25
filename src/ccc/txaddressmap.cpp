// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "txaddressmap.h"
#include "random.h"
#include "utilstrencodings.h"
#include <assert.h>
#include <util.h>
//#include "utilstrencodings.h"
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
    //LogPrintf("CTxAddressMap::GetTxPosList\n");
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
        vTxPos.clear();
        base->GetTxPosList(it->first,vTxPos);         
//        if (it->first.ToString()=="837a12ff6edf48c868dd6e410ef7983ac9158eac OP_CHECKSIG")
//        {
//            LogPrintf("CTxAddressMap::AddNewTxs:script:%s,new cdisktxpos file:%i,pos:%u,txpos:%i\n",it->first.ToString(),it->second.nFile,it->second.nPos,it->second.nTxOffset);
//            LogPrintf("CTxAddressMap::AddNewTxs:script:%s, vtxpos size:%u\n",it->first.ToString(),vTxPos.size());                
//            for (std::vector<CDiskTxPos>::const_iterator it2=vTxPos.begin(); it2!=vTxPos.end(); it2++)                
//                LogPrintf("CTxAddressMap::AddNewTxs:script:%s,existing cdisktxpos file:%i,pos:%u,txpos:%i\n",it->first.ToString(),it2->nFile,it2->nPos,it2->nTxOffset);
//                    
////                LogPrintf("CTxAddressMap::AddNewTxs:script:%s, mapTamList size:%u\n",it->first.ToString(),mapTamList.size());
////                for (std::map<CScript, std::vector<CDiskTxPos> > ::const_iterator it1=mapTamList.begin(); it1!=mapTamList.end(); it1++)
////                    LogPrintf("CTxAddressMap::AddNewTxs:script:%s,maptamlist script:%s\n",it->first.ToString(),it1->first.ToString());
//        }        
        bool found=false;
        for(unsigned int i=0;i<vTxPos.size();i++)
        //if (find(vTxPos.begin(),vTxPos.end(),it->second)==vTxPos.end())
        {
          if ((vTxPos[i].nFile==it->second.nFile)&&(vTxPos[i].nPos==it->second.nPos)&&(vTxPos[i].nTxOffset==it->second.nTxOffset)) {
              found=true;
              break;
          }
        }
        if(!found)
        {
          vTxPos.push_back(it->second);            
          mapTamList.insert(make_pair(it->first,vTxPos));
//            if (it->first.ToString()=="e3e8c272661cc45106028dc9b5564a721d69bb8b OP_CHECKSIG"){
//                LogPrintf("CTxAddressMap::AddNewTxs after:script:%s, vtxpos size:%u\n",it->first.ToString(),vTxPos.size());
//                LogPrintf("CTxAddressMap::AddNewTxs after:script:%s, mapTamList size:%u\n",it->first.ToString(),mapTamList.size());
//                
//            }
        }
        //if (it->first.ToString()=="837a12ff6edf48c868dd6e410ef7983ac9158eac OP_CHECKSIG")
//        {
//            for (std::map<CScript, std::vector<CDiskTxPos> > ::const_iterator it1=mapTamList.begin(); it1!=mapTamList.end(); it1++)
//                 LogPrintf("CTxAddressMap::AddNewTxs after:maptamlist script:%s\n",it1->first.ToString());
//        }         
              //LogPrintf("CTxAddressMap::AddNewTxs after:script:%s, vtxpos size:%u\n",it->first.ToString(),vTxPos.size());
    }    
    return base->BatchWrite(mapTamList);           
}
bool CTxAddressMap::RemoveTxs(const std::map<CScript,CDiskTxPos> &mapTam){
    std::map<CScript, std::vector<CDiskTxPos> >  mapTamList;
    std::vector<CDiskTxPos> vTxPos;
    //LogPrintf("CTxAddressMap::removeTxs called \n");
    for (std::map<CScript,CDiskTxPos>::const_iterator it=mapTam.begin(); it!=mapTam.end(); it++){          
        base->GetTxPosList(it->first,vTxPos);
        //LogPrintf("txaddressmap.cpp:removetxs pos to remove:%i,%i,%i \n",it->second.nFile,it->second.nPos,it->second.nTxOffset);
        //LogPrintf("vtxpos length:%u data:\n",vTxPos.size());
        //BOOST_FOREACH(const CDiskTxPos&pos, vTxPos) 
        //        LogPrintf("%i,%i,%i  ",pos.nFile,pos.nPos,pos.nTxOffset);
        std::vector<CDiskTxPos>::iterator it2=find(vTxPos.begin(),vTxPos.end(),it->second);
        if (it2!=vTxPos.end()){             
                vTxPos.erase(it2);            
                mapTamList.insert(make_pair(it->first,vTxPos));               
//                if (it->first.ToString()=="837a12ff6edf48c868dd6e410ef7983ac9158eac OP_CHECKSIG"){
//                    base->GetTxPosList(it->first,vTxPos);
//                    LogPrintf("CTxAddressMap::removeTxs after:script:%s, vtxpos size:%u\n",it->first.ToString(),vTxPos.size());   
//                }
        }        
    }
    return base->BatchWrite(mapTamList);           
}
