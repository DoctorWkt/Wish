#include "header.h"

int beeplength;

/* This file contains routines for setting the terminal characteristics,
 * and for getting the termcap strings.
 */

void setcbreak()	/* Set terminal to cbreak mode */
{
#ifdef JOB
  extern void checkjobs();
#endif
  int i;
 
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
  int pid;
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


void setcooked()		/* Set terminal to cooked mode */
{
#ifdef ATT

  struct termio tbuf;

  if (ioctl(0,TCGETA,&tbuf)) perror("ioctl in setcooked");
  tbuf.c_lflag = tbuf.c_lflag | ICANON | ECHO;
  tbuf.c_cc[4]=04;              /* set EOT to ^D */
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

bool tflagexist(id,tc)
  char *id,*tc;
{
  int i;

  for (i=0;tc[i+2]!=EOS;i++)
    if (tc[i]==':' && (tc[i+1]==id[0]) && (tc[i+2]==id[1]))
      return(TRUE);
  return(FALSE);
}

/* Set up the termcap string in the given variable.
 * Returns 1 if ok, 0 if no string.
 */
bool gettstring(name,loc)
 char *name, *loc;
 {
  extern char termcapbuf[];
  char bp[1024],*area;
 
  if (tflagexist(name,termcapbuf))
   {
    area=bp;
    tgetstr(name,&area);
    area=bp;
    while (isdigit(*area)) area++;
    strncpy(loc,area,10); return(TRUE);
   }
  else loc[0]=EOS; return(FALSE);
 }

void terminal()
{
  extern int wid,beeplength,disable_auto;
  extern char termcapbuf[],bs[],nd[],cl[],cd[],up[],so[],se[],beep[];
  char term[10];
#ifdef ATT
  char *area;
#endif
 
  if (disable_auto) return;
/* set up cursor control sequences from termcap */
 
  strncpy(term,getenv("TERM"),10);
  tgetent(termcapbuf,term);
 
                        /* The following is a hack to get Clam to */
                        /* work under ATT, as we don't know how to */
                        /* use the termcap emulation under ATT curses */
                        /* If you can fix it, please let us know :-) */
#ifdef ATT
# define yukdollar(x) { int i; for(i=0;x[i];i++) if (x[i]=='$') x[i]=EOS; }
  area=bp;
  strncpy(bs,tgetstr("bc",&area),10);
  strcpy(bs,"\b");      /* Hack to get bs to work with our system */
  if ((wid=tgetnum("co"))==-1) wid=80;
  wid--;
  strncpy(cl,tgetstr("cl",&area),10);
  yukdollar(cl);
  strncpy(cd,tgetstr("cd",&area),10);
  yukdollar(cd);
  strncpy(nd,tgetstr("nd",&area),10);
  yukdollar(nd);
  strncpy(up,tgetstr("up",&area),10);
  yukdollar(up);
  strncpy(so,tgetstr("so",&area),10);
  yukdollar(so);
  strncpy(se,tgetstr("se",&area),10);
  yukdollar(se);
  strncpy(beep,tgetstr("bl",&area),10);
  yukdollar(beep);
  if (!strcmp(beep,"\007")) strcpy(beep,"\007");
  beeplength=strlen(beep);
  /* vset("beep",beep); */
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
 
#else
  if (tflagexist("bs",termcapbuf))
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
  if (!gettstring("bl",beep))
    strcpy(beep,"\007");
  beeplength=strlen(beep);
  /* vset("beep",beep); */
#endif
}
