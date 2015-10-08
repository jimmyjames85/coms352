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
#include <fcntl.h> /* open */

#include "util.h"
#include "List.h"
#include "LList.h"
#include "Job.h"
#define CLI "jsh$ "
#define PRINT_JOBS_EVERY_TIME 0

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
     size_t str_size = sizeof(char) * 2;
     char * cwd = malloc(str_size); 
     char * ret = getcwd(cwd, str_size);
     while((ret = getcwd(cwd, str_size))==NULL)
     {
	  str_size *=2;
	  char * tmp = (char *)realloc(cwd, str_size);
	  if(tmp==NULL)
	  {
	       printf("ERROR with realloc\n");//TODO
	       free(cwd);
	       return;
	  }
	  cwd = tmp;
     }
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

void assignRedirectionFiles(List * alist, char ** in_file, char ** out_file)
{
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
		    free(*in_file); /* in case we've done this before */
		    *in_file =(char *)lremove(alist, cur);
	       }
	  }
	  else if(strstr(curarg,">")==curarg)
	  {
	       cur--;
	       free(lremove(alist,cur)); /* free and remove '>' */
	       if(cur < alist->length)
	       {
		    free(*out_file); /* in case we've done this before */			      
		    *out_file = (char *) lremove(alist, cur);
	       }
	  }
     }     
}

/**
 * Replaces all pipe symbols | in the alist with NULL pointers. This
 * does free the cstrings that contain the pipes 
 *
 * returns the number of pipes replaced
 */
int breakUpPipes(List * alist)
{
     int pipeCount = 0;
     int i;
     for(i=0;i<alist->length;i++)
     {
	  char * cmd = lget(alist, i);
	  if(cmd!=NULL && strstr(cmd, "|")==cmd)
	  {
	       char * c = lreplace(alist, i, NULL);
	       free(c);
	       pipeCount++;
	  }
     }
     return pipeCount;
}

void executeCmd(char * cmd, LList * running_jobs, LList * finished_jobs)
{
     int infd = STDIN_FILENO;
     int outfd = STDOUT_FILENO;
     char * in_file=NULL;
     char * out_file=NULL;
     List * alist = arglist(cmd);
     
     assignRedirectionFiles(alist, &in_file, &out_file);
     
     if(in_file!=NULL)
	  infd = open(in_file, O_RDONLY);

     if(out_file!=NULL)
	  outfd = open(out_file,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );


     if(infd ==-1)
     {
	  printf("%s: Input file error\n", (char *)lget(alist,0));	  
     }
     else if( outfd == -1)
     {
	  printf("%s: Output file error\n", (char *)lget(alist,0));	  
     }
     if(alist->length>1 && (strcmp(lget(alist,alist->length-2), "&")==0) )
     {
	  lremove(alist, alist->length -2);
	  //alist->arr[alist->length-1] = alist->arr[alist->length-2]; //so lfreefree can free the & str
	  //alist->arr[alist->length-2] = NULL;
 
	  Job * job = executebg(lget(alist,0), (char * const *)alist->arr, infd, outfd);
	  if(job!=NULL)
	       lladd(running_jobs,job);
     }
     else
     {
	  /*
	    TODO link pipes together
	    int numPipes = breakUpPipes(alist);
	    int i;

	    List * pipes = lalloc();
	    for(i=0;i<numPipes;i++)
	    {
	    int * pipe = malloc(sizeof(int)*2);
	    pipe(pipe);
	    ladd(pipes, pipe);
	    }
	    close(((int *)lget(pipes, 0))[0]);
	    close(((int *)lget(pipes, pipes->length-1))[0]);

	  */	  
	  
	  int status;
	  executefg(lget(alist,0), (char * const *)alist->arr, &status, infd, outfd);
     }
     lfreefree(alist);
     free(in_file);
     free(out_file);
     updateJobs(running_jobs, finished_jobs, PRINT_JOBS_EVERY_TIME); 
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
	       executeCmd(cmd, running_jobs, finished_jobs);
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

