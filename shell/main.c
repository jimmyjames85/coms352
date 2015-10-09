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
#define CLI "wsh$ "
#define PRINT_JOBS_EVERY_TIME 1

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

Job * getJob(LList * running_jobs, int jobId)
{
     llnode * cur = running_jobs->head;
     while(cur!=NULL && ((Job *)cur->data)->job_id != jobId )
	  cur=cur->next;

     if(cur==NULL)
	  return NULL;

     return (Job *)cur->data;
}

pid_t getJobPid(LList * running_jobs, int jobId)
{
/*     llnode * cur = running_jobs->head;
     while(cur!=NULL && ((Job *)cur->data)->job_id != jobId )
	  cur=cur->next;

     if(cur==NULL)
	  return -1;
	  return ((Job *)cur->data)->pid;*/
     Job * job = getJob(running_jobs, jobId);
     if(job==NULL)
	  return -1;
     return job->pid;
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
	       pid_t pid= getJobPid(running_jobs, waitJobId);
	       
	       if(pid==-1)
		    printf("Job [%d] does not exist.\n",waitJobId);
	       else
	       {
		    int status;
		    waiton(pid, &status);
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
	  printf("Working directory ");
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

int * createListOfOpenFds(List * pipes)
{
     int * ret = malloc(sizeof(int)*2*pipes->length);
     int i=0;
     for(i=0;i<pipes->length;i++)
     {
	  int * fd = (int *)lget(pipes, i);
	  ret[2*i]=fd[0];
	  ret[2*i+1]=fd[1];
     }
     return ret;
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

     int numPipes = breakUpPipes(alist);
     if(infd ==-1)
     {
	  printf("%s: Input file error\n", (char *)lget(alist,0)); 
     }
     else if( outfd == -1)
     {
	  printf("%s: Output file error\n", (char *)lget(alist,0));	  
     }
     else if(alist->length>1 && (strcmp(lget(alist,alist->length-2), "&")==0) )
     {
	  lremove(alist, alist->length -2);
	  Job * job = executebg(lget(alist,0), (char * const *)alist->arr, infd, outfd, NULL,0);
	  if(job!=NULL)
	       lladd(running_jobs,job);
     }
     else if(numPipes==0)
     {
	  int status;
	  executefg(lget(alist,0), (char * const *)alist->arr, &status, infd, outfd, NULL,0);
     }
     else if((infd!=STDIN_FILENO || outfd!=STDOUT_FILENO))
     {
	  printf("Aborting...Mixed pipes and redirection is not implemented\n");
     }
     else
     { 	  
	  List * pipes = lalloc();
	  int i;
	  for(i=0;i<numPipes;i++)
	  {
	       int * new_pipe = malloc(sizeof(int)*2);
	       pipe(new_pipe);
	       ladd(pipes, new_pipe);
	  }
	  //ladd(pipes, NULL);

	  List * pipedJobIds = lalloc();
	  int alist_start=0;
	  for(i=0; i<=numPipes; i++)
	  {
	       int pipe_in = STDIN_FILENO;
	       int pipe_out = STDOUT_FILENO;
	       
	       if(i>0)
		    pipe_in =((int *)lget(pipes,i-1))[0];
	       if(i<numPipes)
		    pipe_out = ((int *)lget(pipes,i))[1];

	       char * file = lget(alist, alist_start);
	       char * const * argList = (char * const *)(alist->arr + alist_start);
	       
	       int * openFDS = createListOfOpenFds(pipes);      
	       Job * job = executebg(file , argList, pipe_in, pipe_out, openFDS, 2*pipes->length);
	       free(openFDS);

	       if(job!=NULL)
	       {
		    lladd(running_jobs,job);
		    int * jobId = malloc(sizeof(int));
		    *jobId = job->job_id;
		    ladd(pipedJobIds, jobId);
	       }
	       else
		    printf("UH oh bad error\n");
	       
	       //advance to next program to execute
	       while(alist_start < alist->length && lget(alist,alist_start)!=NULL)
		    alist_start++;
	       alist_start++;
	  }

	  while(pipes->length>0)
	  {
	       int *p = (int *)lget(pipes,0);
	       if(p!=NULL)
	       {
		    close(p[0]);
		    close(p[1]);
	       }
	       free(lremove(pipes, 0));
	  }
	  lfreefree(pipes);


	  //wait until all jobs are finished
	  while(pipedJobIds->length>0)
	  {
	       int status;
	       int jobId = *((int *)lget(pipedJobIds, 0));
	       Job * job = getJob(running_jobs, jobId);
	       
	       if(job!=NULL)
	       {
		    //updateJobs(running_jobs, finished_jobs, 0);
		    waiton(job->pid,&status); 
	       }
	       free(lremove(pipedJobIds, 0));
	  }
	  lfreefree(pipedJobIds);

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
	  printf("%s",CLI);

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

     printf("Waiting for background jobs to finish...\n");
     while(0<llsize(running_jobs))
     {
	  Job * job = llremove(running_jobs, 0);
	  int status;
	  waiton(job->pid, &status);
	  printf("%s has finished. \n", job->cmd);
	  jfree(job);
     }


     while(0<llsize(finished_jobs))
	  jfree(llremove(finished_jobs, 0));

     llfree(running_jobs);
     llfree(finished_jobs);
     printf("Goodbye.\r\n");

     return 0;
}

