#ifndef _shell_util_h_
#define _shell_util_h_
#include "Job.h"
#include "List.h"
#include <stdio.h>      /* printf, scanf, NULL, FILE , stdin, stdout, stderr*/
//#include <stdarg.h> /* standard header contains a set of macro */


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
int fpreadl(FILE* fstream, char** line);


/**
 *  Uses fpreadl with stdin as the input file stream pointer.
 *
 *  See fpreadl for semantics.
 */
int readl(char** line);



/**
 *  Executes the file and waits for the process to finish.
 *
 *  From the EXEC(3) man page: The first argument, by convention,
 *	should point to the filename associated with the file being
 *	executed.
 *
 *  Returns the pid_t of the child process or -1 on failure. 
 */
pid_t executefg(const char *file, char *const  args[], int * status, char *const  in_file, char *const out_file);

/**
 */
Job * executebg(const char *file, char *const args[], char *const  in_file, char *const out_file);

/**
 * Creates and returns a list of arguments parsed from the c-string
 * cmd. A NULL pointer is appended to the list as well.
 * 
 */
List * arglist(char * cmd);
    
void printArgList(List * argList);


pid_t waiton(pid_t p, int * status);
#endif
