/*****************************************************************************
**                                                                          **
**          The Clam Shell is Copyright (C) 1988 by Callum Gibson.          **
**       This file is part of Clam Shell. You may freely use, copy and      **
**     distribute it, but if you alter any source code do not distribute    **
**   the altered copy. i.e. you may alter this file for your own use only.  **
**                                                                          **
*****************************************************************************/
/******************************************************************************
**                                                                           **
**                                comlined.c                                 **
**                 This file contains the COMmand LINe EDitor.               **
**                                                                           **
******************************************************************************/

#include "header.h"


/* List of commands defined for the command line editor */

#define MARK		256	/* Save position, make a mark */
#define START		257	/* Go to start of line */
#define BAKCH		258	/* Go back one character */
#define INT		259	/* Interrupt this task */
#define DELCH		260	/* Delete the current character */
#define END		261	/* Goto the end of the line */
#define FORCH		262	/* Go forward one char */
#define KILLALL		263	/* Kill the whole line */
#define BKSP		264	/* Backspace over previous character */
#define COMPLETE	265	/* Complete the current word */
#define FINISH		266	/* Finish and execute the line */
#define KILLEOL		267	/* Kill from cursor to end of line */
#define CLREDISP	268	/* Clear screen & redisplay line */
#define NEXTHIST	270	/* Step forward in history */
#define OVERWRITE	271	/* Toggle insert/overwrite mode */
#define BACKHIST	272	/* Step backward in history */
#define XON		273	/* Resume tty output */
#define REDISP		274	/* Redisplay the line */
#define XOFF		275	/* Stop tty output */
#define TRANSPCH	276	/* Transpose current & previous characters */
#define LOOP		277	/* Do a command n times */
#define QUOTE		278	/* Quote next character literally */
#define DELWD		279	/* Delete word backwards */
#define GOMARK		280	/* Goto a mark */
#define YANKLAST	281	/* Yank the previous word into a buffer */
#define SUSP		282	/* Suspend process */

#define MATCHPART	291	/* Match a previous partial command */
#define BAKWD		292	/* Go backwards one word */
#define DELWDF		293	/* Delete word forwards */
#define FORWD		294	/* Go forwards one word */
#define GETHELP		295	/* Get help on a word */
#define PUT		296	/* Insert buffer on the line */
#define YANKNEXT	297	/* Yank the next word into the buffer */
#define SEARCHF		298	/* Search forward for next typed character */
#define SEARCHB		299	/* Search backwards for next typed character */


extern char termcapbuf[];
extern FILE *zin,*zout,*fopen();
extern int errno;

struct keybind {
	char *key;		/* The key we have bound */
	int len;		/* The length of the key */
	int cmd;		/* The CLE command it does */
	struct keybind *next;
	};

static int Keylength=0;			/* The maximum key length */
static struct keybind *Bindhead=NULL;	/* List of bindings */

#ifdef NOTYET
#ifdef __STDC__
void Bind ( int argc , char *argv [])
#else
void Bind(argc, argv)
 int argc;
 char *argv[];
#endif
 {
  int s,cmd,showall;
  char *key, *space= "                     ";
  struct keybind *temp, *Bindtail;

  if (argc>3)
    { fprints(2,"usage: bind [key [value]]\n"); return; }
  showall=0;
  switch(argc)
   {
    case 3: key=argv[1]; cmd=atoi(argv[2]);
	    s=strlen(key);		/* Get the key's length */
	    if (s==0) break;

	    temp=(struct keybind *)malloc(sizeof(struct keybind));
	    if (!temp) { perror("malloc"); return; }
	    temp->key= (char *)malloc(s+1);
	    if (!(temp->key)) { perror("malloc"); return; }
					/* Copy the key */
	    strcpy(temp->key,key);
	    temp->len=s;
	    temp->cmd=cmd;
	    temp->next=NULL;
	    if (s>Keylength) Keylength=s;
  					/* Add to linked list */
	    if (!Bindhead)
		Bindhead=temp;
	    else
	      { for (Bindtail=Bindhead;Bindtail->next;Bindtail=Bindtail->next);
		Bindtail->next=temp;
	      }
	    break;
    case 1: showall=1;
    case 2: for (temp=Bindhead;temp;temp=temp->next)
	      if (showall || !strcmp(temp->key,key))
		{ mprint(temp->key,1);
		  write(1,space,1 + Keylength - temp->len);
		  prints("  does command %d\n",temp->cmd);
		}
   }
 }

