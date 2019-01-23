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
using namespace std;

#define WORDCOUNT 25143
#define MAXWORKERS 10   /* maximum number of workers */
#define TRUE 1
#define FALSE 0

int counter = 0;

double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

/* for finding single word palindromes */
int palindromic(string word){
  int boolean = TRUE;
  for(int i = 0, j=word.length()-1; i<=j;i++,j--){
    if(tolower(word.at(i))!=tolower(word.at(j))){
      boolean=FALSE;
      break;
    }
  }
  return boolean;
}

int binarySearch(string word, string words[]){
  int boolean = FALSE;
  double low = 0;
  double high = WORDCOUNT-1;
  string currentWord;
  int index;
  while(low <= high){
    index = floor((low+high)/2);
    currentWord = words[index];
    int compared = strcasecmp(word.c_str(), currentWord.c_str());
    if(compared < 0){
        high = index-1;
    }
    else if(compared > 0){
      low = index+1;
    }
    else{
        boolean = TRUE;
        break;
      }
  }
  return boolean;
}

int main(int argc, char *argv[]){
  ifstream inFile;
  string words[WORDCOUNT];
  /* read command line args if any */
  int numWorkers = (argc > 1)? atoi(argv[1]) : MAXWORKERS;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  int listSize = WORDCOUNT/numWorkers;

  /* Loading file data onto array */
  inFile.open("words");
    for (int i = 0; i < WORDCOUNT-1; i++)
    {
      inFile >> words[i];
    }
  inFile.close();

  for(int i = 0; i<WORDCOUNT-1;i++){
    if(palindromic(words[i])==TRUE){
      printf("ONE WORD PAL %s\n", words[i].c_str());
      counter++;
    }
    else{
      string copy = words[i];
      reverse(copy.begin(),copy.end());
        if(binarySearch(copy,words)==TRUE){
          counter++;
          printf("TWO WORD PAL %s = %s\n", words[i].c_str(), copy.c_str());
        }
      }
    }
  printf("Counter: %d\n", counter);

}
