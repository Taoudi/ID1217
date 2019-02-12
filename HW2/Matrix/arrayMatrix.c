/* matrix summation using OpenMP

gcc-8 -Wall -fopenmp matrixA.c


   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <limits.h>
#include <stdio.h>
double start_time, end_time;

#include <stdio.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

struct worker{
  int max;
  int maxPos[2];
  int min;
  int minPos[2];
};

omp_lock_t maxlock;
omp_lock_t minlock;

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j, total=0;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  struct worker data[numWorkers]; /* partial data for min and max values */
  int sums[numWorkers]; /* partial sums */

  for(int k=0;k<numWorkers;k++){ /* Init all min and max values */
    data[k].max=INT_MIN;
    data[k].min=INT_MAX;
  }

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
     //printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
      //	  printf(" %d", matrix[i][j]);
	  }
	  //	  printf(" ]\n");
  }
  int myid;
  start_time = omp_get_wtime();
#pragma omp parallel for reduction (+:total) private(j,myid)
  for (i = 0; i < size; i++){
    myid = omp_get_thread_num();
    for (j = 0; j < size; j++){
     if(matrix[i][j]>data[myid].max){
       data[myid].max = matrix[i][j];
       data[myid].maxPos[0] = i;
       data[myid].maxPos[1] = j;
     }
     if(matrix[i][j]<data[myid].min){
       data[myid].min = matrix[i][j];
       data[myid].minPos[0] = i;
       data[myid].minPos[1] = j;
     }
      total += matrix[i][j];
    }
  }
// implicit barrier

  end_time = omp_get_wtime();
  struct worker final;

    final = data[0];
    for (i = 0; i < numWorkers; i++){
      if(final.max < data[i].max){
        final.max = data[i].max;
        final.maxPos[0] = data[i].maxPos[0];
        final.maxPos[1] = data[i].maxPos[1];
      }
      if(final.min > data[i].min){
        final.min = data[i].min;
        final.minPos[0] = data[i].minPos[0];
        final.minPos[1] = data[i].minPos[1];
      }
    }

    //Performance Evaluation
    printf("%f\n", end_time - start_time);
//NORMAL OUTPUT
    //printf("The total is %d\n", total);
    //printf("The max is %d at position (%d,%d)\n", final.max, final.maxPos[1]+1,final.maxPos[0]+1);
    //printf("The min is %d at position (%d,%d)\n", final.min, final.minPos[1]+1,final.minPos[0]+1);
    //printf("The execution time is %g sec\n", end_time - start_time);

}
