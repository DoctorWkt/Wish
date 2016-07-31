#include "header.h"

#define SIGTYPE void	/* One of int or void */

static SIGTYPE (*entryint)(),(*entryquit)();
#ifndef BADSIG
#define BADSIG (SIGTYPE (*)())-1	/* -1 cast to ptr to int function */
#endif

void ignoresig()		/* Ignore interrupt and quit */
 {
  static BOOLEAN first=TRUE;

  if (first)
    {
     first=FALSE;
     entryint=signal(SIGINT,SIG_IGN);
     entryquit=signal(SIGQUIT,SIG_IGN);
     if (entryint==BADSIG || entryquit==BADSIG)
	{ perror("signal"); exit(1); }
    }
  else if (signal(SIGINT,SIG_IGN)==BADSIG ||
	   signal(SIGQUIT,SIG_IGN)==BADSIG)
	{ perror("signal"); exit(1); }
 }

void entrysig()		/* restore int and quit */
 {
  if (signal(SIGINT,entryint)==BADSIG ||
      signal(SIGQUIT,entryquit)==BADSIG)
	{ perror("signal"); exit(1); }
 }


void graceful(sig)	/* Catch signals gracefully */
  int sig;
{

  fprintf(stderr,"Received signal no %d\n",sig);
  exit(1);
}


void setup()		/* Make us catch all signals */
 {
  int i;
#ifdef JOB
  void checkjobs();
  void stopjob();
#endif

  for (i=4; i<=MAXSIG; i++)
   {
      if (i != SIGKILL) /* SIGKILL cannot be caught or ignored */
          signal(i,graceful);
   }
#ifdef JOB
			/* Also catch these for job control */
  signal(SIGCHLD,checkjobs);
  signal(SIGTSTP,stopjob);
#endif
 }
