#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char * argv[])
{
     
    if(argc<2)
	return 0;
    long time = atol(argv[1]);
    pid_t myPid = getpid();
    while(time>0)
    {
	 printf("%d: %d\n",myPid, time);
	 sleep(1);
	 time--;
    }
    printf("%d: %d ...finished\n",myPid, time);
    return 0;
}
