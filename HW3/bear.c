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
#define MAXBEES 10
#define MAXHONEY 10

void *Producer(void *);  /* the two threads */
void *Consumer(void *);

sem_t fillPot, eat, honeypotLock;    /* the global semaphores */
int honeypot;
int honeyAmount;       /* shared buffer         */
int numIters;
int numBees;
pthread_mutex_t printLock;


/* main() -- read command line and create threads, then
             print result when the threads have quit */

int main(int argc, char *argv[]) {
  /* thread ids and attributes */
  honeyAmount = atoi(argv[1]);
  if(honeyAmount>MAXHONEY){
    honeyAmount=MAXHONEY;
  }
  numBees = atoi(argv[2]);
  if(numBees>MAXBEES){
    numBees=MAXBEES;
  }
  honeypot=0;
  pthread_t bear;
  pthread_t bees[numBees];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  sem_init(&fillPot, SHARED, 1);  /* sem fillPot = 1 */
  sem_init(&eat, SHARED, 0);   /* sem eat = 0  */
  sem_init(&honeypotLock, SHARED, 1);   /* sem eat = 0  */

  printf("main started\n");
  for(long l = 0; l<numBees;l++){
    pthread_create(&bees[l],&attr,Producer, (void *) l);
  }
  pthread_create(&bear, &attr, Consumer, NULL);
  //sem_post(&eat);
  //printf("SEND SIGNAL\n");
  for(int i = 0; i<numBees;i++){
    pthread_join(bees[i], NULL);
  }
  pthread_join(bear, NULL);

  printf("main done\n");
}

/* deposit 1, ..., numIters into the data buffer */
void *Producer(void *arg) {
  long myid = (long) arg;
//  printf("Producer created\n");
  while(1) {
    sem_wait(&fillPot);
    printf("Bee nr %d FILLS UP POT to %d !\n", myid,honeypot+1);
    honeypot++;
    if(honeypot==honeyAmount){
      printf("Bee nr %d WAKES UP BEAR!\n",myid);
      sem_post(&eat);
    }
    else
      sem_post(&fillPot);
    sleep(1);
  }
}

/* fetch numIters items from the buffer and sum them */
void *Consumer(void *arg) {
//  printf("Consumer created %d\n");
  while(1) {
    sem_wait(&eat);
    printf("BEAR EATS HONEY\n");
    honeypot=0;
    sem_post(&fillPot);
    sleep(1);
    }
  }
