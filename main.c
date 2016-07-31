#include "header.h"

int Argc;		/* The number of arguments for Clam, or an alias */
char **Argv;		/* The argument list for Clam, or an alias */
char *prompt;		/* A pointer to the prompt string */
int lenprompt;		/* and the length of the prompt */
bool getuline();
bool (*getline)()=getuline;	/* Our input routine defaults to getuline() */


/* Lengthint is used by prprompt to determine the number
 * of chars in the string version of an integer; this is
 * needed so the prompt length is calculated properly.
 */
int lengthint(num)
  int num;
{
  int i;

  for (i=0;num;num=num/10,i++);
  return(i);
}

/* Print out the current time in a set length string */
void printime()
{
  long clock, time();
  struct tm *t, *localtime();

  time(&clock);
  t=localtime(&clock);
  prints("%2d:%02d:%02d",t->tm_hour,t->tm_min,t->tm_sec);
}

/* Prprompt prints out the prompt. It parses all the special options
 * that the prompt can take, and determines the effective length of
 * the prompt as it appears on the screen. If there is no $prompt,
 * it defaults to '% '.
 */
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
  setcooked(); write(1,"\n",1);
  exit(0);
 }

/* Setup does most of the initialisation of the shell. Firstly the default
 * variables & the environ variables are set up. Then the termcap strings
 * are found, and then any other misc. things are done.
 */
void setup()
 {
  extern char currdir[];
  int source();
  char *argv[2];
				/* Initialise the environment */
  if (!EVinit()) fatal("Can't initialise environment");
#ifdef UCB
  if (getwd(currdir))			/* Get the current directory */
#else
  if (getcwd(currdir,MAXPL))
#endif
    EVset("cwd",currdir);
  else
    write(2,"Can't get cwd properly\n",23);
  EVset("prompt","% ");			/* Set the prompt to % for now */
  catchsig();				/* Catch signals */
#ifdef JOB
  settou();				/* We want TTOU for children */
#endif
  terminal();				/* Get the termcap strings */

  argv[0]="source"; argv[1]=".klamrc";
  source(2,argv);
 }


/* Doline is the loop which gets a line from the user, runs it through
 * the metachar expansion, and finally calls command() to execute it.
 */

bool doline()
 {
  extern struct candidate carray[];
  extern int curr_hist, maxhist, ncand;
  char *EVget(), *expline(), *linebuf;
  int i,pid,q;
  int term,command();
  bool getuline(),meta_1();

#ifdef DEBUG
i= (int)sbrk(0);
prints("Brk value is %x\n",i);
prints("Getline f'n ptr is %x\n",getline);
#endif
    linebuf=(char *)malloc(1024);
    if (linebuf==NULL) return(FALSE);
    for (i=0; i<1024; i++) linebuf[i]=0;	/* Clear the linebuf */
    if ((*getline)(linebuf,&q)==TRUE)		/* Get a line from user */
     {
      if (q) return(TRUE);
      setcooked();				/* Set us to cooked mode */
      if (meta_1(linebuf,TRUE)==FALSE)
	return(FALSE);				/* Expand ! */
      savehist(expline(carray));		/* Save the line */
      meta_2();					/* Expand $ and ~ */
      meta_3();					/* Expand * ? and [] */
      term=command(&pid,FALSE,NULL);		/* Actually run it here */

       for (i=0;i<ncand; i++)			/* Free the command words */
	if (carray[i].mode==TRUE)
	 { carray[i].mode=FALSE;
	   free(carray[i].name);
	 }

      if (term!=C_AMP)				/* If we should wait on it */
	waitfor(pid);				/* Wait for it to finish */
#ifdef JOB
      joblist(0);				/* print the list of jobs */
#endif
 		   				/* Close any leftover fds */
    for (i=4; i<20; i++) (void)close(i);
    return(TRUE);
   }
  return(FALSE);
 }

/* Main. This is the bit that starts the whole ball rolling.
 */
main(argc, argv)
 int argc;
 char *argv[];
 {

  Argc= argc;			/* Set up arg variables */
  Argv= argv;
  setup();			/* Set up the vars & the termcap stuff */

  while(1) doline();		/* We only exit when we shutdown() */
 }
