#include "header.h"

bool echo(argc,argv)
 int argc;
 char *argv[];
 {
  int doreturn=1;
  int firstarg=1;

  if (argc>1 && !strcmp(argv[1],"-n")) { doreturn=0; firstarg=2; }

  for (;firstarg<argc;firstarg++)
   {
    write(1,argv[firstarg],strlen(argv[firstarg]));
    write(1," ",1);
   }
  if (doreturn) write(1,"\n",1);
  return(TRUE);
 }

bool builtin(argc,argv)		/* Do builtin */
 int argc;
 char *argv[];
 {
  extern char currdir[];
  char *path,*EVget();

  if (strchr(argv[0],'=')!=NULL)
		asg(argc,argv);
  else if (!strcmp(argv[0],"export"))
		export(argc,argv);
  else if (!strcmp(argv[0],"list"))
		list(argc,argv);
  else if (!strcmp(argv[0],"history"))
		history(argc,argv);
  else if (!strcmp(argv[0],"echo"))
		echo(argc,argv);
  else if (!strcmp(argv[0],"shift"))
		shift(argc,argv);
#ifdef JOB
  else if (!strcmp(argv[0],"bg"))
		bg(argc,argv);
  else if (!strcmp(argv[0],"fg"))
		fg(argc,argv);
  else if (!strcmp(argv[0],"jobs"))
		joblist(argc,argv);
#endif
  else if (!strcmp(argv[0],"cd"))
	 {
	  if (argc>1) path=argv[1];
	  else if((path=EVget("HOME"))==NULL) path=".";
	  if (chdir(path)==-1)
		{ fprints(2,"%s: bad directory\n",path); return(TRUE); }
#ifdef UCB
	  if (getwd(currdir))
#else
	  if (getcwd(currdir,MAXPL))
#endif
	    EVset("cwd",currdir);
	  else write(2,"Can't get cwd properly\n",23);
	 }
  else return(FALSE);
  return(TRUE);
 }