#ifdef __STDC__
void unbind ( int argc , char *argv [])
#else
void unbind(argc,argv)
 int argc;
 char *argv[];
#endif
 {
  int s; char *key;
  struct keybind *temp, *t2;

  if (argc!=2)
    { fprints(2,"usage: unbind key\n"); return; }
  key=argv[1];
  s=strlen(key);		/* Get the key's length */
  if (s==0) return;

  Keylength=0;
  for (temp=Bindhead,t2=Bindhead;temp;t2=temp,temp=temp->next)
    if (s==temp->len && !strcmp(temp->key,key))
      {
	if (temp==Bindhead) Bindhead=temp->next;
        else t2->next=temp->next;
	free(temp->key);
	free(temp);
      }
    else
	if (temp->len>Keylength) Keylength=temp->len;
 }
#endif	/* NOTYET */

#ifdef __STDC__
static int getcomcmd ( FILE *z )
#else
static int getcomcmd(z) /* Get either a character or a command from the */
 FILE *z;               /* user's input */
#endif
 {
  int c,currlen,matches,oldc;
  struct keybind *temp;
 
#ifdef DEBUG
fprints(2,"Inside getcomcmd\n");
#endif
			/* Look for a binding */
  c=getc(z); oldc=0;
  for (matches=1,currlen=0;matches && currlen<Keylength;currlen++)
   {
    matches=0;
#ifdef DEBUG
fprints(2,"Got %x\n",c);
#endif
    for (temp=Bindhead;temp;temp=temp->next)
      {
	if (currlen>=temp->len) continue;	/* Too big */
	if (c == temp->key[currlen])
	  {
	   matches=1;
	   if (currlen==temp->len-1) return(temp->cmd);
	  }
      }
    if (matches) { oldc=c; c=getc(z); }
   }
			/* Default to usual keys */
  if (c==13) return(FINISH);
  if (c==127) return(BKSP);
  if (c==27) { oldc=c; c=getc(z); }
  if (oldc==27)
    switch(c)
      {
	case 16:  return(MATCHPART);
	case 'b':
	case 'B': return(BAKWD);
	case 'd':
	case 'D': return(DELWDF);
	case 'f':
	case 'F': return(FORWD);
	case 'h':
	case 'H': return(GETHELP);
	case 'p':
	case 'P': return(PUT);
	case 'y':
	case 'Y': return(YANKNEXT);
	case '/': return(SEARCHF);
	case '?': return(SEARCHB);
      }
  if (c<32 && c>=0) c+= 256;
#ifdef DEBUG
fprints(2,"Returning %x\n",c);
#endif
  return(c);
 }

#ifndef ATARI_ST
#ifdef PROTO
void mputc ( int c , FILE *f , int curs [])
#else
void mputc(c,f,curs)
  char c;
  FILE *f;
  int curs[];
#endif
{
  extern int wid;

  write(fileno(f),&c,1);
  curs[0]++;
  if (curs[0]>=wid)
  {
    write(fileno(f),"\n",1);			/* goto start of next line */
    curs[0]=curs[0]%wid;			/* hopefully gives zero */
    curs[1]++;
  }
}
#else
#ifdef PROTO
void mputc ( int c , FILE *f , int curs [])
#else
void mputc(c,f,curs)
  int c;
  FILE *f;
  int curs[];
#endif
{
  extern int wid;
  char cc = c;
  
  write(fileno(f),&cc,1);
  curs[0]++;
  if (curs[0]>=wid)
  {
    write(fileno(f),"\n",1);			/* goto start of next line */
    curs[0]=curs[0]%wid;			/* hopefully gives zero */
    curs[1]++;
  }
}
#endif

#ifndef ATARI_ST
#ifdef PROTO
void oputc ( int c )
#else
void oputc(c)
  char c;
#endif
{
  write(fileno(zout),&c,1);
}
#else
#ifdef PROTO
int oputc ( int c )
#else
int oputc(c)
  int c;
