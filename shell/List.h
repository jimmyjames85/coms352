#ifndef _list_h_
#define _list_h_
#define _LIST_INIT_CAPACITY 2 
#include <stdint.h>
typedef struct List
{
     uint16_t capacity;
     uint16_t length;
     void ** arr;
} List;

void lfree(List * list);
void lfreefree(List * list);
int ladd(List * list, void * data);
void * lget(List * list, int i);
void * lremove(List * list, int i);
List * lalloc();

#endif // _list_h_
