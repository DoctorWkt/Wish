/* This file contains functions relevant to aliases, including creation 
 * in two ways, listing, interpreting.
 */

#include "header.h"

struct alias *atop=NULL;	/* The pointer the the list of aliases */
struct adefn *aline;		/* Pointer to each line of the alias */

/* Checkalias returns a pointer to the alias names by aname. If
 * no alias is found, it returns NULL. It also points aline to the
 * beginning of the alias definition.
 */
#ifdef PROTO
struct adefn *checkalias ( char *aname )
#else
struct adefn *checkalias(aname)
  char *aname;
#endif
{
  struct alias *aptr;
  int lex;

/* Aliases are stored in alphabetical order. We exit the loop as soon as
 * we've passed the spot it can be in. If found, return definition pointer.
 */
  for (aptr=atop;aptr;aptr=aptr->next)
    if ((lex=strcmp(aname,aptr->a_name))==0)
      { aline=aptr->defin; return(aline); }
    else if (lex<0)
	   return((struct adefn *)NULL);
  return((struct adefn *)NULL);
}

/* Getaliasline returns the alias one line at a time.
 * It depends on having aline initialised correctly.
 */
bool getaliasline(line, nosave)
 char *line;
 int *nosave;
 {
  int i;

  *nosave=0;
  if (aline==NULL) return(FALSE);	/* No more lines left */
  strcpy(line,aline->a_line);
  aline=aline->nextln;
  return(TRUE);
 }

/* Aliaslist prints out either the entry for the alias aname,
 * or all the aliases if aname is NULL.
 */
#ifdef PROTO
void aliaslist ( char *aname )
#else
void aliaslist(aname)
  char *aname;
#endif
{
  extern void mprint();
  struct alias *aptr;
  struct adefn *dptr, *d_top;

  if (aname!=NULL)  			/* single alias definition */
  {
    d_top=checkalias(aname);
    if (d_top!=NULL)
      for (dptr=d_top;dptr!=NULL;dptr=dptr->nextln)
	mprint(dptr->a_line,0);
    else write(2,"No such alias.\n",15);
    return;
  }
  for (aptr=atop;aptr;aptr=aptr->next)	/* print out all aliases */
  {
    mprint(aptr->a_name,1);
    for (dptr=aptr->defin;dptr;dptr=dptr->nextln)
    {
      write(1,"\t",1);
      mprint(dptr->a_line,0);
    }
  }
}

/* Savealias saves the alias named by aname into the file whose
 * name is given by fname.
 */
#ifdef PROTO
void savealias ( char *aname , *fname )
#else
void savealias(aname,fname)
  char *aname, *fname;
#endif
{
  int fp;
  struct alias *aptr;
  struct adefn *dptr;

  if ((fp=open(fname,O_WRONLY|O_CREAT|O_TRUNC))==-1) return;

  for (aptr=atop;aptr;aptr=aptr->next)
    if (!strcmp(aname,aptr->a_name))
      for (dptr=aptr->defin;dptr;dptr=dptr->nextln)
	fprints(fp,"%s\n",dptr->a_line);
  close(fp);
}

/* @@@ @@@ The following function appears to be too big. Trim it!!!
 */
#ifdef PROTO
void addalias ( char *aname , struct adefn *stdefn )
#else
void addalias(aname,stdefn)
  char *aname;
  struct adefn *stdefn;
#endif
{
  struct alias *aptr,*old,*new;
  struct adefn *dptr,*d;
  int i,lex=(-1);

#ifdef DEBUG
fprints(2,"In addalias\n");
#endif
/* search from top of list until we get to the point that is greater than or
   equal to the alias name. i is 0 if aname less than all. Set lex to -1 in
   case loop never executes and we make the first one. */

  for (i=0,old=aptr=atop;aptr;i++,old=aptr,aptr=aptr->next)
    if ((lex=strcmp(aname,aptr->a_name))==0)
      break;
    else if (lex<0)
	 break;

#ifdef DEBUG
fprints(2,"out of alias search loop\n");
#endif
  if (lex<0)
    if (i)		/* normal insertion between old and aptr */
    {
      new=(struct alias *) malloc ((unsigned)(sizeof(struct alias)));
      if (new==NULL)
      {
	perror("addalias1");
	return;
      }
      old->next=new;			/* insert the new alias node */
      new->next=aptr;
      new->defin=stdefn;		/* point at new definition */
      new->a_name=(char *) malloc ((unsigned)(strlen(aname)+1));
      if (new->a_name==NULL)
      {
	perror("addalias2");
	return;
      }
      strcpy(new->a_name,aname);
    }
    else		/* insertion before atop. old=aptr=atop still */
    {
      old=(struct alias *) malloc ((unsigned)(sizeof(struct alias)));
      if (old==NULL)
      {
	perror("addalias3");
	return;
      }
      old->next=atop;
      old->a_name=(char *) malloc ((unsigned)(strlen(aname)+1));
      if (old->a_name==NULL)
      {
	perror("addalias4");
	return;
      }
      strcpy(old->a_name,aname);
      old->defin=stdefn;
      atop=old;
    }
  else
    if (lex==0)		/* alias already exists, just change definition */
    {
      for (dptr=aptr->defin;dptr;dptr=d)
      {
	d=dptr->nextln;
	free(dptr->a_line);
	free(dptr);
      }
      aptr->defin=stdefn;
    }
    else		/* alias doesn't exist, append to list. */
    {
      new=(struct alias *) malloc ((unsigned)(sizeof(struct alias)));
      if (new==NULL)
      {
	perror("addalias5");
	return;
      }
      new->next=0;
      old->next=new;
      new->a_name=(char *) malloc ((unsigned)(strlen(aname)+1));
      if (new->a_name==NULL)
      {
	perror("addalias6");
	return;
      }
      strcpy(new->a_name,aname);
      new->defin=stdefn;
    }
#ifdef DEBUG
fprints(2,"finished addalias\n");
#endif
}

