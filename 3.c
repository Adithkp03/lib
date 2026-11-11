#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
int buf[5];
pthread_t pid,cid;
pthread_mutex_t m;
sem_t empty,full;
void *producer(void *arg)
{
  (void)arg;
  int n,i=0;
  while(1){
    sem_wait(&empty);
    pthread_mutex_lock(&m);
    n=rand()%10;
    buf[i++]=n;
    printf("Number %d is added at slot %d\n",n,i);
    if(i==5)i=0;
    pthread_mutex_unlock(&m);
    sem_post(&full);
    sleep(1);
  }
}
void *consumer(void *arg){
  (void)arg;
  int key,i=0;
  while(1){
    sem_wait(&full);
    pthread_mutex_lock(&m);
    key=buf[i++];
    printf("Number %d is consumed from slot %d\n",key,i);
    if(i==5)i=0;
    pthread_mutex_unlock(&m);
    sem_post(&empty);
    sleep(2);
  }
}
int main(void){
  pthread_mutex_init(&m,NULL);
  sem_init(&empty,0,5);
  sem_init(&full,0,0);
  pthread_create(&pid,NULL,producer,NULL);
  pthread_create(&cid,NULL,consumer,NULL);
  pthread_join(pid,NULL);
  pthread_join(cid,NULL);
  return 0;
}
