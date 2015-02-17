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

/* These are simplified regions made of a small number of rectangles */

#ifndef __region_h__
#define __region_h__

/* max rectangles allowed in a region */
#define REGION_NUM_BOXES        4

typedef short Ord;

typedef struct
{
    Ord x;
    Ord y;
} Point2D;

typedef struct 
{
    Point2D tl;
    Point2D br;
} Rect2D;

/* access row, col
 * i*N+j - i*(i+1)/2 == i*(2N-i-1)/2+j 
 */
#define BOXAT(_rg, _i, _j) \
      ((_rg).bd_[(((_i)*(2*REGION_NUM_BOXES-(_i)-1))>>1)+(_j)])

typedef struct 
{
    Rect2D      b_[REGION_NUM_BOXES];
    int         bd_[(REGION_NUM_BOXES*(REGION_NUM_BOXES+1))/2];
} Region2D;

void Region2DInit(Region2D*);
void Region2DEmpty(Region2D*);
void Region2DAdd(Region2D*, Rect2D*);
int Region2DBoundingBox(Region2D*, Rect2D*);
int Region2DValid(Region2D*);
void RectUnion2D(Rect2D* ra, Rect2D* rb, Rect2D* rr);

#endif /* region */
