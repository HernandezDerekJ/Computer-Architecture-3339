/******************************
 * Submitted by: Derek Hernandez (djh119)
 * CS 3339 - Spring 2019
 * Project 4 Branch Predictor
 * Copyright 2019, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <array>
#include "BranchPred.h"
using namespace std;

BranchPred::BranchPred() {
  cout << "Branch Predictor Entries: " << BPRED_SIZE << endl;
  /* TODO Initialize your private variables and pred[] array here*/

    pred[BPRED_SIZE] = 0 ;  // stores the 2-bit saturating counter values
    btb[BPRED_SIZE] = uint32_t(0);  // stores the target address of the branch
    predictions = 0;
    pred_takens = 0;
    mispredictions= 0;
    mispred_direction= 0;
    mispred_target= 0;
}

bool BranchPred::predict(uint32_t pc, uint32_t &target) {
  /* target value updated by reference to match the btb entry
   * returns true if prediction is taken, false otherwise
   * increments predictions and if necessary pred_takens count
   */   
  /* TODO implement functionality here, leave the Debug line below right before the return */

    //PC == address in not taken - 4;
    //target == address if taken;
    uint32_t predictedTarget;
    int index = (((pc - 4) >> 2) % BPRED_SIZE);
    int length = 0, average = 0;
    int cache;
    bool empty, predTaken, predictedBool;
    
    //Check
    for (int t = 0; t < 64; t++){
        if (btb[t] != uint32_t(0)){
            empty = false;
            break;
        }
    }
    if(empty){
        //predict False;
        cache = 0;
        predTaken = false;
        btb[index] = pc - 4;
        pred[index] = 0;
        predictions++;
    }
    else{
        for (int x = 0; x < 64; x++){
            if (btb[x] != uint32_t(0)){
                length++;
                average += pred[x];
            }
        }
        //Predict True
        if (((int)(average / length)) >= 2){
            if (0 <= cache < 3){
                predictedTarget = target;
                predictedBool = true;
                btb[index] = target;
                pred[index] = cache;
                predTaken = true;
                predictions++;
                cache++;
            }
            else if (cache == 3){
                predictedTarget = target;
                predictedBool = true;
                btb[index] = target;
                pred[index] = cache;
                predTaken = true;
                predictions++;
            }
            
        }
        //Predict False
        else{
            if (1 <= cache <= 3){
                predictedTarget = pc;
                predictedBool = false;
                btb[index] = pc - 4;
                pred[index] = cache;
                predTaken = false;
                predictions++;
                cache--;
            }
            else if(cache == 0){
                predictedTarget = pc;
                predictedBool = false;
                btb[index] = pc - 4;
                pred[index] = cache;
                predTaken = false;
                predictions++;
            }
        }
        return predTaken;
    }

    
    // Will return the bool value of prediction
    D(cout << endl << "    BPRED: bne/beq@addr " << hex << setw(8) << pc << "pred[] value = " << pred[index] << " predTaken is " << boolalpha << (bool)predTaken; (predTaken==0)? cout << " not taken":cout << " taken target addr " << hex << setw(8) << target;)
  return predTaken; //uncomment this line once you've added the code to calculate predTaken
}

bool BranchPred::isMispredict(bool predTaken, uint32_t predTarget, bool taken, uint32_t target) {
  /* implement a function which will return:
   *  false if prediction is correct, both predTaken and predTarget match actual values
   *  true if prediction is incorrect
   *  also updates mispred_direction, mispred_target, and mispredictions counts
   */
  /* TODO implement functionality here */
    
    //T vs MisT
    if(predTaken == taken){
        pred_takens++;
    }
    else{
        mispred_direction++;
        mispredictions++;
    }
    
    //Miss Direction
    if (taken == predTaken && predTarget != target){
        mispred_target++;
        mispredictions++;
    }
    if(predTarget == target && predTaken == taken){
        return false;
    }
    else if(predTarget != target && predTaken != taken){
        return true;
    }
}

void BranchPred::update(uint32_t pc, bool taken, uint32_t target) {
  /* pred counter value should be updated and
   * if branch is taken, the target address also gets updated
   */
  /* TODO implement functionality here */
    //isMispredict(bool predTaken, uint32_t predTarget, bool taken, uint32_t target)
    int cacheVal, counter;
    int index = (((pc - 4) >> 2) % BPRED_SIZE);
    uint32_t targetVal;
    
    if (taken){
        btb[index] = target;
        pred[index] = transition(counter, taken);
    }
    else{
        btb[index] = pc - 4;
        pred[index] = transition(counter, taken);
    }
    return;
}
//********** Work on Cache
int BranchPred::transition(int counter, bool taken) {
  /* This updates the 2-bit saturating counter values
   * You will need to use it, but shouldn't have to modify.
   */
  int transition;
  switch(counter) {
    case 0: transition = (taken ? 1 : 0);
            break;
    case 1: transition = (taken ? 2 : 0);
            break;
    case 2: transition = (taken ? 3 : 1);
            break;
    case 3: transition = (taken ? 3 : 2);
            break;
    default: cerr << "ERROR: 2-bit saturating counter FSM in illegal state" << endl;
  }
  return transition;
}

void BranchPred::printFinalStats() {
  int correct = predictions - mispredictions;
  int not_takens = predictions - pred_takens;

  cout << setprecision(1);
  cout << "Branches predicted: " << predictions << endl;
  cout << "  Pred T: " << pred_takens << " ("
       << (100.0 * pred_takens/predictions) << "%)" << endl;
  cout << "  Pred NT: " << not_takens << endl;
  cout << "Mispredictions: " << mispredictions << " ("
       << (100.0 * mispredictions/predictions) << "%)" << endl;
  cout << "  Mispredicted direction: " << mispred_direction << endl;
  cout << "  Mispredicted target: " << mispred_target << endl;
  cout << "Predictor accuracy: " << (100.0 * correct/predictions) << "%" << endl;
}
