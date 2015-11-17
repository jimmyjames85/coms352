//# include <stddef.h> 
#include <stdlib.h> /* malloc */
#include <stdint.h> /* size_t */
#include "list.h"


int lresize(list_t * list)
{

     if(list->length == list->capacity)
     {
	  /* grow list */     
	  list->capacity *= 2;
	  void ** newArr = realloc(list->arr, sizeof(void *)*list->capacity);
	  if(newArr==NULL)
	       return -1;
	  list->arr=newArr;
     }
     if(list->capacity>_LIST_INIT_CAPACITY && 2*list->length < list->capacity)
     {
	  /* shrink list */
	  list->capacity /= 2 ;
	  void ** newArr =realloc(list->arr, sizeof(void *)*list->capacity);
	  if(newArr==NULL)
	       return -1;
	  list->arr=newArr;
     }
     return 0;
}

list_t * lalloc()
{
     list_t * ret = malloc(sizeof(list_t));
     ret->capacity=_LIST_INIT_CAPACITY;
     ret->length=0;
     ret->arr = malloc(sizeof(void*) * ret->capacity);
     return ret;

}

int ladd(list_t * list, void * data)
{
     if(lresize(list)!=0)
	  return -1;
     list->arr[list->length++] = data;
     return 0;
}

void * lget(list_t * list, int i)
{
     if(i>=0 && i<list->length)
	  return list->arr[i];
     return NULL;
}

void lfree(list_t * list)
{
     if(list!=NULL && list->arr!=NULL)
     {
	  free(list->arr);
	  free(list);
     }   
}
void lfreefree(list_t * list)
{
     if(list==NULL)
	  return;
     int i=0;
     for(i=0;i<list->length;i++)
	  free(list->arr[i]);
     lfree(list);
}

void * lremove(list_t * list, int i)
{
     void * ret = NULL;
     if(i>=0 && i<list->length)
     {
	  ret = list->arr[i] ;
	  int j;
	  
	  for(j=i;j<list->length-1;j++)
	       list->arr[j] = list->arr[j+1];
	  list->length--;
     }
     lresize(list);
     return ret;
}

void * lreplace(list_t * list, int i, void * newData)
{
     void * ret = NULL;
     if(i>=0 && i<list->length)
     {
	  ret = list->arr[i] ;
	  list->arr[i]=newData;
     }
     return ret;
}
