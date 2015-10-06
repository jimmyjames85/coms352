#ifndef _job_h_
#define  _job_h_
#include <sys/types.h> /* pid_t */
#include "LList.h"
extern int _next_job_number;

void reset_next_job_number(void);

typedef enum {RUNNING, FINISHED, DEAD} job_status;

typedef struct Job
{
    char * cmd;
    pid_t pid;
    int job_id;
    job_status status;
} Job;




Job * jalloc(const char * cmd, pid_t pid, job_status status);
void jfree(Job * job);
void printJobs(LList * running, LList * finished);

#endif // _job_h_
