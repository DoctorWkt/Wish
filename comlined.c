#include "header.h"

/* List of commands defined for the command line editor */
/* Note that the first commands up to SUSP are exactly */
/* 256 higher than the ASCII code for the corresponding key */

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


/* Clam allows keystrokes to be bound to other keystrokes. The following
 * structure is used to hold these bindings.
 */

struct keybind {
	char *key;		/* The key sequence we have bound */
	int len;		/* The length of the key sequence */
	char *cmd;		/* The string it is mapped to */
	struct keybind *next;
	};


char yankbuf[512];			/* Buffer used when yanking words */
static char bindbuf[512];		/* Buffer used to expand bindings */
static char *bindptr;			/* Pointer into bindbuf */
static char *wordterm;			/* Characters that terminate words */
static int Keylength=0;			/* The maximum key length */
static struct keybind *Bindhead=NULL;	/* List of bindings */
int wid;				/* The width of the screen (minus 1) */

/* A quick blurb on the curs[] structure.
 * Curs[0] holds the column pos'n of the cursor, curs[1] holds the # of
 * lines the cursor is below the prompt line. e.g (0,0) is the first char
 * of the prompt, (25,3) is in column 25, 3 lines below the prompt line.
 */
int curs[2];


/* Bind is a builtin. With no arguments, it lists the current key bindings.
 * With 1 arg, it shows the binding (if any) for argv[1]. With 2 args
 * the string argv[1] will be replaced by argv[2].
 */

int Bind(argc, argv)
 int argc;
 char *argv[];
 {
  int s,showall;
  char *key, *cmd;
  struct keybind *temp, *Bindtail;

  if (argc>3)
    { fprints(2,"usage: bind [key [value]]\n"); return(1); }
  showall=0;
  switch(argc)
   {
    case 3: key=argv[1]; cmd=argv[2];	/* Bind a string to another */
	    s=strlen(key);		/* Get the key's length */
	    if (s==0) break;

	    temp=(struct keybind *)malloc(sizeof(struct keybind));
	    if (!temp) { perror("malloc"); return(1); }
	    temp->key= (char *)malloc(s+1);
	    if (!(temp->key)) { perror("malloc"); return(1); }

	    strcpy(temp->key,key);	/* Copy the key */
	    temp->len=s;
	    temp->cmd= (char *)malloc(strlen(cmd)+1);
	    if (!(temp->cmd)) { perror("malloc"); return(1); }
	    strcpy(temp->cmd,cmd);
	    temp->next=NULL;
	    if (s>Keylength) Keylength=s;

	    if (!Bindhead)		/* Add to linked list */
		Bindhead=temp;
	    else
	      { for (Bindtail=Bindhead;Bindtail->next;Bindtail=Bindtail->next);
		Bindtail->next=temp;
	      }
	    break;
    case 1: showall=1;			/* Print one or more bindings */
    case 2: for (temp=Bindhead;temp;temp=temp->next)
	      if (showall || !strcmp(temp->key,key))
		{ mprint(temp->key,1);
		  for (s=Keylength-temp->len; s; s--) write(1," ",1);
		  prints(" bound to ");
		  mprint(temp->cmd,0);
		}
   }
  return(0);
 }

/* Unbind is a builtin which removes argv[1] from the list of key bindings */

int unbind(argc,argv)
 int argc;
 char *argv[];
 {
  int s; char *key;
  struct keybind *temp, *t2;

  if (argc!=2)
    { fprints(2,"usage: unbind string\n"); return(1); }
  key=argv[1];
  s=strlen(key);		/* Get the key's length */
  if (s==0) return(1);

  Keylength=0;
  for (temp=Bindhead,t2=Bindhead;temp;t2=temp,temp=temp->next)
    if (s==temp->len && !strcmp(temp->key,key))
      {
	if (temp==Bindhead) Bindhead=temp->next;
        else t2->next=temp->next;
	free(temp->key);
	free(temp->cmd);
	free(temp);
      }
    else
	if (temp->len>Keylength) Keylength=temp->len;
  return(0);
 }

