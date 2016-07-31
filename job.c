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
  if (status.w_stopval==WSTOPPED && pid!=0)
    printf("Process %d stopped by signal %d\n",pid,status.w_stopsig);
  else
   {
    if (status.w_termsig>=MAXSIG) return;
    if (pid!=0) printf("Process %d: ",pid);
    if (status.w_retcode!=0) printf("exited %d ",status.w_retcode);
    if (status.w_termsig<MAXSIG)
	printf(sigmsg[status.w_termsig]);
    else printf("signal %d",status.w_termsig);
    if (status.w_coredump) printf(" (core dumped)");
    printf("\n");
   }
#else
  if (status!=0 && pid!=0)
    printf("Process %d: ",pid);
  if lowbyte(status)==0)
    {
     if ((code==highbyte(status))!=0) printf("Exit code %d\n",code);
    }
  else
    {
     if ((code = status & 0177)<=MAXSIG)
	printf("%s",sigmsg[code]);
     else
	printf("Signal %d",code);
     if ((status & 0200)==0200)
	printf("-core dumped");
     printf("\n");
    }
#endif
 }

void waitfor(pid)	/* Wait for child */
 int pid;
 {
  int wpid;
#ifndef JOB

  while((wpid=wait(&status))!=pid && wpid!=-1)
	statusprt(wpid,status);
  if (wpid==pid)
	statusprt(0,status);
#else
  union wait status;

  Headwait.w_status=0;		/* Set status we're waiting on to 0 */
				/* Pause until checkjobs sets it */
  while (Headwait.w_status==0) pause();
#endif
 }
  
#ifdef JOB
/* Checkjobs is only called when SIGCHLD is sent. It receives the new status
 * of the child, and prints that out. If it's the child we are pause()d on,
 * it sets Headwait, thus breaking out of the pause() loop.
 */
void checkjobs()

 {
				/* Get status of our children */
  while ((wpid=wait3(&status,WNOHANG|WUNTRACED,&rusage))!=Headpid && wpid!=-1)
	statusprt(wpid,status); /* and print them out for now */
  if (wpid==Headpid)		/* Set Headwait if we're pause()d on it */
	{ statusprt(0,status); Headwait=status; }
  signal(SIGCHLD,checkjobs);
 }


/* Stopjob is only called when we received a SIGTSTP. If we are pause()d on
 * a pid, we send a SIGSTOP to that pid. This should then cause checkjobs()
 * to be called, which will alter Headwait.
 */
void stopjob()
 {
  if (Headpid!=0)
    {
     kill(Headpid,SIGSTOP);
     printf("Just stopped pid %d\n",Headpid);
    }
  signal(SIGTSTP,stopjob);
 }
#endif
