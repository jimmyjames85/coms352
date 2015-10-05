#ifndef _job_h_
#define  _job_h_
#include <sys/types.h> /* pid_t */
#include "List.h"
typedef struct Job
{
     char * cmd;
     pid_t pid;
     int job_id;

} Job;

extern int _next_job_number;

Job * jalloc();
void jfree(Job * job);
List * arglist(char * cmd);
#endif // _job_h_
