#include <pthread.h>
#include <stdlib.h>
#include <cstdio>
#include <unistd.h>
#define MAXBIRDS 20
#define MAXWORMS 100
class BirdMonitor
{
private:
  int worms;
  int maxworms;
  pthread_mutex_t nestLock;
  pthread_cond_t babyCond;
  pthread_cond_t parentCond;
  pthread_cond_t eat;
  int go;
  int prod;
public:
  void init(int max){
    pthread_mutex_init(&nestLock, NULL);
    pthread_cond_init(&babyCond, NULL);
    pthread_cond_init(&parentCond, NULL);
    pthread_cond_init(&eat, NULL);
    maxworms = max;
    worms = max;
    go = 1;
    prod=0;
  }

  void produce(){
    //pthread_mutex_lock(&nestLock);
    while(prod==0){
      pthread_cond_wait(&parentCond,&nestLock);
    }
    worms = maxworms;
    prod=0;
    printf("Parent fills nest \n");
    pthread_cond_signal(&eat);
    pthread_mutex_unlock(&nestLock);
    sleep(1);
  }


  void consume(long id){
    pthread_mutex_lock(&nestLock);
    while(go==0){
      pthread_cond_wait(&babyCond,&nestLock);
    }
    go=0;

    if(worms==0){
      prod=1;
      pthread_cond_signal(&parentCond);
      printf("Bird %d CHIRPS!\n",id);
      pthread_cond_wait(&eat,&nestLock);
    }

    worms--;
    printf("Bird %d eats, %d left!\n",id,worms);
    go=1;
    pthread_cond_signal(&babyCond);
    pthread_mutex_unlock(&nestLock);
    sleep(1);
  }
};



void *Producer(void *);  /* the two threads */
void *Consumer(void *);
BirdMonitor monitor;
int wormAmount;



int main(int argc, char *argv[]){
  wormAmount = atoi(argv[1]);
  if(wormAmount>MAXWORMS){
    wormAmount=MAXWORMS;
  }
  int threads = atoi(argv[2]);
  if(threads>MAXBIRDS){
    threads=MAXBIRDS;
  }
  monitor.init(wormAmount);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_t parent;
  pthread_t children[threads];
  printf("main started\n");
  printf("parent  created\n");
  pthread_create(&parent, &attr, Producer, NULL);
  for(long l = 0; l<threads;l++){
    printf("baby nr %d created\n",l);
    pthread_create(&children[l],&attr,Consumer, (void *) l);
  }

  //sem_post(&eat);
  //printf("SEND SIGNAL\n");
  for(int i = 0; i<threads;i++){
    pthread_join(children[i], NULL);
  }
  pthread_join(parent, NULL);
  return 0;
}

void *Producer(void *arg) {
  while(1) {
    monitor.produce();
  }
}

/* fetch numIters items from the buffer and sum them */
void *Consumer(void *arg) {
  long myid = (long) arg;
  while(1) {
    monitor.consume(myid);
    }
  }
