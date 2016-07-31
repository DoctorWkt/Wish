#include "header.h"

/* Getline gets one line of text from the keyboard, putting it in the
 * given buffer. Currently the max number of chars is ignored. The line
 * should end with either a \0 or a \n
 * Returns TRUE is line ends with \n, FALSE is end of file.
 */

BOOLEAN getline(buf,len)
 char *buf;
 int len;
 {
  int ch;

  while((ch=getchar())!='\n' && (ch!=0) && (ch!=-1)) *(buf++)=ch;
  *(buf++)=ch; *buf=0;
  if (ch=='\n') return TRUE; else return FALSE;
 }
