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
  pthread_cond_t notEmpty;
  pthread_cond_t isEmpty;
  pthread_cond_t eat;
  int go;
public:
  void init(int max){
    pthread_mutex_init(&nestLock, NULL);
    pthread_cond_init(&notEmpty, NULL);
    pthread_cond_init(&isEmpty, NULL);
    pthread_cond_init(&eat, NULL);
    maxworms = max;
    worms = max;
  }
  void start(){
    pthread_cond_signal(&notEmpty);
    go = 1;
  }
  void produce(){
    pthread_cond_wait(&isEmpty,&nestLock);
    worms = maxworms;
    printf("Parent fills nest \n");
    pthread_cond_signal(&eat);
    sleep(1);
  }
  void consume(long id){
  pthread_mutex_lock(&nestLock);
  while(go==0){
    pthread_cond_wait(&notEmpty,&nestLock);
  }
  go=0;
  if(worms==0){
    pthread_cond_signal(&isEmpty);
    printf("Bird %d CHIRPS!\n",id);
    pthread_cond_wait(&eat,&nestLock);
  }
  worms--;
  printf("Bird %d eats, %d left!\n",id,worms);
  go=1;
  pthread_cond_signal(&notEmpty);
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
  for(long l = 0; l<threads;l++){
    printf("baby nr %d created\n",l);
    pthread_create(&children[l],&attr,Consumer, (void *) l);
  }
  printf("parent  created\n");
  pthread_create(&parent, &attr, Producer, NULL);
  //sem_post(&eat);
  //printf("SEND SIGNAL\n");
  for(int i = 0; i<threads;i++){
    pthread_join(children[i], NULL);
  }
  pthread_join(parent, NULL);
  return 0;
}

void *Producer(void *arg) {
  monitor.start();
  while(1) {
    monitor.produce();
  }
}

/* fetch numIters items from the buffer and sum them */
void *Consumer(void *arg) {
  long myid = (long) arg;

//  printf("Consumer created %d\n");
  while(1) {
    monitor.consume(myid);
    }
  }
