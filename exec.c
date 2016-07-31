#include "header.h"

/* Dupup dups the file descriptors 0,1,2 up to 10,11,12. It returns -1 if
 * an error occurs. 
 */

static char dupdone=FALSE;	/* We only ever dupup once */
int dupup()
 {
  if (dupdone==TRUE) return(0);
  if (dup2(0,10)==-1 || dup2(1,11)==-1 || dup2(2,12)==-1)
    { close(10);
      close(11); close(12);
fprints(2,"Could not dup 10,11 or 12\n");
      return(-1); }
  dupdone=TRUE; return(0);
 }


/* Dupdown dups the file descriptors 10,11,12 up to 0,1,2.
 */
void dupdown()
 {
  if (dupdone==FALSE) return;
  close(0); close(1); close(2);
  dup2(10,0); dup2(11,1); dup2(12,2);
  close(10); close(11); close(12);
  dupdone=FALSE;
 }


/*
 * Redirect redirects the file descriptors for fds 0 to 9 for the
 * current process; for each fd it also takes a bitmap indicating whether
 * to append to the output file, or to open from an input file.
 * Returns -1 & errno on error.
 */
int 
redirect(newfd)
    struct rdrct newfd[10];
{
    int i, appnd;
    int mode, flags;

#ifdef DEBUG
fprints(12,"Redirect...\n");
#endif

  for (i=0; i<10; i++)		/* Do each fd in turn */
   {

    appnd = newfd[i].how & H_APPEND;

    if (newfd[i].fd>2)			/* Fd is already open */
      { close(i);
        if (dup2(newfd[i].fd,i)==-1)
	  { fprints(12,"Could not dup2(%d,%d)\n",newfd[i].fd,i);
            return(-1);
	  }
        close(newfd[i].fd);
      }
    else if (newfd[i].file!=NULL)	/* Open this file */
      { close(i);
        if (newfd[i].how & H_FROMFILE)	/* for reading */
         {
	    flags= O_RDONLY;
	    if (open(newfd[i].file, flags, mode) == -1)
	      { fprints(12, "Can't open %s\n", newfd[i].file); return (-1); }
         }
        else				/* for writing */
         {
	    flags = O_WRONLY | O_CREAT;
	    if (!appnd)
	        flags |= O_TRUNC;
	    mode = 0777;
	    if (open(newfd[i].file, flags, mode) == -1)
	    { fprints(12, "Can't open %s\n", newfd[i].file); return (-1); }
	    if (appnd) lseek(1, 0L, 2);
         }
      }
   }
}

/* Runalias checks to see if there is an alias, and then runs it.
 * It either returns the return value of the alias, or -1 if there
 * was no alias.
 *
 * CURRENT ASSUMPTION. We were fork'd, thus we can rearrange Argv.
 */
int runalias(argc, argv)
 int argc;
 char *argv[];
 {
  extern bool getaliasline(), (*getline)();
  extern int Argc;
  extern char **Argv;
  struct adefn *checkalias();
  bool (*oldgetline)();
  void catchsig();
  int nosave;
  
  catchsig();		/* Set back to Clam's defaults, in case of fork() */

  Argc= argc; Argv=argv;

  if (checkalias(argv[0])!=NULL)
    {
#ifdef DEBUG
fprints(11,"About to send the alias through doline()\n");
#endif
	oldgetline= getline;
	getline= getaliasline;
	while(doline());
	getline= oldgetline;
	return(0);
    }
  else return(-1);
 }


/* Invoke simple command. This takes the args to pass to the executed
 * process/builtin, the list of new file descriptors, and a bitmap
 * indicating how to run the process. We only use the append and
 * background bits at the moment
 */

int invoke(argc,argv,newfd,how)
 int argc,how;
 char *argv[];
 struct rdrct newfd[];
 {
  int pid,i;
  int builtin();

			/* Firstly redirect the input/output */
  redirect(newfd);
			/* Try builtins & unredirect if yes */
  if (argc==0 || (builtin(argc,argv)!=-1))
    { return(0); }

			/* Else fork/exec the process */
  switch(pid=fork())
   {
    case -1: fprints(12,"Can't create new process\n");
	     return;
			/* Restore signals to normal if fg */
    case 0:  if (!(how & H_BCKGND)) dflsig();
	     else
	       {	/* Move process to new proc-grp if bg */
#ifdef JOB
#ifdef DEBUG
		fprints(11,"About to setpgrp on bckgnd process\n");
#endif
    		setpgrp(0,getpid());
#endif
	       }
             if (!EVupdate())
		fatal("Can't update environment");
			/* The fork only wants fds 0,1,2 */
	     for (i=3; i<20; i++) close(i);
			/* Finally exec() the beast */

	     if ((i=runalias(argc,argv))!=-1) exit(i);

	     /* fprints(12,"Execing %s as pid %d\n",argv[0],getpid()); */
	     execvp(argv[0],argv);
			/* Failed, exit the child */
	     fprints(12,"Can't execute %s\n",argv[0]);
	     exit(0);
			/* Unredirect our I/O */
    default: 
#ifdef DEBUG
fprints(11,"Just forked %s, pid %d\n",argv[0],pid);
#endif
#ifdef JOB
	     addjob(pid,argv[0]);
#endif
			/* and return the new pid */
	     return(pid);
   }
 }
