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


void graceful(sig)
  int sig;
{

  fprintf(stderr,"Received signal no %d\n",sig);
  exit(1);
}


void setup()
 {
  int i;
  void checkjobs();
  void stopjob();

  for (i=4; i<=27; i++)
   {
      if (i != SIGKILL) /* SIGKILL cannot be caught or ignored */
          signal(i,graceful);
   }
  /* signal(SIGCHLD,checkjobs); */
  signal(SIGCHLD,SIG_IGN);
  signal(SIGTSTP,stopjob);
 }
