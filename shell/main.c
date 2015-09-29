#include <stdio.h>

int main(int argc, char * argv[])
{
  int i;
  printf("Hello World!\n");

  for(i=0; i < argc ;i++)
  {
    printf("Argument %i: %s\n", i,  argv[i]);
  }
}
