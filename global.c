/*****************************************************************************
**                                                                          **
**          The Clam Shell is Copyright (C) 1988 by Callum Gibson.          **
**       This file is part of Clam Shell. You may freely use, copy and      **
**     distribute it, but if you alter any source code do not distribute    **
**   the altered copy. i.e. you may alter this file for your own use only.  **
**                                                                          **
*****************************************************************************/
/******************************************************************************
**                                                                           **
**                               global.c                                    **
**          Some global declarations for the entire program.                 **
**                                                                           **
******************************************************************************/

#include "header.h"

#ifndef lint
static char copyright[]="Clam shell, (c)1988 by Callum Gibson";
static char version[]="Version 1.4.0 beta 901029";
#endif

char termcapbuf[1024],bs[10],nd[10],cl[10],cd[10],up[10],
     so[10],se[10],beep[20];

char yankbuf[512];
FILE *zin, *zout;
int disable_auto, wid;
