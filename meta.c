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
    carray[ncand++].mode=TRUE|C_SPACE;		/* Was malloc'd */
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
#ifdef USES_DIRECT
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
  struct passwd *entry, *getpwuid(), *getpwnam();
  char *a;

  word++;
  if (*word=='/')			/* use $HOME */
   { a= word;
     entry=getpwuid(getuid());
   }
  else
   { a=strchr(word,'/');
     if (a!=NULL) *a=EOS;
     entry=getpwnam(word);
   }
  endpwent();
  if (entry==NULL) { *dir=EOS; return; }
  strcpy(dir,entry->pw_dir);		/* Form the real partial name */
  if (a!=NULL) { *a='/'; strcat(dir,a); }
 }

/* Dollar expands a variable. It takes a pointer to a candidiate, and a 
 * pointer to the $ sign in the candidiate, and replaces it with the
 * variable's value.
 */
void dollar(cand, doll)
 struct candidate *cand;
 char *doll;
 {
  extern int Argc;
  extern char **Argv;
  char *start, *end="", *value, *EVget();
  char c=EOS;
  int mode=cand->mode&C_QUOTEBITS;
  unsigned int length, base, mall=0;	/* Mall is 1 if we've malloc'd */

  *(doll++)=EOS;			/* Remove the $ sign */

  if (!strcmp(doll,"$"))		/* Replace with pid */
   {
    value= (char *)malloc(10); length=getpid();
    if (value) { sprints(value,"%d",length); mall=1; }
   }
  else if (!strcmp(doll,"#"))		/* Replace with # of args */
   {
    value= (char *)malloc(10); length=getpid();
    if (value) { sprints(value,"%d",Argc-1); mall=1; }
   }
  else if (!strcmp(doll,"0"))		/* Replace with Argv[0] */
   {
    value= Argv[0];
   }
  else if (!strcmp(doll,"*"))		/* Insert all args into carray */
   {
    if (Argc>1)
     {
      base=ncand;			/* Save start of expansion */
      ncand += (Argc-1);
      carray[ncand-1].next=cand->next;
      carray[ncand-1].name= Argv[Argc-1];
      carray[ncand-1].mode= mode|C_SPACE;
      cand->next= &carray[base];
      for (length=1;base<ncand-1;base++,length++)
       { carray[base].name= Argv[length];
         carray[base].mode= mode|C_SPACE;
         carray[base].next= &carray[base+1];
       }
     }
    if (cand->mode&TRUE) { free(cand->name); cand->mode=FALSE; }
    cand->name=NULL;
    return;
   }
  else if ((length=atoi(doll))!=0)	/* Replace with Argv[i] */
   {
    value= (length<Argc ? Argv[length] : "" );
   }
  else 
   {					 /* Find the last char of the var */
    for (end= doll; end!=EOS && isalnum(*end); end++);
    c= *end; *end=EOS;			/* Terminate the name */
    value= EVget(doll);			/* Get the value */
    if (!value) fprints(2,"No such variable: %s\n",doll);
   }
#ifdef DEBUG
if (value) prints("Found variable $%s, value %s\n",doll,value);
#endif

  if (*(cand->name)==EOS && c==EOS && !mall)	/* If only the variable */
   {
    if (cand->mode&TRUE) free(cand->name); /* we can replace totally */
    cand->name= value;
    cand->mode= mode|C_SPACE;
   }
  else
   {					/* insert the value */
    *end=c; if (!value) value="";
    length= strlen(cand->name) + strlen(value) + strlen(end) + 1;
    start= (char *)malloc(length);
    if (start) { strcpy(start,cand->name);
		 strcat(start,value);
		 strcat(start,end);
	       }
    if (cand->mode&TRUE) free(cand->name);
    cand->name= start;
    cand->mode= mode|TRUE|C_SPACE;
   }
 }
 

/* Expline takes a linked list of words, and converts them into a normal
 * string. This is used to save history.
 */