#endif
{
    char cc = c;
    return write(fileno(zout),&cc,1);
}
#endif

#ifdef PROTO
void go ( int curs [], int hor , int vert )
#else
void go(curs,hor,vert)
  int curs[],hor,vert;
#endif
{
  extern char bs[],nd[],up[];
  int hdiff,vdiff;

  vdiff=vert-curs[1];			/* vertical difference between */
					/* current and future positions */

  if (vdiff<=0)				/* if negative go up */
    for(;vdiff;vdiff++) tputs(up,1,oputc);
  else					/* else go down */
  {
    for(;vdiff;vdiff--) write(1,"\n",1);
    curs[0]=0;
  }
    
  hdiff=hor-curs[0];			/* horizontal difference between */
					/* current and future positions */
  curs[0]=hor; curs[1]=vert;		/* a new current pos, hopefully */
	/* assigned here because hor changed below and curs needed above */

  if (hdiff<0)				/* if negative go back */
    if (-hdiff<=hor)			/* if shorter distance just use ^H */
      for(;hdiff;hdiff++) write(fileno(zout),bs,strlen(bs));
    else				/* else cr and go forward */
    {
      write(fileno(zout),"\r",1);
      for (;hor;hor--) tputs(nd,1,oputc);
    }
  else					/* have to go forward */
   for (;hdiff;hdiff--) tputs(nd,1,oputc);

}

#ifdef PROTO
void backward ( int curs [])
#else
void backward(curs)
  int curs[];
#endif
{
  extern int wid;
  extern char bs[];

  if (curs[0]==0)
    go(curs,wid-1,curs[1]-1);
  else
  {
    write(fileno(zout),bs,strlen(bs));
    curs[0]--;
  }
}

#ifdef PROTO
void forward ( int curs [])
#else
void forward(curs)
  int curs[];
#endif
{
  extern int wid;
  extern char nd[];

  curs[0]++;
  if (curs[0]>=wid)
  {
    write(fileno(zout),"\n",1);		/* goto start of next line */
    curs[0]=curs[0]%wid;		/* hopefully gives zero */
    curs[1]++;
  }
  else tputs(nd,1,oputc);
}

#ifdef PROTO
void clrscrn ( void )
#else
void clrscrn()
#endif
{
  extern char cl[];

  tputs(cl,1,oputc);			/* clear the screen */
}

#ifdef PROTO
void insert ( char *line , int pos , int letter , int curs [])
#else
void insert(line,pos,letter,curs)
  char *line,letter;
  int pos,curs[];
#endif
{
  extern int wid;
  int i,horig,vorig;
  char c;

  for (i=pos;line[i];i++);			/* goto end of line */
  for (;i!=pos;i--) line[i]=line[i-1];		/* copy characters forward */
  line[pos]=letter;				/* insert new char */
  if (letter<32 || letter==127) horig=curs[0]+2;
  else horig=curs[0]+1;
  vorig=curs[1];
  if (horig>wid-1)
  {
    horig=horig%wid;
    vorig++;
  }
  for (c=line[pos];c;c=line[++pos])		/* write out rest of line */
  {
    if (c<32 || c==127)				/* if it's a control char */
    {
      mputc('^',zout,curs);			/* print out the ^ and */
      mputc(c+64,zout,curs);			/* the equivalent char for it*/
    }
    else mputc(c,zout,curs);
  }
  go(curs,horig,vorig);
}

#ifdef PROTO
void overwr ( char *line , int pos , int letter , int curs [])
#else
void overwr(line,pos,letter,curs)
  char *line,letter;
  int pos,curs[];
#endif
{
  extern int wid;
  int ctrl,horig,vorig;
  char c;

  ctrl=0;
  line[pos]=letter;				/* overwrite new char */
  if (letter<32 || letter==127)
  {
    horig=curs[0]+2;
    ctrl=1;
  }
  else horig=curs[0]+1;
  vorig=curs[1];
  if (horig>wid-1)
  {
    horig=horig%wid;
    vorig++;
  }
  if (ctrl)
  {
    for (c=line[pos];c;c=line[++pos])		/* write out rest of line */
    {
      if (c<32 || c==127)			/* if it's a control char */
      {
	mputc('^',zout,curs);			/* print out the ^ and */
	mputc(c+64,zout,curs);			/* the equivalent char for it*/
      }
      else mputc(c,zout,curs);
    }
  }
  else mputc(letter,zout,curs);
  go(curs,horig,vorig);	/* do this in case we moved, and to update curs[] */
}

