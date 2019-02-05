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
#define SAMPLESIZE 100
#define SAMPLES 10

using namespace std;

ofstream writeFile;
ifstream inFile;

int main(){
  writeFile.open("performance.dat");

  for(int i = 0; i<SAMPLES;i++){
    inFile.open("pal"+to_string(i+1));
    double total_time = 0;

    for(int j = 0; j<SAMPLESIZE;j++){
      double input;
      inFile >> input;
      total_time = total_time+input;
    }

    inFile.close();
    printf("avg: %f\n", total_time/SAMPLESIZE);
    string output = to_string(i+1) + "\t" + to_string(total_time/SAMPLESIZE) + "\n";
    writeFile<<output;
  }

  writeFile.close();
  return 0;
}
