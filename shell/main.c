#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>  /* strstr strcmp*/

#include <ctype.h> /*Char type*/
#include <sys/wait.h> /* waitpid*/
#include <sys/types.h> /* pid_t ??? i think*/
#include <stdarg.h> /* standard header contains a set of macro
		     * definitions that define how to step through an
		     * argument list. */
#include "util.h"
#include "List.h"
#include "LList.h"
#include "Job.h"
#define CLI "jsh$ "

void updateJobs(LList * running, LList * finished)
{
     int i;
     int status;
     Job * job;
     pid_t pid;

     
     i=0;
     while(i<finished->length)
     {
	  job=llget(finished,i);
	  if(job->status==FINISHED)
	  {
	       printf("swtiching status to DEAD [%d]\n",job->job_id);
	       job->status=DEAD;
	  }
	  else if(job->status==DEAD)
	  {
	       job = llremove(finished, i);
	       printf("removing %s [%d]\n", job->cmd, job->job_id);
	       if(job!=NULL)
	       {
		    printf("freeing job...\n");
		    jfree(job);
	       }
	       else
		    printf("uhoh\n");

	       continue;
	  }
	  i++;
     }
     i=0;
     while(i<running->length)
     {
	  job=llget(running,i);
	  pid = waitpid(job->pid, &status, WNOHANG | WUNTRACED);
	  if(pid!=0)
	  {
	       job = llremove(running, i);
	       if(job==NULL)
	       {
		    printf("unable to remove %i", i);
	       }
	       job->status=FINISHED; 
	       lladd(finished, job);
	       continue;
	  }
	  i++;
     }

}


int main(int argc, char * argv[])
{
     
     LList * running_jobs = llalloc();
     LList * finished_jobs = llalloc();
     char * cmd;
     char keepGoing = 1;

     while( keepGoing )
     {
	  printf("%s",CLI);

	  if( readl(&cmd) ) 
	  {
	       keepGoing=0;
	       continue;/* skip free(cmd) below*/
	  }
	  else if((strstr(cmd, "exit")==cmd) || strstr(cmd, "quit")==cmd )
	  {
	       keepGoing=0;
	       printf("Goodbye.\r\n");
	  }
	  else if((strstr(cmd, "jobs")==cmd))
	  {
	       updateJobs(running_jobs, finished_jobs);	       	       
	       printJobs(running_jobs, finished_jobs);
	  }
	  else
	  {
	       
	       List * alist = arglist(cmd);
	       if(alist->length>1 && (strcmp(lget(alist,alist->length-2), "&")==0) )
	       {
		    alist->arr[alist->length-1] = alist->arr[alist->length-2]; //so lfreefree can free the & str
		    alist->arr[alist->length-2] = NULL;
		    Job * job = executebg(lget(alist,0), (char * const *)alist->arr);
		    if(job!=NULL)
			 lladd(running_jobs,job);

	       }
	       else
	       {
		    executefg(lget(alist,0), (char * const *)alist->arr);
	       }
	       lfreefree(alist);
	       updateJobs(running_jobs, finished_jobs);	       
	       printJobs(running_jobs, finished_jobs);
	  }
	  free(cmd);
     }

/*     for(i=0;i<jobs->length;i++)
	  jfree(jobs->arr[i]);
*/
     llfree(running_jobs);
     llfree(finished_jobs);
     return 0;
}

