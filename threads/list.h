#ifndef _list_h_
#define _list_h_
#define _LIST_INIT_CAPACITY 2 
#include <stdint.h>
typedef struct List
{
     uint16_t capacity;
     uint16_t length;
     void ** arr;
} list_t;

void lfree(list_t * list);
void lfreefree(list_t * list);
int ladd(list_t * list, void * data);
void * lget(list_t * list, int i);
void * lremove(list_t * list, int i);
list_t * lalloc();
void * lreplace(list_t * list, int i, void * newData);
#endif // _list_h_