/* Exbind: get one or more characters from the user, expanding bindings
 * along the way. This routine is recursive & hairy! When called from
 * getcomcmd(), inbuf is NULL, indicating we want user keystrokes.
 * These are read in, and if there are no partial bind matches, are
 * returned to getcomcmd(). If any partial matches, they are buffered
 * in bindbuf until either no partials or 1 exact match. Once a match is
 * found, we call ourselves with inbuf pointing to the replacement string.
 * Thus bindings can recurse, up to the size of bindbuf. Note also that
 * after we recurse once, we check to see if there are any leftover chars
 * in inbuf, and recurse on them as well.
 */

static void expbind(inbuf)	/* Expand bindings from user's input */
 char *inbuf;
 {
  char a, *startptr, *exactptr;
  int c,currlen,partial,exact;
  struct keybind *temp;

  if (inbuf==NULL) bindptr=bindbuf;
  startptr=bindptr;
  currlen=0;
  
  while(1)			/* Look for a keystroke binding */
   {
    partial=exact=0;
    if ((inbuf==NULL) || ((a= *(inbuf++))==EOS)) c=read(0,&a,1);
    if (c!=-1)			/* Decide which char to put in the buffer */
      { *(bindptr++)=a;
        *bindptr=EOS; currlen++;
        if ((int)(bindptr-bindbuf)>510) return; }

    for (temp=Bindhead;temp!=NULL;temp=temp->next)
     {				/* Count the # of partial & exact matches */
      if (currlen>temp->len) continue;
      if (!strcmp(startptr,temp->key)) { exact++; exactptr=temp->cmd; }
      if (!strncmp(startptr,temp->key,currlen)) partial++;
     }
    if (partial==0) break;		/* No binding at all */
    if (partial==1 && exact==1)		/* An exact match, call ourselves */
      { bindptr=startptr;		/* with the matched word */
        expbind(exactptr);
        break; }
   }
				
    if (inbuf!=NULL && *inbuf!=EOS)	/* If any part of our word left over */
      expbind(inbuf);			/* check it as well */
 }

/* Getcomcmd converts a user's keystokes into the commands used by the CLE.
 * If there are no chars handy in bindbuf, we call expbind() to get some.
 * Then we scan thru the chars and deliver chars or commands to the CLE.
 * Hopefully because we use commands>255, the CLE will work with 8-bit
 * extended ASCII.
 */

static int getcomcmd()	/* Get either a character or a command from the */
 {			/* user's input */
  int c,oldc;
 
#ifdef DEBUG
fprints(2,"Inside getcomcmd\n");
fprints(2,"Bindbuf is ");
mprint(bindbuf,0);
#endif
  oldc=0;
  while(1)
   {			/* If no chars, get chars from stdin and */
			/* expand bindings */
    while ((c= *(bindptr++))==0) { expbind(NULL); bindptr=bindbuf; }

			/* Default to usual keys */
    if (c==13) return(FINISH);
    if (c==127) return(BKSP);
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
        default : bindptr--; return(27);
       }
    if (c==27) { oldc=c; continue; }
    if (c<32 && c>=0) c+= 256;	/* Most ctrl chars become commands */
#ifdef DEBUG
fprints(2,"Returning %x\n",c);
#endif
    return(c);
   }			
 }

/* Mputc prints out a character and updates the cursor position.
 * It also handles control chars by preceding them with a caret.
 */

void mputc(c,f)
  char c;
  int f;
{
  write(f,&c,1);
  curs[0]++;
  if (curs[0]>=wid)
  {
    write(f,"\n",1);			/* goto start of next line */
    curs[0]=curs[0]%wid;		/* hopefully gives zero */
    curs[1]++;
  }
}

/* Oputc is used by tputs to print out one character of a string at
 * a time to the screen.
 */

