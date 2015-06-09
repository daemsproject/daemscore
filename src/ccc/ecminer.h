// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CCCOIN_EC_MINER_H
#define CCCOIN_EC_MINER_H

#include <stdint.h>
#include <string>
#include <vector>
class CWallet;
class CPubKey;
class CKey;
/** Run the miner threads */
bool VanityGen(bool fGenerate,CWallet* pwallet,std::vector<std::string> vstrTarget, CPubKey& pubkey, int nThreads=-1);
void EcMiner(CWallet* pwallet,const std::vector<std::string> vstrTarget,const CPubKey& pubKey);

extern double dEcsPerSec;
extern int64_t nEcPSTimerStart;
extern bool fEcHeaderFound;
#endif // CCCOIN_EC_MINER_H
