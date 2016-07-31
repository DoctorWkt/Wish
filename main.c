#include "header.h"

#ifndef MAXARG
#define MAXARG  20
#endif

main()
 {
  extern int Headpid;		/* Head of the pipeline */
  char *prompt,*EVget();
  int fd;
  TOKEN term,command();

  ignoresig();			/* Ignore SIGINT & SIGQUIT */
  setup();			/* Catch other signals */
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  printf("%s",prompt);

  while(1)
   {				/* Run a command */
    term=command(&Headpid,FALSE,NULL);
    if (term!=T_AMP && Headpid!=0)
	waitfor(Headpid);	/* Wait for it to finish */
    if (term==T_NL)
    	printf("%s",prompt);	/* and close any leftover file descs */
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
