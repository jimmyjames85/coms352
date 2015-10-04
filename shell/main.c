#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>  /* strstr strcmp*/
#include <unistd.h> /* pid_t fork */
#include <ctype.h> /*Char type*/

#include <stdarg.h> /* standard header contains a set of macro
		     * definitions that define how to step through an
		     * argument list. */
#define CR 0x0d
#define LF 0x0a
#define CLI "jsh$ "
#define MAX_JOB 20

/**
 * Argument node used to make a linked list of arguments for a command
 */
typedef struct anode
{
     char * arg;
     struct anode *next;
} anode;

typedef struct job
{
     char * arg;
     pid_t pid;
} job_t;

job_t jobs[MAX_JOB];
int job_total=0;

  
/**
 * Creates and returns an anode linked list of arguments from the
 * c-string cmd. The linked list is terminated by a NULL pointer. 
 * 
 * This function modifies the cmd string by replacing all spaces
 * outside of quotes with the terminator '\0'. The returned linked
 * list contains pointers to the begining of each subsequent cmd.
 *
 */
anode *arg_to_linked_list(char * cmd)
{
     char *c = cmd; /* the current character we are processing */   
     anode *head = (anode *) calloc(0, sizeof(anode));
     if(head==NULL)
     {
	  printf("CALLOC FAILED!!!\r\n");
	  return head; /* failure */
     }

     anode *tail = head;
     char *ab; /*argument begining*/

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
	       int i=0;
	       while((*c)!=' ' && (*c)!='\0')
		    c++;
	       /*c points to end of unquoted argument */
	  }

	  /* Terminate the current arg */
	  if((*c) != '\0')
	  {
	       *c = '\0';
	       c++;
	  }

	  tail->arg = ab;
	  if(*c != '\0') 
	  {
	       //we have more to parse
	       tail->next = (anode *) calloc(0, sizeof(anode));
	       if(tail->next==NULL)
	       {
		    printf("calloc Errored\r\n");
		    /* TODO dealloc the rest of the anodes */
		    return head; /* failure!!!! */
	       }
	  }
	  tail = tail->next;
     }
     return head;
}

char ** arg_linked_list_to_char_arr(anode * head)
{
     int i = 0;
     anode * cur=head;
     while(cur!=NULL)
     {
	  i++;
	  cur=cur->next;
     }
     i++; //so we can add NULL pointer as the end of the list

     char** arr = malloc(sizeof(char**)*i);
     i=0;
     cur=head;
     while(cur!=NULL)
     {
	  *(arr+i)=cur->arg;
	  cur=cur->next;
	  i++;
     }

     *(arr+i)=NULL;
     return arr;
}

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

void executebg(const char *file, char *const args[])
{
     pid_t p = execute(file, args);
     if(p==-1)
     {
	  printf("Unable to execute: %s\r\n", file);
	  return;
     }
     else
     {
	  jobs[job_total].pid = p;
	  jobs[job_total].arg = (char *) malloc((sizeof(char)*strlen(file)));
	  strcpy(jobs[job_total].arg,file);
	  job_total++; 
     }
}

void printJobs(void)
{
     int i=0;
     for(i=0;i<job_total;i++)
     {
	  printf("[%d] pid=(%d)\r\n",i,jobs[i]);
     }
}
     

/**
 * Releases the memory of every anode in the list 
 *
 */
void freeANodeList(anode * head)
{
     /* Releasing list */
     anode *cur = head;
     while(cur!=NULL)
     {
	  anode *parent = cur;
	  cur = cur->next;
	  free(parent);
     }
}

/**
 *  Prints the arg of every anode in the list
 *
 */
void printANodeList(anode * head)
{
     anode * cur = head;
     while(cur!=NULL)
     {
	  printf("%s ", cur->arg);
	  cur = cur->next;
     }
     printf("\n");
}


int main(int argc, char * argv[])
{

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
	  else
	  {
	       anode *list = arg_to_linked_list(cmd);
	       printANodeList(list);
	       char **const argv = arg_linked_list_to_char_arr(list);

	       printf("%s\r\n",argv[0]);
	       executefg(argv[0], argv);
	       free(argv);
	       freeANodeList(list);
	  }
	  free(cmd);
     }
     return 0;
}

