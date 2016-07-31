#include "header.h"

/* Most of the functions that handle signals are kept in this file.
 * Exceptions are some signal() calls done in job.c
 */

#define SIGTYPE void	/* One of int or void */

void graceful(sig)	/* Catch signals gracefully */
  int sig;
{
  fprints(2,"Received signal no %d\n",sig);
  exit(1);
}


void catchsig()		/* Make us catch all signals */
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

void dflsig()		/* Uncatch all signals */
 {
  int i;

  for (i=1; i<=MAXSIG; i++) signal(i,SIG_DFL);
 }