#ifdef PROTO
void makelinealias ( char *aname , char *aline )
#else
void makelinealias(aname,aline)
  char *aname,*aline;
#endif
{
  struct adefn *ptr;

#ifdef DEBUG
fprints(2,"In makelinealias\n");
#endif
  ptr=(struct adefn *) malloc ((unsigned)(sizeof(struct adefn)));
  if (ptr==NULL)
  {
    perror("mlinalias1");
    return;
  }
  ptr->a_line=(char *) malloc ((unsigned)(strlen(aline)+1));
  if (ptr->a_line==NULL)
  {
    perror("maklinalias2");
    return;
  }
  strcpy(ptr->a_line,aline);
  ptr->nextln=0;
  addalias(aname,ptr);
#ifdef DEBUG
fprints(2,"finished makelinealias\n");
#endif
}

#ifdef PROTO
void makealias ( char *aname , char *filename)
#else
void makealias(aname, filename)
  char *aname, *filename;
#endif
{
  extern bool fileopen(), getfileline();
  extern void fileclose();
  struct adefn *defn_top,*dptr;
  char line[MAXLL];
  int i,q;

#ifdef DEBUG
fprints(2,"In makealias\n");
#endif
  dptr=defn_top=NULL;
  if (fileopen(filename)==FALSE)
    { prints("Couldn't open %s\n",filename); return; }
  while (1)
  {
    for (i=0;i<MAXLL;i++) line[i]=EOS;
    if (getfileline(line,&q))		/* nostrip is set */
    {
#ifdef DEBUG
fprints(2,"alias line:\n%s<<-\n",line);
#endif
      if (dptr!=NULL)	/* dptr==NULL iff defn_top==NULL */
      {
	dptr->nextln=(struct adefn *) malloc ((unsigned)(sizeof(struct adefn)));
	if (dptr->nextln==NULL)
  	{
    	 perror("makalias1");
    	 return;
  	}
	dptr=dptr->nextln;
      }
      else
      {
	defn_top=(struct adefn *) malloc ((unsigned)(sizeof(struct adefn)));
	if (defn_top==NULL)
  	{
    	 perror("makalias2");
    	 return;
  	}
	dptr=defn_top;
      }
      dptr->a_line=(char *) malloc ((unsigned)(strlen(line)+1));
      if (dptr->a_line==NULL)
  	{
    	 perror("makalias3");
    	 return;
  	}
      strcpy(dptr->a_line,line);
    }
   else break;
  }
  if (dptr!=NULL) dptr->nextln=NULL;
  addalias(aname,defn_top);
  fileclose();
}

#ifdef PROTO
void delalias ( char *aname )
#else
void delalias(aname)
  char *aname;
#endif
{
  struct alias *ptr,*old;
  struct adefn *dpt1,*dpt2;
  int i,lex;

  for (i=0,old=ptr=atop;ptr;i++,old=ptr,ptr=ptr->next)
    if ((lex=strcmp(aname,ptr->a_name))==0)
      if (i)
      {
	for (dpt1=dpt2=ptr->defin;dpt2;dpt1=dpt2)
	{
	  dpt2=dpt2->nextln;
	  free(dpt1->a_line);
	  free(dpt1);
	}
	old->next=ptr->next;
	free(ptr->a_name);
	free(ptr);
	return;
      }
      else
      {
	atop=atop->next;
	free(ptr->a_name);
	free(ptr);
	return;
      }
    else if (lex<0)
	   break;
  fprints(2,"No such alias : %s\n",aname);
}

#ifdef PROTO
int alias ( int argc , char *argv [] )
#else
int alias(argc,argv)
  int argc;
  char *argv[];
#endif
{
  char line[MAXLL];
  int i;

#ifdef DEBUG
  fprints(2,"In alias with argc %d argv[1] %s\n",argc,argv[1]);
#endif

  if (argc==1)
  {
    aliaslist(NULL);
    return(0);
  }
  if (argc==2)
  {
    aliaslist(argv[1]);
    return(0);
  }
#ifdef NOTYET
  if (!strcmp(argv[1],"-e"))
    for (i=2;i<argc;i++)
      editalias(argv[2]);
#endif
  else if (!strcmp(argv[1],"-l"))
    for (i=2;i<argc;i++)
    {
      makealias(argv[i],argv[i]);
    }
  else if (!strcmp(argv[1],"-s"))
    for (i=2;i<argc;i++)
    {
      savealias(argv[i],argv[i]);
    }
  else
  {
    line[0]=EOS;
    for (i=2;i<argc;i++)
    {
      if (i!=2) strcat(line," ");
      strcat(line,argv[i]);
    }
    makelinealias(argv[1],line);
  }
 return(0);
}

#ifdef PROTO
int unalias ( int argc , char *argv [] )
#else
int unalias(argc,argv)
  int argc;
  char *argv[];
#endif
{
  int i;

  for (i=1;argv[i];i++)
    delalias(argv[i]);
  return(0);
}
