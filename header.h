/* This is at the moment a severe mangle of the Clam header.h
 * I will tidy it up one day.
 * Note most structures are in the files where they need it, unless
 * routines from different files need the structs.
 *
 * Also note, now only define one thing in the Makefile, header.h does
 * the rest of the work.
 */

#ifdef SYSVPYR		/* Pyramid Dual Universe machine under SysV */
# define UNIVERSE
# define ATT
# define SCRIPT
# define VARARGS
#endif

#ifdef BSDPYR		/* # Pyramid Dual Universe machine under BSD 4.x */
# define UNIVERSE
# define UCB
# define SCRIPT
# define JOB
# define USES_DIRECT	/* Uses struct direct, not struct dirent */
# define VARARGS
#endif

#ifdef GENSYSV		/* Generic SysV machine */
# define ATT
# define SCRIPT
# define VARARGS
#endif

#ifdef GENBSD		/* Generic BSD 4.x machine */
# define UCB
# define SCRIPT
# define JOB
# define VARARGS
#endif

#ifdef SUN		/* Sun OS */
# define UCB
# define SCRIPT
# define JOB
# define VARARGS
#endif

#ifdef COHERENT		/* Coherent */
# define NEED_GETCWD
#endif

#ifdef MINIX		/* Minix */
# define STDARG
#endif

#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#if defined(__STDC__) && __STDC__
# define PROTO
# ifndef __GNUC__
#  include <stdlib.h>
# endif
#endif

#if !defined(ATT) && !defined(UCB) && !defined(MINIX) && !defined(COHERENT)
+++
        You must define one of ATT, UCB, MINIX or COHERENT
+++
#endif

#ifdef ATT
# include <string.h>
# include <dirent.h>
# include <termio.h>
# include <sys/file.h>
# include <sys/time.h>
#endif

#ifdef MINIX
# include <string.h>
# include <stdio.h>		/* Because strings NULL won't work with ACK */
# include <sgtty.h>
# include <dirent.h>
# include <fcntl.h>
# include <time.h>
#endif

#ifdef COHERENT
# include <string.h>
# include <dirent.h>
# include <sgtty.h>
# include <sys/fcntl.h>
# include <time.h>
#endif

#ifdef UCB
# include <strings.h>
# include <sgtty.h>
# include <sys/time.h>
# include <sys/file.h>
# include <sys/resource.h>
#  ifdef USES_DIRECT
#   include <sys/dir.h>
#  else
#   include <dirent.h>
#  endif
#  ifdef SUN
#   define mc68000 1
#   include <string.h>
#   include <sys/wait.h>
#  else
#   include <strings.h>
#   include <wait.h>
#   define strchr  index
#   define strrchr rindex
extern char *strpbrk();
extern char *strtok();
extern int strspn();
#  endif
#endif

#ifndef NULL				/* Generic null pointer, May need */
#define NULL ((void *)0)		/* changing on some machines, to */
#endif					/* (char *)0, or even just 0 */

#define UNDEF	-1			/* Used when setting cbreak etc. */
#define EOS	'\0'			/* End of string terminator */
#ifndef MINIX
#define EOF	-1			/* End of file error value */
#endif

#ifndef MAXSIG
#define MAXSIG	27			/* Maximum signals known to Clam */
#endif

#define BADFD -2
#define MAXARG   300			/* Max # args passed to processes */
#define MAXWORD  200			/* Only used in parse, try to lose */
#define MAXFNAME 200			/* Only used in parse, try to lose */
#define MAXPL    512                    /* Path length */
#define MAXLL   2048			/* Used in comlined & hist */
#define MAXWL    512			/* Used in clex & meta */
#define MAXCAN  1000			/* maximum number of candidates */


typedef enum {FALSE,TRUE} bool;		/* Boolean type */

#define fatal(mess) { fprints(2,"%s\n",mess); exit(1); }

/* These only used in job */
#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w) >> 8)


/* Aliases
 *
 * These structures are used to store the alias definitions.
 */

struct adefn
{
  char *a_line;
  struct adefn *nextln;
};

