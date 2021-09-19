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
    for(int i = EXE1; i < WB; i++) {
        // Find the location of register in pipline, then bubble whats infront
        if(resultReg[i] == r){
            // Bubble remaining pipeline
            int buble =  WB - i;
            //correctly add bubble to the pipeline
            while (buble > 0){
                //Bubble increment
                bubble();
                //increment while loop 
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
//Increment by 1 bubble when called
void Stats::bubble() {
    bubbles++;
    cycles++;
}
