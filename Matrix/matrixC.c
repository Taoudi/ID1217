/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

struct worker{
  int max;
  int maxPos[2];
  int min;
  int minPos[2];
  int total;
};

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
pthread_mutex_t lock;
pthread_mutex_t bagLock;
int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */
struct worker data;       /* holds values for max and min */
int bag = 0;

/* timer */
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

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */


void printStuff(int start_time){
  end_time = read_timer();
  printf("The total is %d\n", data.total);
  printf("The max is %d at position (%d,%d)\n", data.max, data.maxPos[1]+1,data.maxPos[0]+1);
  printf("The min is %d at position (%d,%d)\n", data.min, data.minPos[1]+1,data.minPos[0]+1);
  printf("The execution time is %g sec\n", end_time - start_time);
}

int getTaskFromBag(){
  pthread_mutex_lock(&bagLock);
  int task = bag;
  bag++;
  pthread_mutex_unlock(&bagLock);
  return task;
}

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  data.max = INT_MIN;       /* min value */
  data.min = INT_MAX;       /* max value */
  data.total = 0;           /* total value */
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&bagLock,NULL);
//  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;


  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] =rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++){
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  }
  for (l = 0; l < numWorkers; l++){
    pthread_join(workerid[l],NULL);
  }
  printStuff(start_time);
  pthread_exit(NULL);

}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int task;
  //int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  while(bag<size){
    task = getTaskFromBag();
    /* sum values in my strip */
    pthread_mutex_lock(&lock);
    for (int i = 0; i < size; i++){
        if(data.min>matrix[task][i]){
          data.min = matrix[task][i];
          data.minPos[0]=task;
          data.minPos[1]=i;
        }
        if(data.max<matrix[task][i]){
          data.max = matrix[task][i];
          data.maxPos[0]=task;
          data.maxPos[1]=i;
        }
        data.total += matrix[task][i];
      }
    pthread_mutex_unlock(&lock);
  }
    /* get end time */
    /* print results */
  }
