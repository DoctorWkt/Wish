#include "header.h"
#define MAXMATCH 512
#define OK	0
#define NX_ERR -1
#define BR_ERR -2

extern struct candidate carray[];	/* The matched files */
int ncand;
struct candidate *wordlist;		/* The list of words for the parser */

/* Match takes a string, and a pattern, which can contain *, ? , \ and [],
 * and returns 0 if the strings matched the pattern, otherwise a negative
 * number. If the pattern ends in a '/', matchdir is called with the
 * arg accumdir/string to find further matches. Match also adds each match
 * into the matches array given above.
 */
static
int match(string,pattern,accumdir)
  char *string,*pattern,*accumdir;
{
  char c,*findex,*pindex,*fmatch,*pmatch,rempat[MAXWL],where[MAXPL];
  int i,mismatch,found,star;

  findex=string;			/* Initialise our vars */
  pindex=pattern;
  mismatch=star=0;
  while (*findex && !mismatch)		/* While no mismatch */
  {
    switch(*pindex)			/* Match the pattern's char */
    {
      case '*' : pindex++;		/* Skip the '*' */
		 pmatch=pindex;		/* We must match from here on */
		 fmatch=findex;
		 star=1;		/* Found a star */
		 break;
      case '?' : pindex++;   /* just increment, letter automatically matches. */
		 findex++;
		 break;
      case '[' : found=0;		/* For all chars in [] */
		 for (;(*pindex) && (*pindex!=']');pindex++)
		 {
		   switch (*pindex)
		   {
		     case '\\':
		       if (*(pindex+1)==*findex) /* Try match on next char */
		       found=1;
		       break;
		     case '-' :
		       if ((*(pindex+1)==']') || (*(pindex-1)=='['))
			 if (*pindex==*findex)
			 {
			   found=1;	/* Try a match on '-' */
			   break;
			 }		/* or try all chars in the range */
		       for (c= *(pindex-1)+1;c <= *(pindex+1) && !found && c!=']';c++)
			 if (c==*findex)
			   found=1;
		       break;
		     default  :		/* Default: just match that char */
		       if (*pindex==*findex)
			 found=1;
		       break;
		   }
		 }
			/* We could exit the loop as soon as found is set to 1
			 * but we have to keep going to the end of the list so
			 * we can recommence matching after the ']'. This also
			 * allows us to test for the unmatched bracket.
			 */
		 if (*pindex==EOS)
		 {
		   prints(2,"Unmatched bracket '['\n");
		   return(BR_ERR);
		 }
		 if (found)
		 {
		   pindex++;		/* move past the ']' */
		   findex++;		/* and on to the next letter */
		   break;
		 }
		 if (!star)
		   mismatch=1;		/* failure for this letter of file */
		 else
		 {
		   pindex=pmatch;
		   findex= ++fmatch;
		 }
		 break;
      case '\\': pindex++;		/* go on to next symbol */

      default  : if (*findex!=*pindex)	/* No match and not a star */
		   if (!star)		/* gives an error */
		     mismatch=1;
		   else
		   {
		     pindex=pmatch;	/* Move back to the char we must */
		     findex= ++fmatch;	/* match, & move along string */
		   }
		 else
		 {
		   pindex++;		/* Yes, a match, move up */
		   findex++;
		 }
		 break;
    }
  }
  if (mismatch) return(NX_ERR);		/* it didn't match, go on to next one */
  for (;*pindex=='*';pindex++);       /* get rid of trailing stars in pattern */
  if (*findex==*pindex)
  {
					/* add it to the candidate list */
    where[0]=EOS;
    if (*accumdir)
    {
      strcpy(where,accumdir);
      strcat(where,"/");
    }
    strcat(where,string);
    if (ncand==MAXCAN) return(OK);
    carray[ncand].name=(char *) malloc ((unsigned)(strlen(where)+4));
    if (carray[ncand].name==NULL) return(OK);
    strcpy(carray[ncand].name,where);
    carray[ncand++].mode=TRUE;		/* Was malloc'd */
    return(OK);
  }
  else
    if (*pindex=='/')			/* test if file is a directory */
    {
      for (i=0;*pindex;i++,pindex++)
        rempat[i]= *(pindex+1);		/* even copies null across */
      strcpy(where,accumdir);
      if (where[0]!=EOS)
      {
	strcat(where,"/");
	strcat(where,string);
      }
      else strcpy(where,string);
      return(matchdir(where,rempat));	/* recursively call this pair again */
    }
    else				/* it just doesn't match */
      return(NX_ERR);
}

/* Matchdir takes a directory name and a pattern, and returns 0 if any
 * files in the directory match the pattern. If none, it returns a negative
 * number. Note that this may be recursive, as it calls match(), which calls
 * matchdir().
 */
static
int matchdir(directory,pattern)
  char *directory,*pattern;
{
  DIR *dirp,*opendir();
#ifdef UCB
  struct dirent *entry,*readdir();
#else
  struct direct *entry,*readdir();
#endif
  int foundany=0;

  if (*directory!=EOS)
  {
    if ((dirp=opendir(directory))==NULL)
      return(NX_ERR);
  }
  else
    if ((dirp=opendir("."))==NULL)
      return(NX_ERR);
  while((entry=readdir(dirp))!=NULL)
    if (strcmp(entry->d_name,".") && strcmp(entry->d_name,"..") &&
        !match(entry->d_name,pattern,directory))
      		foundany++;
  closedir(dirp);
  if (foundany) return(OK);
  else return(NX_ERR);
}


