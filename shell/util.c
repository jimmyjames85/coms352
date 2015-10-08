#include "util.h"
#include <stdlib.h>     /* malloc, free, rand */
#include <unistd.h> /* pid_t fork */
#include <sys/wait.h> /* waitpid*/
#include <string.h>
#include <fcntl.h> /* open */
#define CR 0x0d
#define LF 0x0a

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



pid_t execute(const char *file, char *const  args[], char *const  in_file, char *const out_file)
{
     pid_t p = fork();
     if(p==0) /* child */
     {
	  if(in_file!=NULL)
	  {
	       int infd = open(in_file, O_RDONLY);
	       if(infd!=-1)
		    dup2(infd,0);
	       else
	       {
		    printf("%s: Input file error\n", file);
		    exit(-1);
	       }
	  }

	  if(out_file!=NULL)
	  {
	       int outfd = open(out_file,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

	       if(outfd !=-1)
		    dup2(outfd,1);
	  }

	  int err = execvp(file, args);
	  printf("Unable to execute: %s\n", file);
	  exit(err); /* Terminate this child process on error */
     }
     return p;
}

/**
 *  Executes the file and waits for the process to finish.
 *
 *  From the EXEC(3) man page: The first argument, by convention,
 *	should point to the filename associated with the file being
 *	executed.
 *
 *  Returns the pid of the child process on exit or -1 on failure.
 */
pid_t executefg(const char *file, char *const  args[], int * status, char *const  in_file, char *const out_file)
{
     pid_t p = execute(file, args, in_file, out_file);
     if(p==-1)
	  return -1;

     return waiton(p, status);
}


pid_t waiton(pid_t p, int * status)
{
     return waitpid(p, status, 0);
}


Job * executebg(const char *file, char *const args[], char *const  in_file, char *const out_file)
{
     char * createStringFromArgList(char *const args[]);
     Job * ret =NULL;
     pid_t p = execute(file, args, in_file, out_file);
     if(p==-1)
     {
	  printf("Unable to execute: %s\r\n", file);
	  return NULL;
     }
     else
     {
	  char * argString = createStringFromArgList(args);
	  ret=jalloc(argString, p, RUNNING);
	  free(argString);
     }
     return ret;
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

char * createStringFromArgList(char *const args[])
{
     int len = 0;
     int capacity = 10;  /*10 is arbitrary*/
     char * str = malloc(sizeof(char)*capacity);
     char * str_end=str;

     int i;
     for(i=0;args[i]!=NULL;i++)
     {
	  char * cmd = args[i];
	  if(cmd!=NULL)
	  {
	       int cmdLen = strlen(cmd)+1; /* include \0 */
	       if(cmdLen+len>=capacity)
	       {
		    capacity = (len + cmdLen)*2;
		    char * tmp =  realloc(str, sizeof(char)*capacity);
		    if(tmp == NULL)
			 printf ("Unable to realloc!");
		    else
			 str =  tmp;
	       }
	       sprintf(str_end, " %s ", cmd);
	       printf("\n%s",str_end);
	       len=strlen(str);
	       str_end+=cmdLen+1; /* 1 for space */
	  }
     }
     str_end='\0';

     return str;
}
