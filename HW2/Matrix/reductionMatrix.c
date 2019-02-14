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

struct workerMax{
  int max;
  int maxPos[2];
};

struct workerMin{
  int min;
  int minPos[2];
};

omp_lock_t maxlock;
omp_lock_t minlock;

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

struct workerMax mymax(int new, int x, int y, struct workerMax data){
//  printf("%d\t vs \t %d\n" , new, data.max);

  if(new>data.max){
    data.max = new;
    data.maxPos[0] = x;
    data.maxPos[1] = y;
  }
  return data;
}

struct workerMin mymin(int new, int x, int y, struct workerMin data){
  //printf("%d\t vs \t %d\n" , new, data.min);

  if(new<data.min){
    data.min = new;
    data.minPos[0] = x;
    data.minPos[1] = y;
  }
  return data;
}


/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j, total=0;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;


  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
  //   printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
  //    	  printf(" %d", matrix[i][j]);
	  }
	 // 	  printf(" ]\n");
  }

  start_time = omp_get_wtime();


  #pragma omp declare reduction (customMax:struct workerMax:omp_out=mymax(omp_in.max,omp_in.maxPos[0],omp_in.maxPos[1], omp_out))\
  initializer(omp_priv = {INT_MIN, {0,0}})

  #pragma omp declare reduction (customMin:struct workerMin:omp_out=mymin(omp_in.min,omp_in.minPos[0],omp_in.minPos[1], omp_out))\
  initializer(omp_priv = {INT_MAX, {0,0}})

  struct workerMax dataMax= {INT_MIN, {0,0}};
  struct workerMin dataMin = {INT_MAX, {0,0}}; /* partial data for min and max values */

#pragma omp parallel for reduction (+:total) reduction (customMax:dataMax) reduction (customMin:dataMin) private(j)
  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++){
      dataMax = mymax(matrix[i][j],i,j,dataMax);
      dataMin = mymin(matrix[i][j],i,j,dataMin);

      total += matrix[i][j];
    }

// implicit barrier

  end_time = omp_get_wtime();


    //Performance Evaluation
    printf("%f\n", end_time - start_time);


  // NORMAL PRINT
  /*  printf("minimum %d at (%d,%d)\n", dataMin.min,dataMin.minPos[1]+1,dataMin.minPos[0]+1);
    printf("max is %d at (%d,%d)\n", dataMax.max,dataMax.maxPos[1]+1,dataMax.maxPos[0]+1);
    printf("the total is %d\n", total);
    printf("it took %g seconds\n", end_time - start_time);
    */
}