#ifdef PROTO
void show ( char *line , int curs [], bool clearing )
#else
void show(line,curs,clearing)
  char *line;
  int curs[];
  bool clearing;
#endif
{
  extern int lenprompt;
  int pos=0,horig,vorig;
  char c;

  horig=curs[0];vorig=curs[1];			/* save original values */
  if (clearing==TRUE)
  {
    curs[0]=lenprompt;curs[1]=0;
  }
  else
  {
    go(curs,lenprompt,0);			/* goto start of line */
  }
  for (c=line[pos];c!=EOS;c=line[++pos])	/* write out rest of line */
    if (c<32 || c==127)				/* if it's a control char */
    {
      mputc('^',zout,curs);			/* print out the ^ and */
      mputc(c+64,zout,curs);			/* the equivalent char for it*/
    }
    else mputc(c,zout,curs);
  go(curs,horig,vorig);			/* go back from whence you came */
}

#ifdef PROTO
void goend ( char *line , int *pos , int curs [])
#else
void goend(line,pos,curs)
  char *line;
  int *pos,curs[];
#endif
{
  char c;

  for (c=line[*pos];c;c=line[++(*pos)])
  {
    if (c<32 || c==127)
    {
      mputc('^',zout,curs);
      mputc(c+64,zout,curs);
    }
    else mputc(c,zout,curs);
  }
}

#ifdef PROTO
void copyback ( char *line , int pos , int curs [], int count )
#else
void copyback(line,pos,curs,count)
  char *line;
  int pos,curs[],count;
#endif
{
  char c;
  int i,horig,vorig,wipe;

#ifdef DEBUG
  fprints(2,"pos %d count %d\n",pos,count);
#endif
  if ((i=pos+count)<MAXLL)
  {
    wipe=0;
    for (horig=pos;horig<i;horig++)
      if (line[horig]<32 || line[horig]==127) wipe+=2;	/* calculate amount to blank */
      else wipe++;
    for (;line[i]!=EOS;++i)		/* copy line back count chars */
      line[i-count]=line[i];
    for (horig=i-count;i>horig && i<MAXLL+1;i--)
      line[i-1]=EOS;			/* end with nulls */
  }
  else
  {
    write(2,"count passed to copyback is too big\n",35);
    return;
  }
  horig=curs[0];vorig=curs[1];
  for (c=line[pos];c;c=line[++pos])
    if (c<32 || c==127)
    {
      mputc('^',zout,curs);
      mputc(c+64,zout,curs);
    }
    else mputc(c,zout,curs);
#ifdef DEBUG
  fprints(2,"wipe %d\n",wipe);
#endif
  for (i=0;i<wipe;i++) mputc(' ',zout,curs);	/* blank old chars */
  go(curs,horig,vorig);
}

#ifdef PROTO
void delnextword ( char *line , int pos , int curs [])
#else
void delnextword(line,pos,curs)
  char *line;
  int pos,curs[];
#endif
{
  int inword=0,l=1,charcount=0;

  while(l)
    switch(line[pos+charcount])
    {
      case EOS:
	l=0;
	break;
      case ' ':case '\t':case '>':case '<':case '|':case '/':
      case ';':case '=':case '+':case '&':case '`':
	if (inword) l=0;
	else charcount++;
	break;
      default:
	inword=1;
	charcount++;
    }
#ifdef DEBUG
  fprints(2,"Deleting %d chars\n",charcount);
#endif
  copyback(line,pos,curs,charcount);
}

#ifdef PROTO
void delprevword ( char *line , int *pos , int curs [])
#else
void delprevword(line,pos,curs)
  char *line;
  int *pos,curs[];
