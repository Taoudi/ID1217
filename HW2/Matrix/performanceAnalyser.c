#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cmath>
#define SAMPLESIZE 3
#define SAMPLES 8

using namespace std;

ofstream writeFile;
ifstream inFile;
double arr[SAMPLES][SAMPLESIZE];
int comp (const void *e1, const void *e2){
  double f = *((double*)e1);
  double s = *((double*)e2);
  if(f>s)return 1;
  if(f<s)return -1;
  return 0;
}
int main(int argc, char *argv[]){
  int id = (argc > 1)? atoi(argv[1]) : 0;
  writeFile.open("performance" + to_string(id) + ".dat");
  for(int i = 0; i<SAMPLES;i++){
    inFile.open("mat"+to_string(i+1));
    double total_time = 0;

    for(int j = 0; j<SAMPLESIZE;j++){
      double input;
      inFile >> arr[i][j];
    }
    qsort(arr[i], SAMPLESIZE, sizeof(double), comp);
    inFile.close();

  //  printf("median: %f\n", arr[i][SAMPLESIZE/2]);
    printf("speedup: %f\n", arr[0][SAMPLESIZE/2]/arr[i][SAMPLESIZE/2]);

    string output = to_string(i+1) + "\t" + to_string(arr[0][SAMPLESIZE/2]/arr[i][SAMPLESIZE/2]) + "\n";
    writeFile<<output;
  }

  writeFile.close();
  return 0;
}
