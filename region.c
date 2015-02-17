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

#include "region.h"
#include "2di.h"

/** Region2D *************************************************************/

void Region2DInit(Region2D* rg)
{
    memset(rg, 0, sizeof(Region2D));
}

void Region2DEmpty(Region2D* rg)
{
    Region2DInit(rg);
}

static int rectWasteCost(Rect2D* ra, Rect2D* rb)
{
    /* calculate the area wasted if we combine two rectangles
     * this is the area of, union(A,B) - A - B + intersection(A,B)
     */
    Rect2D ur, ir;
    int ua, ia, aa, ba;

    aa = BOXAREA(*ra);
    ba = BOXAREA(*rb);

    BOXUNION(*ra, *rb, ur);
    ua = BOXAREA(ur);

    BOXINTERSECT(*ra, *rb, ir);
    ia = BOXAREA(ir);

    return ua - aa - ba + ia;
}

static void changeBoxAt(Region2D* rg, int i, Rect2D* box)
{
    int j;
    int c;
    Rect2D b = *box;
    BOXAT(*rg, i, i) = 1;

 again:
    rg->b_[i] = b;
    
    /* recompute cost metrics */
    for (j = 0; j < REGION_NUM_BOXES; ++j) {
        if (j == i || !BOXAT(*rg, j, j)) continue;
        c = rectWasteCost(&rg->b_[j], &rg->b_[i]);

        if (!c) {
            /* box `j' is deleted. */
            BOXUNION(rg->b_[j], rg->b_[i], b);
            BOXAT(*rg, j, j) = 0;
            goto again;
        }

        if (j < i) {
            BOXAT(*rg, j, i) = c;
        }
        else {
            BOXAT(*rg, i, j) = c;
        }
    }
}

static int makeSpareBox(Region2D* rg)
{
    int i, j;
    int minc;
    int mini, minj;
    Rect2D ru;

    /* first quickly scan for am existing spare box */
    for (i = 0; i < REGION_NUM_BOXES; ++i) {
        if (!BOXAT(*rg, i,i)) return i; // already spare
    }

    /* all boxes full, find the combination with the min waste
     * metric and combine.
     */
    minc = 0;
    for (i = 0; i < REGION_NUM_BOXES; ++i) {
        for (j = i+1; j < REGION_NUM_BOXES; ++j) {
            int c = BOXAT(*rg, i, j);
            if (!minc || c < minc) {
                minc = c;
                mini = i; 
                minj = j;
            }
        }
    }

    /* mini + minj -> mini */
    BOXAT(*rg, minj,minj) = 0; // mark as empty.
    BOXUNION(rg->b_[mini], rg->b_[minj], ru);    
    changeBoxAt(rg, mini, &ru);
    return minj;
}

void Region2DAdd(Region2D* rg, Rect2D* r)
{
    /* first look for a spare slot.
     * this is indicated by examining the box difference for (i,i)
     */
    int i;

    if (BOXOK(*r)) {  // reject any empty boxes.
        i = makeSpareBox(rg);
        changeBoxAt(rg, i, r);
    }
}

int Region2DBoundingBox(Region2D* rg, Rect2D* r)
{
    /* find the overall bounding box for this region.
     * return 0 if no valid bounds.
     */
    Rect2D b;
    int begun = 0;
    int i;
    for (i = 0; i < REGION_NUM_BOXES; ++i) {
        if (BOXAT(*rg, i, i)) {
            if (!begun) {
                /* start with the first valid box */
                b = rg->b_[i];
                begun = 1;
            }
            else {
                /* otherwise expand to include this box */
                const Rect2D* bi = &rg->b_[i];
                EXTBOX(b, bi->tl.x, bi->tl.y);
                EXTBOX(b, bi->br.x, bi->br.y);
            }
        }
    }

    if (begun) *r = b;
    else {
        /* fill out a null box */
        r->tl.x = 0; r->tl.y = 0;
        r->br.x = 0; r->br.y = 0;
    }
    return begun;
}

int Region2DValid(Region2D* rg)
{
    /* return 0 iff `rg' is empty */
    int i;
    for (i = 0; i < REGION_NUM_BOXES; ++i) {
        if (BOXAT(*rg, i, i)) {
            return 1;
        }
    }
    return 0;
}


/** Rect2D Helpers ********************************************************/

void RectUnion2D(Rect2D* ra, Rect2D* rb, Rect2D* rr)
{
    BOXUNION(*ra, *rb, *rr);
}



