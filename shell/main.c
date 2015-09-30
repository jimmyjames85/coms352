#include <stdio.h>
#include <unistd.h>

#define INPUT_STRING_BUFFER_SIZE 256

int main(int argc, char * argv[])
{
     /*  int i;
	 for(i=0; i < argc ;i++)
	 {
	 printf("Argument %i: %s\n", i,  argv[i]);
	 }*/

     char * inputString = ((char *)malloc((sizeof(char)*INPUT_STRING_BUFFER_SIZE)));
     *inputString = '\0';
     
     fgets(inputString, INPUT_STRING_BUFFER_SIZE-1, stdin);
     printf("You entered: %s\r\n",inputString);

     fflush(stdin);
     free((void *)inputString);
     return 0;
}
