/******************************************************************************
**                                                                           **
**                                prints.c                                   **
**    This file contains functions that replace printf, fprintf and sprintf, **
**       which are smaller and do not use stdio buffering. They can only     **
**     handle %c %s %o %d and %x as format types, although %2x and %02x can  **
**        be done as well. Note that fprintf takes as its first argument a   **
**                   file descriptor, not a FILE pointer.                    **
**                                                                           **
**      These routines were derived from the prints routine in Minix 1.5.    **
**                                                                           **
******************************************************************************/
#include "header.h"
#define	MAXDIG		11	/* 32 bits in radix 8 */
#define TRUNC_SIZE 128
static char Buf[TRUNC_SIZE], *Bufp;
static char Intbuf[MAXDIG];
static int *Valp;
static char *S;
static int Out;
static char *Dest;

#ifdef PROTO
static void _itoa ( char *p , unsigned num , int radix )
#else
static void _itoa(p, num, radix)
register char *p;
register unsigned num;
register radix;
#endif
{
  register i;
  register char *q;

  q = p + MAXDIG;
  do {
        i = (int) (num % radix);
        i += '0';
        if (i > '9') i += 'A' - '0' - 10;
        *--q = i;
  } while (num = num / radix);
  i = p + MAXDIG - q;
  do
        *p++ = *q++;
  while (--i);
  *p=0;
}

#ifdef PROTO
static void _put ( int c )
#else
static void _put(c)
char c;
#endif
{
  if (Bufp < &Buf[TRUNC_SIZE]) *Bufp++ = c;
}

#ifdef PROTO
static void printvoid ( void )
#else
static void printvoid()
#endif
{
  register w;
  int k, x, radix;
  char *p, *p1, c, fillchar;

  Bufp = Buf;
  while (*S != '\0') {
	if (*S != '%') {
		_put(*S++);
		continue;
	}
	w = 0; fillchar= ' ';
	S++;
	while (*S >= '0' && *S <= '9') {
		if (*S=='0' && !w) fillchar= '0';
		w = 10 * w + (*S - '0');
		S++;
	}

	switch (*S) {
	    case 'c':
		k = *Valp++;
		_put(k);
		S++;
		continue;
	    case 's':
		p = *((char **) Valp);
		Valp += sizeof(char *) / sizeof(int);
		p1 = p;
		while (c = *p++) _put(c);
		for (x=strlen(p1);w>x;w--) _put(fillchar);
		S++;
		continue;
	    case 'x': 
		radix= 16; goto printnum;
	    case 'd':
		radix= 10; goto printnum;
	    case 'o':
		radix= 8;
printnum:
		x= (int) *Valp;
		Valp++; S++;
		_itoa(Intbuf,x,radix);
		p=Intbuf;
		for (x=strlen(p);w>x;w--) _put(fillchar);
		while (c = *p++) _put(c);
		continue;
	    default:
		_put(*S++);
		continue;
	}

  }
				/* write everything in one blow. */
  if (Out==-1)
    {  *Bufp++ = 0;
#ifdef UCB
	bcopy(Buf,Dest,(int) (Bufp - Buf));
#else
	memcpy(Dest,Buf,(int) (Bufp - Buf));
#endif
    }
  else
  	write(Out, Buf, (int) (Bufp - Buf));
}

#ifdef PROTO
void fprints ( int out , char *s , int *arglist )
#else
/*VARARGS*/
void fprints(out, s, arglist)
int out;
register char *s;
int *arglist;
#endif
{
 Valp = (int *) &arglist;
 S= s;
 Out= out;
 printvoid();
}

#ifdef PROTO
void sprints ( char *dest , char *s , int *arglist )
#else
/*VARARGS*/
void sprints(dest, s, arglist)
char *dest;
register char *s;
int *arglist;
#endif
{
 Valp = (int *) &arglist;
 S= s;
 Dest= dest;
 Out= -1;
 printvoid();
}

#ifdef PROTO
void prints ( char *s , int *arglist )
#else
/*VARARGS*/
void prints(s, arglist)
register char *s;
int *arglist;
#endif
{
 Valp = (int *) &arglist;
 S= s;
 Out= 1;
 printvoid();
}
