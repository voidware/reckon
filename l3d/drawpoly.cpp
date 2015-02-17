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

#include <stdlib.h>
#include "misc.h"
#include "l3defs.h"
#include "2d.h"
#include "primitiv.h"
#include "fill.h"
#include "drawpoly.h"
#include "triangle.h"
#include "render.h"
#include "l3api.h"

#define POINT_CLIPPED(_p) ((_p)->tz != 0)

           
/*
 *     The algorithm is to order the edges from smallest
 *     y to largest by partitioning the array into a left
 *     edge list and a right edge list.  The algorithm used
 *     to traverse each edge is a dda whith y as the
 *     major axis.
 */
void draw_polygon(int col, int n, Point3 **pts)
{                  
    int bxl, bxr;
    int xl, xr;         /* x vals of left and right edges  */
    int ml, mr;         /* left edge slope right slope     */
    int y;                       /* current scanline                */
    int i;                       /* loop counter                    */
    int left, right;     /* indices to second endpoints     */
    int ymax;                    /* y-extents of polygon            */
    SCREEN_UNIT *screenp;

    Point3** p = pts;
    
    left = 0;
    y = ymax = (*p++)->ty;
    for (i = 1; i < n; ++i)
    {
        Point3* pt = *p;
        if (pt->ty < y)
        {
            left = i;
            y = pt->ty;
        } else if (pt->ty > ymax)
            ymax = pt->ty;
        p++;
    }
    right = left;

    /*
     *  loop through all edges of the polygon
     */

    screenp = DestMem + (y * DestPr->w_);
    do {
        if (pts[left]->ty == y)
        {
            int dy;
            
            xr = pts[left]->tx;
            dy = -pts[left]->ty;

            if (++left >= n) left = 0;
            
            dy += pts[left]->ty;
            
            bxr = xr << TRI_SHIFT;
            mr = (pts[left]->tx - xr) << TRI_SHIFT;
            if (dy) mr /= dy;
        }

        if (pts[right]->ty == y)
        {
            int dy;
            
            xl  = pts[right]->tx;
            dy = -pts[right]->ty;
            
            if (!right) right = n;
            --right;

            dy += pts[right]->ty;
            bxl = xl << TRI_SHIFT;
            ml = (pts[right]->tx - xl) << TRI_SHIFT;
            if (dy) ml /= dy;
        }

        /*
         *  generate scans to fill while we still have
         *  a right edge as well as a left edge.
         */     

        i = min(pts[left]->ty, pts[right]->ty) - y;

        if (i < 0) return;
        /* in case we're called with non-convex polygon */
        
        y += i;

        if (y == ymax) ++i;
        DRAW_SPAN_UP(screenp, col, i, bxl, bxr, ml, mr);
        
    } while (y != ymax);
}


#define SET_COL 0xF0

void draw_wire_polygon(int n, Point3 **pts, Plane* pp)
{
    int i;
    Bitmap2D* pr = current_view->current;
    Point3* p1;
    Point3* p2;

    int inv = pp->pixel;
    int fcol = SET_COL;
    int bcol = 0;

    if (inv)
    {
        // invert colours
        bcol = fcol;
        fcol = 0;
    }
    
    draw_polygon(bcol, n, pts);
    for (i = 1; i < n; ++i)
    {
        p1 = pts[i-1];
        p2 = pts[i];
        
        bool allClip = POINT_CLIPPED(p1) && POINT_CLIPPED(p2);
        if ((fcol || p1->refs >= 4 || p2->refs >= 4) && !allClip)
            line(pr, fcol, p1->tx, p1->ty, p2->tx, p2->ty);
    }
    
    p1 = pts[0];
    bool allClip = POINT_CLIPPED(p1) && POINT_CLIPPED(p2);
    if ((fcol || p1->refs >= 4 || p2->refs >= 4) && !allClip)
        line(pr, fcol, p2->tx, p2->ty, p1->tx, p1->ty);
}

#ifdef SUPPORT_GOURAUD
void draw_gpolygon(int basecol, int count, Point3 **pts)
{
    int bxl, bxr;
    int xl, xr;         /* x vals of left and right edges  */
    int ml, mr;         /* left edge slope right slope     */
    int mil,mir,il,ir;
    int y;                       /* current scanline                */
    int left, right;             /* indices to first endpoints      */
    int i;                       /* loop counter                    */
    int nextleft, nextright;     /* indices to second endpoints     */
    int ymax;                    /* y-extents of polygon            */
    SCREEN_UNIT *screenp;

    Point3** p = pts;
    int n;
    
    nextleft = 0;
    y = ymax = (*p++)->ty;
    for (i = 1, n = count-1; n; n--, i++) 
    {
        Point3* pt = *p;
        if (pt->ty < y)
        {
            nextleft = i;
            y = pt->ty;
        } else if (pt->ty > ymax)
            ymax = pt->ty;
        p++;
    }
    nextright = nextleft;

    /*
     *  loop through all edges of the polygon
     */

    screenp = DestMem + (y * DestPr->w_);
    do {
        if (pts[nextleft]->ty == y)
        {
            int dx, dy, di;

            left = nextleft;
                                /*  find the next edge, considering the end
                                    conditions of the array. */
            nextleft++;
            if (nextleft >= count)
                nextleft = 0;

            dy = pts[nextleft]->ty - pts[left]->ty,
            xl  = pts[left]->tx;
            il  = pts[left]->ti;
            dx = pts[nextleft]->tx  - xl;
            di = pts[nextleft]->ti - il;
            bxl = xl << TRI_SHIFT;
            il  = il << TRI_SHIFT;

            ml = DIVIDE(dx, dy);
            mil = DIVIDE(di, dy);
        }

        if (pts[nextright]->ty == y)
        {
            int dx, dy, di;

            right = nextright;
                                /*  find the next edge, considering the end
                                    conditions of the array. */
            nextright--;
            if (nextright < 0)
                nextright = count-1;

            dy  = pts[nextright]->ty - pts[right]->ty,
            xr  = pts[right]->tx;
            ir  = pts[right]->ti;
            di  = pts[nextright]->ti - ir;
            dx  = pts[nextright]->tx - xr;
            bxr = xr << TRI_SHIFT;
            ir  = ir << TRI_SHIFT;
            mr = DIVIDE(dx, dy);
            mir = DIVIDE(di, dy);
        }

        /*
         *  generate scans to fill while we still have
         *  a right edge as well as a left edge.
         */     
        i = min(pts[nextleft]->ty, pts[nextright]->ty) - y;

        /* in case we're called with non-convex polygon */
        y += i;

        while (i-- > 0)
        {    
            if (xl > xr) 
            {
                int w = xl - xr;
                gdraw_span(basecol, screenp + xr, w, ir, (il - ir)/w);
            }

            screenp += DestPr->w_;

            bxl += ml;
            bxr += mr;
            il  += mil;
            ir  += mir;

            xl = bxl >> TRI_SHIFT;
            xr = bxr >> TRI_SHIFT;
            
        }
    } while (y != ymax);
}
#endif // SUPPORT_GOURAUD
