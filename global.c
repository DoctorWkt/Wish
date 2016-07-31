/******************************************************************************
**                                                                           **
**                               global.c                                    **
**          Some global declarations for the entire program.                 **
**                                                                           **
******************************************************************************/

#include "header.h"


char termcapbuf[1024],bs[10],nd[10],cl[10],cd[10],up[10],
     so[10],se[10],beep[20];

char yankbuf[512];
char currdir[128];
FILE *zin, *zout;
int disable_auto, wid;
