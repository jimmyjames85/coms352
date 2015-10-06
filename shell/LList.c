#include <stdlib.h> /* malloc */
#include <stdint.h> /* size_t */
#include "LList.h"

void llfree(LList * list)
{
    if(list!=NULL)
	 free(list);
}
void llfreefree(LList * list)
{
}
int llsize(LList * list)
{
     return list->length;
} 
int lladd(LList * list, void * data)
{
     llnode * elem = malloc(sizeof(llnode));
     elem->data = data;
     elem->next = NULL;
     elem->prev = NULL;

     if(list->length==0)
     {
	  list->head = elem;
	  list->tail = elem;
     }
     else if(list->length==1)
     {
	  list->head->next = elem;
	  elem->prev = list->head;	  
	  list->tail = elem;  
     }
     else
     {
	  list->tail->next = elem;
	  elem->prev = list->tail;	  
	  list->tail = elem; 
     }
     list->length++;
     
     return 0;
}

llnode * _get_node(LList * list, int i)
{
     if(i<0 || i>list->length)
	  return NULL;

     llnode * cur = list->head;
     while(i>0 && cur!=NULL)
     {
	  
	  cur = cur->next;
	  i--;
     }
     if(i!=0)
	  return NULL;

     return cur;
}

void * llget(LList * list, int i)
{
     llnode * ret = _get_node(list, i);
     if(ret ==NULL)
	  return(void *) NULL;
     return ret->data;
}

void * llremove(LList * list, int i)
{
     llnode * node = _get_node(list, i);
     if(node==NULL)
	  return (void *)NULL;

     if(node->prev != NULL)
     {
	  if(node->next !=NULL)
	  {
	       node->prev->next = node->next;
	       node->next->prev = node->prev;
	  }
	  else
	  {
	       node->prev->next = NULL;
	       list->tail = node->prev;
	  }
     }
     else
     {
	  if(node->next !=NULL)
	  {
	       node->next->prev = NULL;
	       list->head = node->next;
	  }
	  else
	  {
	       list->head=NULL;
	       list->tail=NULL;
	  }
     }

     list->length--;
     void * rett = node->data;
     free(node);
     return rett;
}


LList * llalloc(void)
{
     LList * ret = malloc(sizeof(LList));
     ret->head=NULL;
     ret->tail=NULL;
     ret->length=0;
     return ret;
}