void oputc(c)
  char c;
{
  write(1,&c,1);
}

/* Go moves the cursor to the position (vert,hor), and updates the cursor */
void go(hor,vert)
  int hor,vert;
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
      for(;hdiff;hdiff++) write(1,bs,strlen(bs));
    else				/* else cr and go forward */
    {
      write(1,"\r",1);
      for (;hor;hor--) tputs(nd,1,oputc);
    }
  else					/* have to go forward */
   for (;hdiff;hdiff--) tputs(nd,1,oputc);

}

/* Backward: Move the cursor backwards one character */
void backward()
{
  extern char bs[];

  if (curs[0]==0)
    go(wid-1,curs[1]-1);
  else
  {
    write(1,bs,strlen(bs));
    curs[0]--;
  }
}

/* Forward: Move the cursor forwards one character */
void forward()
{
  extern char nd[];

  curs[0]++;
  if (curs[0]>=wid)
  {
    write(1,"\n",1);			/* goto start of next line */
    curs[0]=curs[0]%wid;		/* hopefully gives zero */
    curs[1]++;
  }
  else tputs(nd,1,oputc);
}

/* Show is a routine that replaces four routines in the old version of Clam.
 * The flag variable holds the id of which `routine' to emulate:
 *
 * case 0: Insert	Insert the letter at position pos in the line,
 *			updating the cursor, and redrawing the line.
 * case 1: Overwrite	Overwrite the letter at pos in the line,
 *			updating the cursor, and redrawing the line.
 *			Note that to toggle ins/ovw, we can have a
 *			variable ovwflag, and do ovwflag= 1-ovwflag.
 * case 2: Show		Just redisplay the line.
 * case 3: Goend	Goto the end of the line.
 */
int Show(line,pos,letter,flag)
  char *line,letter;
  int pos,flag;
{
  extern int lenprompt;
  int i,horig,vorig;
  char c;

  switch(flag)
   {						/* Case 0: insert character */
    case 0: for (i=pos;line[i];i++);		/* goto end of line */
  	    for (;i!=pos;i--)			/* copy characters forward */
  		line[i]=line[i-1];
    case 1: line[pos]=letter;			/* Case 1: overwrite char */
	    if (letter<32 || letter==127) horig=curs[0]+2;
	    else horig=curs[0]+1;		/* Calculate the new cursor */
	    vorig=curs[1];			/* position */
	    if (horig>wid-1)
  	     {  horig=horig%wid;
		vorig++;
  	     }
	    break;
    case 2: pos=0;				/* Case 2: show the line */
	    horig=curs[0]; vorig=curs[1];	/* save original values */
	    curs[0]= lenprompt; curs[1]=0;
   }						/* Case 3: goto end of line */
  for (c=line[pos];c;c=line[++pos])		/* write out rest of line */
  {
    if (c<32 || c==127)				/* if it's a control char */
    {
      mputc('^',1);				/* print out the ^ and */
      mputc(c+64,1);				/* the equivalent char for it*/
    }
    else mputc(c,1);				/* else just show it */
  }
  if (flag!=3) go(horig,vorig);			/* h/vorig unused for goend */
  return(pos);					/* goend only uses this value */
}


/* I'm not exactly sure what copyback() does yet - Warren */
void copyback(line,pos,count)
  char *line;
  int pos,count;
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
      if (line[horig]<32 || line[horig]==127)
        wipe+=2;			/* calculate amount to blank */
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
      mputc('^',1);
      mputc(c+64,1);
    }
    else mputc(c,1);
#ifdef DEBUG
  fprints(2,"wipe %d\n",wipe);
#endif
  for (i=0;i<wipe;i++) mputc(' ',1);	/* blank old chars */
  go(horig,vorig);
}

#define delnextword(line,pos)		nextword(line,&pos,0)
#define forword(line,pos)	 	nextword(line,pos,1)
#define yanknext(line,pos)		nextword(line,&pos,2)

