#include "header.h"

#ifndef MAXARG
#define MAXARG  20
#endif

char linebuf[1000];
char *prompt;
int lenprompt;

/* Print out the prompt */
void prprompt()
 {
  prints("%s",prompt);
 }

/* Set the terminal back to normal
 * before leaving the shell
 */
void leave_shell()
 {
  setcooked();
  exit(0);
 }

main()
 {
  extern FILE *zin, *zout;
  extern char currdir[];
  extern int curr_hist, maxhist;
  char *EVget();
  int i,fd,pid,q;
  TOKEN term,command();

  zin=stdin; zout=stdout;		/* Set up out I/O */
#ifdef UCB
  if (getwd(currdir))			/* Get the current directory */
#else
  if (getcwd(currdir,MAXPL))
#endif
        EVset("cwd",currdir);
  else write(2,"Can't get cwd properly\n",23);

  catchsig();				/* Catch signals */
#ifdef JOB
 /* setownterm(getpid());		/* We own the terminal */
  settou();				/* and want TTOU for children */
#endif
  terminal();				/* Get the termcap strings */
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  lenprompt=strlen(prompt);
  prprompt();

  while(1)
   {						/* Run a command */
    for (i=0; i<1000; i++) linebuf[i]=0;
    setcbreak();				/* Set cbreak mode */
    if (getuline(linebuf,&q,FALSE)==TRUE)	/* Get a line from user */
     {
      savehist(linebuf,curr_hist++,maxhist);	/* Save the line */
      meta_1(linebuf,TRUE);			/* Expand metachars */
      meta_2();					/* Expand metachars */
      setcooked();
      term=command(&pid,FALSE,NULL);		/* Actually run it here */
#ifdef DEBUG
      fprints(2,"Got pid %d %d %d\n",pid,Headpid,term);
#endif
      if (term!=T_AMP)				/* If we should wait on it */
	waitfor(pid);				/* Wait for it to finish */
#ifdef JOB
      joblist(0);				/* print the list of jobs */
#endif
     }
    prprompt();					/* Print the prompt again */
 		   				/* & close any leftover fds */
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
