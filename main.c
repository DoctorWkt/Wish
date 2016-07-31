#include "header.h"

#ifndef MAXARG
#define MAXARG  20
#endif

char linebuf[1000];
char *prompt;
int lenprompt;

void prprompt()
 {
  prints("%s",prompt);
 }

void leave_shell()
 {
  setcooked();
  exit(0);
 }

main()
 {
  extern FILE *zin, *zout;
  extern int Headpid;		/* Head of the pipeline */
  extern char *parsebuf;
  extern char *cwd;
  char *EVget();
  int i,fd,pid,q;
  TOKEN term,command();

  zin=stdin; zout=stdout;
  cwd="/wherever";
  catchsig();			/* Catch signals */
#ifdef JOB
  setownterm(getpid());		/* We own the terminal */
  settou();			/* and want TTOU for children */
#endif
  terminal();			/* Get the termcap strings */
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  lenprompt=strlen(prompt);
  prprompt();

  while(1)
   {						/* Run a command */
    for (i=0; i<1000; i++) linebuf[i]=0;
    setcbreak();			/* Set cbreak mode */
    if (getline(linebuf,&q,FALSE)==TRUE)	/* Get a line from user */
     {
      parsebuf=linebuf;
      setcooked();
      term=command(&pid,FALSE,NULL);
#ifdef DEBUG
      fprints(2,"Got pid %d %d %d\n",pid,Headpid,term);
#endif
      if (pid && term!=T_AMP) Headpid=pid;
      if (term!=T_AMP && Headpid!=0)
	waitfor(Headpid);			/* Wait for it to finish */
#ifdef JOB
      joblist();
#endif
     }
    prprompt();
    				/* and close any leftover file descs */
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
