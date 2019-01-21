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
int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */
struct worker data;       /* holds values for max and min */


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
  //int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  int first = myid*stripSize;
  int last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  pthread_mutex_lock(&lock);
  for (int i = first; i <= last; i++)
    for (int j = 0; j < size; j++){
      if(data.min>matrix[i][j]){
        data.min = matrix[i][j];
        data.minPos[0]=i;
        data.minPos[1]=j;
      }
      if(data.max<matrix[i][j]){
        data.max = matrix[i][j];
        data.maxPos[0]=i;
        data.maxPos[1]=j;
      }
      data.total += matrix[i][j];
    }
  pthread_mutex_unlock(&lock);
    /* get end time */
    /* print results */
  }
