// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fai/ecminer.h"
#include "amount.h"
//#include "main.h"
//#include "init.h"
#include "net.h"

#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "base58.h"
#include "utilstrencodings.h"
#include "core_io.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// EcMiner
//




// Internal miner
//
double dEcsPerSec = 0.0;
int64_t nEcPSTimerStart = 0;

bool fEcHeaderFound=false;



void EcMiner(CWallet* pwallet,const std::vector<std::string> vstrTarget,const CPubKey& pubKey)
{
    CPubKey pub=pubKey;    
    CKey stepKey;
    stepKey.MakeNewKey(true);
    CPubKey stepPub;
    stepKey.GetPubKey(stepPub);
    LogPrintf("ecMiner started\n");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("ec-miner");
    unsigned int nMaxLen=0;
    uint64_t nSteps=0;
    for(unsigned int i=0;i<vstrTarget.size();i++)   { 
        LogPrintf("ecminer target:%s\n",vstrTarget[i]);
        if (vstrTarget[i].size()>nMaxLen)
            nMaxLen=vstrTarget[i].size();    
    }
    try {
        //int64_t nStart = GetTime();
        while(true)
        {
            unsigned int nHashesDone = 0;
                while(true)
                {    
                    pub.AddSteps(stepPub,1);
                    nSteps++;
                    nHashesDone += 1;
                    string strB32=CBitcoinAddress(pub).GetHeader(nMaxLen);                    
//                    CKey tempstep=stepKey.GetMultipliedTo(nSteps);
//                    CKey tempstep2=stepKey.GetMultipliedTo(1);
//                    CPubKey tempbasepub=pubKey;
//                    tempbasepub.AddSteps(tempstep.GetPubKey(),1);
//                    CPubKey tempbasepub2=pubKey;
//                    long step=(long)nSteps;
//                    tempbasepub2.AddSteps(stepPub,step);
//                    LogPrintf("GetMultipliedTo1 origin:%s result:%s",HexStr(stepKey.begin(),stepKey.end()),HexStr(tempstep2.begin(),tempstep2.end()));
//                    LogPrintf("ecminer add1:%s\n multi:%s\n,addmulti:%s\n",CBitcoinAddress(pub).ToString(),CBitcoinAddress(tempbasepub2).ToString(),CBitcoinAddress(tempbasepub).ToString());
                    for(unsigned int i=0;i<vstrTarget.size();i++)  
                    {
                        //LogPrintf("ecminer result:%s\n",strB32.substr(0,vstrTarget[i].size()));
                        if (B32Equal(strB32.substr(0,vstrTarget[i].size()), vstrTarget[i]))
                        {
                        // Found a solution
                        fEcHeaderFound=true;
                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        LogPrintf("ecMiner:header found: %s pub:%s\n", vstrTarget[i],CBitcoinAddress(pub).ToString());  
                        CKey resultStepKey;
                        stepKey.GetMultipliedTo(nSteps,resultStepKey);
                        pwallet-> NotifyEcMinerResult(pubKey,resultStepKey,vstrTarget[i]);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);                            
                        break;
                        }
                    }                                        
                    if(fEcHeaderFound)
                        return;
                    boost::this_thread::interruption_point();
                    if((nSteps&0xFF)==0)
                        break;
                }

                 //Meter ecs/sec
                static int64_t nEcCounter;
                if (nEcPSTimerStart == 0)
                {
                    nEcPSTimerStart = GetTimeMillis();
                    nEcCounter = 0;
                }
                else
                    nEcCounter += nHashesDone;
                if (GetTimeMillis() - nEcPSTimerStart > 4000)
                {
                    static CCriticalSection cs;
                    {
                        LOCK(cs);
                        if (GetTimeMillis() - nEcPSTimerStart > 4000)
                        {
                            dEcsPerSec = 1000.0 * nEcCounter / (GetTimeMillis() - nEcPSTimerStart);
                            nEcPSTimerStart = GetTimeMillis();
                            nEcCounter = 0;
                            static int64_t nLogTime;
                            if (GetTime() - nLogTime > 30 * 60)
                            {
                                nLogTime = GetTime();
                                LogPrintf("ecmeter %6.3f kec/s\n", dEcsPerSec/1000.0);
                            }
                        }
                    }
                }
        }

    }
    catch (boost::thread_interrupted)
    {
        LogPrintf("ecMiner thread terminated\n");      
        //throw;
    }
}

bool VanityGen(bool fGenerate,CWallet* pwallet,std::vector<std::string> vstrTarget, CPubKey& pubkey, int nThreads)
{
    static boost::thread_group* minerThreads = NULL;

    if (nThreads < 0) {
        // In regtest threads defaults to 1
        if (Params().DefaultMinerThreads())
            nThreads = Params().DefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }

    if (minerThreads != NULL)
    {
        minerThreads->interrupt_all();
        
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0)
        return false;
    if (!fGenerate)
        return true;
    if(vstrTarget.size()==0)
        return false;
    fEcHeaderFound=false;
    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++)
    {
       
        minerThreads->create_thread(boost::bind(&EcMiner,pwallet, vstrTarget,pubkey));
    }
    return true;
}

//#endif // ENABLE_WALLET
