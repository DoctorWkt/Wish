/* This file contains functions that replace printf, fprintf and sprintf,
 * which are smaller and do not use stdio buffering. They can only handle
 * %c %s %o %d and %x as format types, although %2x and %02x can be done as
 * well. Note that fprintf takes as its first argument a file descriptor,
 * not a FILE pointer.
 *
 * These routines were derived from the prints routine in Minix 1.5.
 */
#include "header.h"

/* If your compiler/system supports one of the following,
 * you should use it, preferably stdarg.h.
 */
#ifdef STDARG
# include <stdarg.h>
#endif
#ifdef VARARGS
# include <varargs.h>
#endif

#if !defined(STDARG) && !defined(VARARGS)

/* Otherwise we must make do with these */
#define va_list		char **
#define va_arg(x,t)	(t)*(x++)
#endif

#define	MAXDIG		 11		/* 32 bits in radix 8 */
#define TRUNC_SIZE 	1024

static char Buf[TRUNC_SIZE], *Bufp;
static char Intbuf[MAXDIG];
static int Out;
static char *Dest;

#ifdef PROTO
static void _itoa ( char *p , unsigned int num , int radix )
#else
static void _itoa(p, num, radix)
char *p;
unsigned int num;
int radix;
#endif
{
  int i;
  char *q;

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
void printvoid ( const char *S, va_list ap )
#else
void printvoid(S, ap)
  char *S;
  va_list ap;
#endif
{
  int w;
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
		k = va_arg(ap, int);
		_put(k);
		S++;
		continue;
	    case 's':
		p = va_arg(ap, char *);
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
		x= va_arg(ap, int);
		S++;
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

#ifdef VARARGS
/* For systems that have varargs.h, use the following three routines
 */
void fprints(va_alist)
va_dcl
{ va_list argptr; char *S;

  va_start(argptr); Out= va_arg(argptr,int);
  S= va_arg(argptr, char*); printvoid(S,argptr); va_end(argptr);
}
void sprints(va_alist)
va_dcl
{ va_list argptr; char *S;
  va_start(argptr); Dest= va_arg(argptr, char *); Out= -1;
  S= va_arg(argptr, char*); printvoid(S,argptr); va_end(argptr);
}
void prints(va_alist)
va_dcl
{ va_list argptr; char *S;
  va_start(argptr); Out= 1; S= va_arg(argptr, char*);
  printvoid(S,argptr); va_end(argptr);
}
#endif

#ifdef STDARG
/* For systems that have stdarg.h, use the following three routines
 */
#ifdef PROTO
void fprints(int fd, const char *S, ...)
#else
void fprints(fd,S)
int fd;
char *S;
#endif
{ va_list argptr;
  va_start(argptr,S); Out= fd;
  printvoid(S,argptr); va_end(argptr);
}
#ifdef PROTO
void sprints(char *out, const char *S, ...)
#else
void sprints(out,S)
char *out;
char *S;
#endif
{ va_list argptr;
  va_start(argptr,S); Dest= out;
  Out= -1; printvoid(S,argptr); va_end(argptr);
}
#ifdef PROTO
void prints(const char *S, ...)
#else
void prints(S)
char *S;
#endif
{ va_list argptr;
  va_start(argptr,S); Out= 1; printvoid(S,argptr); va_end(argptr);
}
#endif

#if !defined(STDARG) && !defined(VARARGS)
/* For systems that have neither, we use these.
 */
#ifdef PROTO
void fprints ( int fd , char *S, char *argptr )
#else
void fprints(fd, S, argptr)
int fd;
char *S;
char *argptr;
#endif
{
 Out=fd; printvoid(S,&argptr);
}
#ifdef PROTO
void sprints ( char *out , char *S, char *argptr )
#else
void sprints(out, S, argptr)
char *out;
char *S;
char *argptr;
#endif
{
 Dest=out; Out= -1; printvoid(S,&argptr);
}
#ifdef PROTO
void prints ( char *S, char *argptr )
#else
void prints(S, argptr)
char *S;
char *argptr;
#endif
{
 Out=1; printvoid(S,&argptr);
}
#endif
