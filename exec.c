#include "header.h"

/*
 * Redirect redirects the file descriptors for input,output & error for the
 * current process, returning the new names of the old descriptors. It also
 * takes a bitmap indicating whether to append to the output file, and if the
 * process is running in the background. If oldfd pointf to NULL, the old
 * file descriptors are not saved. Returns -1 & errno on error.
 */
int 
redirect(newfd, oldfd, how)
    struct rdrct newfd, *oldfd;
    int how;
{
    int appnd, bckgnd;
    int fd, mode, flags;

#define Ifil	newfd.ifil
#define Ofil	newfd.ofil
#define Efil	newfd.efil
#define Infd	newfd.infd
#define Outfd	newfd.outfd
#define Errfd	newfd.errfd
#define Oldin	oldfd->infd
#define Oldout	oldfd->outfd
#define Olderr	oldfd->errfd

    bckgnd = how & H_BCKGND;
    appnd = how & H_APPEND;
    if (oldfd!=NULL)
     { Oldin=0; Oldout=1; Olderr=2; }

    if (Infd == 0 && bckgnd)	/* don't allow read from background */
    {
	strcpy(Ifil, "/dev/null");
	Infd = (-2);		/* now take input from /dev/null */
    }
    if (Infd != 0)		/* Redirect input */
    {
	if (oldfd != NULL)
	    if ((Oldin = dup(0)) == -1)
	    {
		fprintf(2, "dup: old stdin\n"); return (-1);
	    }
	close(0);
	if (Infd > 0)
	{
	    if (dup(Infd) != 0)	/* panic, did not dup stdin */
	    {
		fprintf(2, "dup: stdin\n"); return (-1);
	    }
	}
	else if (open(Ifil, O_RDONLY, 0) == -1)
	{
	    fprintf(2, "Can't open %s\n", Ifil); return (-1);
	}
    }
    if (Outfd != 1)		/* Redirect output */
    {
	if (oldfd != NULL)
	    if ((Oldout = dup(1)) == -1)
	    {
		fprintf(2, "dup: old stdout\n"); return (-1);
	    }
	close(1);
	if (Outfd > 1)
	{
	    if (dup(Outfd) != 1)/* panic, did not dup stdout */
	    {
		fprintf(2, "dup: stdout\n"); return (-1);
	    }
	}
	else
	{
#if defined(MINIX) || defined(COHERENT)
	    flags = O_WRONLY;	/* Minix has old open call */
	    mode = 0777;
	    errno = OK;		/* Set no current errors */
	    if ((!appnd) || (((open(Ofil, flags)) == -1) && (errno == EINVAL)))
	    {
		if (creat(Ofil, mode) == -1)
		{
		    fprintf(2, "Can't open %s\n", Ofil); return (-1);
		}
		if (open(Ofil, flags) == -1)
		{
		    fprintf(2, "Can't open %s\n", Ofil); return (-1);
		}
	    }
#else
	    flags = O_WRONLY | O_CREAT;
	    if (!appnd)
		flags |= O_TRUNC;
	    mode = 0777;
	    if (open(Ofil, flags, mode) == -1)
	    {
		fprintf(2, "Can't open %s\n", Ofil); return (-1);
	    }
#endif
	    if (appnd) lseek(1, 0L, 2);
	}
    }
#ifdef NOTYET
    if (Errfd != 2)		/* Redirect error */
    {
	if (oldfd != NULL)
	    if ((Olderr = dup(2)) == -1)
	    {
		fprintf(2, "dup: old stderr\n"); return (-1);
	    }
	close(2);
	if (Errfd > 2)
	{
	    if (dup(Errfd) != 2)
	    {
		fprintf(2, "dup: stderr\n"); return (-1);
	    }
	}
	else
	{
#if defined(MINIX) || defined(COHERENT)
	    flags = O_WRONLY;
	    mode = 0777;
	    errno = OK;		/* Set no current errors */
	    if (creat(Efil, mode) == -1)
	    {
		fprintf(2, "Can't open %s\n", Efil); return (-1);
	    }
	    if (open(Efil, flags) == -1)
	    {
		fprintf(2, "Can't open %s\n", Efil); return (-1);
	    }
#else
	    flags = O_WRONLY | O_CREAT | O_TRUNC;
	    mode = 0777;
	    if (open(Efil, flags, mode) == -1)
	    {
		fprintf(2, "Can't open %s\n", Efil); return (-1);
	    }
#endif
	}
    }
#endif
}


/* Invoke simple command. This takes the args to pass to the executed
 * process/builtin, the list of new file descriptors, and a bitmap
 * indicating how to run the process. We only use the append and
 * background bits at the moment
 */

int invoke(argc,argv,newfd,how)
 int argc,how;
 char *argv[];
 struct rdrct newfd;
 {
  int pid;
  BOOLEAN builtin();
  struct rdrct oldfd;

  redirect(newfd,&oldfd,how);
  if (argc==0 || builtin(argc,argv,newfd)) { redirect(oldfd,0,0); return(0); }

  switch(pid=fork())
   {
    case -1: printf("Can't create new process\n");
	     return;
    case 0:  if (!(how & H_BCKGND)) entrysig();
             if (!EVupdate())
		fatal("Can't update environment");
	     execvp(argv[0],argv);
	     fprintf(stderr,"Can't execute %s\n",argv[0]);
	     exit(0);
    default: redirect(oldfd,0,0);
	     return(pid);
   }
 }
