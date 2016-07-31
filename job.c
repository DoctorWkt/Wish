#include "header.h"

#ifdef JOB
int Headpid;			/* The process we are waiting on */
union wait Headwait;		/* The returned value of the Headpid */
#endif

void statusprt(pid,status)	/* Interpret status code */
 int pid;
#ifdef JOB
 union wait status;
#else
 int status
#endif
 {
  int code;
  static char *sigmsg[]= { "","Hangup","Interrupt","Quit",
                        "Illegal Instruction","Trace/BPT Trap","IOT Trap",
                        "EMT Trap","Floating Point Exception","Killed",
                        "Bus Error","Segmentation Violation","Bad System Call",
                        "Broken Pipe","Alarm", "Terminated"
#ifdef JOB
			,"Urgent Socket Condition","Stopped (signal)",
                        "Stopped","Continue", "Child Status Change",
                        "Stopped (tty input)","Stopped (tty output)","I/O",
                        "Cpu Time Limit","File Size Limit",
                        "Virtual Time Alarm","Profile Alarm"
#endif
			};

#ifdef JOB
  if (status.w_stopval==WSTOPPED)
   {
    prints("Process %d ",pid);
    if (status.w_stopsig<MAXSIG)
	prints(sigmsg[status.w_stopsig]);
    else prints("signal %d",status.w_stopsig);
   }
  else
   {
    if (status.w_termsig>=MAXSIG) return;
    if (pid!=0) prints("Process %d: ",pid);
    if (status.w_retcode!=0) prints("exited %d ",status.w_retcode);
    if (status.w_termsig<MAXSIG)
	prints(sigmsg[status.w_termsig]);
    else prints("signal %d",status.w_termsig);
    if (status.w_coredump) prints(" (core dumped)");
   }
    prints("\n");
#else
  if (status!=0 && pid!=0)
    prints("Process %d: ",pid);
  if lowbyte(status)==0)
    {
     if ((code==highbyte(status))!=0) prints("Exit code %d\n",code);
    }
  else
    {
     if ((code = status & 0177)<=MAXSIG)
	prints("%s",sigmsg[code]);
     else
	prints("Signal %d",code);
     if ((status & 0200)==0200)
	prints("-core dumped");
     prints("\n");
    }
#endif
 }

void waitfor(pid)	/* Wait for child */
 int pid;
 {
  int wpid;
#ifndef JOB
  int status;

  while((wpid=wait(&status))!=pid && wpid!=-1)
	statusprt(wpid,status);
  if (wpid==pid)
	statusprt(0,status);
#else

  Headwait.w_status=0;		/* Set status we're waiting on to 0 */
				/* Pause until checkjobs sets it */
#ifdef DEBUG
  prints("paused\n");
#endif
  while (Headpid!=0)
   { pause();
#ifdef DEBUG
     prints("Headpid is now %d\n",Headpid);
#endif
   }
#endif
 }
  
#ifdef JOB
/* Checkjobs is only called when SIGCHLD is sent. It receives the new status
 * of the child, and prints that out. If it's the child we are pause()d on,
 * it sets Headwait, thus breaking out of the pause() loop.
 */
void checkjobs()
 {
  union wait status;
  int wpid;
  struct rusage rusage;
				/* Get status of our children */
  signal(SIGCHLD,checkjobs);
#ifdef DEBUG
  prints("In checkjobs\n");
#endif
  while ((wpid=wait3(&status,WNOHANG|WUNTRACED,&rusage))>0)
   {
#ifdef DEBUG
    printf("--- Pid %d status %x\n",wpid,status);
#endif
    if (wpid==Headpid)            /* Set Headwait if we're pause()d on it */
        { statusprt(0,status);
          Headwait=status;
          Headpid=0;
 	}
    else statusprt(wpid,status); /* and print them out for now */
   }
 }


/* Stopjob is only called when we received a SIGTSTP. If we are pause()d on
 * a pid, we send a SIGSTOP to that pid. This should then cause checkjobs()
 * to be called, which will alter Headwait.
 */
void stopjob()
 {
  signal(SIGTSTP,stopjob);
  if (Headpid!=0)
    {
#ifdef DEBUG
     prints("About to stop pid %d\n",Headpid);
#endif
     kill(Headpid,SIGSTOP);
    }
 }
#endif

/* Builtins */
void bg(argc,argv)
 int argc;
 char *argv[];
 {
  int pid;

  if (argc!=2) prints("usage: bg pid\n");
  else
   {
    pid=atoi(argv[1]);
    setpgrp(pid,pid);
#ifdef DEBUG
    prints("About to SIGCONT %d\n",pid);
#endif
    kill(pid,SIGCONT);
   }
 }

void fg(argc,argv)
 int argc;
 char *argv[];
 {
  int pid;

  if (argc!=2) prints("usage: fg pid\n");
  else
   {
    pid=atoi(argv[1]);
#ifdef DEBUG
    prints("About to setpgrp pid%d to us\n",pid);
#endif
    if (setpgrp(pid,getpgrp(0))==-1)  /* set process group to the shell's */
     { perror("fg setpgrp"); }
#ifdef DEBUG
    prints("About to SIGCONT %d\n",pid);
#endif
    kill(pid,SIGCONT);
    Headpid=pid;
   }
 }
