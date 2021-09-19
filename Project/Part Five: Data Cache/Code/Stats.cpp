/******************************
 * Name:  Derek Hernandez (djh119)
 * CS 3339 - Spring 2019
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}
void Stats::clock() {
  cycles++;

  // advance all pipeline flip flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
  }
  // inject a no-op into IF1
  resultReg[IF1] = -1;
}
// Passing in RS/Rt
void Stats::registerSrc(int r) {
    //Src's are available in the EXE stage but require to reach finished in WB
   int buble = 0;
   for(int i = EXE1; i < WB; i++) {
        // Find the location of register in pipline, then bubble whats infront
        if (r == 0){
           break;
	}
        if(resultReg[i] == r){
            // No need to Bubble WB
            buble =  WB - i;
            //correctly add bubble to the pipeline
            while (buble > 0){
                bubble();
                --buble;
            }
            //Bubble incremented based on r location,
            break;
        }
    }
}
//Pass in Rd (destination)
void Stats::registerDest(int r) {
    resultReg[ID] = r;
}
// Number of flushes passed
void Stats::flush(int count) { // count == how many ops to flush
   for (int i = 0; i < count; i++){
        flushes++;
        clock();
    }
}
//Increment bubble when called
void Stats::bubble() {
    bubbles++;
    cycles++;

    for (size_t x = WB; x < EXE1; x--){
	resultReg[x] = resultReg[x-1];
    }
    resultReg[EXE1] = -1;
}
// Number of stalls is passed
void Stats::stall(int amount) {
    for (int i = 0; i < amount; i++){
        stalls++;
        cycles++;
    }
}
