#include "header.h"

/* Most of the functions that handle signals are kept in this file.
 * Exceptions are some signal() calls done in job.c
 */

#define SIGTYPE void		/* One of int or void */

/* Graceful is set to catch most of the unused signals in Clam. All it
 * does is print out an error message and exit. It should probably call
 * setcooked() or do some other things.
 */
void graceful(sig)
  int sig;
{
  fprints(2,"Received signal no %d\n",sig);
  exit(1);
}


/* Catchsig is called once at Clam startup, and it sets graceful to
 * catch most of the signals unused by Clam.
 */
void catchsig()
 {
  int i;
#ifdef JOB
  void checkjobs();
  void stopjob();
#endif

  signal(SIGINT,SIG_IGN); 		/* Taken out for debugging */
  signal(SIGQUIT,SIG_IGN);
#ifdef NOTYET
  for (i=4; i<=MAXSIG; i++)
   {
      if (i!=SIGKILL && i!=SIGCONT) /* SIGKILL cannot be caught or ignored */
          signal(i,graceful);
   }
#endif
#ifdef JOB
					/* Also catch these for job control */
  signal(SIGCHLD,checkjobs);
  signal(SIGTSTP,stopjob);
#endif
 }

/* Dflsig sets all the signals to their default handlers, so that exec'd
 * programs will have a standard signal environment.
 */
void dflsig()
 {
  int i;

  for (i=1; i<=MAXSIG; i++) signal(i,SIG_DFL);
 }