char *expline(list)
 struct candidate *list;
 {
  struct candidate *q;
  char *out, *a, quote;
  int i,mode;
					/* Find the amount to malloc */
  for (i=0,q=list;q!=NULL;q=q->next) if (q->name) i+= strlen(q->name) +4;

  out= (char *)malloc((unsigned) i+4);
  if (out!=NULL)
   {
    for (a=out,q=list;q!=NULL;q=q->next)
      { mode=q->mode;
        if (q->name)
         { switch(mode&C_QUOTEBITS)
	    { case C_QUOTE     : quote= '\''; break;
	      case C_DBLQUOTE  : quote= '"'; break;
	      case C_BACKQUOTE : quote= '`'; break;
	      default:		 quote= EOS;
	    }
	   if (quote) *(a++)=quote;
	   strcpy(a,q->name); a+= strlen(q->name); 
	   if (quote) *(a++)=quote;
           if (q->mode&C_SPACE) *(a++)=' ';
         }
       else switch(mode&C_WORDMASK)
	{ case C_SEMI   : strcpy(a,"; ");  a+=2; break;
	  case C_PIPE   : strcpy(a,"| ");  a+=2; break;
	  case C_DBLPIPE: strcpy(a,"|| "); a+=3; break;
	  case C_AMP    : strcpy(a,"& ");  a+=2; break;
	  case C_DBLAMP : strcpy(a,"&& "); a+=3; break;
	  case C_LT     : strcpy(a,"< ");  a+=2; break;
	  case C_LTLT   : strcpy(a,"<< "); a+=3; break;
	  case C_GT     : strcpy(a,"> ");  a+=2; break;
	  case C_GTGT   : strcpy(a,">> "); a+=3; break;
        }
       *a=EOS;
      }
   }
  return(out);
 }

#ifdef DEBUG
/* Wordlist is used for debugging to print out the carray linked list */
void listcarray()
 {
  struct candidate *curr;
  prints("Here's the wordlist:\n");
  for (curr=carray; curr!=NULL; curr=curr->next)
   { prints("--> ");
     if (curr->name) prints("%s",curr->name);
     if (curr->mode&C_WORDMASK) switch(curr->mode&C_WORDMASK)
      { case C_SEMI    : prints(";    C_SEMI"); break;
        case C_PIPE    : prints("|    C_PIPE"); break;
        case C_DBLPIPE : prints("||   C_DBLPIPE"); break;
        case C_AMP     : prints("&    C_AMP"); break;
        case C_DBLAMP  : prints("&&   C_DBLAMP"); break;
        case C_LT      : prints("<    C_LT"); break;
        case C_LTLT    : prints("<<   C_LTLT"); break;
        case C_GT      : prints("%d>   C_GT",curr->mode&C_FD); break;
        case C_GTGT    : prints("%d>>  C_GTGT",curr->mode&C_FD); break;
        default	     : prints("%o  C_UNKNOWN",curr->mode);
      }
     else
      { if (curr->mode&C_SPACE) prints("   C_SPACE");
        if (curr->mode&C_QUOTE) prints("   C_QUOTE");
        if (curr->mode&C_DBLQUOTE) prints("   C_DBLQUOTE");
        if (curr->mode&C_BACKQUOTE) prints("   C_BACKQUOTE");
        if (curr->mode&TRUE) prints("   (malloc'd)");
      }
     write(1,"\n",1);
   }
}
#endif

/* Addword is used by Meta_1 to add a word into the carray list.
 * This is where history is expanded.
 */
void addword(string,mode)
 char *string;
 int mode;
 {
  bool meta_1();
  char *b, *gethist();

  if (string!=NULL)
   {
    if (*string=='!' && (b=gethist(++string))!=NULL)
      { meta_1(b,FALSE); return; }        /* Expand it too */
#ifdef OLDMALLOC
    carray[ncand].name=(char *)malloc((unsigned)strlen(string)+4);
    if (carray[ncand].name!=NULL)
     { strcpy(carray[ncand].name,string); mode|=TRUE; }
#else
    { carray[ncand].name= string; mode&= ~FALSE; }
#endif
   }
  else carray[ncand].name=NULL;
  carray[ncand].mode=mode;
  carray[ncand-1].next= &carray[ncand];
  carray[ncand++].next=NULL;
 }

/* Meta_1 takes the user's input line, and builds a linked list of words
 * in the carray. It also expands history. Meta_1 returns TRUE if
 * successful; if any errors occur, it returns FALSE. If start==TRUE, the
 * carray is made empty. i.e when called from main, use TRUE.
 */
