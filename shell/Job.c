#include <stdlib.h> /* malloc, free, rand */
#include <string.h> /* strstr strcmp*/
#include <stdio.h>  /* sprintf*/

#include "Job.h"
int _next_job_number=0;

Job * jalloc(const char * cmd, pid_t pid, job_status status)
{
    Job * ret = malloc(sizeof(Job));
    if(ret==NULL)
	return NULL;


    ret->job_id = _next_job_number++;
    ret->cmd = (char *) malloc((sizeof(char)*strlen(cmd)));
    ret->pid = pid;
    strcpy(ret->cmd,cmd);
    ret->status = status;
    return ret;
}

void jfree(Job * job)
{
    if(job==NULL)
	return;
    if(job->cmd!=NULL)
	free(job->cmd);
    free(job);
}


void printJobs(LList * running, LList * finished)
{
     int i;
     Job * job;
     printf("Running:\r\n");     
     for(i=0;i<running->length;i++)
     {
	  job = llget(running,i);
	  printf("\t[%d] %s\r\n",job->job_id,job->cmd);
     }
     printf("Finished:\r\n");
     for(i=0;i<finished->length;i++)
     {
	  job = llget(running,i);
	  printf("\t[%d] %s\r\n",job->job_id,job->cmd);
     }     
}
