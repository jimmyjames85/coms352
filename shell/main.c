#include <stdio.h>    /// Includes FILE typedef, NULL 

//#include <unistd.h> <---- replaces syscalls.h????
#include <ctype.h> //Char type

#include <stdarg.h> /* standard header contains a set of macro
		     * definitions that define how to step through an
		     * argument list. */

#define INPUT_STRING_BUFFER_SIZE 256





int main(int argc, char * argv[])
{

     void sayHello(void)
     {
	  printf("Hello!\r\n");
     };
     /*  int i;
	 for(i=0; i < argc ;i++)
	 {
	 printf("Argument %i: %s\n", i,  argv[i]);
	 }*/
     void tryToSayHello(void);     
/*
  char * inputString = ((char *)malloc((sizeof(char)*INPUT_STRING_BUFFER_SIZE)));
  *inputString = '\0';
     
  fgets(inputString, INPUT_STRING_BUFFER_SIZE-1, stdin);
  printf("You entered: %s\r\n",inputString);

  fflush(stdin);
  free((void *)inputString);
  return 0;
*/
     sayHello();
     tryToSayHello();
     char c;
     while((c = getchar()) != EOF )
	  putchar(tolower(c));


     return 0;
}



void tryToSayHello(void)
{
     sayHello();
}
