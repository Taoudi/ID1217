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

int numWorkers;
int listSize;
int counter[MAXWORKERS];
string words[WORDCOUNT];
pthread_mutex_t counter_lock;
ofstream writeFile;

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
  string copy = word;
  reverse(copy.begin(),copy.end());
  int boolean = FALSE;
  double low = 0;
  double high = WORDCOUNT-1;
  string currentWord;
  int index;
  while(low <= high){
    index = floor((low+high)/2);
    currentWord = words[index];
    int compared = strcasecmp(copy.c_str(), currentWord.c_str());
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
void *findPalindromes(void *);

int main(int argc, char *argv[]){
  pthread_t workerid[MAXWORKERS];
  pthread_mutex_init(&counter_lock, NULL);
  int total = 0;
  pthread_attr_t attr;

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  ifstream inFile;
  /* read command line args if any */
  numWorkers = (argc > 1)? atoi(argv[1]) : MAXWORKERS;
  if(numWorkers>MAXWORKERS){
    numWorkers=MAXWORKERS;
  }
  listSize = WORDCOUNT/numWorkers;

  /* Loading file data onto array */
  writeFile.open("palindromes");
  inFile.open("words");
    for (int i = 0; i < WORDCOUNT-1; i++)
    {
      inFile >> words[i];
    }
  inFile.close();

  double start = read_timer();
  for (int l = 0; l < numWorkers; l++){
    pthread_create(&workerid[l], &attr, findPalindromes, (void *) l);
  }
  for (int l = 0; l < numWorkers; l++){
    pthread_join(workerid[l],NULL);
  }
  double end = read_timer();
  for(int i =0; i<MAXWORKERS;i++){
    total = total + counter[i];
    printf("Worker %d is done with %d counted\n", i,counter[i]);

  }
  printf("TOTAL: %d\n", total);
  printf("Execution time: %f sec\n", end-start);

  printf("DONE\n");

  //pthread_exit(NULL);
}
void Barrier(){

}


void *findPalindromes(void *arg){
  int mycounter = 0;
  int myid = (long)arg;
  int strip = listSize*myid;
  //printf("worker %d (pthread id %d) has started\n", myid, pthread_self());

  for(int i = listSize*myid; i<listSize*(myid+1);i++){
    if(palindromic(words[i])==TRUE){
    //  printf("ONE WORD: %s, %d\n ",words[i].c_str(),myid);
      pthread_mutex_lock(&counter_lock);
      counter[myid]++;
      writeFile << words[i]+"\n";
      pthread_mutex_unlock(&counter_lock);
    }
    else if(binarySearch(words[i],words) == TRUE){
    //  printf("TWO WORD: %s, %d\n",words[i].c_str(),myid);
      pthread_mutex_lock(&counter_lock);
      counter[myid]++;
      writeFile << words[i]+"\n";
      pthread_mutex_unlock(&counter_lock);
    }
  }
}
