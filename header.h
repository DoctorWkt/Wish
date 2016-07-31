/* This is at the moment a sever mangle of the Clam header.h
 * I will tidy it up one day.
 * Note most structures are in the files where they need it, unless
 * routines from different files need the structs.
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
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
# include <dirent.h>
# include <termio.h>
# include <sys/file.h>
# include <sys/time.h>
#endif

#ifdef MINIX
# include <sgtty.h>
# include <dirent.h>
# include <fcntl.h>
# include <time.h>
#  ifndef ATARI_ST
#   include <sys/dir.h>
#  endif
#endif

#ifdef COHERENT
# include <string.h>
# include <dirent.h>
# include <sgtty.h>
# include <sys/fcntl.h>
# include <time.h>
#endif

#ifdef UCB
# include <sgtty.h>
# include <sys/dir.h>
# include <sys/time.h>
# include <sys/file.h>
# include <sys/resource.h>
#  ifdef SUN
#   define mc68000 1
#   include <sys/wait.h>
#  else
#   include <wait.h>
#  endif
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif
#define UNDEF -1
#define EOS 0

#ifndef MAXSIG
#define MAXSIG 27
#endif

#define BADFD -2
#define MAXARG 30
#define MAXWORD 200
#define MAXFNAME 200
#define MAXPL   128                    /* path length */
# define MAXLL  2048


typedef enum {FALSE,TRUE} bool;
typedef enum {T_WORD,T_BAR,T_AMP,T_SEMI,T_GT,T_GTGT,T_LT, T_NL,T_EOF} TOKEN;

#define fatal(mess) { fprintf(stderr,"%s\n",mess); exit(1); }

#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w) >> 8)

/* Execution */

/* The how parameter to execute() indicates how the process should be
 * executed. Most of the bits are currently defined but not used.
 */

#define H_APPEND        001             /* Process will append to outfd */
#define H_BCKGND        002             /* Process running in background */
#define H_PIPEDIN       004             /* Process has piped input */
#define H_PIPEDOUT      010             /* Process has piped output */
#define H_FROMFILE      020             /* Process has file input */
#define H_TOFILE        040             /* Process has file output */
 
/* Redirection */

/* The rdrct structure holds the new file descriptors for the process,
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
