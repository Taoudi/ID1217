#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <omp.h>

using namespace std;

#define WORDCOUNT 25143
#define MAXWORKERS 10   /* maximum number of workers */
#define TRUE 1
#define FALSE 0
#define LOOPS 10

struct word_struct{
  int palindrome;
  int special;
  string word;
  string sanitized;
};

int numWorkers;
int listSize;
int counter[MAXWORKERS];


ofstream writeFile;
struct word_struct words[WORDCOUNT];


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

int symbol(string currentWord){
  for(int i = 0; i<currentWord.length();i++){
    if(currentWord.at(i) == 39){
      return TRUE;
    }
  }
  return FALSE;
}

string sanitize (string currentWord){
  string new_string = "";
  for(int i = 0; i<currentWord.length();i++){
    if(currentWord.at(i) != 39){
      new_string += currentWord.at(i);
    }
  }
  return new_string;
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



int binarySearch(string word, struct word_struct words[]){
  string copy = word;
  std::reverse(copy.begin(),copy.end());
  int boolean = FALSE;
  double low = 0;
  double high = WORDCOUNT-1;
  string currentWord;
  int index;


  while(low <= high){
    index = floor((low+high)/2);
    string non_sanitized = words[index].word;
    if(words[index].special==TRUE){
      currentWord = words[index].sanitized;
    }
    else{
      currentWord = words[index].word;
    }


    int compared = strcasecmp(copy.c_str(), currentWord.c_str());
    if(compared < 0){
        high = index-1;
    }
    else if(compared > 0){
      low = index+1;
    }
    else if(compared == 0 && 0==strcasecmp(non_sanitized.c_str(), currentWord.c_str())){
        boolean = TRUE;
        break;
      }
      else low++;
  }
  return boolean;
}

int main(int argc, char *argv[]){

  int total = 0;
  int i;

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
      inFile >> words[i].word;
      if(symbol(words[i].word)==TRUE){
        words[i].special = TRUE;
        words[i].sanitized = sanitize(words[i].word);
      }
      else{
        words[i].special = FALSE;
      }
    }
  inFile.close();

  omp_set_num_threads(numWorkers);
  double start = omp_get_wtime();
  #pragma omp parallel for //reduction (+:total)
  for(i = 0; i<WORDCOUNT;i++){
    if(palindromic(words[i].word)==TRUE){
    //  printf("ONE WORD: %s, %d\n ",words[i].c_str(),myid);
      //total++;
      words[i].palindrome = TRUE;
    }
    else if(binarySearch(words[i].word,words) == TRUE){
    //  printf("TWO WORD: %s, %d\n",words[i].c_str(),myid);
    //  total++;
      words[i].palindrome = TRUE;
    }
  }
  double end = omp_get_wtime();
  for(int i =0; i<numWorkers;i++){
    //total = total + counter[i];
    //printf("Worker %d is done with %d counted\n", i,counter[i]);

  }

  total = 0;
  for(int i = 0; i<WORDCOUNT-1;i++){
    if(words[i].palindrome){
     printf("%s\n",words[i].word.c_str());
      total++;
      writeFile << words[i].word+"\n";
    }
  }
  printf("TOTAL: %d\n", total);
  printf("Execution time: %f sec\n", end-start);
  printf("DONE\n");

}
