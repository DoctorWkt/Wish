#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define BADFD -2
#define MAXARG 30
#define MAXWORD 200
#define MAXFNAME 200
#define MAXPL   128                    /* path length */

typedef enum {FALSE,TRUE} BOOLEAN;
typedef enum {T_WORD,T_BAR,T_AMP,T_SEMI,T_GT,T_GTGT,T_LT, T_NL,T_EOF} TOKEN;

#define fatal(mess) { fprintf(stderr,"%s\n",mess); exit(1); }

#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w) >> 8)

/* Execution */

/* The how parameter to execute() indicates how the process should be
 * executed.
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
