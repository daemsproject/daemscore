// Copyright (c) 2012-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "script2txposdb.h"
#include "random.h"
#include "utilstrencodings.h"
#include <assert.h>
#include <util.h>
//#include "utilstrencodings.h"
bool CScript2TxPosDBView::GetTxPosList(const CScript scriptPubkey, std::vector<CDiskTxPos> &vTxPos) {
    LogPrintf("virtual gettxposlist called\n");
    return false;};

bool CScript2TxPosDBView::BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> > &mapScriptTxPosList) {
    LogPrintf("virtual BatchWrite called\n");
    return false;
};

bool CScript2TxPosDBView::Write(const CScript &scriptPubKey,const std::vector<CDiskTxPos> &vTxPos) {return false;};

CScript2TxPosDB::CScript2TxPosDB(CScript2TxPosDBView *viewIn):base(viewIn) { }
  
bool CScript2TxPosDB::GetTxPosList(const CScript &scriptPubKey,std::vector<CDiskTxPos> &vTxPos) {  
    //LogPrintf("CScript2TxPosDB::GetTxPosList\n");
        return base->GetTxPosList(scriptPubKey,vTxPos);
}        
   
bool CScript2TxPosDB::BatchWrite(const std::map<CScript, std::vector<CDiskTxPos> >& mapScriptTxPosList) {return false;}
bool CScript2TxPosDB::Write(const CScript &scriptPubKey, const std::vector<CDiskTxPos>& vTxPos) {return false;}  
bool CScript2TxPosDB::AddNewTxs(const  std::map<CScript,CDiskTxPos> &mapScriptTxPos){
    
    std::map<CScript, std::vector<CDiskTxPos> >  mapScriptTxPosList;
   
    std::vector<CDiskTxPos> vTxPos;
    
    for (std::map<CScript,CDiskTxPos>::const_iterator it=mapScriptTxPos.begin(); it!=mapScriptTxPos.end(); it++){
        //Script scriptPubKey=it->first;     
        vTxPos.clear();
        base->GetTxPosList(it->first,vTxPos);        
     
        bool found=false;
        for(unsigned int i=0;i<vTxPos.size();i++)        
        {
          if ((vTxPos[i].nFile==it->second.nFile)&&(vTxPos[i].nPos==it->second.nPos)&&(vTxPos[i].nTxOffset==it->second.nTxOffset)) {
              found=true;
              break;
          }
        }
        if(!found)
        {
          vTxPos.push_back(it->second);            
          mapScriptTxPosList.insert(make_pair(it->first,vTxPos));
        }
    }    
    return base->BatchWrite(mapScriptTxPosList);           
}
bool CScript2TxPosDB::RemoveTxs(const std::map<CScript,CDiskTxPos> &mapScriptTxPos){
    std::map<CScript, std::vector<CDiskTxPos> >  mapScriptTxPosList;
    std::vector<CDiskTxPos> vTxPos;
    
    for (std::map<CScript,CDiskTxPos>::const_iterator it=mapScriptTxPos.begin(); it!=mapScriptTxPos.end(); it++){          
        base->GetTxPosList(it->first,vTxPos);        
        std::vector<CDiskTxPos>::iterator it2=find(vTxPos.begin(),vTxPos.end(),it->second);
        if (it2!=vTxPos.end()){             
                vTxPos.erase(it2);            
                mapScriptTxPosList.insert(make_pair(it->first,vTxPos));  
        }        
    }
    return base->BatchWrite(mapScriptTxPosList);           
}
