#include "header.h"

BOOLEAN getargs(argcp,argv,max)		/* Read and parse args */
 int *argcp;
 char *argv[];
 int max;
 {
  static char cmd[100];
  char *cmdp, *strtok();
  int i;

  if (gets(cmd)==NULL) exit(0);
  cmdp=cmd;
  for (i=0; i<=max; i++)
   {
    if ((argv[i]=strtok(cmdp," \t"))==NULL) break;
    cmdp=NULL;		/* Tell strtok to keep going */
   }
  if (i>max)
    { printf("Too many args\n"); return(FALSE); }
  *argcp=i;
  return(TRUE);
 }
