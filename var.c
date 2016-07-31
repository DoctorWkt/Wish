#include "header.h"
char *malloc(), *realloc();

#define MAXVAR	40

static struct varslot {
	char *name;		/* Variable name */
	char *val;		/* Variable's value */
	BOOLEAN exported;	/* To be exported? */
	} sym[MAXVAR];

static struct varslot *find(name)	/* Find a symbol table entry */
 char *name;
 {
  int i;
  struct varslot *v;

  v=NULL;
  for (i=0; i<MAXVAR; i++)
    if (sym[i].name==NULL)
     {
      if (v==NULL) v= &sym[i];
     }
    else
      if (!strcmp(sym[i].name,name))
       {
	v= &sym[i]; break;
       }
  return(v);
 }


static BOOLEAN assign(p,s)		/* Initialise name or value */
 char **p, *s;
 {
  int size;

  size=strlen(s)+1;
  if (*p==NULL)
   {
    if ((*p=malloc(size))==NULL) return(FALSE);
   }
  else if ((*p=realloc(*p,size))==NULL) return(FALSE);
  strcpy(*p,s);
  return(TRUE);
 }


BOOLEAN EVset(name,val)		/* Add name and value to the environment */
 char *name, *val;
 {
  struct varslot *v;

  if ((v=find(name))==NULL) return(FALSE);
  return(assign(&v->name,name) && assign(&v->val,val));
 }

	
BOOLEAN EVexport(name)		/* Set variable to be exported */
 char *name;
 {
  struct varslot *v;

  if ((v=find(name))==NULL) return(FALSE);
  if (v->name==NULL)
    if (!assign(&v->name,name) || !assign(&v->val,"")) return(FALSE);
  v->exported=TRUE;
  return(TRUE);
 }


char *EVget(name)		/* Get value of variable */
 char *name;
 {
  struct varslot *v;

  if ((v=find(name))==NULL || v->name==NULL) return(NULL);
  return(v->val);
 } 


BOOLEAN EVinit()		/* Initialise symtable from environment */
 {
  extern char **environ;
  int i,namelen;
  char name[20];

  for (i=0; environ[i]!=NULL; i++)
   {
    namelen=strcspn(environ[i],"=");
    strncpy(name,environ[i],namelen);
    name[namelen]='\0';
    if (!EVset(name,&environ[i][namelen+1]) || !EVexport(name))
      return(FALSE);
   }
  return(TRUE);
 }

BOOLEAN EVupdate()		/* Build envp from symbol table */
 {
  extern char **environ;
  int i,envi,nvlen;
  struct varslot *v;
  static BOOLEAN updated=FALSE;

  if (!updated)
    if ((environ=(char **)malloc((MAXVAR+1)*sizeof(char *)))==NULL)
      return(FALSE);
  envi=0;
  for (i=0; i<MAXVAR; i++)
   {
    v= &sym[i];
    if (v->name==NULL || !v->exported) continue;
    nvlen=strlen(v->name) + strlen(v->val) + 2;
    if (!updated)
     {
      if ((environ[envi]=malloc(nvlen))==NULL)
        return(FALSE);
     }
    else if ((environ[envi]=realloc(environ[envi],nvlen))==NULL)
           return(FALSE);
    sprints(environ[envi],"%s=%s",v->name,v->val);
    envi++;
   }
  environ[envi]=NULL;
  updated=TRUE;
  return(TRUE);
 }

void EVprint()		/* Print environment */
 {
  int i;

  for (i=0; i<MAXVAR; i++)
    if (sym[i].name!=NULL)
      prints("%d %3s %s=%s\n",i,sym[i].exported ? "[E]" : "",
		sym[i].name,sym[i].val);
 }


void asg(argc,argv)		/* Assignment command */
 int argc;
 char *argv[];
 {
  char *name, *val, *strtok();

  if (argc!=1) prints("Extra args\n");
  else
   {
    name=strtok(argv[0],"=");
    val=strtok(NULL,"\1");	/* get all that's left */
    if (!EVset(name,val))
      prints("Cant assign\n");
   }
 }


void set(argc,argv)		/* Set command */
 int argc;
 char *argv[];
 {
  if (argc!=1) prints("Extra args\n");
  else EVprint();
 }


void export(argc,argv)		/* Export command */
 int argc;
 char *argv[];
 {
  int i;

  if (argc==1) { set(argc,argv); return; }
  for (i=0; i<argc; i++)
    if (!EVexport(argv[i]))
     { prints("Can't export %s\n",argv[i]); return; }
 }
