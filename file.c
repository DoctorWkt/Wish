#include "header.h"

extern char yankbuf[512];
static int filedesc, bufbytes;
static char *bufptr;

/* Fileopen opens the named file read-only, and sets the number of bytes
 * buffered in yankbuf to zero.
 */
bool fileopen(filename)
 char *filename;
 {
  if ((filedesc=open(filename, O_RDONLY, 0)) == -1)
        {
            fprints(2, "Can't open %s\n", filename); return (FALSE);
        }
  bufptr=yankbuf; bufbytes=0;
  return(TRUE);
 }

void fileclose()
 { close(filedesc);
 }

/* Getfileline obtains a line from the opened file.
 */
bool getfileline(line, nosave)
 char *line;
 int *nosave;
 {
  int i;

  *nosave=0;
  while(1)				/* Until we have a line, copy stuff */
   {					/* We've already got some to copy */
    for (;bufbytes && (*bufptr==' ' || *bufptr=='\t'); bufbytes--, bufptr++);
    if (bufbytes==0) goto doread;	/* Yuk, a goto */

    if (*bufptr=='#') *nosave=1;
    for (;bufbytes && *bufptr!='\n';bufbytes--)
	*line++ = *bufptr++;

    if (bufbytes)			/* We hit a \n, so return */
     { *line=EOS; bufptr++; bufbytes--; return(TRUE); }

doread:
    bufptr=yankbuf; 
    bufbytes=read(filedesc,yankbuf,512);/* Get more characters */
    if (bufbytes<1) return(FALSE);	/* End of file */
   }
 }

int source(argc, argv)
 int argc;
 char *argv[];
 {
  extern bool (*getline)();
  bool (*oldgetline)();
  int nosave;
  
  if (argc!=2) { prints("Usage: source file\n"); return(1); }

#ifdef DEBUG
prints("About to fileopen %s\n",argv[1]);
#endif

  if (fileopen(argv[1])==FALSE)
    { prints("Unable to source file %s\n",argv[1]); return(1); }
  else
    {
#ifdef DEBUG
prints("About to send the file through doline()\n");
#endif
	oldgetline= getline;
	getline= getfileline;
	while(doline());
	getline= oldgetline;
    }
  close(filedesc); return(0);
 }