#endif
{
  int inword=0,l=1,charcount=0;

  while(l)
    if ((*pos)>0)
      switch (line[(*pos)-1])
      {
	case ' ':case '\t':case '<':case '>':case '|':case '/':
	case ';':case '=':case '+':case '&':case '`':
	  if (inword) l=0;
	  else
	  {
	    charcount++;
	    (*pos)--;
	  }
	  break;
	default:
	  inword=1;
	  charcount++;
	  (*pos)--;
      }
    else l=0;
#ifdef DEBUG
  fprints(2,"Deleting %d chars\n",charcount);
#endif
/* l should be zero already, aren't I naughty! */
  for (;l<charcount;l++) backward(curs);	/* move back */
  copyback(line,*pos,curs,charcount);		/* and copy the line on top */
}

#ifdef PROTO
void backword ( char *line , int *pos , int curs [])
#else
void backword(line,pos,curs)
  char *line;
  int *pos,curs[];
#endif
{
  int inword=0,l=1;
  char c;

  while(l)
    if ((*pos)>0)
      switch (c=line[(*pos)-1])
      {
	case ' ':case '\t':case '<':case '>':case '|':
	case ';':case '=':case '+':case '&':case '`':
	  if (inword) l=0;
	  else
	  {
	    backward(curs);
	    (*pos)--;
	  }
	  break;
	default:
	  inword=1;
	  if (c<32 || c==127) backward(curs);
	  backward(curs);
	  (*pos)--;
      }
    else l=0;
}

#ifdef PROTO
void forword ( char *line , int *pos , int curs [])
#else
void forword(line,pos,curs)
  char *line;
  int *pos,curs[];
#endif
{
  int inspace=0,l=1;
  char c;

  while(l)
    switch (c=line[*pos])
    {
      case EOS:
	l=0;
	break;
      case ' ':case '\t':case '<':case '>':case '|':
      case ';':case '=':case '+':case '&':case '`':
	inspace=1;
	forward(curs);
	(*pos)++;
	break;
      default:
	if (inspace) l=0;
	else
	{
	  if (c<32 || c==127) forward(curs);
	  forward(curs);
	  (*pos)++;
	}
    }
}

#ifdef PROTO
void yanknext ( char *line , int pos , char *yankbuf )
#else
void yanknext(line,pos,yankbuf)
  char *line,*yankbuf;
  int pos;
#endif
{
  int l=1,inword=0,i=0;

  while(l)
    switch(line[pos])
    {
      case EOS:
	l=0;
	yankbuf[i]=EOS;
	break;
      case ' ':case '\t':case '<':case '>':case '|':
      case ';':case '=':case '+':case '&':case '`':
	if (inword)
	{
	  l=0;
	  yankbuf[i]=EOS;
	}
	else
	  yankbuf[i++]=line[pos++];
	break;
      default:
	inword=1;
	yankbuf[i++]=line[pos++];
    }
}

#ifdef PROTO
void yankprev ( char *line , int pos , char *yankbuf )
#else
void yankprev(line,pos,yankbuf)
  char *line,*yankbuf;
  int pos;
#endif
{
  int stpos=pos,inword=0,l=1;

  while(l && stpos>0)			/* this loop finds the start of the */
    switch(line[stpos-1])		/* previous word (at stpos) */
    {
      case ' ':case '\t':case '>':case '<':case '|':case '"':
      case ';':case '=':case '+':case '&':case '`':case '\'':
	if (inword) l=0;
	else stpos--;
	break;
      default:
	inword=1;
	stpos--;
    }
  for (l=0;stpos<pos;stpos++,l++)	/* then copy from stpos upto pos */
    yankbuf[l]=line[stpos];
  yankbuf[l]=EOS;
}

#ifdef PROTO
void clrline ( char *line , int pos , int curs [])
#else
void clrline(line,pos,curs)
  char *line;
  int pos,curs[];
#endif
{
  extern bool tflagexist();
  extern char cd[];
  int i,horig,vorig;

  if (tflagexist("cd",termcapbuf)==TRUE)	/* If there's a special char for clr */
  {					/* then use it */
    tputs(cd,1,oputc);
    for (i=pos;line[i];i++) line[i]=EOS;
#ifdef DEBUG
fprints(2,"cleared ok\n");
#endif
  }
  else					/* else we gotta use spaces. */
  {
    horig=curs[0];vorig=curs[1];	/* Orginal values for curs set. */

    for (i=pos;line[i];i++)		/* Wipe out all those chars */
    {					/* with spaces (so slow), */
      mputc(' ',zout,curs);
      if (line[i]<32 || line[i]==127) mputc(' ',zout,curs);
      line[i]=EOS;
    }
    go(curs,horig,vorig);		/* restore original curs position */
  }
}

