#include "header.h"
#define MAXMATCH 512
#define OK	0
#define NX_ERR -1
#define BR_ERR -2

extern struct candidate carray[];	/* The matched files */
static int numcand;
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
    if (numcand==MAXCAN) return(OK);
    carray[numcand].name=(char *) malloc ((unsigned)(strlen(where)+4));
    if (carray[numcand].name==NULL) return(OK);
    strcpy(carray[numcand].name,where);
    carray[numcand++].mode=TRUE;		/* Was malloc'd */
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
  struct direct *entry,*readdir();
#else
  struct dirent *entry,*readdir();
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


/* Meta_1 takes the user's input line, and builds a linked list of words
 * in the carray. It also expands tildes.
 */
void meta_1(old)
 char *old;
 {
  char *a;
  struct candidate *q;
  char tildir[MAXWL];

  numcand=0; wordlist=carray;
  while(*old!=0)				/* Parse each word */
   {
    for (a=old;*a!=' '&&*a!='\t'&&*a!='\n'&&*a!=0;a++);	/* Find a space */
    *a=0;					/* Null term the word */
    if (*old=='~')
     {
      tilde(old,tildir);
      if (*tildir!=0)				/* Append to the carray */
       {
        carray[numcand].name=(char *)malloc((unsigned)strlen(tildir)+4);
        if (carray[numcand].name==NULL)
         { carray[numcand].name=old;
           carray[numcand++].mode=FALSE;
         }
        else 
         { strcpy(carray[numcand].name,tildir);
           carray[numcand++].mode=TRUE;
         }
       }
     }
    else
     { carray[numcand].name=old;
       carray[numcand++].mode=FALSE;		/* (not malloc'd) */
     }
    carray[numcand-1].next=&carray[numcand];	/* Join the linked list */
    if (numcand==MAXCAN) break;

    for (old=++a; *old==' '&& *old=='\t'; old++); /* Bypass whitespace */
   }
  carray[numcand-1].next=NULL;			/* Terminate the linked list */
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
      base=numcand;				/* Save start of expansion */
      dir[0]=EOS; finddir(curr->name,dir);	/* expand them */
      a=curr->next;
      if (!matchdir(dir,curr->name))
       {
        qsort((char *)&carray[base],numcand-base,sizeof(struct candidate),compare);
						/* Now insert into list */
        carray[numcand-1].next=a;
	curr->next=&carray[base];
	for (;base<numcand-1;base++) carray[base].next=&carray[base+1];
       }
      if (curr->mode==TRUE) { free(curr->name); curr->mode=FALSE; }
      curr->name=NULL;
      curr=a;
     }
    else curr=curr->next;
   }
#ifdef DEBUG
prints("meta_1: Here's the wordlist:\n");
for (curr=carray; curr!=NULL; curr=curr->next)
  prints("--> %s\n",curr->name);
#endif
 }
