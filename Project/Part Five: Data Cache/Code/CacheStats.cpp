/******************************
 * CacheStats.cpp submitted by: Derek Hernandez (djh119)
 * CS 3339 - Spring 2019
 * Project 4 Branch Predictor
 * Copyright 2019, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  loads = 0;
  stores = 0;
  load_misses = 0;
  store_misses = 0;
  writebacks = 0;

  /* TODO: your code here */
    tag = uint32_t(0);
    index = uint32_t(0);
    stallLatency = 0;
    for(int a = 0; a < SETS; a++){
        w[a]=0;
        for(int b = 0; b < WAYS; b++){
            tags[a][b] = uint32_t(0);
            modify[a][b] = 0;
            valid[a][b] = 0;
        }
    }
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // cache is off
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }

  /* TODO: your code here */
    // Vars
    // Offset == log2 (32)
    // Set == 3 (2^3)
        index = (addr >> 5) & 0x7;
    //Data to store
        tag = addr >> 8;
    //Reset Stall
        stallLatency = 0;
    //Loads and Stores
    if (type == STORE)
        stores++;
    else if(type == LOAD)
        loads++;
    
    //HIT
    //tags match and its not empty(vaild)
    for(int a = 0; a < WAYS; a++){
        if((tags[index][a] == tag) && (valid[index][a])){
            stallLatency = LOOKUP_LATENCY;
            if(type == STORE)
                modify[index][a] = 1;
            return stallLatency;
        }
    }
    // Miss two types Clean, Dirty
    if(type == STORE)
        store_misses++;
    else if(type == LOAD)
        load_misses++;
    //Clean Modify == 0 Latency == 30
    if(modify[index][w[index]] == 0){
        stallLatency = stallLatency + READ_LATENCY;
    }
    //Dirty Modify == 1 Latency == 10+30
    else if(modify[index][w[index]] == 1){
        stallLatency = stallLatency + READ_LATENCY + WRITE_LATENCY;
   	writebacks++;
    }
    //Install into cache
    tags[index][w[index]] = tag;
    valid[index][w[index]] = 1;

    //Determine Modify,based on type
    if(type == LOAD){
        modify[index][w[index]] = 0;
    }
    else if(type == STORE){
        modify[index][w[index]] = 1;
    }

    //Control w values, round robin 4 way, never go over 3
    if (w[index] == 3){
        w[index] = 0;
    }
    else{
        w[index]++;
    }

    return stallLatency;
}
void CacheStats::printFinalStats() {
  /* TODO: your code here (don't forget to drain the cache of writebacks) */
    // Write Back == Dirty/Modified (Whenever modify is true)
    for(int x = 0; x < SETS; x++){
        for(int y = 0; y < WAYS; y++){
            if(modify[x][y])
                writebacks++;
        }
    }
  int accesses = loads + stores;
  int misses = load_misses + store_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