struct alias
{
  char *a_name;
  struct adefn *defin;
  struct alias *next;
};

/* Clex and Meta
 *
 * The following structure is used by both clex.c and meta.c. Meta
 * uses it to hold candidates files that matched a ^D or <tab> expression.
 * Here the mode holds the mode of each file as obtained by stat().
 * Clex uses the struct to build a linked list of words that will
 * eventually be parsed by the parser. Here, the mode is a complex bitfield.
 * For normal words,the mode is used as a bitfield to indicate:
 *	- if the name has been malloc'd (mode&TRUE)
 *	- if the name has an invisible space in the end (mode&C_SPACE)
 *	- if the name is in single quotes (mode&C_QUOTE)
 *	- if the name is in double quotes (mode&C_DBLQUOTE)
 *	- if the name is in backquotes (mode&C_BACKQUOTE)
 * Often there will be nodes in the list with mode==0 & name==NULL, these
 * indicate words removed during meta and should be skipped by the parser.
 */

#define C_SPACE		002		/* Word has a space on the end */
#define C_QUOTE		004		/* Word in single quotes */
#define C_DBLQUOTE	010		/* Word in double quotes */
#define C_BACKQUOTE	020		/* Word in backquotes */
#define C_QUOTEBITS	034		/* Bits used for quotes */

/* Some words have name==NULL & mode!=0; these are special words for the
 * parser. They are separate, non-malloc'd, non-quoted words, and the bits
 * used above can be reused. The bits must be in C_WORDMASK
 */
#define C_WORDMASK	0740		/* Bits must fall in here */

#define C_SEMI		 040		/* The word is a semicolon */
#define C_DOUBLE	C_SEMI		/* Add this to `double' the symbol */
#define C_PIPE		0100		/* The word is a pipe */
#define C_DBLPIPE	0140		/* The word is a double pipe */
#define C_AMP		0200		/* The word is an ampersand */
#define C_DBLAMP	0240		/* The word is a double ampersand */
#define C_LT		0300		/* The word is a less-than. */
#define C_LTLT		0340		/* The word is two less-thans. */
#define C_FD		 017		/* File descriptor bits */
#define C_GT		0400		/* The word is a greater-than. Bits */
					/* in the mask C_FD hold the fd (0-9) */
#define C_GTGT		0440		/* The word is two greater-thans.Bits */
					/* in the mask C_FD hold the fd (0-9) */
struct candidate
	{ char *name;			/* The file's name */
	  struct candidate *next;	/* Next field in linked list */
	  int mode;			/* File's mode, or malloc'd bool */
	};

/* Execution
 *
 * The how parameter to execute() indicates how the process should be
 * executed. Most of the bits are currently defined but not used.
 */

#define H_APPEND        001             /* Process will append to outfd */
#define H_BCKGND        002             /* Process running in background */
#define H_PIPEDIN       004             /* Process has piped input */
#define H_PIPEDOUT      010             /* Process has piped output */
#define H_FROMFILE      020             /* Process has file input */
#define H_TOFILE        040             /* Process has file output */
 
/* Redirection
 *
 * The rdrct structure holds the new file descriptors for the process,
 * and their file names (if any). If a file descriptor is negative, the
 * respective file must be opened. If the fd values are not 0,1 and 2,
 * redirect() will dup them to be 0,1 and 2.
 */

struct rdrct {
        int infd;                       /* The input file descriptor */
        int outfd;                      /* The output file descriptor */
        int errfd;                      /* The error file descriptor */
        char *ifil;                     /* Input file's name */
        char *ofil;                     /* Output file's name */
        char *efil;                     /* Error file's name */
        };

/* Command Line Editing
 *
 * Several old routines have been subsumed by one routine, Show().
 * Unfortunately, these are used in comlined, clex and hist, so the
 * defines that map the old onto the new have to be out here.
 */

#define insert(a,b,c)		(void)Show(a,b,c,0)
#define show(a,c)		(void)Show(a,0,0,2)
#define goend(a,b)		Show(a,b,0,3)
#define yankprev(line,pos)	prevword(line,&pos,2)
#define Beep			write(1,beep,beeplength)
