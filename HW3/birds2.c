/* a simple producer/consumer using semaphores and threads

   usage on Solaris:
     gcc thisfile.c -lpthread -lposix4
     a.out numIters

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define SHARED 1
#define MAXBIRDS 10
#define INITWORMS 10

void *Producer(void *);  /* the two threads */
void *Consumer(void *);

sem_t empty, full, lock;    /* the global semaphores */
int food;
int amtFood;       /* shared buffer         */
int numIters;
int numBirds;
pthread_mutex_t printLock;


/* main() -- read command line and create threads, then
             print result when the threads have quit */

int main(int argc, char *argv[]) {
  /* thread ids and attributes */
  amtFood = atoi(argv[1]);
  if(amtFood>INITWORMS){
    amtFood=INITWORMS;
  }
  food=amtFood;
  numBirds = atoi(argv[2]);
  if(numBirds>MAXBIRDS){
    numBirds=MAXBIRDS;
  }
  pthread_t bigBird;
  pthread_t babyBird[numBirds];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_mutex_init(&printLock, NULL);

  sem_init(&empty, SHARED, 0);  /* sem empty = 1 */
  sem_init(&full, SHARED, amtFood);   /* sem full = 0  */
  sem_init(&lock, SHARED, 1);   /* sem full = 0  */

  printf("main started\n");
  for(long l = 0; l<numBirds;l++){
    pthread_create(&babyBird[l],&attr,Consumer, (void *) l);
  }
  pthread_create(&bigBird, &attr, Producer, NULL);
  //sem_post(&full);
  //printf("SEND SIGNAL\n");
  for(int i = 0; i<numBirds;i++){
    pthread_join(babyBird[i], NULL);
  }
  pthread_join(bigBird, NULL);

  printf("main done\n");
}

/* deposit 1, ..., numIters into the data buffer */
void *Producer(void *arg) {

//  printf("Producer created\n");
  while(1) {
      sem_wait(&empty);
      sem_wait(&lock);
      for(food = 0;food<amtFood;food++){
        printf("Parent produces food!, %d LEFT\n",food);
        sem_post(&full);
      }
      sem_post(&lock);
  }
}

/* fetch numIters items from the buffer and sum them */
void *Consumer(void *arg) {
  long myid = (long) arg;
//  printf("Consumer created %d\n",myid);
  while(1) {
    sem_wait(&full);
    sem_wait(&lock);
    if(food>0){
      food--;
      printf("BIRD %d EATS! %d LEFT\n",myid,food);
      if(food==0){
        printf("BIRD %d CHIRPS!\n",myid, food);
        sem_post(&empty);
      }
    }
    sleep(1);
    sem_post(&lock);
  }
//  printf("for %d iterations, the total is %d\n", numIters, total);
}
