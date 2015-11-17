#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <pthread.h>
#include <semaphore.h>

#define NITER 1000000

int cnt = 0;

void * count(void * a)
{
     int i, tmp;
     for(i=0; i<NITER; i++)
     {
	  tmp = cnt;      /* copy the global cnt locally */
	  tmp = tmp+1;    /* increment the local copy */
	  cnt = tmp;      /* store the local value into the global cnt */ 
     }

     return NULL;
}


int main(int argc, char * argv[])
{
     pthread_t tid1, tid2;

     if(pthread_create(&tid1, NULL, count, NULL))
     {
	  printf("ERROR creating thread 1\b");
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
     
     if(pthread_join(tid2,NULL)) /* wait for the thread 1 to finish */
     {
	  printf("ERROR joining thread 2\n");
     }


     if(cnt < 2 * NITER)
	  printf("BOOM! cnt is [%d], should be %d\n",cnt, 2*NITER);
     else
	  printf("OK! cnt is [%d]\n",cnt);

     pthread_exit(NULL);

}

