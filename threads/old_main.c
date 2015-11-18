//
// Created by jtappe on 11/16/2015.
//

#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <pthread.h>
#include <semaphore.h>
#include "list.h"
#define NITER 1000000
int cnt = 0;
sem_t mutex;

void * count(void * a)
{
    int i, tmp;
    sem_wait(&mutex);
    for(i=0; i<NITER; i++)
    {

        tmp = cnt;      /* copy the global cnt locally */
        tmp = tmp+1;    /* increment the local copy */
        cnt = tmp;      /* store the local value into the global cnt */

    }
    sem_post(&mutex);
    return NULL;
}


int main(int argc, char * argv[])
{

    int i=0;
    for(i=0;i<argc;i++)
        printf("%s\r\n",argv[i]);
    printf("-----------------------------\r\n");


    pthread_t tid1, tid2;
    sem_init(&mutex, 0, 1);

    if(pthread_create(&tid1, NULL, count, NULL))
    {
        printf("ERROR creating thread 1\n");
        exit(1);
    }

    if(pthread_create(&tid2, NULL, count, NULL))
    {
        printf("ERROR creating thread 2\n");
        exit(1);
    }

    if(pthread_join(tid1,NULL)) /* wait for the thread 1 to finish */
    {
        printf("ERROR joining thread 1\n");
    }

    if(pthread_join(tid2,NULL)) /* wait for the thread 2 to finish */
    {
        printf("ERROR joining thread 2\n");
    }


    if(cnt < 2 * NITER)
        printf("BOOM! cnt is [%d], should be %d\n",cnt, 2*NITER);
    else
        printf("OK! cnt is [%d]\n",cnt);

    pthread_exit(NULL);
}