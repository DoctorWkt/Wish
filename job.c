#include "header.h"

#ifndef MAXSIG
#define MAXSIG 27
#endif

void statusprt(pid,status)	/* Interpret status code */
 int pid;
  union wait status;
 {
  int code;
  static char *sigmsg[]= { "","Hangup","Interrupt","Quit",
                        "Illegal Instruction","Trace/BPT Trap","IOT Trap",
                        "EMT Trap","Floating Point Exception","Killed",
                        "Bus Error","Segmentation Violation","Bad System Call",
                        "Broken Pipe","Alarm", "Terminated"
			,"Urgent Socket Condition","Stopped (signal)",
                        "Stopped","Continue", "Child Status Change",
                        "Stopped (tty input)","Stopped (tty output)","I/O",
                        "Cpu Time Limit","File Size Limit",
                        "Virtual Time Alarm","Profile Alarm"
			};
#define mc68000 1

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
 }

void waitfor(pid)	/* Wait for child */
 int pid;
 {
  struct rusage rusage;
  int wpid;
  union wait status;

#ifdef OLDCODE
  while((wpid=wait(&status))!=pid && wpid!=-1)
	statusprt(wpid,status);
  if (wpid==pid)
	statusprt(0,status);
#endif
  while ((wpid=wait3(&status,WNOHANG|WUNTRACED,&rusage))!=pid && wpid!=-1)
	statusprt(wpid,status);
  if (wpid==pid)
	statusprt(0,status);
 }


void checkjobs()
{
  int wpid;

  struct rusage rusage;
  union wait status;

  wpid=wait3(&status,WNOHANG|WUNTRACED,&rusage);
  printf("Got wpid %d, status %x in checkjobs\n",wpid,status);
  signal(SIGCHLD,checkjobs);
}


void stopjob()
 {
  extern int Headpid;		/* The pid we want to stop */

  if (Headpid!=0)
    {
     kill(Headpid,SIGSTOP);
     printf("Just stopped pid %d\n",Headpid);
    }
 }
