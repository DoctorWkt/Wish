#include "header.h"

/* Given a word, fill up an array of partial matches to that word */

struct candidate { char *name;
		   int mode;
	};


static struct candidate carray[MAXCAN];
static int numcand,maxlen;


static int compare(a,b)
  struct candidate *a,*b;
{
  return(strcmp(a->name,b->name));
}


/* Print out the maxlen partial match on the word
 */
static void extend(line,pos,curs,word)
  char *line;
  int *pos,curs[];
  char *word;
 {
  extern char beep[];
  extern int beeplength;
  int i,j,nostop=1;
  char *newword, *t;

  t= strrchr(word,'/');		/* Go to the last '/' */
  if (t!=NULL) word= ++t;

  switch(numcand)
   {
    case 0:  write(1,beep,beeplength); return;
    case 1:  newword= carray[0].name;
    default: if ((newword=(char *)malloc((unsigned)maxlen+1))==NULL) return;
  	     for (i=0; i<maxlen+1; i++) newword[i]=EOS;
	     strcpy(newword,word);		/* Set up as much as we have */

             for (i=strlen(word);nostop && i<maxlen;i++)
              for (j=0;j<numcand;j++)
               { if (strlen(carray[j].name)<i) continue;
                 if (newword[i]==0) newword[i]=carray[j].name[i];
                 if (newword[i]!=carray[j].name[i])
		   { newword[i]=EOS; nostop=0; break; }
               }
   }
#ifdef DEBUG
prints("\n%s\n",newword);
#endif
  for (i=strlen(word); i<strlen(newword); i++)
	insert(line,(*pos)++,newword[i],curs);

  if (numcand==1)
   {
    if ((carray[0].mode & S_IFMT)== S_IFDIR)
	insert(line,(*pos)++,'/',curs);
    else
	insert(line,(*pos)++,' ',curs);
   }
  else { write(1,beep,beeplength); free(newword); }
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
      if ((carray[index].mode & S_IFMT) == S_IFLNK)
	strcat(carray[index].name,"@");
#endif
      if (carray[index].mode & 0111)
	strcat(carray[index].name,"*");
      prints(format,carray[index].name);
      fflush(stdout);
    }
  }
  write(1,"\n",1);
}


static void findfile(word)
 char *word;
 {
  extern char currdir[];
  int i,j;
  char partdir[MAXWL];
  char *match;
  DIR *dirp;
  struct dirent *entry;
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
	  carray[numcand].name= (char *)malloc((unsigned)(j+1));

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


static void findpasswd(word)
 char *word;
 {
  int i,j;

  struct passwd *entry;

  if (word!=NULL && *word!=EOS)
    i=strlen(word);
  else i=0;

  while((entry=getpwent())!=NULL && numcand<MAXCAN)
   {
    if (i==0 || !strncmp(entry->pw_name,word,i))
     {
      j= strlen(entry->pw_name);
      carray[numcand].name= (char *)malloc((unsigned)(j+1));
 
      if (carray[numcand].name==NULL) break;
      strcpy(carray[numcand].name,entry->pw_name);
 
      if (j>maxlen) maxlen=j;
      carray[numcand++].mode= S_IFDIR;
     }
   }
  endpwent();
 }

static void findbin(word)
 char *word;
 {
  extern char *EVget();
  int i;
  char word2[MAXWL];
  char *thispath;
  char *temp;
  
  temp=EVget("PATH");		/* Get the PATH value */
  if (temp==NULL) return;
  thispath= (char *)malloc((unsigned)strlen(temp)+1);
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
  free(thispath);
 }


void complete(line,pos,curs,how)
 char *line;
 int *pos,curs[];
 bool how;
 {
  extern char beep[];
  extern int beeplength;
  char word[MAXWL];
  int startpos;

  numcand=maxlen=0;

  if ((*pos==0) || (line[(*pos)-1]==' '))
    { strcpy(word,""); startpos=*pos; }	/* nothing there to get */
  else				/* first get the thing we've got so far */
    startpos= yankprev(line,*pos,word);
  
  if (startpos==0)
    {
     if (*word=='.' || *word=='/') findfile(word);
     else findbin(word);
    }
  else
    {
     if (*word=='~') findpasswd(&word[1]);
     else findfile(word);
    }

  if (how==TRUE)
     extend(line,pos,curs,word);
  else
   {
    if (numcand==0) write(1,beep,beeplength);
    else
     {
      qsort((char *)carray,numcand,sizeof(struct candidate),compare);
      colprint();
      prprompt();
      show(line,curs,TRUE);
     }
   }
  while ((--numcand) >=0) free(carray[numcand].name);
 }
