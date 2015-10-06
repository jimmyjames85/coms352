#ifndef _llist_h_
#define _llist_h_

#include <stdint.h>

typedef struct llnode
{
    struct llnode * prev;
    struct llnode * next;
    void * data;
    
} llnode;

typedef struct LList
{
    uint16_t length;
    llnode * head;
    llnode * tail;
} LList;

void llfree(LList * list);
void llfreefree(LList * list);
int lladd(LList * list, void * data);
void * llget(LList * list, int i);
void * llremove(LList * list, int i);
LList * llalloc(void);
int llsize(LList * list); 

#endif // _llist_h_
