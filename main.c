#include "header.h"

#ifndef MAXARG
#define MAXARG  20
#endif

int Headpid;		/* Head of the pipeline */
main()
 {
  char *prompt,*EVget();
  int fd;
   TOKEN term,command();

  ignoresig();
  setup();
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  printf("%s",prompt);

  while(1)
   {
    term=command(&Headpid,FALSE,NULL);
    if (term!=T_AMP && Headpid!=0)
	waitfor(Headpid);
    if (term==T_NL)
    	printf("%s",prompt);
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
