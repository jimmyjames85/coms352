#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>  /* strstr strcmp*/
#include <unistd.h> /* pid_t fork */
#include <ctype.h> /*Char type*/
#include <sys/wait.h> /* waitpid*/
#include <sys/types.h> /* pid_t ??? i think*/
#include <stdarg.h> /* standard header contains a set of macro
		     * definitions that define how to step through an
		     * argument list. */


#include "List.h"
#include "Job.h"
#define CR 0x0d
#define LF 0x0a
#define CLI "jsh$ "


/**
 * Reads in a line from a file until it reaches a CR,LF, or a CRLF.
 * Removes the CR,LF,or CRLF and replaces it with '\0'
 *
 * FILE* fstream 	file to read from (assumed to be open)
 * char **line 		address of a pointer to a char (where to put the string)
 *
 * uses malloc to allocate memory at *line
 * DOES NOT FREE MEMORY if successfull
 *
 *
 *	returns  0 if succesful
 *
 *	returns -2 if malloc or realloc could not allocate memory
 *
 * DOES NOT FREE MEMORY allocated to **line if function ends successfully
 * */
int fpreadl(FILE* fstream, char** line)
{

     /*Start with 1 byte and if we need more then we double this each
      * time*/
     unsigned totBytes = 1;
     *line = (char *) malloc(totBytes * sizeof(char));
     if ((*line) == NULL)
	  return -2;

     int endOfLine = 0;
     int pos = 0;
     while (!feof(fstream) && !endOfLine)
     {
	  if (pos > (totBytes >> 2))
	  {
	       /*Then we need to reallocate some memory.
		* So we double the amount we need.*/
	       totBytes *= 2;
	       char* tmp = (char*) realloc(*line, totBytes * sizeof(char));
	       /*if reallocation fails*/
	       if (tmp == NULL)
	       {
		    /*free what we have and exit*/
		    free(*line);
		    return -2;
	       }
	       /*otherwise lets continue*/
	       *line = tmp;
	  }

	  /*readin our next char*/
	  (*line)[pos] = fgetc(fstream);
	  /*check for endOfLine*/
	  switch ((*line)[pos])
	  {
	  case CR:
	  {
	       /*Check for WINDOWS CRLF*/
	       if (!feof(fstream))
	       {
		    (*line)[pos] = fgetc(fstream);
		    /*If it is not a lineFeed then put it back*/
		    if ((*line)[pos] != LF)
			 ungetc((*line)[pos], fstream);
	       }
	       endOfLine = 1;
	       break;
	  }
	  case LF:
	  {
	       /*if we have CR or CRLF or LF set endOfLine*/
	       endOfLine = 1;
	       break;
	  }
	  }
	  pos++;
     }

     /**  If this while loop was successful then pos points to 1 past
      *  the last character that we read in AND the last character we
      *  read in was either a CR or a LF so we need to decrement pos
      *  and change that last character to the null character
      *  \0. However if the while loop was never entered then pos=0
      *  and we set that character to \0 and return an empty
      *  string.*/

     if (endOfLine || feof(fstream))
	  pos--;
     (*line)[pos] = '\0';

     /*success!*/
     return 0;
}

/**
 *  Uses fpreadl with stdin as the input file stream pointer.
 *
 *  See fpreadl for semantics.
 */
int readl(char** line)
{
     return fpreadl(stdin, line);
}

/**
 *  Executes the file and waits for the process to finish.
 *
 *  From the EXEC(3) man page: The first argument, by convention,
 *	should point to the filename associated with the file being
 *	executed.
 *
 *  Returns the pid_t of the child process or -1 on failure. 
 */
pid_t execute(const char *file, char *const  args[])
{
     pid_t p = fork();
     if(p==0) /* child */
     {
	  int err = execvp(file, args);
	  printf("Error(%d) occured executing: %s\n", err, file);
	  exit(0); /* Terminate this child process on error */
     }
     return p;
}

void executefg(const char *file, char *const  args[])
{
     pid_t p = execute(file, args);
     if(p==-1)
     {
	  printf("Unable to fork \r\n");
	  return;
     }

     signed int status;
     waitpid(p, &status, 0);
     return;
}

Job * executebg(const char *file, char *const args[])
{
     Job * ret =NULL;
     pid_t p = execute(file, args);
     if(p==-1)
     {
	  printf("Unable to execute: %s\r\n", file);
	  return NULL;
     }
     else
     {
	  ret=jalloc(file, p);
     }
     return ret;
}

void printJobs(List * jobList)
{
     int i;
     Job * job;
     for(i=0;i<jobList->length;i++)
     {
	  job = lget(jobList,i);
	  int status;
	  waitpid(job->pid, &status, WNOHANG | WUNTRACED);
	  //int stat = 314;
/*	  if(status) 
	  stat = WIFEXITED(status); */

	  printf("[%d] pid=(%d) %s %d %d\r\n",job->job_id,job->pid,job->cmd, status, 0);
     }
}

void printArgList(List * argList)     
{
     int i;
     for(i=0;i<argList->length;i++)
     {
	  char * cmd = lget(argList,i);
	  if(cmd!=NULL)
	       printf("'%s' ",cmd);
     }
     printf("\n");
}


int main(int argc, char * argv[])
{
     List * jobs = lalloc();
     char * cmd;
     char keepGoing = 1;
     int i;
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
	       printJobs(jobs);
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
			 ladd(jobs,job);
	       }
	       else
	       {
		    executefg(lget(alist,0), (char * const *)alist->arr);
	       }

	       
	       lfreefree(alist);
	  }
	  free(cmd);
     }

     for(i=0;i<jobs->length;i++)
	  jfree(jobs->arr[i]);
     lfree(jobs);
     return 0;
}

