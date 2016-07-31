#include "header.h"

int export(), list(), history(), echo(), shift();
int Bind(), unbind(), Cd(), source(), alias(), unalias();
#ifdef JOB
int bg(), fg(), joblist();
#endif

struct builptr {
	char *name;
	int  (*fptr)();
} buillist[]= {
	{ "cd",		Cd }	  ,
	{ "echo",	echo }	  ,
	{ "shift",	shift }	  ,
	{ "list",	list }	  ,
	{ "export",	export }  ,
	{ "history",	history } ,
	{ "bind",	Bind }	  ,
	{ "unbind",	unbind }  ,
	{ "source",	source }  ,
	{ "alias",	alias }   ,
	{ "unalias",	unalias } ,
#ifdef JOB
	{ "bg",		bg }	  ,
	{ "fg",		fg }	  ,
	{ "jobs",	joblist } ,
#endif
	{ NULL,		NULL } };
    

int echo(argc,argv)
 int argc;
 char *argv[];
 {
  int doreturn=1;
  int firstarg=1;

  if (argc>1 && !strcmp(argv[1],"-n")) { doreturn=0; firstarg=2; }

  for (;firstarg<argc;firstarg++)
   {
    (void)write(1,argv[firstarg],strlen(argv[firstarg]));
    (void)write(1," ",1);
   }
  if (doreturn) (void)write(1,"\n",1);
  return(0);
 }

/* Here is the global definition for the current directory variable */
char currdir[128];

int Cd(argc,argv)
 int argc;
 char *argv[];
 {
  char *path,*EVget();
#ifdef UCB
  char *getwd();
#else
  char *getcwd();
#endif

  if (argc>1) path=argv[1];
  else if((path=EVget("HOME"))==NULL) path=".";
  if (chdir(path)==-1)
	{ fprints(2,"%s: bad directory\n",path); return(1); }
#ifdef UCB
  if (getwd(currdir))
#else
  if (getcwd(currdir,MAXPL))
#endif
    { EVset("cwd",currdir); return(0); }
  else { (void)write(2,"Can't get cwd properly\n",23); return(1); }
 }

int builtin(argc,argv)		/* Do builtin. This returns either the */
 int argc;			/* positive int value of the builtin, */
 char *argv[];			/* or -1 indicating there was no builtin */
 {
  struct builptr *bptr;

  if (strchr(argv[0],'=')!=NULL)
	return(asg(argc,argv));
  for (bptr=buillist; bptr->name!=NULL; bptr++)
    if (!strcmp(argv[0],bptr->name))
	return((*(bptr->fptr))(argc,argv));
  return(-1);
 }
