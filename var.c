#include "header.h"

/* The structure that holds the variables */

static struct varslot {
	char *name;		/* Variable name */
	char *val;		/* Variable's value */
	bool exported;		/* To be exported? */
	struct varslot *next;	/* Pointer to next variable */
	};

static struct varslot *vtop;	/* Pointer to the linked list of vars */

static bool assign(p,s)		/* Initialise name or value */
 char **p, *s;
 {
  int size;

  size=strlen(s)+1;
  if (*p==NULL)
   {
    if ((*p=(char *)malloc(size))==NULL) return(FALSE);
   }
  else if ((*p=(char *)realloc(*p,size))==NULL) return(FALSE);
  strcpy(*p,s);
  return(TRUE);
 }

/* Find returns a pointer to the varslot which
 * holds the variable with given name. If no
 * variable exists, it returns NULL. If value is
 * non-null, a slot is created if none exist, and
 * the value is overwritten.
 */
static struct varslot *find(name,value)	/* Find a symbol table entry */
 char *name, *value;
 {
  struct varslot *v, *w;

  for (w=v=vtop;v!=NULL;w=v,v=v->next)
    if (v->name!=NULL && !strcmp(v->name,name)) break;
  if (value==NULL) return(v);

  				/* Otherwise update the value */
  if (v==NULL)
   { v=(struct varslot *)malloc((unsigned)sizeof(struct varslot));
     if (v==NULL) { perror("find"); return(v); }
     assign(&v->name,name);
     v->exported=FALSE;
   }
  assign(&v->val,value);
  if (vtop==NULL) vtop=v; else w->next=v;
  return(v);
 }



bool EVset(name,val)		/* Add name and value to the environment */
 char *name, *val;
 {
  if ((find(name,val))==NULL) return(FALSE);
  return(TRUE);
 }

	
bool EVexport(name)		/* Set variable to be exported */
 char *name;
 {
  struct varslot *v;

  if ((v=find(name,NULL))==NULL) return(FALSE);
  v->exported=TRUE;
  return(TRUE);
 }


char *EVget(name)		/* Get value of variable */
 char *name;
 {
  struct varslot *v;

  if ((v=find(name,NULL))==NULL || v->name==NULL) return(NULL);
  return(v->val);
 } 


bool EVinit()		/* Initialise symtable from environment */
 {
  extern char **environ;
  int i;
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

  for (i=0,v=vtop;v!=NULL;v=v->next)
    i+=v->exported;

  if (!updated)
    if ((environ=(char **)malloc((i+1)*sizeof(char *)))==NULL)
      return(FALSE);
  envi=0;
  for (v=vtop;v!=NULL;v=v->next)
   {
    if (v->name==NULL || !v->exported) continue;
    nvlen=strlen(v->name) + strlen(v->val) + 2;
    if (!updated)
     {
      if ((environ[envi]=(char *)malloc(nvlen))==NULL)
        return(FALSE);
     }
    else if ((environ[envi]=(char *)realloc(environ[envi],nvlen))==NULL)
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
  struct varslot *v;

  if (envonly)
   {
    for (v=vtop;v!=NULL;v=v->next)
      if (v->name!=NULL && v->exported)
        prints("%s=%s\n",v->name,v->val);
   }
  else
   {
    for (v=vtop;v!=NULL;v=v->next)
      if (v->name!=NULL)
        prints("%s=%s\n",v->name,v->val);
   }
 }


void asg(argc,argv)		/* Assignment command */
 int argc;
 char *argv[];
 {
  char *name, *val, *strtok();

  if (argc<2) val="";
  else
    val=argv[1];
  name=strtok(argv[0],"=");
  if (!EVset(name,val))
      prints("Can't assign\n");
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
