#include "header.h"

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