#define delprevword(line,pos)		prevword(line,pos,0)
#define backword(line,pos)	 	prevword(line,pos,1)

/* The following two routines each replace three separate ones from old Clam */

/* Nextword works on the word after/at the cursor poition, depending on flag:
 *
 * case 0: Delnextword		The word after the cursor is removed
 *				from the line, and the display is updated.
 * case 1: Forword		The cursor is moved to the start of the
 *				next word.
 * case 2: Yanknext		The word after the cursor is put into yankbuf.
 *
 * Although pos is passed as a pointer, only forword() updates the value.
 */
void nextword(line,p,flag)
  char *line;
  int *p,flag;
{
  int inword=0,l=1,pos= *p,charcount=0;
  char c;

  while(l)
   {
    if ((c=line[pos])==EOS) { l=0; break; }
    if (strchr(wordterm,c)!=NULL)	/* Found end of a word */
      { charcount++; pos++; if (inword) l=0; }
    else { inword=1; charcount++; pos++; }
   }
 
#ifdef DEBUG
  fprints(2,"Deleting %d chars\n",charcount);
#endif
  switch(flag)
   {
    case 0: copyback(line,*p,charcount);
	    break;
    case 1: for (;l<charcount;l++) forward();	/* move forward */
	    *p=pos;
    case 2: for (pos= *p; l<charcount; l++,pos++) yankbuf[l]=line[pos];
	    yankbuf[l]=EOS;
   }
 }

/* Prevword works on the word before the cursor poition, depending on flag:
 *
 * case 0: Delprevword		The word before the cursor is removed
 *				from the line, and the display is updated.
 * case 1: Backword		The cursor is moved to the start of the
 *				previous word.
 * case 2: Yankprev		The word before the cursor is put into yankbuf.
 *
 * Although pos is passed as a pointer, only delprevword and
 * backword update the value.
 */
void prevword(line,p,flag)
  char *line;
  int *p,flag;
{
  int inword=0,l=1,pos= *p,charcount=0;

  while(l)
   {
    if (pos==0) {l=0; break; }
    if (strchr(wordterm,line[pos-1])!=NULL)	/* Found end of a word */
      { if (inword) l=0; else { charcount++; pos--; } }
    else { inword=1; charcount++; pos--; }
   }
#ifdef DEBUG
  fprints(2,"Deleting %d chars\n",charcount);
#endif

  switch(flag)
   { case 0: for (;l<charcount;l++) backward();		/* move back */
	     copyback(line,pos,charcount);	/* and copy the line on top */
	     *p=pos;
	     break;
     case 1: for (;l<charcount;l++) backward();		/* move back */
	     *p=pos;
	     break;
     case 2: for (;l<charcount;l++,pos++) yankbuf[l]=line[pos];
	     yankbuf[l]=EOS;
  }
}


/* Clrline: The line from the position pos is cleared */
void clrline(line,pos)
  char *line;
  int pos;
{
  extern bool tflagexist();
  extern char cd[];
  int i,horig,vorig;

  if (*cd!=EOS)			/* If there's a special char for clr */
  {				/* then use it */
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
      mputc(' ',1);
      if (line[i]<32 || line[i]==127) mputc(' ',1);
      line[i]=EOS;
    }
    go(horig,vorig);		/* restore original curs position */
  }
}

/* Transpose transposes the characters at pos and pos-1 */
void transpose(line,pos)
  char *line;
  int pos;
{
  char temp;

  if (line[pos-1]<32 || line[pos-1]==127) backward();
  backward();
  temp=line[pos];
  line[pos]=line[pos-1];
  line[pos-1]=temp;
  if (line[pos-1]<32 || line[pos-1]==127)
  {
    mputc('^',1);
    mputc(line[pos-1]+64,1);
  }
  else mputc(line[pos-1],1);
  if (line[pos]<32 || line[pos]==127)
  {
    mputc('^',1);
    mputc(line[pos]+64,1);
  }
  else mputc(line[pos],1);
  if (line[pos]<32 || line[pos]==127) backward();
  backward();
}

