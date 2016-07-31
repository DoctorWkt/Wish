#include "header.h"

#ifdef JOB
#include <termio.h>	/* Need this for TCGETA definition */

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
bool jchange=FALSE;		/* Has a job changed state? */
#endif

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

struct job *findjob();
void rmjob();

void statusprt(pid,status)	/* Interpret status code */
 int pid;
#ifdef JOB
 union wait status;
#else
 int status;
#endif
 {
  int code;

#ifdef JOB
  if (status.w_stopval!=WSTOPPED && status.w_termsig)
   {
    if (status.w_termsig>=MAXSIG) return;
    if (status.w_retcode!=0) prints("exited %d ",status.w_retcode);
    prints(siglist[status.w_termsig]);
    if (status.w_coredump) prints(" (core dumped)");
    prints("\n");
   }
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

#ifndef JOB
void waitfor(pid)	/* Wait for child */
 int pid;
 {
  int wpid;
  int status;

  while((wpid=wait(&status))!=pid && wpid!=-1)
	statusprt(wpid,status);
  if (wpid==pid)
	statusprt(0,status);
 }
#endif

#ifdef JOB
/* Under BSD, instead of using a wait(), we merely pause(). We are woken
 * up when a signal arrives. If it was a SIGCHLD, checkjobs() caught it.
 * If it was the current job, checkjobs() will have modified the ptr or
 * the status. This is how we escape from the loop.
 */
void waitfor(pid)       /* Wait for child */
 int pid;
 {
  struct job *findjob();

  if (pid>0)
    {
     if (currentjob && currentjob->status.w_stopval!=WSTOPPED &&
						currentjob->pid!=pid)
      { prints("Looney! I'm waiting on pid %d, how can I wait on %d ?\n",
	   currentjob->pid,pid);
        return;
      }
     else currentjob=findjob(pid);
    }
  while (currentjob && currentjob->status.w_stopval!=WSTOPPED)
   {
#ifdef DEBUG
    prints("paused\n");
#endif
    pause();
   }
 }


/* Everything below this point is only used when JOB is defined.
 */

/* Setownterm sets the user's terminal as being owned by the
 * the given process group id. Processes that are not in this
 * process group are sent SIGTTOUs if they try to write to it.
 *
 * @@@ NOTE @@@ This isn't currently being used, because it stops
 *		the shell from working under script etc. I'm just
 *		leaving it here until I'm sure I don't need it.
 */
setownterm(pid)
 int pid;
 {
  if (pid)
   {
    if (ioctl(0,TIOCSPGRP,&pid)) perror("ioctl spg");
   }
 }


/* Settou sets up the terminal so that SIGTTOUs are sent to
 * processes which are not members of the owning process group
 */
void settou()
 {
  struct termio tbuf;
 
  if (ioctl(0,TCGETA,&tbuf)) perror("ioctl in settou");
  tbuf.c_lflag |= TOSTOP;
  if (ioctl(0,TCSETA,&tbuf)) perror("ioctl s");
 }


/* Checkjobs is only called when SIGCHLD is sent. It receives the new status
 * of the child, and prints that out. If it's the child we are pause()d on,
 * rmjob will set currentjob to NULL, thus breaking out of the loop.
 */
void checkjobs()
 {
  union wait status;
  int wpid;
  struct rusage rusage;
  struct job *thisjob;
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
    thisjob=findjob(wpid);
    thisjob->status=status;
    jchange=TRUE;
    statusprt(wpid,status);
    if (status.w_stopval!=WSTOPPED) rmjob(wpid);
   }
 }


/* Stopjob is only called when we received a SIGTSTP. If we are pause()d on
 * a pid, we send a SIGSTOP to that pid. This should then cause checkjobs()
 * to be called, which will break out of the pause() loop.
 */
