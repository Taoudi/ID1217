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
using namespace std;

#define WORDCOUNT 25143
#define MAXWORKERS 10   /* maximum number of workers */
#define TRUE 1
#define FALSE 0

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
    if(word.at(i)!=word.at(j)){
      boolean=FALSE;
      break;
    }
  }
  return boolean;
}
/* for finding two word palindromes */
int palindromic(string first, string second){
  int boolean = TRUE;
  if(sizeof(first)!=sizeof(second))
    return FALSE;
  for(int i = 0, j=word.length()-1; i<=j;i++,j--){
    if(word.at(i)!=word.at(j)){
      boolean=FALSE;
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
    for (int i = 0; i < WORDCOUNT; i++)
    {
      inFile >> words[i];
    }
  inFile.close();



}
