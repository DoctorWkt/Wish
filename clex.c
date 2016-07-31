#include "header.h"

/* Here is the declaration of the candidate array. This is used in meta.c
 * and probably in parse.c, as well as several million other places.
 */
struct candidate carray[MAXCAN];

static int numcand,maxlen;	/* Holds # of candidates */

/* Compare is the routine used by qsort to reorder the elements
 * in the carray.
 */
int compare(a,b)
  struct candidate *a,*b;
{
  return(strcmp(a->name,b->name));
}


/* Print out the maxlen partial match on the word, placing the result at
 * pos in the given line.
 */
static void extend(line,pos,word)
  char *line;
  int *pos;
  char *word;
 {
  extern char beep[];
  extern int beeplength;
  int i,j,nostop=1;
  char *newword, *t;

  if (*word=='~') word++;
  t= strrchr(word,'/');		/* Go to the last '/' */
  if (t!=NULL) word= ++t;

  switch(numcand)
   {
    case 0:  Beep; return;
    case 1:  newword= carray[0].name;
    default: if ((newword=(char *)malloc((unsigned)maxlen+2))==NULL) return;
  	     for (i=0; i<maxlen+1; i++) newword[i]=EOS;
	     strcpy(newword,word);		/* Set up as much as we have */

             for (i=strlen(word);nostop && i<maxlen;i++)
              for (j=0;j<numcand;j++)
               {
		 if (strlen(carray[j].name)<=i) { nostop=0; break; }
                 if (newword[i]==0) newword[i]=carray[j].name[i];
                 if (newword[i]!=carray[j].name[i])
		   { newword[i]=EOS; nostop=0; break; }
               }
   }
  for (i=strlen(word); i<strlen(newword); i++)
	insert(line,(*pos)++,newword[i]);

  if (numcand==1)
   {
    if ((carray[0].mode & S_IFMT)== S_IFDIR)
	insert(line,(*pos)++,'/');
    else
	insert(line,(*pos)++,' ');
   }
  else { Beep; free(newword); }
 }


/* Print out the candidates found in columns.
 */
static void colprint()
{
  extern int wid;
  int i,j,collength,numperline,index;
  char format[6];

  maxlen+=2;
  numperline=wid/maxlen;
  collength=numcand/numperline;
  if (numcand % numperline) collength++;
  sprints(format,"%%%ds",maxlen);
  for (i=0;i<collength;i++)
  {
    write(1,"\n",1);
    for (j=0;j<numperline;j++)
    {
      index=i+j*collength;
      if (index>=numcand) break;
      if ((carray[index].mode & S_IFMT) == S_IFDIR)
	strcat(carray[index].name,"/");
#ifdef S_IFLNK
      else
      if ((carray[index].mode & S_IFMT) == S_IFLNK)
	strcat(carray[index].name,"@");
#endif
      else
      if (carray[index].mode & 0111)
	strcat(carray[index].name,"*");
      prints(format,carray[index].name);
    }
  }
  write(1,"\n",1);
}


/* Find the name of a file, given a partial word to match against. The
 * word may be an absolute path name, or a relative one. Any matches
 * against the word are added to the carray.
 */
static void findfile(word)
 char *word;
 {
  extern char currdir[];
  int i,j;
  char partdir[MAXWL];
  char *match;
  DIR *dirp;
#ifdef USES_DIRECT
  struct direct *entry, *readdir();
#else
  struct dirent *entry, *readdir();
#endif
  struct stat statbuf;

  for (i=0; i<MAXWL; i++) partdir[i]=EOS;

  if (word!=NULL && *word!=EOS)
   {
			/* Make full pathname */
    if (*word!='/') { strcpy(partdir,currdir);
		      strcat(partdir,"/"); }
    strcat(partdir,word);

			/* Find the directory name */
    if ((match=strrchr(partdir,'/'))==NULL)
      { prints("Looney! No / in word %s\n",partdir); return; }
    *match=EOS; match++;
   
    i=strlen(match);
   }
  else { strcpy(partdir,currdir); i=0; }

  if ((dirp=opendir(partdir))==NULL)
    { prints("Could not open the directory %s\n",partdir); return; }

  if (chdir(partdir)==0)
    while ((entry=readdir(dirp))!=NULL && numcand<MAXCAN)
     {
				/* Ignore dot and dot-dot */
      if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;

				/* If we find a partial match add to list */
      if (i==0 || !strncmp(entry->d_name,match,i))
        {
	  j= strlen(entry->d_name);
	  carray[numcand].name= (char *)malloc((unsigned)(j+4));

	  if (carray[numcand].name==NULL) break;
	  strcpy(carray[numcand].name,entry->d_name);

	  if (j>maxlen) maxlen=j;

				/* and get the mode as well */
	  if (stat(entry->d_name,&statbuf)==0)
	    carray[numcand++].mode= statbuf.st_mode;
	  else carray[numcand++].mode=0;
        }
    }
  closedir(dirp);
  chdir(currdir);
 }


