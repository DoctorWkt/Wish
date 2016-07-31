#include "header.h"

/* The parser uses the symbols parsed by meta.c, but it needs a few more
 * definitions for proper parsing. These are below.
 */

#define C_WORD	C_SPACE		/* This indicates the returned word is a */
				/*  usual word */
#define C_EOF	FALSE		/* Returned when no more tokens left */

extern struct candidate carray[];	/* The list of words to parse */
static struct candidate *pcurr;		/* A ptr to the word we are parsing */

/* Gettoken returns the symbol which stands for the current word, or 0.
 * If 0, word points to a string that holds a normal word. This is guaranteed
 * not to be clobbered until we get back out to main().
 */
static int gettoken(word,fd)
 char **word;  
 int *fd;
 {
  int mode;
  
					/* Skip any null words */
  while(1)
   {
    if (pcurr==NULL) return(C_EOF);
    mode=pcurr->mode;

    if (mode&C_WORDMASK)		/* We've found a token, return */
      { *word=NULL;
	*fd= mode & 15;
	 mode &=C_WORDMASK;		/* (stripping fd for now) */
         pcurr=pcurr->next; return(mode);
      }
    if (((*word=pcurr->name)!=NULL) && **word!=EOS)
      { *word=pcurr->name;
         mode=C_WORD;
         pcurr=pcurr->next;
         return(mode);
      }
    pcurr=pcurr->next;			/* Not a valid word or symbol */
   }
 }


/* Command parses the input and calls invoke() to redirect and execute
 * each simple command in the parsebuf. It returns the pid to wait on
 * in waitpid, or 0 if no pid to wait on, as well as the token that ended
 * the pasebuf. This routine is recursive, and when passed makepipe=TRUE,
 * makes a pipe, and returns the fd of the writing end in pipefdp. It is
 * normally called with tok=command(&pid,FALSE,NULL);
 */
int command(waitpid,makepipe,pipefdp)	/* Do simple command */
 int *waitpid, *pipefdp;
 bool makepipe;
 {
  int token,term,gettoken();
  int argc,pid,pfd[2];
  char *argv[MAXARG+1];
  char *word;
  int i, fd, how=0;
  struct rdrct newfd[10];

  argc=0; argv[0]=NULL;
  for (i=0; i<10; i++) { newfd[i].fd=0; newfd[i].file=NULL; }
  if (makepipe==FALSE) 
    { pcurr=carray;		/* Start parsing the carray */
      if (dupup())		/* Dup up our fds */
       { fprints(2,"Could not dup our fds\n"); return(C_EOF); }
    }
  while(1)
   {
    token=gettoken(&word, &fd);
#ifdef DEBUG
prints("Token is %o",token); if (word!=NULL) prints(" word is %s",word);
write(1,"\n",1);
#endif
    switch(token)
     {
      case C_WORD: if (argc==MAXARG)
		     { fprints(2,"Too many args\n"); break; }
		   argv[argc++]=word;
		   continue;
      case C_LT:   if (makepipe)
		     { fprints(2,"Extra <\n"); break; }
		   if (gettoken(&(newfd[fd].file),&i)!=C_WORD)
		     { fprints(2,"Illegal <\n"); break; }
		   newfd[fd].how= H_FROMFILE;
		   continue;
      case C_GT  :
      case C_GTGT: if (newfd[fd].fd!=0 || newfd[fd].file!=NULL)
		     { fprints(2,"Extra > or >>\n"); break; }
		   if (gettoken(&(newfd[fd].file),&i)!=C_WORD)
		     { fprints(2,"Illegal > or >>\n"); break; }
		   if (token==C_GTGT) newfd[fd].how = H_APPEND;
		   continue;
      case C_AMP :		/* If a pipe, call ourselves to get */
      case C_SEMI:		/* the write file descriptor */
      case C_EOF :
      case C_PIPE: argv[argc]=NULL;
		   if (token==C_PIPE)
		    {
		     if (newfd[1].fd!=0 || newfd[1].file!=NULL)
		      { fprints(2,"> or >> conflicts with |\n"); break; }
		     term=command(waitpid,TRUE,&(newfd[1].fd));
		    }		/* and set up the terminal token */
		   else term=token;
				/* If called by us, make the needed pipe */
		   if (makepipe)
		     {
		      if (pipe(pfd)==-1)
		       { perror("pipe"); break; }
				/* and return the write file descriptor */
		      *pipefdp=pfd[1];
		      newfd[0].fd=pfd[0];
		     }
		   if (term==C_AMP) how = H_BCKGND;
		   pid=invoke(argc,argv,newfd,how);
				/* End of command line, return pid to wait */
		   if (token!=C_PIPE) *waitpid=pid;
		   if (argc==0 && token!=C_EOF)
			fprints(2,"Missing command\n");
  		   if (makepipe==FALSE) dupdown();
		   return(term);
      default:     prints("Unknown token %o in command()\n",token);
     }
   }
 }
