#include <pthread.h>
#include <stdlib.h>
#include <cstdio>
#include <unistd.h>
#define MAXBEES 20
#define MAXHONEY 100
class BearMonitor
{
private:
  int honeypot;
  int maxpot;
  pthread_mutex_t honeypotLock;
  pthread_cond_t notFull;
  pthread_cond_t isFull;
public:
  void init(int max){
    pthread_mutex_init(&honeypotLock, NULL);
    pthread_cond_init(&notFull, NULL);
    pthread_cond_init(&isFull, NULL);
    maxpot = max;
    honeypot = 0;
  }
  void start(){
    pthread_cond_signal(&notFull);
  }
  void produce(long id){
    pthread_cond_wait(&notFull,&honeypotLock);
    honeypot++;
    printf("Bee nr %d FILLS UP POT to %d !\n",id,honeypot);
    if(honeypot==maxpot){
      printf("Bee %d WAKES UP BEAR!\n",id);
      pthread_cond_signal(&isFull);
    }
    else
      pthread_cond_signal(&notFull);
    sleep(1);
  }
  void consume(){
    pthread_cond_wait(&isFull,&honeypotLock);
    printf("BEAR EATS HONEY\n");
    honeypot=0;
    pthread_cond_signal(&notFull);
    sleep(1);
  }
};



void *Producer(void *);  /* the two threads */
void *Consumer(void *);
BearMonitor monitor;
int honeyAmount;



int main(int argc, char *argv[]){
  honeyAmount = atoi(argv[1]);
  if(honeyAmount>MAXHONEY){
    honeyAmount=MAXHONEY;
  }
  int numBees = atoi(argv[2]);
  if(numBees>MAXBEES){
    numBees=MAXBEES;
  }
  monitor.init(honeyAmount);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_t bear;
  pthread_t bees[numBees];
  printf("main started\n");
  for(long l = 0; l<numBees;l++){
    printf("Bee nr %d created\n",l);
    pthread_create(&bees[l],&attr,Producer, (void *) l);
  }
  printf("Bear  created\n");
  pthread_create(&bear, &attr, Consumer, NULL);
  //sem_post(&eat);
  //printf("SEND SIGNAL\n");
  for(int i = 0; i<numBees;i++){
    pthread_join(bees[i], NULL);
  }
  pthread_join(bear, NULL);
  return 0;
}

void *Producer(void *arg) {
  long myid = (long) arg;
//  printf("Producer created\n");
  while(1) {
    monitor.produce(myid);
  }
}

/* fetch numIters items from the buffer and sum them */
void *Consumer(void *arg) {
  monitor.start();
//  printf("Consumer created %d\n");
  while(1) {
    monitor.consume();
    }
  }
