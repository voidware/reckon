/**
 *
 * Copyright (c) 2010-2015 Voidware Ltd.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Voidware Public Source Licence version
 * 1.0 (the 'Licence'). You may not use this file except in compliance with the
 * Licence or with expressly written permission from Voidware.  Please obtain a
 * copy of the Licence at http://www.voidware.com/legal/vpsl1.txt and read it
 * before using this file.
 * 
 * The Original Code and all software distributed under the Licence are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND VOIDWARE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING
 * WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Please see the Licence for the specific language governing rights and 
 * limitations under the Licence.
 *
 * contact@voidware.com
 */

#ifndef MD_H
#define MD_H

#include <stdlib.h>
extern void* xmalloc(size_t);
extern void  xfree(void*);

#define SCREEN_UNIT unsigned char
#define Memory		xmalloc
#define MemoryFree	xfree

#define FILESEP	'\\'

#define MAXPATHLEN 64
#define rindex strrchr
#define READ_BINARY  "rb"
#define WRITE_BINARY "wb"
#define READ_TEXT "rt"
#define WRITE_TEXT "wt"
#define MAX_FILENAME	128
#define MAX_FILENAME_LENGTH 12


#define PATH_MAXPATH	80
#define PATH_MAXDRIVE	8 
#define PATH_MAXDIR	66
#define PATH_MAXFILE	9
#define PATH_MAXEXT	5

#endif /* MD_H */