/* Strip takes the line, and removes leading spaces. If the first non-space
 * character is a hash, it returns 1. This should also remove trailing
 * comments; I might just move the whole thing into meta_1.
 */
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


/* Getuline gets a line from the user, returning a flag if the line
 * should be saved.
 */
bool getuline(line,nosave)
  char *line;
  int *nosave;
{
  extern char *EVget();
  extern char beep[],cl[];
  extern int errno,lenprompt,curr_hist,maxhist;
  extern void prprompt(), setcbreak();
  char a;
  int c,times=1,i,pos=0,hist=curr_hist,
      hsave=lenprompt,vsave=0,possave=0;
  int beeplength=strlen(beep);

  curs[0]=lenprompt;	/*lenprompt global set by prprompt or when prompt set*/
  curs[1]=0;		/* start on line 0 */
  wordterm=EVget("wordterm");	/* Determine the word terminators */
  if (wordterm==NULL || *wordterm==EOS)
	wordterm= " \t><|/;=&`";
  bindptr=bindbuf;	/* Set up the pointer to the bind buffer */
  *bindptr=EOS;

  prprompt();		/* Print out our prompt */
  setcbreak();		/* and set the terminal into cbreak mode */
  while (1)
  {
    c=getcomcmd();	/* Get a character or a command */
    for (;times>0;times--)
    switch(c)
    {
      case EOF : fprints(2,"%d\n",errno); perror("comlined");
		 exit(1);
      case MARK: hsave=curs[0];			/* save position (make mark) */
		 vsave=curs[1];
		 possave=pos;
		 break;
      case START: go(lenprompt,0);	/* goto start of the line */
		 pos=0;
		 break;
      case BAKCH: if (pos>0)			/* if not at home, go back */
		   { if (line[pos-1]<32 || line[pos-1]==127) backward();
		     backward();
		     pos--;
		   }
		 else Beep;		/* else ring bell */
		 break;
      case INT : pos=goend(line,pos);
		 write(1,"\n",1);
		 return(FALSE);
      case DELCH: if (line[0]==EOS)
		   { leave_shell(); /*eof*/
		     return(FALSE); /* return if no exit */
		   }
		 else if (line[pos]!=EOS)
			copyback(line,pos,1); /* delete char */
		      else complete(line,&pos,FALSE);
		 break;
      case END: pos=goend(line,pos);	/* goto end of the line */
		 break;
      case FORCH: if (line[pos]!=EOS)		/* if not at end, go forward */
		   { if (line[pos]<32 || line[pos]==127) forward();
		     forward();
		     pos++;
		   }
		 else Beep;		/* else ring bell */
		 break;
      case KILLALL: go(lenprompt,0);	/* goto start */
		 clrline(line,0);		/* and kill from pos=0 */
		 hist=curr_hist;		/* reset hist */
		 hsave=curs[0];			/* save position (make mark) */
		 vsave=curs[1];
		 for (pos=0;pos<MAXLL;pos++) line[pos]=EOS;
		 pos=possave=0;
		 break;
      case BKSP: if (pos>0)			/* if not at home, delete */
		  { if (line[pos-1]<32 || line[pos-1]==127) backward();
		    backward();
		    copyback(line,--pos,1); /*move line back on to */
		  } 				 /*prev char*/
		 else Beep;		/* else ring bell */
		 break;
      case COMPLETE: if (line[0]!=EOS)		/* try to complete word */
			complete(line,&pos,TRUE);
		 else Beep;
		 break;
      case FINISH: pos=goend(line,pos);
		 write(1,"\n",1);
		 line[pos++]=EOS;
		 *nosave=strip(line);			/* process it now */
		 if (line[0]!=EOS)
		   return(TRUE);
		 else return(FALSE);
      case NEXTHIST: if (hist<curr_hist)	/* put next hist in line buf */
		   loadhist(line,&pos,++hist);
		 else Beep;
		 break;
      case BACKHIST: if (hist>curr_hist-maxhist && hist>1) /* put prev hist */
		  { if (hist==curr_hist) 		/* in line buf */
		     (void) savehist(line,curr_hist,maxhist);
		    loadhist(line,&pos,--hist);
		  }
		 else Beep;
		 break;
      case KILLEOL: clrline(line,pos);	/* kill line from cursor on */
		 break;
      case REDISP:
      case CLREDISP: tputs(cl,1,oputc);		/* Clear the screen */
		 prprompt(1);			/* Reprint the prompt and */
		 show(line,TRUE);		/* the line typed so far. */
		 break;
      case XON : continue;	/* can't use this */
      case XOFF: continue;	/* can't use this */
      case TRANSPCH: if (pos>0 && line[pos]!=EOS) /* if not home or at end */
		       transpose(line,pos);/* swap current and prev char */
		 else Beep;		/* else ring bell */
		 break;
      case LOOP: times=0;
		 read(0,&a,1); c=a;
		 while(isdigit(c))
		 { times=times*10+c-48;
		   read(0,&a,1); c=a;
		 }
		 times++;	/* need to add 1 because it's dec'ed */
		 break;		/* immediately at end of for loop */
      case QUOTE: if (pos>=MAXLL)
		 { Beep;
		   continue;
		 }
		 mputc('"',1); backward();	/* literal char */
		 read(0,&a,1); c=a;
		 if (c)			/* don't allow EOS (null) */
		   insert(line,pos++,c);
		 break;
      case DELWD: if (pos) delprevword(line,&pos);
		 else Beep;
		 break;
      case GOMARK : pos=possave;
		 go(hsave,vsave);
		 break;
      case YANKLAST: if (pos!=0)			/* if not at home */
		   yankprev(line,pos);			/* yank previous word */
		 else Beep;		/* else ring bell */
		 break;
      case SUSP: continue;
     case BAKWD: if (pos>0) backword(line,&pos);
		 else Beep;
		 break;
     case DELWDF: if (line[pos]!=EOS) delnextword(line,pos);
		 else Beep;
		 break;
     case FORWD: if (line[pos]!=EOS) forword(line,&pos);
		 else Beep;
		 break;
     case PUT: if (pos>MAXLL-strlen(yankbuf))
		 { Beep;
		break;
		 }
		for (i=0;yankbuf[i];i++)	/* insert yank buffer */
		  insert(line,pos++,yankbuf[i]);
		break;
     case YANKNEXT: if (line[pos]!=EOS)	/* if not at end */
		  yanknext(line,pos);	/* yank next word */
		else Beep;		/* else ring bell */
		break;
     case SEARCHF: if (line[pos])		/* search forward */
		     { read(0,&a,1); c=a;	/* char to search for */
		       for (i=pos+1;line[i] && c!=line[i];i++);
		       if (line[i])
			 while (pos<i)
			  { pos++;
			    if (line[pos-1]<32 || line[pos-1]==127)
			     forward();
			    forward();
			  }
		       else Beep;	/* not found */
		     }
		else Beep;	/* at end of line */
		break;
     case SEARCHB : if (pos>0)		/* search backwards */
		      { read(0,&a,1); c=a;	/* char to search for */
			for (i=pos-1;i>=0 && c!=line[i];i--);
			if (i>=0)
			  while (pos>i)
			   { pos--;
			     if (line[pos]<32 || line[pos-1]==127)
				backward();
			     backward();
			   }
		       else Beep;	/* not found */
		     }
		else Beep;	/* at end of line */
		break;
      default:	if (pos>=MAXLL-1)
		 { Beep;
		   break;
		 }
		insert(line,pos++,c);
		break;
    }
   times=1;
  }
}