#ifdef PROTO
void transpose ( char *line , int pos , int curs [])
#else
void transpose(line,pos,curs)
  char *line;
  int pos,curs[];
#endif
{
  char temp;

  if (line[pos-1]<32 || line[pos-1]==127) backward(curs);
  backward(curs);
  temp=line[pos];
  line[pos]=line[pos-1];
  line[pos-1]=temp;
  if (line[pos-1]<32 || line[pos-1]==127)
  {
    mputc('^',zout,curs);
    mputc(line[pos-1]+64,zout,curs);
  }
  else mputc(line[pos-1],zout,curs);
  if (line[pos]<32 || line[pos]==127)
  {
    mputc('^',zout,curs);
    mputc(line[pos]+64,zout,curs);
  }
  else mputc(line[pos],zout,curs);
  if (line[pos]<32 || line[pos]==127) backward(curs);
  backward(curs);
}

#ifdef PROTO
int strip ( char *line )
#else
int strip(line)
  char *line;
#endif
{
  int i,nosave=0;

  for (i=0;line[i]==' ';i++);
  if (line[i]=='#')
  {
    nosave=1;
    i++;
  }
  if (i) strcpy(line,&line[i]);
  return(nosave);
}

/* Getline gets a line, returning a flag is it should be saved.
 * God knows what feature_off does!
 */