bool meta_1(old,start)
 char *old;
 bool start;
 {
  char *a, *b, *gethist();
  char c;
  int mode;
  struct candidate *curr;

  if (start==TRUE) { ncand=0; wordlist=carray; }
  while(1)						/* Parse each word */
   { a=strpbrk(old," \t\n\"'`;|<>&=");			/* Find possible end */
    if (a!=NULL)
     {
      c= *a; *a=EOS;					/* Terminate the word */
      switch(c)
       {
        case '"' : mode=C_DBLQUOTE; goto quotes ;	/* Yuk, a goto */
        case '\'': mode=C_QUOTE; goto quotes ;
        case '`' : mode=C_BACKQUOTE;
quotes:						/* Get the quoted word */
		   if (a>old) addword(old,0);
		   old=a+1; a=strchr(old,c);
		   if (a==NULL) {fprints(2,"Unmatched %c\n",c); return(FALSE);}
		   *(a++)=EOS; if (*a==' '||*a=='\t') {a++; mode|=C_SPACE;}
		   addword(old,mode);
		   break;
        case '|' : mode=C_PIPE; goto doubles ;		/* Yuk, another goto */
        case '&' : mode=C_AMP; goto doubles ;
        case '<' : mode=C_LT;
doubles:						/* Get the symbol */
		   if (a>old) addword(old,C_SPACE);
		   a++; if (*a==c) { a++; mode+=C_DOUBLE; }
		   addword(NULL,mode);
		   break;
        case ';' : if (a>old) addword(old,C_SPACE);	/* Semicolons and > */
		   addword(NULL,C_SEMI);		/* are special */
		   a++;
		   break;
        case '=' : *a=c; c= *(++a); *a=EOS;		/* Save the char */
		   addword(old,C_SPACE); *a=c;		/* after the = */
		   break;
	case '>' : mode=1; c= *(a-1)-'0';		/* Get any fd number */
		   if (c>=0 && c<10) { mode=c; *(a-1)=EOS; }
		   if (a>old) addword(old,C_SPACE);
		   a++; if (*a=='>') { a++; mode|=C_GTGT; }
		   else mode|=C_GT;
		   addword(NULL,mode); break;
	case '\n':
	case ' ' :					/* Just add the word */
	case '\t': addword(old,C_SPACE); a++; break;
        default  : fatal("meta_1");
       }
     }
    else { addword(old,0); break; }

    if (a==NULL || ncand==MAXCAN) break;		/* No more to do */
    for (old=a; *old==' '|| *old=='\t'; old++);		/* Bypass whitespace */
   }
#ifdef DEBUG
  listcarray();
#endif
  return(TRUE);
 }

/* Meta_2 expands $ and ~ in the carray */
void meta_2()
 {
  struct candidate *curr;
  char *a, *b, tildir[MAXWL];

  for (curr=carray;curr!=NULL;curr=curr->next)
   {
    a=curr->name;
    if (a==NULL || curr->mode&C_QUOTE) continue;	/* Skip 'word' & sym */

    if ((b=strchr(a,'$')))				/* If a variable */
      dollar(curr,b);					/* expand it */

    if (*a=='~')					/* If a ~ */
      { tilde(a,tildir);				/* expand it too */
#ifdef DEBUG
        prints("tildir is %s\n",tildir);
#endif
        if (curr->mode&TRUE) free(curr->name);
        curr->name=(char *)malloc((unsigned) strlen(tildir)+4);
        if (curr->name!=NULL)
          { strcpy(curr->name,tildir); curr->mode=TRUE|C_SPACE; }
      }
   }
#ifdef DEBUG
  listcarray();
#endif
 }


/* Meta_3 expands * ? and [] in the carray */
void meta_3()
 {
  extern int compare();
  char dir[MAXWL];
  struct candidate *curr, *a;
  int base;

  for(curr=carray;curr!=NULL;)
   {						/* Skip quoted words & syms */
    if (curr->mode&C_QUOTEBITS || curr->mode&C_WORDMASK || curr->name==NULL)
      {curr=curr->next; continue;}
						/* If we find these */
    if (strpbrk(curr->name,"*?["))
     {
      base=ncand;				/* Save start of expansion */
      dir[0]=EOS; finddir(curr->name,dir);	/* expand them */
      a=curr->next;
      if (!matchdir(dir,curr->name))
       {
        qsort((char*)&carray[base],ncand-base,sizeof(struct candidate),compare);
						/* Now insert into list */
        carray[ncand-1].next=a;
	curr->next= &carray[base];
	for (;base<ncand-1;base++) carray[base].next= &carray[base+1];
       }
      if (curr->mode&TRUE) { free(curr->name); curr->mode=FALSE; }
      curr->name=NULL;
      curr=a;
      continue;
     }
    curr=curr->next;
   }
#ifdef DEBUG
  listcarray();
#endif
 }
