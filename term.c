#include "header.h"

int beeplength;
char termcapbuf[1024],bs[20],nd[20],cl[20],cd[20],up[20],so[20],se[20],beep[20];

/* This file contains routines for setting the terminal characteristics,
 * and for getting the termcap strings.
 */

#ifdef DEBUG
/* Printctrl prints out a terminal sequence that Clam got
 * from termcap. Only used when debugging the ensure Clam
 * is actually getting the strings, and the right ones.
 */
void printctrl(name,str)
 char *name, *str;
 {
  int i;

  prints("%s: ",name);
  for (i=0;str[i];i++)
  if (str[i]>31) putchar(str[i]);
  else { putchar('^');
         if (str[i]>26) putchar(str[i]+64);
         else putchar(str[i]+96);
       }
  putchar('\n');
 }
#endif

/* Setcbreak: Set terminal to cbreak mode */
void setcbreak()
{
 
#ifdef ATT
  struct termio tbuf;
 
  if (ioctl(0,TCGETA,&tbuf)) perror("ioctl in setup");
  tbuf.c_lflag = tbuf.c_lflag & (~ICANON) & (~ECHO);
  tbuf.c_cc[4]=1;               /* read 1 char before returning like CBREAK */
  if (ioctl(0,TCSETA,&tbuf)) perror("ioctl s");

#else
  struct sgttyb t, *termod= &t;
  struct tchars s, *setsigc= &s;
# ifdef UCB
  struct ltchars moresigc;
# endif

/* setup terminal with ioctl calls */

  if (ioctl(0,TIOCGETP,termod))         /* get the sgttyb struct */
    perror("ioctl in setcbreak");
  termod->sg_flags |= CBREAK;           /* cbreak mode to get each char */
  termod->sg_flags &= (~ECHO);          /* do not echo chars to screen */
  if (ioctl(0,TIOCSETP,termod))         /* put it back, modified */
    perror("ioctl1 su");
  if (ioctl(0,TIOCGETC,setsigc))        /* get the tchars struct */
    perror("ioctl2 in setcbreak");
  setsigc->t_intrc=(UNDEF);             /* no interrupt or quitting */
  /*setsigc->t_quitc=(UNDEF); 		   Allow quit while debugging */
  setsigc->t_eofc=(UNDEF);              /* or eof signalling */
  if (ioctl(0,TIOCSETC,setsigc))        /* put it back, modified */
    perror("ioctl2 scb");
# ifdef UCB
  moresigc.t_suspc=(UNDEF);             /* no stopping */
  moresigc.t_dsuspc=(UNDEF);            /* or delayed stopping */
  moresigc.t_rprntc=(UNDEF);            /* or reprinting */
  moresigc.t_flushc=(UNDEF);            /* or flushing */
  moresigc.t_werasc=(UNDEF);            /* or word erasing */
  moresigc.t_lnextc=(UNDEF);            /* or literal quoting */
  if (ioctl(0,TIOCSLTC,&moresigc))      /* set ltchars struct to be all undef */    perror("ioctl3");
# endif
#endif
}


/* Setcooked: Set terminal to cooked mode */
void setcooked()
{
#ifdef ATT

  struct termio tbuf;

  if (ioctl(0,TCGETA,&tbuf)) perror("ioctl in setcooked");
  tbuf.c_lflag = tbuf.c_lflag | ICANON | ECHO;
  tbuf.c_cc[4]=04;              	/* set EOT to ^D */
  if (ioctl(0,TCSETA,&tbuf)) perror("ioctl sd");

#else
  struct sgttyb t, *termod= &t;
  struct tchars s, *setsigc= &s;
# ifdef UCB
  struct ltchars moresigc;
# endif

  if (ioctl(0,TIOCGETP,termod))         /* get the sgttyb struct */
    perror("ioctl in setdown");
  termod->sg_flags &= (~CBREAK);        /* reset cooked mode */
  termod->sg_flags |= ECHO;             /* echo chars to screen */
  if (ioctl(0,TIOCSETP,termod))         /* put it back, modified */
    perror("ioctl1 sd");
  if (ioctl(0,TIOCGETC,setsigc))        /* get the tchars struct */
    perror("ioctl2 in setdown");
  setsigc->t_intrc=3;                   /* set interrupt and quitting */
  setsigc->t_quitc=28;
  setsigc->t_eofc=4;                    /* and eof signalling */
  if (ioctl(0,TIOCSETC,setsigc))        /* put it back, modified */
    perror("ioctl2 sd");

# ifdef UCB
  moresigc.t_suspc=26;                  /* stopping */
  moresigc.t_dsuspc=25;                 /* delayed stopping */
  moresigc.t_rprntc=18;                 /* reprinting */
  moresigc.t_flushc=15;                 /* flushing */
  moresigc.t_werasc=23;                 /* word erasing */
  moresigc.t_lnextc=22;                 /* literal quoting */
  if (ioctl(0,TIOCSLTC,&moresigc))      /* set ltchars struct to be default */
    perror("ioctl3 in setcooked");
# endif
#endif
}

/* gettstring: Given the name of a termcap string, gets the string
 * and places it into loc.  * Returns 1 if ok, 0 if no string.
 * If no string, loc[0] is set to EOS.
 */
bool gettstring(name,loc)
 char *name, *loc;
 {
  extern char termcapbuf[];
  char bp[50], *area=bp;
 
  if (tgetstr(name,&area)!=NULL)
   {
    area=bp;
    strncpy(loc,area,20); return(TRUE);
   }
  else loc[0]=EOS; return(FALSE);
 }


/* Terminal is called at the beginning of Clam to get the termcap
 * strings needed by the Command Line Editor. If they are not got,
 * or the wrong ones are found, the CLE will act strangely. Clam
 * should at this stage default to a dumber CLE.
 */
void terminal()
{
  extern char *EVget();
  extern int wid,beeplength;
  extern char termcapbuf[],bs[],nd[],cl[],cd[],up[],so[],se[],beep[];
  char term[20];
 
/* set up cursor control sequences from termcap */
 
  strncpy(term,EVget("TERM"),10);
  tgetent(termcapbuf,term);
  if (tgetflag("bs")==1)
    { bs[0]='\b'; bs[1]='\0'; }
  else gettstring("bc",bs);
  if ((wid=tgetnum("co"))==-1) wid=80;
  wid--;                /* this is to eliminate unwanted auto newlines */
  gettstring("cl",cl);
  gettstring("cd",cd);
  gettstring("nd",nd);
  gettstring("up",up);
  gettstring("so",so);
  gettstring("se",se);
  gettstring("bl",beep);
  if (*beep==EOS) strcpy(beep,"\007");
  beeplength=strlen(beep);
#ifdef DEBUG
  printctrl("bs",bs);
  printctrl("cl",cl);
  printctrl("cd",cd);
  printctrl("nd",nd);
  printctrl("up",up);
  printctrl("so",so);
  printctrl("se",se);
  printctrl("beep",beep);
#endif
}
