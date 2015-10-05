#include <stddef.h> /* size_t */
#include <stdlib.h> /* malloc */
#include "List.h"
#define INIT_CAPACITY 2 //TODO move to .h file and include _h_gaurd

/*
typedef struct List
{
     void ** arr;
     uint16_t capacity;
     uint16_t length;
     size_t * element_size;
} List;
*/

List * List(size_t * elementSize)
{
     List * ret = malloc(sizeof(List));

     ret->capacity=INIT_CAPACITY;
     ret->length=0;
     ret->arr = malloc(ret->capacity * INIT_CAPACITY);
}

List * List_add(List * list, void * data)
{
     
     if(list->length < list->capacity)
	  
     
}