void stopjob()
 {
  signal(SIGTSTP,stopjob);
#ifdef DEBUG
  prints("In stopjobs\n");
#endif
  if (currentjob)
    {
#ifdef DEBUG
     prints("About to stop pid %d\n",currentjob->pid);
#endif
     kill(currentjob->pid,SIGSTOP);
    }
 }

/* Print out the list of current jobs and their status.
 * Note although this is a builtin, it is called from
 * main with an argc value of 0, to show new jobs only.
 */
void joblist(argc,argv)
 int argc;
 char *argv[];
{
  struct job *ptr;

  if (argc>1) { prints("Usage: jobs\n"); return; }
  if (argc==0 && jchange==FALSE) return;
  for (ptr=jtop;ptr;ptr=ptr->next)
  {
    if (currentjob && currentjob->pid==ptr->pid)
      prints("    + [%d] %d",ptr->jobnumber,ptr->pid);
    else
      prints("      [%d] %d",ptr->jobnumber,ptr->pid);
    if (ptr->status.w_stopval==WSTOPPED)
	prints(" %s   ",siglist[ptr->status.w_stopsig]);
    prints(" %s\n",ptr->name);
  }
  jchange=FALSE;
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


/* Add the pid and it's argv[0] to the job list.
 *
 * @@@ Callum, surely this can be tidied up a bit ???
 */
#ifdef PROTO
int addjob ( int pid , char *name )
#else
int addjob(pid,name)
  int pid;
  char *name;
#endif
{
  extern char currdir[];
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
  ptr->dir=(char *) malloc ((unsigned)(strlen(currdir)+1));
  if (ptr->dir)
    (void) strcpy(ptr->dir,currdir);
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
	if (ptr==currentjob) currentjob=NULL;
	free(ptr);
	break;
      }
      else
      {
	jtop=jtop->next;
	free(ptr->name);
	if (ptr==currentjob) currentjob=NULL;
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
  char *c;

  if (argc>2) { prints("usage: bg [pid]\n"); return; }
  if (argc==1)
   {
#ifdef DEBUG
prints("Trying to use current job pointer\n");
#endif
    if (currentjob)
    {
      pid=currentjob->pid;
    }
   }
  else
   {
    c= argv[1];
    if (*c=='%') pid=pidfromjob(atoi(++c));
    else pid=atoi(c);
   }
  if (pid<1)
   { fprints(2,"No such job number: %s\n",c); return; }
  setpgrp(pid,pid);
#ifdef DEBUG
  prints("About to SIGCONT %d\n",pid);
#endif
  kill(pid,SIGCONT);
 }


void fg(argc,argv)
 int argc;
 char *argv[];
 {
  extern char currdir[];
  int pid;
  struct job *ptr;
  char *c;

  if (argc>2) { prints("usage: fg [pid]\n"); return; }
  if (argc==1)
   {
#ifdef DEBUG
prints("Trying to use current job pointer\n");
#endif
    if (currentjob)
    {
      pid=currentjob->pid;
    }
   }
  else
   {
    c= argv[1];
    if (*c=='%') pid=pidfromjob(atoi(++c));
    else pid=atoi(c);
   }
  if (pid<1)
   { fprints(2,"No such job number: %s\n",c); return; }
#ifdef DEBUG
  prints("About to setpgrp pid%d to us\n",pid);
#endif
  if (setpgrp(pid,getpgrp(0))==-1)  /* set process group to the shell's */
   { perror("fg setpgrp"); return; }

  ptr=findjob(pid);
  if (strcmp(ptr->dir,currdir))		/* If directory has changed */
    fprints(2,"[%d] %d %s (wd now: %s)\n",ptr->jobnumber,ptr->pid,
					ptr->name,ptr->dir);
  else
    fprints(2,"[%d] %d %s\n",ptr->jobnumber,ptr->pid,ptr->name);
  ptr->status.w_status=0;
  currentjob=ptr;
					/* and finally wake them up */
#ifdef DEBUG
  prints("About to SIGCONT %d\n",pid);
#endif
  kill(pid,SIGCONT);
 }
#endif /* JOB */
