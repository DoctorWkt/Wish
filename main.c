#include "header.h"

int Argc;
char **Argv;
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

main(argc, argv)
 int argc;
 char *argv[];
 {
  extern struct candidate carray[];
  extern char currdir[], *parsebuf;
  extern int curr_hist, maxhist, ncand;
  char *a, *EVget(), *expline();
  int i,fd,pid,q;
  TOKEN term,command();

#ifdef UCB
  if (getwd(currdir))			/* Get the current directory */
#else
  if (getcwd(currdir,MAXPL))
#endif
        EVset("cwd",currdir);
  else write(2,"Can't get cwd properly\n",23);

  Argc= argc;				/* Set up arg variables */
  Argv= argv;
  catchsig();				/* Catch signals */
#ifdef JOB
 /* setownterm(getpid());		/* We own the terminal */
  settou();				/* and want TTOU for children */
#endif
  terminal();				/* Get the termcap strings */
  if (!EVinit()) fatal("Can't initialise environment");
  if ((prompt=EVget("PS2"))==NULL) prompt="> ";
  lenprompt=strlen(prompt);

  while(1)
   {						/* Run a command */
#define DEBUG
#ifdef DEBUG
i= (int)sbrk(0);
prints("Brk value is %x\n",i);
#endif
    prprompt();
    for (i=0; i<1000; i++) linebuf[i]=0;
    setcbreak();				/* Set cbreak mode */
    if (getuline(linebuf,&q,FALSE)==TRUE)	/* Get a line from user */
     {
      meta_1(linebuf,TRUE);			/* Expand metachars */
      savehist(expline(carray),curr_hist++,maxhist); /* Save the line */
      meta_2();					/* Expand metachars */
      setcooked();
      parsebuf=a=expline(carray);
      strcat(parsebuf,"\n");
      term=command(&pid,FALSE,NULL);		/* Actually run it here */
      free(a);
						/* Free the command words */
       for (i=0;i<ncand; i++)
	if (carray[i].mode==TRUE)
	 { carray[i].mode=FALSE;
	   free(carray[i].name);
	 }
      if (term!=T_AMP)				/* If we should wait on it */
	waitfor(pid);				/* Wait for it to finish */
#ifdef JOB
      joblist(0);				/* print the list of jobs */
#endif
     }
 		   				/* Close any leftover fds */
    for (fd=3; fd<20; fd++) (void)close(fd);
   }
 }
