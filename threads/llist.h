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
} llist_t;

void llfree(llist_t * list);
void llfreefree(llist_t * list);
int lladd(llist_t * list, void * data);
void * llget(llist_t * list, int i);
void * llremove(llist_t * list, int i);
llist_t * llalloc(void);
int llsize(llist_t * list);

#endif // _llist_h_
