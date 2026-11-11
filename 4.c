#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define N 5

sem_t ch[N];

void *philosopher(void *arg)
{
    int i = *(int *)arg;
    int left = i;
    int right = (i + 1) % N;

    int first = (left < right) ? left : right;
    int second = (left < right) ? right : left;

    printf("Philosopher %d is hungry\n", i);

    sem_wait(&ch[first]);
    sem_wait(&ch[second]);

    printf("Philosopher %d is eating\n", i);
    sleep(5);

    sem_post(&ch[second]);
    sem_post(&ch[first]);

    printf("Philosopher %d has finished eating\n", i);

    return NULL;
}

int main(void)
{
    int id[N];
    pthread_t pid[N];

    for (int i = 0; i < N; i++)
        sem_init(&ch[i], 0, 1);

    for (int i = 0; i < N; i++)
    {
        id[i] = i;
        pthread_create(&pid[i], NULL, philosopher, &id[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(pid[i], NULL);

    for (int i = 0; i < N; i++)
        sem_destroy(&ch[i]);

    return 0;
}