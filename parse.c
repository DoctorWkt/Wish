#include "header.h"

char *parsebuf;			/* The buffer we are parsing */

static TOKEN gettoken(word)	/*correct and classify token*/
 char *word;  
 {
  enum {NEUTRAL,GTGT,INQUOTE,INWORD} state=NEUTRAL;
  int c; char *w;  

  w=word;
  while((c= *(parsebuf++))!=0)
   {
    switch(state)
     {
      case NEUTRAL: switch(c)
		     {
		      case ';':  return(T_SEMI);
		      case '&':  return(T_AMP);
		      case '|':  return(T_BAR);
		      case '<':  return(T_LT);
		      case '\n': return(T_NL);
		      case ' ':
		      case '\t': continue;
		      case '>':  state=GTGT;
				 continue;
		      case '"':  state=INQUOTE;
				 continue;
		      default:   state=INWORD;
				 *w++=c;
				 continue;
		     }
      case GTGT:    if (c=='>') return(T_GTGT);
		    parsebuf--;
		    return(T_GT);
      case INQUOTE: switch(c)
		     {
		      case '\\' : *w++= *(parsebuf++);
				  continue;
		      case '"':   *w='\0';
				  return(T_WORD);
		      default:    *w++=c;
				  continue;
		     }
      case INWORD:  switch(c)
		     {
		      case ';':
		      case '&':
		      case '|':
		      case '<':
		      case '>':
		      case ' ':
		      case '\n':
		      case '\t': parsebuf--;
				 *w='\0';
				 return(T_WORD);
		      default:   *w++=c;
				 continue;
		     }
     }
   }
  return(T_EOF);
 }


/* Command parses parsebuf and calls invoke() to redirect and execute
 * each simple command in the parsebuf. It returns the pid to wait on
 * in waitpid, or 0 if no pid to wait on, as well as the token that ended
 * the pasebuf. This routine is recursive, and when passed makepipe=TRUE,
 * makes a pipe, and returns the fd of the writing end in pipefdp. It is
 * normally called with tok=command(&pid,FALSE,NULL);
 */
TOKEN command(waitpid,makepipe,pipefdp)	/* Do simple command */
 int *waitpid, *pipefdp;
 bool makepipe;
 {
  TOKEN token,term,gettoken();
  int argc,pid,pfd[2];
  char *argv[MAXARG+1],srcfile[MAXFNAME],dstfile[MAXFNAME];
  char word[MAXWORD], *malloc();
  int how=0;
  struct rdrct newfd;

#define srcfd newfd.infd
#define dstfd newfd.outfd

  argc=0; srcfd=0; dstfd=1;
  while(1)
   {
    switch(token=gettoken(word))
     {
      case T_WORD: if (argc==MAXARG)
		     { fprints(2,"Too many args\n"); break; }
		   if ((argv[argc]=malloc(strlen(word)+1))==NULL)
		     { fprints(2,"Out of arg memory\n"); break; }
		   strcpy(argv[argc],word);
		   argc++;
		   continue;

      case T_LT:   if (makepipe)
		     { fprints(2,"Extra <\n"); break; }
		   if (gettoken(srcfile)!=T_WORD)
		     { fprints(2,"Illegal <\n"); break; }
		   srcfd= BADFD;
		   continue;
      case T_GT:
      case T_GTGT: if (dstfd!=1)
		     { fprints(2,"Extra > or >>\n"); break; }
		   if (gettoken(dstfile)!=T_WORD)
		     { fprints(2,"Illegal > or >>\n"); break; }
		   dstfd= BADFD;
		   if (token==T_GTGT) how |= H_APPEND;
		   continue;
      case T_BAR:
      case T_AMP:		/* If a pipe, call ourselves to get */
      case T_SEMI:		/* the write file descriptor */
      case T_NL:   argv[argc]=NULL;
		   if (token==T_BAR)
		    {
		     if (dstfd!=1)
		      { fprints(2,"> or >> conflicts with |\n"); break; }
		     term=command(waitpid,TRUE,&dstfd);
		    }		/* and set up the terminal token */
		   else term=token;
				/* If called by us, make the needed pipe */
		   if (makepipe)
		     {
		      if (pipe(pfd)==-1)
		       { perror("pipe"); break; }
				/* and return the write file descriptor */
		      *pipefdp=pfd[1];
		      srcfd=pfd[0];
		     }
		   newfd.ifil=srcfile;
		   newfd.ofil=dstfile;
		   if (term==T_AMP) how |= H_BCKGND;
		   if (token==T_SEMI || token==T_NL) how |= H_PARENT;
		   pid=invoke(argc,argv,&newfd,how);
				/* End of command line, return pid to wait */
		   if (token!=T_BAR) *waitpid=pid;
		   if (argc==0 && (token!=T_NL || srcfd>1))
			fprints(2,"Missing command\n");
		   while(--argc>=0) free(argv[argc]);
		   return(term);
      case T_EOF:  return(token);
     }
   }
 }
