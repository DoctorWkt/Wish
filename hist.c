#include "header.h"

/* The command history is stored as a singly linked list, each node holding
 * a command, its history number, and a pointer to the next node.
 */
struct histlist
{
  int hnum;
  char *hline;
  struct histlist *next;
};

struct histlist *htop=NULL;		/* The start of the history */
int curr_hist=1;
int maxhist=25;
bool nohistdup=TRUE;


/* Savehist saves the given line into the history with the given history
 * number, and ensuring that there are no more than max histories.
 * It returns 1 if saved, or 0 if a duplicate or other errors.
 */
int savehist(line)
  char *line;
{
  struct histlist *ptr,*old;
  extern bool nohistdup;

		/* Find the end of the list, or up to curr_hist */
  for (old=ptr=htop;ptr && ptr->hnum!=curr_hist;old=ptr,ptr=ptr->next);
  if (ptr)				/* change existing history */
  {
    if (nohistdup && !strcmp(old->hline,line))
      return(0);			/* don't save duplicates */

					/* otherwise modify the string */
    free(ptr->hline);
    ptr->hline=line;
  }
  else
    if (htop)				/* there is a history, isn't there? */
    {
#ifdef DEBUG
fprints(2,"nohistdup %d old %s line %s.\n",nohistdup,old->hline,line);
#endif
      if (nohistdup && !strcmp(old->hline,line))
	return(0);			/* don't save duplicates */
      old->next=ptr=(struct histlist *) malloc((unsigned)(sizeof(struct histlist))); 
      ptr->hline=line;
      ptr->hnum=curr_hist;
      ptr->next=NULL;
      while (curr_hist-(htop->hnum)>maxhist)	/* Free old histories */
      {
	ptr=htop;
	htop=htop->next;
	free(ptr->hline);free(ptr);
      }
    }
    else				/* no? well, let's make one then. */
    {
      htop=(struct histlist *) malloc ((unsigned)(sizeof(struct histlist)));
      htop->hline=line;
      htop->hnum=curr_hist;
      htop->next=NULL;
    }
  curr_hist++; return(1);
}

/* Loadhist finds the command line with the given histnum, and loads it
 * at the given pos into the command line.
 */
void loadhist(line,pos,histnum)
  char *line;
  int *pos,histnum;
{
  extern void go(),clrline();
  extern int lenprompt,Show();
  struct histlist *ptr;

  *pos=0;					/* pos at home */
  go(lenprompt,0);				/* goto start of line and */
  clrline(line,0);				/* clear it */
		/* Find either the last hist or the histnum one */
  for (ptr=htop;ptr && ptr->hnum!=histnum;ptr=ptr->next);
  if (ptr)
  {
    strcpy(line,ptr->hline);	/* copy into the command line */
    *pos= goend(line,*pos);
  }
  else;				/* we can't find that number so load nothing */
				/* maybe it should beep here */
}

/* Mprint is a small utility routine that prints out a line with control
 * characters converted to Ascii, e.g ^A etc. If nocr is true, no \n is
 * appended.
 */
#ifdef PROTO
void mprint ( char *line , int nocr )
#else
void mprint(line,nocr)
  char *line;
  int nocr;
#endif
{
  char buf[MAXLL];
  int i,j;

  for (j=0,i=0;line[i];i++,j++)
    if (line[i]<32 || line[i]==127)
    {
      buf[j++]='^';
      buf[j]=line[i]+64;
    }
    else buf[j]=line[i];
  buf[j]=EOS;
  write(1,buf,strlen(buf));
  if (!nocr) write(1,"\n",1);
}

/* The history builtin prints out the current history list.
 * None of the arguments are used.
 */
#ifdef PROTO
void history ( int argc; char *argv [])
#else
void history(argc,argv)
  int argc;
  char *argv[];
#endif
{
  struct histlist *ptr;
  extern int curr_hist,maxhist;

  for (ptr=htop;ptr && ptr->hnum<curr_hist;ptr=ptr->next)
    if (ptr->hnum>=curr_hist-maxhist)
    {
      prints("%d  ",ptr->hnum);
      mprint(ptr->hline,0);	       /* this routine intercepts the ^ chars */
    }
}

/* Getnumhist returns the command line with the given history number.
 * If none exists, NULL is returned.
 */
#ifdef PROTO
char *getnumhist ( int histnum )
#else
char *getnumhist(histnum)
  int histnum;
#endif
{
  struct histlist *ptr;

  for (ptr=htop;ptr;ptr=ptr->next)
    if (ptr->hnum==histnum)
      return(ptr->hline);
  return(NULL);
}

/* Gethist takes a event string, and returns a match from
 * the history. The event can be one of the following:
 *	   Value			Return value
 *	an integer		The command line with the given histnum
 *	"!"			The last command line
 *	-integer		The line integer histories ago
 *	word			The last line that strncmps the word
 *
 * If no event is found, NULL is returned.
 */
#ifdef PROTO
char *gethist ( char *event )
#else
char *gethist(event)
  char *event;
#endif
{
  extern int curr_hist;
  struct histlist *ptr;
  char *oldline;
  int histnum,f=0;

  if ((*event>='0') && (*event<='9') || (*event=='-') || (*event=='!'))
  {
    switch(*event)
    {
      case '-': histnum=curr_hist-atoi(event+1);
		break;
      case '!': histnum=curr_hist-1;
		break;
      default : histnum=atoi(event);
    }
#define DEBUG
#ifdef DEBUG
prints("Histnum is %d\n",histnum);
#endif
   return(getnumhist(histnum));
  }
  else
  {
    histnum=strlen(event);	/* double use of histnum to store len */
    oldline=NULL;

		/* If the history matches the requested event then that
		 * history line is copied into the event pointer. Note that
		 * the first histnum characters of event will not change and
		 * thus further searches for more recently matching histories
		 * will still be valid. f is set as a found flag.
		 */
    for (ptr=htop;ptr;ptr=ptr->next)
      if (!strncmp(event,ptr->hline,histnum))
      {
	oldline= ptr->hline;
	f=1;
      }
      return(oldline);
  }
}

#ifdef PROTO
int matchhist ( int histnum )
#else
int matchhist(histnum)
  int histnum;
#endif
{
  extern int curr_hist;
  struct histlist *ptr;
  char *histptr,temp[MAXLL];
  int lastmatch=(-1);

  if ((histptr=getnumhist(histnum))!=NULL)
    strncpy(temp,histptr,MAXLL);
  else return(0);
  for (ptr=htop;ptr && ptr->hnum<histnum;ptr=ptr->next)
    if (!strcmp(temp,ptr->hline))
      lastmatch=ptr->hnum;
			/* return the value of the next line after
			 * matching one. If none match return 0.
			 */
  return(lastmatch+1);
}

#ifdef PROTO
int matchhline ( char *line , int startnum )
#else
int matchhline(line,startnum)
  char *line;
  int startnum;
#endif
{
  extern int curr_hist;
  struct histlist *ptr;
  int lastmatch=0,ll;

  				/* finds match backwards for line */
  ll=strlen(line);
  for (ptr=htop;ptr && ptr->hnum<startnum;ptr=ptr->next)
    if (!strncmp(line,ptr->hline,ll))
      lastmatch=ptr->hnum;
  return(lastmatch);
}
