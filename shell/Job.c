#include "Job.h"
#include <stdlib.h> /* malloc, free, rand */
#include <string.h> /* strstr strcmp*/
#include <stdio.h>  /* sprintf*/

int _next_job_number=0;

Job * jalloc(char * cmd, pid_t pid)
{
     Job * ret = malloc(sizeof(Job));
     if(ret==NULL)
	  return NULL;
     
     ret->job_id = _next_job_number++;
     ret->cmd = (char *) malloc((sizeof(char)*strlen(cmd)));
     ret->pid = pid;
     strcpy(ret->cmd,cmd);
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

/**
 * Creates and returns a list of arguments parsed from the c-string
 * cmd. A NULL pointer is appended to the list as well.
 * 
 */
List * arglist(char * cmd)
{
     List * alist = lalloc();
     char *c = cmd; /* the current character we are processing */   
     char *ab; /*argument begining*/
     char * curcmd;
     while(*c != '\0')
     {
	  while(*c==' ') /* eat up spaces */
	       c++;

	  ab = c;
	  if(*c=='"')
	  {
	       c++;/* start after the first quote */
	       ab = c; 
	       while(*c!='"' && *c!='\0')
		    c++;
	       /*c points to end quote of quoted argument */
	  }
	  else
	  {
	       while((*c)!=' ' && (*c)!='\0')
		    c++;
	       /*c points to end of unquoted argument */
	  }
	  int strsize= (c - ab + 1);
	  
	  if(strsize>1)
	  {
	       curcmd = malloc(sizeof(char *)*strsize);
	       sprintf(curcmd,ab,strsize);
	       *(curcmd+strsize-1)='\0'; /* terminate the string */
	       ladd(alist,curcmd);
	  }

	  /* c should point to either a space or a quote and we want
	   * to advance it past that unless the the cmd is ill
	   * formated and abruptly ended */
	  if((*c) != '\0')
	       c++;
     }
     ladd(alist, NULL);
     return alist;
}

