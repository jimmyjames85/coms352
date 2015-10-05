#include "List.h"

typedef struct List
{
     void ** arr;
     uint16_t capacity;
     uint16_t length;
     size_t * element_size;     
} List;


