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

void printJobLList(LList * ll)
{
     printf(":: s=%d :::::: ", llsize(ll));
     int i=0;
     while(i<llsize(ll))
     {
	  printf("[%d] ", ((Job *)llget(ll,i))->job_id);
	  i++;
     }

     printf("\n");
}

void updateJobs(LList * running, LList * finished)
{
     int i;
     Job * job;
     pid_t pid;
     Job * jobr ;            

     while(llsize(finished)> 0)
     {
	  jobr = llremove(finished, 0);
	  if(jobr!=NULL)
	       jfree(jobr);
     }

     i=0;
     int status;
     while(i< llsize(running))
     {
	  job=llget(running,i);
	  pid = waitpid(job->pid, &status, WNOHANG | WUNTRACED);

	  if(pid!=0)
	  {
	       jobr =llremove(running, i);
	       if(jobr!=NULL)
	       {
		    job->status=FINISHED; 
		    lladd(finished, job);
	       }
	       continue;
	  }
	  i++;
     }
     printJobs(running, finished);
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

	  if(llsize(running_jobs)==0 && llsize(finished_jobs)==0)
	       reset_next_job_number();

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
	  }
	  free(cmd);
     }

     while(0<llsize(running_jobs))
	  jfree(llremove(running_jobs, 0));

     while(0<llsize(finished_jobs))
	  jfree(llremove(finished_jobs, 0));

     llfree(running_jobs);
     llfree(finished_jobs);
     return 0;
}

