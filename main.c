#include "header.h"

int Argc;
char **Argv;
char linebuf[1000];
char *prompt;
int lenprompt;

int lengthint(num)
  int num;
{
  int i;

  for (i=0;num;num=num/10,i++);
  return(i);
}

void printime()
{
  long clock, time();
  struct tm *t, *localtime();

  time(&clock);
  t=localtime(&clock);
  prints("%2d:%02d:%02d",t->tm_hour,t->tm_min,t->tm_sec);
}

void prprompt()
{
  extern int curr_hist;
  extern char *EVget(),so[],se[];
  char c;
  int i,len;

  lenprompt=0;
  prompt= EVget("prompt");
  if (prompt==NULL) prompt= "% ";
  for (i=0;prompt[i];i++)
    if (prompt[i]=='%')
      switch(prompt[++i])
      {
	case EOS:
	case '%': write(1,"%",1);
		  lenprompt++;
		  break;
	case '!':
	case 'h': lenprompt+=lengthint(curr_hist);
		  prints("%d",curr_hist);
		  break;
	case 'd': len=strlen(EVget("cwd"));
		  lenprompt+=len;
		  write(1,EVget("cwd"),len);
		  break;
	case 'S': prints("%s",so);
		  break;
	case 's': prints("%s",se);	/* temp stop standout */
		  break;
	case '@':
	case 't': lenprompt+=8;
		  printime();
		  break;
	default : write(1,"%",1);
		  lenprompt+=2;
		  if (prompt[i]<32 || prompt[i]==127)
		  {
		    write(1,"^",1);
		    c=prompt[i]+64;
		    write(1,&c,1);
		    lenprompt++;
		  } else write(1,&prompt[i],1);
      }
    else
    {
      if (prompt[i]<32 || prompt[i]==127)
      {
	write(1,"^",1);
	c=prompt[i]+64;
	write(1,&c,1);
	lenprompt++;
      } else write(1,&prompt[i],1);
      lenprompt++;
    }
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
  EVset("prompt","% ");

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
    if (getuline(linebuf,&q,FALSE)==TRUE && q==0) /* Get a line from user */
     {
      if (meta_1(linebuf,TRUE)==FALSE) continue; /* Expand ! */
      savehist(expline(carray));		/* Save the line */
      meta_2();					/* Expand $ and ~ */
      meta_3();					/* Expand * ? and [] */
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
