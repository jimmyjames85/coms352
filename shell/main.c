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


/**
 * Argument node used to make a linked list of arguments for a command
 */
typedef struct anode
{
     char * arg;
     struct anode *next;
} anode;

void debug(char * msg)
{
     printf("%s",msg );
}

/**
 * Creates and returns an anode with arg and next both pointing to
 * NULL. If malloc fails then it returns NULL;
 */
anode *new_anode()
{
     anode *newanode = (anode *) malloc(sizeof(anode)); 
     if(newanode!=NULL)
     {
	  newanode->arg = NULL;
	  newanode->next = NULL;
     }
     return newanode;
}
char * eatWhiteSpace(char * s)
{ 
     while((*s==' ') && (*s!='\0'))
	  s++;
     return s; 
}

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
     anode *head = new_anode();
     if(head==NULL) return head; /* failure */

     anode *tail = head;
     char *ab; /*argument begining*/

     while(*c != '\0')
     {
	  c = eatWhiteSpace(c);
	  ab = c; /* argument begining */
	  if(*c=='"')
	  {
	       c++;/* after the quote */
	       ab = c; 
	       while(*c!='"' && *c!='\0')
		    c++;
	       
	       /*c points to end of quoted argument */
	  }
	  else
	  {
	       int i=0;
	       while((*c)!=' ' && (*c)!='\0')
		    c++;
	       /*c points to end of unquoted argument */
	  }

	  if((*c) != '\0')
	  {
	       *c = '\0';
	       c++;
	  }

	  tail->arg = ab;
	  if(*c != '\0') 
	  {
	       //we have more to parse
	       tail->next = new_anode();
	       if(tail->next==NULL)
	       {
		    /* TODO dealloc the rest of the anodes */
		    return head; /* failure */
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
     i++; //for NULL

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
 * DOES NOT FREE MEMORY allocated to **line if function ends successfully
 * */
int fpreadl(FILE* fstream, char** line)
{

/*Start with 1 byte and if we need more then we double this each time*/
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

/** If this while loop was successful then pos points to 1 past the last character that we read in
 *  AND the last character we read in was either a CR or a LF so we need to decrement pos and
 *  change that last character to the null character \0. However if the while loop was never entered
 *  then pos=0 and we set that character to \0 and return an empty string.*/

     if (endOfLine || feof(fstream))
	  pos--;
     (*line)[pos] = '\0';

     /*success!*/
     return 0;
}

int readl(char** line)
{
     return fpreadl(stdin, line);
}

int execute(const char *file, char *const  args[])
{
     char isChild = 0;     
     pid_t p = fork();
     if(p == -1) /* TODO handle error*/
	  printf("Unable to fork\r\n");
     else if(p==0) /* child */
     {
	  isChild=1;
	  int err = execvp(file, args);
	  printf("Error(%d) occured executing: %s\n", err, file);
	  exit(0);
     }
     else /* parent */
     {
	  signed int status;
	  waitpid(p, &status, 0);
	  //printf("\npid(%d) finished with status=%d\r\n",p,status);
     }
}


freeANodeList(anode * head)
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
printANodeList(anode * head)
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
     char isChild = 0;

     while( keepGoing )
     {
	  if(isChild)
	       printf("CHILD$ ");
	  else
	       printf("%s",CLI);

	  if( readl(&cmd) ) 
	  {
	       keepGoing=0;
	       continue;
	  }
	  else if((strstr(cmd, "exit")==cmd) || strstr(cmd, "quit")==cmd )
	  {
	       keepGoing=0;
	       printf("Goodbye.\r\n");
	  }
	  else
	  {
	       anode *list = arg_to_linked_list(cmd);
	       char **const argv = arg_linked_list_to_char_arr(list);
	       
	       execute(argv[0], argv);
	       free(argv);
	       freeANodeList(list);
	  }

	  free(cmd);

     }

/*     void bprintf(const char * fmt, ...)
       {
       char buffer[];
       va_list args;
       va_start(args, fmt);
       vsprintf(buffer, fmt, args);
       sendString(buffer);
       va_end(args);
       }
*/
     
/*
  char * inputString = ((char *)malloc((sizeof(char)*INPUT_STRING_BUFFER_SIZE)));
  *inputString = '\0';
     
  fgets(inputString, INPUT_STRING_BUFFER_SIZE-1, stdin);
  printf("You entered: %s\r\n",inputString);

  fflush(stdin);
  free((void *)inputString);
  return 0;
*/



     return 0;
}