bool getline(line,nosave,feature_off)
  char *line;
  int *nosave,feature_off;
{
  extern char beep[],yankbuf[];
  extern int lenprompt,wid;
  char remline[MAXLL];
  int c,times=1,i,pos=0,curs[2],
      hsave=lenprompt,vsave=0,possave=0,try=0;
  int beeplength=strlen(beep);

  curs[0]=lenprompt;	/*lenprompt global set by prprompt or when prompt set*/
  curs[1]=0;					/* start on line 0 */

  while (1)
  {
    c=getcomcmd(zin);
    for (;times>0;times--)
    switch(c)
    {
      case EOF : fprints(2,"%d\n",errno); perror("comlined");
		 exit(1);
      case MARK: hsave=curs[0];			/* save position (make mark) */
		 vsave=curs[1];
		 possave=pos;
		 break;
      case START: go(curs,lenprompt,0);	/* goto start of the line */
		 pos=0;
		 break;
      case BAKCH: if (pos>0)			/* if not at home, go back */
		 {
		   if (line[pos-1]<32 || line[pos-1]==127) backward(curs);
		   backward(curs);
		   pos--;
		 }
		 else write(1,beep,beeplength);		/* else ring bell */
		 break;
      case INT : goend(line,&pos,curs);
		 write(fileno(zout),"\n",1);
		 return(FALSE);
      case DELCH: if (line[0]==EOS && !feature_off)
		 {
		   leave_shell(); /*eof*/
		   return(FALSE); /* return if no exit */
		 }
		 else if (line[pos]!=EOS)
			copyback(line,pos,curs,1); /* delete char */
		 break;
      case END: goend(line,&pos,curs);		/* goto end of the line */
		 break;
      case FORCH: if (line[pos]!=EOS)		/* if not at end, go forward */
		 {
		   if (line[pos]<32 || line[pos]==127) forward(curs);
		   forward(curs);
		   pos++;
		 }
		 else write(1,beep,beeplength);		/* else ring bell */
		 break;
      case KILLALL: go(curs,lenprompt,0);		/* goto start */
		 clrline(line,0,curs);		/* and kill from pos=0 */
		 /* hist=curr_hist;		/* reset hist */
		 hsave=curs[0];			/* save position (make mark) */
		 vsave=curs[1];
		 for (pos=0;pos<MAXLL;pos++) line[pos]=EOS;
		 pos=possave=0;
		 break;
      case BKSP: if (pos>0)			/* if not at home, delete */
		 {
		   if (line[pos-1]<32 || line[pos-1]==127) backward(curs);
		   backward(curs);
		   copyback(line,--pos,curs,1); /*move line back on to prev char*/
		 }
		 else write(1,beep,beeplength);		/* else ring bell */
		 break;
      case FINISH: goend(line,&pos,curs);
		 write(fileno(zout),"\n",1);
		 line[pos++]='\n';
		 if (feature_off) return(TRUE);
		 *nosave=strip(line);			/* process it now */
		 if (line[0]!=EOS)
		   return(TRUE);
		 else return(FALSE);
      case KILLEOL: clrline(line,pos,curs);	/* kill line from cursor on */
		 break;
      case REDISP:
      case CLREDISP: if (feature_off) break;
		 clrscrn();			/* Clear the screen */
		 prprompt(1);			/* Reprint the prompt and */
		 show(line,curs,TRUE);		/* the line typed so far. */
		 break;
      case XON : continue;	/* can't use this */
      case XOFF : continue;	/* can't use this */
      case TRANSPCH: if (pos>0 && line[pos]!=EOS)	/* if not home or at end */
		   transpose(line,pos,curs);	/* swap current and prev char */
		 else write(1,beep,beeplength);		/* else ring bell */
		 break;
      case LOOP: times=0;
		 c=getc(zin);
		 while(isdigit(c))
		 {
		   times=times*10+c-48;
		   c=getc(zin);
		 }
		 times++;	/* need to add 1 because it's dec'ed */
		 break;		/* immediately at end of for loop */
      case QUOTE: if (pos>=MAXLL)
		 {
		   write(1,beep,beeplength);
		   continue;
		 }
		 mputc('"',zout,curs); backward(curs);	/* literal char */
		 c=getc(zin);
		 if (c)			/* don't allow EOS (null) */
		   insert(line,pos++,c,curs);
		 break;
      case DELWD: if (pos) delprevword(line,&pos,curs);
		 else write(1,beep,beeplength);
		 break;
      case GOMARK : pos=possave;
		 go(curs,hsave,vsave);
		 break;
      case YANKLAST : if (pos!=0)			/* if not at home */
		   yankprev(line,pos,yankbuf);		/* yank previous word */
		 else write(1,beep,beeplength);		/* else ring bell */
		 break;
      case SUSP: continue;
     case BAKWD: if (pos>0) backword(line,&pos,curs);
			      else write(1,beep,beeplength);
			      break;
     case DELWDF: if (line[pos]!=EOS) delnextword(line,pos,curs);
			      else write(1,beep,beeplength);
			      break;
     case FORWD: if (line[pos]!=EOS) forword(line,&pos,curs);
			      else write(1,beep,beeplength);
			      break;
     case PUT: if (pos>MAXLL-strlen(yankbuf))
			      {
				write(1,beep,beeplength);
				break;
			      }
			      for (i=0;yankbuf[i];i++)	/* insert yank buffer */
				insert(line,pos++,yankbuf[i],curs);
			      break;
     case YANKNEXT : if (line[pos]!=EOS)	/* if not at end */
				yanknext(line,pos,yankbuf);/* yank next word */
			      else write(1,beep,beeplength);/* else ring bell */
			      break;
     case SEARCHF : if (line[pos])		/* search forward */
			      {
				c=getc(zin);		/* char to search for */
				for (i=pos+1;line[i] && c!=line[i];i++);
				if (line[i])
				  while (pos<i)
				  {
				    pos++;
				    if (line[pos-1]<32 || line[pos-1]==127) forward(curs);
				    forward(curs);
				  }
				else write(1,beep,beeplength);	/* not found */
			      }
			      else write(1,beep,beeplength);/* at end of line */
			      break;
     case SEARCHB : if (pos>0)		/* search backwards */
			      {
				c=getc(zin);		/* char to search for */
				for (i=pos-1;i>=0 && c!=line[i];i--);
				if (i>=0)
				  while (pos>i)
				  {
				    pos--;
				    if (line[pos]<32 || line[pos-1]==127) backward(curs);
				    backward(curs);
				  }
				else write(1,beep,beeplength);	/* not found */
			      }
			      else write(1,beep,beeplength);	/* at end of line */
			      break;
      /* case  29 : continue; */
      default  : if (pos>=MAXLL-1)
		 {
		   write(1,beep,beeplength);
		   break;
		 }
		 insert(line,pos++,c,curs);
		 break;
    }
   times=1;
  }
}