#ifdef PROTO
static void finddir ( char *word , char *dir )
#else
static
void finddir(word,dir)
  char *word,*dir;
#endif
{
  char c;
  int i=0,j,l=1;

/* This function finds the directory to start the matching from. */
  while(l)
    switch(c=word[i])
    {
      case '*':case '?':case '[':
	for (;dir[i]!='/' && i;i--);		/* go back to end of previous component */
	if (i) dir[i++]=EOS;
	else
	  if (dir[0]=='/') dir[++i]=EOS;
	  else 
	    dir[0]=EOS;
	l=0;
        break;
      default :
	dir[i++]=c;
    }
  for (l=i,j=0;word[l];l++,j++)
    word[j]=word[l];
  word[j]=EOS;
}


/* Tilde takes the word beginning with a ~, and returns the word with the
 * first part replaced by the directory name. If ~/, we use $HOME
 * e.g ~fred -> /u1/staff/fred
 *     ~jim/Dir/*.c -> /usr/tmp/jim/Dir/*.c
 *     ~/Makefile -> /u1/staff/warren/Makefile
 * If it cannot expand, it returns dir as a zero-terminated string
 */
void tilde(word,dir)
 char *word, *dir;
 {
  struct passwd *entry;
  char *a;

  word++;
  if (*word=='/')			/* use $HOME */
   { a= word;
     entry=getpwuid(getuid());
   }
  else
   { a=strchr(word,'/');
     if (a!=NULL) *a=0;
     entry=getpwnam(word);
   }
  endpwent();
  if (entry==NULL) { *dir=0; return; }
  strcpy(dir,entry->pw_dir);		/* Form the real partial name */
  if (a!=NULL) { *a='/'; strcat(dir,a); }
 }

/* Expline takes a linked list of words, and converts them into a normal
 * string. This is used to save history.
 */
char *expline(list)
 struct candidate *list;
 {
  struct candidate *q;
  char *out, *a;
  int i;
					/* Find the amount to malloc */
  for (i=0,q=list;q!=NULL;q=q->next) if (q->name) i+= strlen(q->name) +1;

  out= (char *)malloc((unsigned) i+4);
  if (out!=NULL)
   {
    for (a=out,q=list;q!=NULL;q=q->next)
      if (q->name)
       {
	strcpy(a,q->name);
	a+= strlen(q->name);
	*(a++)=' ';
       }
    *(--a)='\0';
   }
  return(out);
 }

/* Meta_1 takes the user's input line, and builds a linked list of words
 * in the carray. It also expands tildes. If start==TRUE, the carray is
 * made empty. i.e when called from main, use TRUE.
 */
void meta_1(old,start)
 char *old;
 bool start;
 {
  char *a, *b, *gethist();
  char c;
  struct candidate *q;
  char tildir[MAXWL];

  if (start==TRUE) { ncand=0; wordlist=carray; }
  while(1)						/* Parse each word */
   {
    for (a=old;*a!=' '&&*a!='\t'&&*a!='\n'&&*a!=0;a++);	/* Find a space */
    c= *a; *a=0; *tildir=0;				/* Null term the word */
    switch (*old)
     {
      case '!': b= gethist(++old);
		if (b) meta_1(b,FALSE);		/* Expand it too */
		break;
      case '~': tilde(old,tildir);
		old=tildir;
      default:					/* Append to the carray */
		if (start==FALSE || *tildir)
		 {
      	          carray[ncand].name=(char *)malloc((unsigned) strlen(old)+4);
        	  if (carray[ncand].name!=NULL)
         	   { strcpy(carray[ncand].name,old);
           	     carray[ncand++].mode=TRUE;
         	   }
		 }
		else
		 {
		  carray[ncand].name=old;
		  carray[ncand++].mode=FALSE;
		 }
     }
    carray[ncand-1].next=&carray[ncand];	/* Join the linked list */

    if (start==FALSE) *a=c; if (c==EOS || ncand==MAXCAN) break;
    for (old=++a; *old==' '&& *old=='\t'; old++); /* Bypass whitespace */
   }
  carray[ncand-1].next=NULL;			/* Terminate the linked list */
 }


/* Meta_2 copies the old line to the new, expanding metachars */
void meta_2()
 {
  extern int compare();
  char dir[MAXWL];
  struct candidate *curr, *a;
  int base;

  for(curr=carray;curr!=NULL;)
   {
    if (strpbrk(curr->name,"*?[\\"))	/* If we find these */
     {
      base=ncand;				/* Save start of expansion */
      dir[0]=EOS; finddir(curr->name,dir);	/* expand them */
      a=curr->next;
      if (!matchdir(dir,curr->name))
       {
        qsort((char *)&carray[base],ncand-base,sizeof(struct candidate),compare);
						/* Now insert into list */
        carray[ncand-1].next=a;
	curr->next=&carray[base];
	for (;base<ncand-1;base++) carray[base].next=&carray[base+1];
       }
      if (curr->mode==TRUE) { free(curr->name); curr->mode=FALSE; }
      curr->name=NULL;
      curr=a;
     }
    else curr=curr->next;
   }
#define DEBUG
#ifdef DEBUG
prints("meta_2: Here's the wordlist:\n");
for (curr=carray; curr!=NULL; curr=curr->next)
  if (curr->name) prints("--> %s\n",curr->name);
#endif
 }
