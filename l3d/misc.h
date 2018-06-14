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

#ifndef MISC_H
#define MISC_H
#include "md.h"

/* miscellaneous structures */

#ifndef NULL
#define NULL 0L
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef min
#define min(_a, _b)  ((_a) <= (_b) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)  ((_a) >= (_b) ? (_a) : (_b))
#endif

/* and some useful macros */

#define SWAP(a, b) { int t = a; a = b; b = t; }

#define sign(n)         ((n) < 0 ? -1 : ((n) > 0 ? 1 : 0))

#define ACCURACY 14
#define ROUNDING  (1 << (ACCURACY-1 ))
#define MAX_NUM 65536

#define Z_CLIP_DEPTH 10  
#define PERSPECTIVE_2 100L
#define PERSP_1 2048L
#define PERSPECTIVE_1 (PERSP_1 << 10)

#define TRI_SHIFT 16
#define TRI_ROUND (1<<(TRI_SHIFT-1))
#define TABLE_SIZE 4096
#define DOT_SIZE 3

#define MODELS_MAX 4

#define SET_FLAG(num, flag)   ((num) |= flag)
#define CLEAR_FLAG(num, flag) ((num) &= (~flag))
#define CHECK_FLAG(num, flag) ((num) & (flag))
#define TOGGLE_FLAG(num, flag) ((num) ^= (flag))

#endif /* MISC_H */

