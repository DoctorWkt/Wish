#include "header.h"

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

  /* signal(SIGINT,SIG_IGN); 		For debugging */
  signal(SIGQUIT,SIG_IGN);
  for (i=4; i<=MAXSIG; i++)
   {
      if (i!=SIGKILL && i!=SIGCONT) /* SIGKILL cannot be caught or ignored */
          signal(i,graceful);
   }
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

setownterm(pid)		/* Set the terminal's process group */
 int pid;
 {
#ifdef UCB
  if (pid)
   {
    if (ioctl(0,TIOCSPGRP,&pid)) perror("ioctl spg");
   }
#endif
 }

#ifdef JOB
void settou()		/* Set terminal to force SIGTTOU */
 {
  struct termio tbuf;
 
  if (ioctl(0,TCGETA,&tbuf)) perror("ioctl in settou");
  tbuf.c_lflag |= TOSTOP;
  if (ioctl(0,TCSETA,&tbuf)) perror("ioctl s");
 }
#endif
