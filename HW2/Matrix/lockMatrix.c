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
  struct worker data; /* partial data for min and max values */

    data.max=INT_MIN;
    data.min=INT_MAX;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    // printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
      	  //printf(" %d", matrix[i][j]);
	  }
	  	 // printf(" ]\n");
  }
  omp_init_lock(&maxlock);
  omp_init_lock(&minlock);
  start_time = omp_get_wtime();

#pragma omp parallel for reduction (+:total) private(j)
  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++){
     //omp_set_lock(&maxlock);
    #pragma omp critical(max)
     {
      if(matrix[i][j]>data.max){
        data.max = matrix[i][j];
        data.maxPos[0] = i;
        data.maxPos[1] = j;
      }
    }
      //omp_unset_lock(&maxlock);
      //omp_set_lock(&minlock);
    #pragma omp critical(min)
    {
      if(matrix[i][j]<data.min){
        data.min = matrix[i][j];
        data.minPos[0] = i;
        data.minPos[1] = j;
      }
    }
      //omp_unset_lock(&minlock);
      total += matrix[i][j];
    }

// implicit barrier

  end_time = omp_get_wtime();

  #pragma omp master
  {
    //Performance Evaluation
    printf("%f\n", end_time - start_time);


  // NORMAL PRINT
  /*  printf("minimum %d at (%d,%d)\n", data.min,data.minPos[1]+1,data.minPos[0]+1);
    printf("max is %d at (%d,%d)\n", data.max,data.maxPos[1]+1,data.maxPos[0]+1);
    printf("the total is %d\n", total);
    printf("it took %g seconds\n", end_time - start_time);*/
}

}
