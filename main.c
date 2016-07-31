#include "header.h"

#ifndef MAXARG
#define MAXARG  20
#endif

char linebuf[1000];

main()
 {
  extern int Headpid;		/* Head of the pipeline */
  extern char *parsebuf;
  char *prompt,*EVget();
  int fd,pid;
  TOKEN term,command();

  catchsig();			/* Catch signals */
  setownterm(getpid());		/* We own the terminal */
  /* setcbreak();			/* Set cbreak mode */
  settou();			/* and want TTOU for children */
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  prints("%s",prompt);

  while(1)
   {				/* Run a command */
    getline(linebuf,1000);	/* Get a line from user */
    parsebuf=linebuf;
    term=command(&pid,FALSE,NULL);
    if (term==T_EOF) { setcooked(); exit(1); }
#ifdef DEBUG
    fprints(2,"Got pid %d %d %d\n",pid,Headpid,term);
#endif
    if (pid && term!=T_AMP) Headpid=pid;
    if (term!=T_AMP && Headpid!=0)
	waitfor(Headpid);	/* Wait for it to finish */
    if (term==T_NL)
    	prints("%s",prompt);	/* and close any leftover file descs */
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
