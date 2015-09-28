#ifndef MHASH_H
#define MHASH_H
#include <stdlib.h>
#include <stdint.h>
#include "uint256.h"
void mixHash(uint256* input,const unsigned int height);
unsigned char* mixHash(unsigned char* hashIn,const int height);
void mixAdd(uint256* roller);
void getMHashRandom(uint256* mHashRnd);

#endif