/* Find the name of a file, or a user, given the partial word. If there
 * are no slashes, just go for a user, else get the home dir & call
 * findfile. Any matches against the word are added to the carray.
 */
static void findpasswd(word)
 char *word;
 {
  int i,j;
  char dir[MAXWL];
  char *a;

  struct passwd *entry, *getpwent();

  if ((a=strchr(word,'/'))!=NULL)	/* We have to find a file */
   {
    tilde(word,dir);
    if (*dir!=0) findfile(dir);
    return;
   }

  word++;				/* Skip over tilde */
  i=strlen(word);
  while((entry=getpwent())!=NULL && numcand<MAXCAN)
   {
    if (i==0 || !strncmp(entry->pw_name,word,i))
     {
      j= strlen(entry->pw_name);
      carray[numcand].name= (char *)malloc((unsigned)(j+2));
 
      if (carray[numcand].name==NULL) break;
      strcpy(carray[numcand].name,entry->pw_name);
 
      if (j>maxlen) maxlen=j;
      carray[numcand++].mode= S_IFDIR;
     }
   }
  endpwent();
 }

/* Find the name of a file by looking through $PATH.
 * Any matches against the word are added to the carray.
 */
static void findbin(word)
 char *word;
 {
  extern char *EVget();
  int i;
  char word2[MAXWL];
  char *Path, *thispath, *temp;
  
  temp=EVget("PATH");		/* Get the PATH value */
  if (temp==NULL) return;
  Path=thispath= (char *)malloc((unsigned)strlen(temp)+4);
  if (thispath==NULL) return;
  strcpy(thispath,temp);
  while (thispath!=NULL)
   {
    for (i=0; i<MAXWL; i++) word2[i]=EOS;
    temp=thispath; while (*temp!=EOS && *temp!=' ' && *temp!=':') temp++;
    if (*temp==EOS) temp=NULL;
    else *(temp++)=EOS;

    strcpy(word2,thispath);
    strcat(word2,"/");
    if (word!=NULL || *word!=EOS) strcat(word2,word);

    findfile(word2);

    while (temp!=NULL && *temp!=EOS && (*temp==' ' || *temp==':')) temp++;
    if (temp==NULL || *temp==EOS) thispath=NULL;
    else thispath=temp;
   }
  free(Path);
 }


/* Complete subsumes the work of two routines in old Clam, depending on how:
 *
 * case 0: 	Print out a columnated list of files that match the word
 *		at position pos. The line is unchanged.
 * case 1:	Try to complete as much as possible the word at pos, by
 *		using the find routines above. Add the completion to the line.
 */
void complete(line,pos,how)
 char *line;
 int *pos;
 bool how;
 {
  extern char beep[], yankbuf[];
  extern int beeplength;
  int startpos;

  numcand=maxlen=0;

  if ((*pos==0) || (line[(*pos)-1]==' '))
    { strcpy(yankbuf,""); startpos= *pos; }	/* nothing there to get */
  else				/* first get the thing we've got so far */
    startpos= yankprev(line,*pos);
  
  if (startpos==0)
    {
     if (*yankbuf=='.' || *yankbuf=='/') findfile(yankbuf);
     else findbin(yankbuf);
    }
  else
    {
     if (*yankbuf=='~') findpasswd(yankbuf);
     else findfile(yankbuf);
    }

  if (how==TRUE)
     extend(line,pos,yankbuf);
  else
   {
    if (numcand==0) Beep;
    else
     {
      qsort((char *)carray,numcand,sizeof(struct candidate),compare);
      colprint();
      prprompt();
      show(line,TRUE);
     }
   }
   while ((--numcand)>=0)
     free(carray[numcand].name);
 }
