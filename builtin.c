#include "header.h"

BOOLEAN builtin(argc,argv)		/* Do builtin */
 int argc;
 char *argv[];
 {
  char *path,*EVget();;

  if (strchr(argv[0],'=')!=NULL)
		asg(argc,argv);
  else if (!strcmp(argv[0],"export"))
		export(argc,argv);
  else if (!strcmp(argv[0],"set"))
		set(argc,argv);
#ifdef JOB
  else if (!strcmp(argv[0],"bg"))
		bg(argc,argv);
  else if (!strcmp(argv[0],"fg"))
		fg(argc,argv);
#endif
  else if (!strcmp(argv[0],"cd"))
	 {
	  if (argc>1) path=argv[1];
	  else if((path=EVget("HOME"))==NULL) path=".";
	  if (chdir(path)==-1)
		fprints(2,"%s: bad directory\n",path);
	 }
  else return(FALSE);
  return(TRUE);
 }
