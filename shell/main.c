#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>  /* strstr strcmp*/

#include <ctype.h> /*Char type*/
#include <sys/wait.h> /* waitpid*/
#include <sys/types.h> /* pid_t ??? i think*/
#include <stdarg.h> /* standard header contains a set of macro
		     * definitions that define how to step through an
		     * argument list. */
#include <unistd.h>
#include "util.h"
#include "List.h"
#include "LList.h"
#include "Job.h"
#define CLI "jsh$ "
#define PRINT_JOBS_EVERY_TIME 0

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

void updateJobs(LList * running, LList * finished, char print_jobs)
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
     
     if(print_jobs)
	  printJobs(running, finished);
}

void handleWaitCmd(char * cmd, LList * running_jobs)
{

     char * numStr = (cmd + 5);
     while(*numStr == ' ')
	  numStr++;

     if(isdigit(*numStr))
     {
	  int waitJobId = atoi((cmd+5));
	  if(llsize(running_jobs))
	  {
	       llnode * cur = running_jobs->head;
	       while(cur!=NULL && ((Job *)cur->data)->job_id != waitJobId )
		    cur=cur->next;

	       if(cur==NULL)
		    printf("Job [%d] does not exist.\n",waitJobId);
	       else
	       {
		    int status;
		    waiton(((Job *)cur->data)->pid,&status);
	       }
	  }
	  else
	       printf("No jobs to wait on.\n");
     }
     else
     {
	  printf("wait n - where n is a number\n");
     }
}

/**
 *
 * Does Not Print new line
 */
void pwd()
{
     char * cwd = get_current_dir_name(); 
     printf("%s",cwd);
     free(cwd);     
}

void handleChdirCmd(char * cmd)
{
     char * path = (cmd + 3);
     while(*path == ' ')
	  path++;

     int errNo = chdir(path);
     if(errNo)
	  printf("Unable to change directory to: %s\n",path);
     else
     {
	  pwd();
	  printf("\n");
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
	  printf("\n");
	  pwd();
	  printf("\n%s",CLI);

	  if(llsize(running_jobs)==0 && llsize(finished_jobs)==0)
	       reset_next_job_number();


	  if(readl(&cmd))
	  {
	       keepGoing=0;
	       continue;/* skip free(cmd) below*/
	  }
	  else
	  {
	       while(*cmd == ' ') //advance to first non-space input
		    cmd++;
	  }
	  
	  if((strstr(cmd, "exit")==cmd) || strstr(cmd, "quit")==cmd )
	  {
	       keepGoing=0;
	       printf("Goodbye.\r\n");
	  }
	  else if((strstr(cmd, "jobs")==cmd))
	  {
	       updateJobs(running_jobs, finished_jobs, 1); 
	  }
	  else if((strstr(cmd, "wait ")==cmd))
	  {
	       handleWaitCmd(cmd, running_jobs);
	  }
	  else if((strstr(cmd, "cd ")==cmd))
	  {
	       handleChdirCmd(cmd);
	  }
	  else
	  {
	       char * in_file=NULL;
	       char * out_file=NULL;
	       List * alist = arglist(cmd);

	       
	       int cur = 0;
	       while(cur < alist->length)
	       {
		    char * curarg = (char *)lget(alist, cur++);
		    if(curarg==NULL)
			 continue;
		    else if(strstr(curarg,"<")==curarg)
		    {
			 cur--;
			 free( lremove(alist,cur)); /* free and remove '<' */
			 if(cur < alist->length)
			 {
			      free(in_file); /* in case we've done this before */
			      in_file =(char *)lremove(alist, cur);
			 }
		    }
		    else if(strstr(curarg,">")==curarg)
		    {
			 cur--;
			 free(lremove(alist,cur)); /* free and remove '>' */
			 if(cur < alist->length)
			 {
			      free(out_file); /* in case we've done this before */			      
			      out_file = (char *) lremove(alist, cur);
			 }
		    }
	       }

	       if(alist->length>1 && (strcmp(lget(alist,alist->length-2), "&")==0) )
	       {
		    alist->arr[alist->length-1] = alist->arr[alist->length-2]; //so lfreefree can free the & str
		    alist->arr[alist->length-2] = NULL;

		    
		    
		    Job * job = executebg(lget(alist,0), (char * const *)alist->arr, in_file, out_file);
		    if(job!=NULL)
			 lladd(running_jobs,job);
	       }
	       else
	       {
		    int status;
		    executefg(lget(alist,0), (char * const *)alist->arr, &status, in_file, out_file);
	       }
	       lfreefree(alist);
	       free(in_file);
	       free(out_file);
	       updateJobs(running_jobs, finished_jobs, PRINT_JOBS_EVERY_TIME); 
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

