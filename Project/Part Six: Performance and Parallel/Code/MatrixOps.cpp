/* CS3339 Assignment 6 Problem 1
 * Modified by Derek Hernandez (djh119)
 * Original Lee Hinkle
 * Some references used: https://software.intel.com/en-us/node/529735
 *                       http://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
 * Timing code based off work by Dr. Martin Burtscher, Texas State University
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

int run(int a){
    cout << "Project 6 Array Traversal: ";
    int size = a;  // this is the #rows and #columns or n in nxn matrix
    cout << " Array size = " << (sizeof(size) / sizeof(int)) * size << " Bytes" << endl;
    int *A[size];

    for (int r=0; r<size; r++){
        A[r] = (int *)malloc(size * sizeof(int));
        if (A[r] == NULL) {
            printf( "\n ERROR: Can't allocate memory for matrix. Aborting... \n\n");
            free(*A);
            return 1;
        }
    }

    double runtime; // for time measurement
    struct timeval start, end;  // for time measurement
    //initialize matrix
    for (int r = 0; r < size; r++){
        for (int c = 0; c < size; c++){
            A[r][c] = (int)r*c+1;
        }
    }

    gettimeofday(&start, NULL);  // start timer
    // perform matrix operation - process row first, column scans on inner loop
    for (int r = 0; r < size; r++){
        for (int c = 0; c < size; c++){
            A[r][c] = 2*A[r][c];
        }
    }
    gettimeofday(&end, NULL);  //end timer
    //compute and display results
    runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
    printf("by row compute time: %.4f seconds ", runtime);
    printf("mega_elements/sec: %.3f\n", size*size*0.000001 / runtime);

    //re-initialize matrix outside of timing measurements
    for (int r = 0; r < size; r++){
        for (int c = 0; c < size; c++){
            A[r][c] = (int)r*c+1;
        }
    }
    gettimeofday(&start, NULL);  // start timer
    // perform matrix operation - process column first, row scans on inner loop
    //Opposite of first matrix operations
    for (int a = 0; a < size; a++){
        for (int b = 0; b < size; b++){
            A[a][b] = 2 * A[a][b];
        }
    }

    gettimeofday(&end, NULL);  // end timer
    //compute and display results
    runtime = end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0;
    printf("by column compute time: %.4f seconds ", runtime);
    printf("mega_elements/sec: %.3f\n", size*size*0.000001 / runtime);
    free(*A);

    return 0;
}
//Helper Function, for part 1-D
int main()
{
    for(int x = 1; x <= 3; x++ ){
        cout<< "Test: " + x << endl;
        run(500);
        cout << endl;
        run(10000);
        cout << endl;
        run(15000);
        cout << endl;
        run(18000);
        cout << endl;
        run(20001);
        cout << "Test " << x << " Done: " << endl;
    }
}
