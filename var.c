#include "header.h"

/* Again, these routines are taken verbatim from `Advanced Unix Programming',
 * but will be replaced with the real ones eventually.
 */

char *malloc(), *realloc();

#define MAXVAR	40

static struct varslot {
	char *name;		/* Variable name */
	char *val;		/* Variable's value */
	bool exported;		/* To be exported? */
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


static bool assign(p,s)		/* Initialise name or value */
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


bool EVset(name,val)		/* Add name and value to the environment */
 char *name, *val;
 {
  struct varslot *v;

  if ((v=find(name))==NULL) return(FALSE);
  return(assign(&v->name,name) && assign(&v->val,val));
 }

	
bool EVexport(name)		/* Set variable to be exported */
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


bool EVinit()		/* Initialise symtable from environment */
 {
  extern char **environ;
  int i,namelen;
  char *name, *val;

  for (i=0; environ[i]!=NULL; i++)
   {
    name=environ[i]; val=strchr(name,'=');
    *(val++)='\0';
    if (!EVset(name,val) || !EVexport(name))
      return(FALSE);
   }
  return(TRUE);
 }

bool EVupdate()		/* Build envp from symbol table */
 {
  extern char **environ;
  int i,envi,nvlen;
  struct varslot *v;
  static bool updated=FALSE;

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

void EVprint(envonly)		/* Print environment */
 int envonly;
 {
  int i;

  if (envonly)
   {
    for (i=0; i<MAXVAR; i++)
      if (sym[i].name!=NULL && sym[i].exported)
        prints("%s=%s\n",sym[i].name,sym[i].val);
   }
  else
   {
    for (i=0; i<MAXVAR; i++)
      if (sym[i].name!=NULL)
        prints("%s=%s\n",sym[i].name,sym[i].val);
   }
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


void list(argc,argv)		/* Set command */
 int argc;
 char *argv[];
 {
  if (argc>2 || (argc==2 && strcmp(argv[1],"env")))
    prints("Usage: list [env]\n");
  else EVprint(argc-1);
 }


void export(argc,argv)		/* Export command */
 int argc;
 char *argv[];
 {
  int i;

  if (argc<2) { prints("Usage: export var [var] ...\n");  return; }
  for (i=1; i<argc; i++)
     if (!EVexport(argv[i]))
       { prints("Can't export %s\n",argv[i]); return; }
 }

void shift(argc,argv)
 int argc;
 char *argv[];
 {
  extern int Argc;
  extern char **Argv;
  int i=1;

  if (argc>2) { prints("Usage: shift [val]\n"); return; }
  if (argc==2) i=atoi(argv[1]);
  Argv += i;
  Argc -= i;
 }
