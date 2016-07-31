#include "header.h"

int Headpid;			/* The process we are waiting on */
#ifdef JOB
union wait Headwait;		/* The returned value of the Headpid */

/* The job structure hold the information needed to manipulate the
 * jobs, using job numbers instead of pids.
 */
struct job
{
  int jobnumber;		/* The job number */
  int pid;			/* The pid of the job */
  char *name;			/* Job's argv[0]; */
  /* bool rdrout;		/* Was it redirected? */
  char *dir;			/* The job's working directory */
  union wait status;		/* Job's status */
  long lastmod;			/* Time of last modification? */
  struct job *next;		/* Pointer to next job */
};

struct job *jtop=NULL,*exitop,*currentjob;
#endif


void statusprt(pid,status)	/* Interpret status code */
 int pid;
#ifdef JOB
 union wait status;
#else
 int status;
#endif
 {
  int code;
  static char *siglist[]= { "","Hangup","Interrupt","Quit",
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
	prints(siglist[status.w_stopsig]);
    else prints("signal %d",status.w_stopsig);
   }
  else
   {
    if (status.w_termsig>=MAXSIG) return;
    if (pid!=0) prints("Process %d: ",pid);
    if (status.w_retcode!=0) prints("exited %d ",status.w_retcode);
    if (status.w_termsig<MAXSIG)
	prints(siglist[status.w_termsig]);
    else prints("signal %d",status.w_termsig);
    if (status.w_coredump) prints(" (core dumped)");
   }
    prints("\n");
#else
  if (status!=0 && pid!=0)
    prints("Process %d: ",pid);
  if (lowbyte(status)==0)
    {
     if ((code=highbyte(status))!=0) prints("Exit code %d\n",code);
    }
  else
    {
     if ((code = status & 0177)<=MAXSIG)
	prints("%s",siglist[code]);
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
/* Everything below this point is only used when JOB is defined.
 */
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

/* Print out the list of current jobs and their status */
#ifdef PROTO
void joblist ( void )
#else
void joblist()
#endif
{
  struct job *ptr;

  for (ptr=jtop;ptr;ptr=ptr->next)
  {
    if (currentjob->pid==ptr->pid)
      prints("    + [%d] %d",ptr->jobnumber,ptr->pid);
    else
      prints("      [%d] %d",ptr->jobnumber,ptr->pid);
    if (ptr->status.w_stopval==WSTOPPED)
	prints(" %s   ",siglist[ptr->status.w_stopsig]);
    prints(" %s\n",ptr->name);
  }
}


/* Given a process id, return a pointer to it's job structure */
#ifdef PROTO
struct job *findjob ( int pid )
#else
struct job *findjob(pid)
  int pid;
#endif
{
  struct job *ptr;

  for (ptr=jtop;ptr;ptr=ptr->next)
    if (ptr->pid==pid) return(ptr);
  return(0);
}


/* Given a job number, return it's process id */
#ifdef PROTO
int pidfromjob ( int jobno )
#else
int pidfromjob(jobno)
  int jobno;
#endif
{
  struct job *ptr;

  for (ptr=jtop;ptr;ptr=ptr->next)
    if (ptr->jobnumber==jobno) return(ptr->pid);
  return(-1);
}


/* Add the pid and it's argv[0] to the job list */
#ifdef PROTO
int addjob ( int pid , char *name )
#else
int addjob(pid,name)
  int pid;
  char *name;
#endif
{
  int jobno,diff=(-1);
  struct job *ptr,*old,*new;
  char execdir[MAXPL];

  for (old=ptr=jtop,jobno=1;ptr;jobno++,old=ptr,ptr=ptr->next)
    if (jtop->jobnumber>1 || (diff=old->jobnumber-ptr->jobnumber+1)<0) break;
  if (diff<0)
    if (jobno!=1)			/* insertion between old and ptr */
    {
      new=(struct job *) malloc ((unsigned)(sizeof(struct job)));
      if (new==NULL)
      {
        perror("addjob");
	return(jobno);
      }
      old->next=new;
      new->next=ptr;
      ptr=new;
    }
    else			/* insertion before jtop */
    {
      old=(struct job *) malloc ((unsigned)(sizeof(struct job)));
      if (old==NULL)
      {
        perror("addjob");
	return(jobno);
      }
      old->next=jtop;
      ptr=jtop=old;
    }
  else					/* append at end */
  {
    ptr=(struct job *) malloc ((unsigned)(sizeof(struct job)));
    if (ptr==NULL)
    {
      perror("addjob");
      return(jobno);
    }
    old->next=ptr;
    ptr->next=0;
  }
  ptr->jobnumber=jobno;
  ptr->pid=pid;
  ptr->name=(char *) malloc ((unsigned)(strlen(name)+1));
  if (ptr->name)
    (void) strcpy(ptr->name,name);
  else
      perror("addjob");
  ptr->status.w_status=0;
  ptr->dir=(char *) malloc ((unsigned)(strlen(cwd)+1));
  if (ptr->dir)
    (void) strcpy(ptr->dir,cwd);
  else
    perror("addjob");
  ptr->lastmod=time((long *)0);
  currentjob=ptr;			/* a new current job */
#ifdef DEBUG
fprints(2,"added jobno %d pid %d\n",jobno,pid);
#endif
  return(jobno);
}

/* Given a process id, remove the matching job from the job list */
#ifdef PROTO
void rmjob ( int pid )
#else
void rmjob(pid)
  int pid;
#endif
{
  struct job *ptr,*old;
  int i;

#ifdef DEBUG
fprints(2,"removing pid %d from list\n",pid);
#endif
  for (i=0,old=ptr=jtop;ptr;i++,old=ptr,ptr=ptr->next)
    if (ptr->pid==pid)
      if (i)
      {
	old->next=ptr->next;
	free(ptr->name);
	free(ptr);
	break;
      }
      else
      {
	jtop=jtop->next;
	free(ptr->name);
	free(ptr);
	break;
      }
}

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
#endif /* JOB */
