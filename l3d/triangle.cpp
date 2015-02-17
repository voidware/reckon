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

#include <string.h>
#include "render.h"
#include "fill.h"

#define  SHADE_ADJUST   8

void draw_triangle(Plane *pln)
{              
    int  height1, height2;
    SCREEN_UNIT* screenp;
    long m1;
    long m2;
    long m3;
    long leftx;
    long rightx;
    int dy,dx;
    int colour;
    Point3 *tm;
    Point3 *p1 = pln->p1;
    Point3 *p2 = pln->p2;
    Point3 *p3 = pln->p3;

    if (p1->ty > p2->ty)
    {
        tm = p1;
        p1 = p2;
        p2 = tm;
    }
    if (p1->ty > p3->ty)
    {
        tm = p1;
        p1 = p3;
        p3 = tm;
    }
    if (p2->ty > p3->ty)
    {
        tm = p2;
        p2 = p3;
        p3 = tm;
    }                                             

    dy  = p3->ty - p1->ty;
    if (dy == 0) return;

    colour = pln->pixel;

    dx  = p3->tx - p1->tx;
    m3 = DIVIDE(dx, dy);

    dx  = p3->tx - p2->tx;
    height2 = dy  = p3->ty - p2->ty;
    m2 = DIVIDE(dx, dy);


    height1 = dy  = p2->ty - p1->ty;

    if (height1 == 0) 
    {
        screenp  = DestMem + DestPr->w_ * p1->ty; 

        if (m2 - m3 > 0)
        {
            rightx     = p1->tx << TRI_SHIFT; 
            leftx      = p2->tx << TRI_SHIFT;
            DRAW_SPAN_UP(screenp, colour, height2, leftx, rightx, m2, m3);
            return;
        } 
        else
        {
            leftx      = p1->tx << TRI_SHIFT; 
            rightx     = p2->tx << TRI_SHIFT;
            DRAW_SPAN_UP(screenp, colour, height2, leftx, rightx, m3, m2);
            return;
        }
    }
    else 
        m1 = DIVIDE(dx, dy);

    screenp  = DestMem + DestPr->w_ * p1->ty; 
    leftx = rightx = p1->tx << TRI_SHIFT; 

    if (m3 - m1 > 0) 
    {
        DRAW_SPAN_UP(screenp, colour, height1, leftx, rightx, m1, m3);
        leftx     = p2->tx << TRI_SHIFT;
        DRAW_SPAN_UP(screenp, colour, height2, leftx, rightx, m2, m3);
    } else {
        DRAW_SPAN_UP(screenp, colour, height1, leftx, rightx, m3, m1);
        rightx     = p2->tx << TRI_SHIFT;
        DRAW_SPAN_UP(screenp, colour, height2, leftx, rightx, m3, m2);
    }
}

#ifdef SUPPORT_GOURAUD

void gdraw_span(int col, SCREEN_UNIT *dst, int w, int i1, int di)
{
    int c = col << TRI_SHIFT;
    while (w--) 
    {
        int v = (c + i1)>>TRI_SHIFT;
        if (v > 0xff) v = 0xff;
        *dst++ = v;
        i1 += di;
    }
}

void draw_gtriangle(Plane *pln)
{              
    int  height1, height2;
    SCREEN_UNIT* screenp;
    int m1;
    int m2;
    int m3;
    int left_grad;
    int right_grad;
    int leftx;
    int rightx;
    int left_igrad;
    int right_igrad;
    int lefti;
    int dy,dx,di;
    int mi1,mi2,mi3;
    int x1,y1,x2,y2,x3,y3,i1,i2,i3;
    Point3 *tm;
    Point3 *p1 = pln->p1;
    Point3 *p2 = pln->p2;
    Point3 *p3 = pln->p3;
    int gstep;
    int colour;

    colour = GET_COLOUR(pln);

    if (p1->ty > p2->ty)
    {
        tm = p1;
        p1 = p2;
        p2 = tm;
    }
    if (p1->ty > p3->ty)
    {
        tm = p1;
        p1 = p3;
        p3 = tm;
    }
    if (p2->ty > p3->ty)
    {
        tm = p2;
        p2 = p3;
        p3 = tm;
    }                                             

    x1 = p1->tx;
    y1 = p1->ty;
    i1 = p1->ti;

    x2 = p2->tx;
    y2 = p2->ty;
    i2 = p2->ti;

    x3 = p3->tx;
    y3 = p3->ty;
    i3 = p3->ti;

    dx  = x3 - x1;
    dy  = y3 - y1;
    di  = i3 - i1;

    if (dy == 0) return;

    m3 = DIVIDE(dx, dy);
    mi3 = DIVIDE(di, dy);

    dx  = x3 - x2;
    di  = i3 - i2;
    height2 = dy  = y3 - y2;

    m2 = DIVIDE(dx, dy);
    mi2 = DIVIDE(di, dy);
    height1 = dy  = y2 - y1;

    if (height1 == 0)
    {
        screenp  = DestMem + DestPr->w_ * y1; 
        leftx = rightx = x1 << TRI_SHIFT; 
        lefti = i1 << TRI_SHIFT; 

        if (m2 - m3 > 0)
        {
            left_grad   = m2;
            right_grad  = m3;
            left_igrad  = mi2;
            right_igrad = mi3;
            leftx       = x2 << TRI_SHIFT;
            lefti       = i2 << TRI_SHIFT;
        }
        else 
        {
            left_grad   = m3;
            right_grad  = m2;
            left_igrad  = mi3;
            right_igrad = mi2;
            rightx      = x2 << TRI_SHIFT;
        }

        gstep = (right_grad >> (TRI_SHIFT - SHADE_ADJUST)) -
            (left_grad >> (TRI_SHIFT - SHADE_ADJUST));
        if (gstep) 
            gstep = ((right_igrad << SHADE_ADJUST) -
                     (left_igrad << SHADE_ADJUST))/gstep;
        for (; height2; height2--)
        {
            int w;
            int xl;

            xl = leftx  >> TRI_SHIFT;
            w  = (int)(rightx >> TRI_SHIFT) - xl;
            if (w > 0)
                gdraw_span(colour, screenp + xl, w, lefti, gstep);
            screenp += DestPr->w_;
            rightx  += right_grad;
            lefti   += left_igrad;
            leftx   += left_grad;
        }
        return;
    }
    else 
    {
        dx  = x2 - x1;
        di  = i2 - i1;

        m1 = DIVIDE(dx, dy);
        mi1 = DIVIDE(di, dy);
    }

    screenp  = DestMem + DestPr->w_ * y1; 
    leftx = rightx = x1 << TRI_SHIFT; 
    lefti = i1 << TRI_SHIFT; 

    if (m3 - m1 > 0)
    {
        left_grad   = m1;
        right_grad  = m3;        
        left_igrad  = mi1;
        right_igrad = mi3;        

        gstep = (right_grad >> (TRI_SHIFT - SHADE_ADJUST)) -
            (left_grad >> (TRI_SHIFT - SHADE_ADJUST));
        if (gstep) 
            gstep = ((right_igrad << SHADE_ADJUST) -
                     (left_igrad << SHADE_ADJUST))/gstep;
        for (; height1; height1--)
        {
            int w;
            int xl;

            xl = leftx  >> TRI_SHIFT;
            w  = (int)(rightx >> TRI_SHIFT) - xl;
            if (w > 0)
                gdraw_span(colour, screenp + xl, w, lefti, gstep);
            screenp += DestPr->w_;
            rightx  += right_grad;
            leftx   += left_grad;
            lefti   += left_igrad;
        }

        left_grad  = m2;
        leftx      = x2 << TRI_SHIFT;
        left_igrad = mi2;
        lefti      = i2 << TRI_SHIFT;

        gstep = (right_grad >> (TRI_SHIFT - SHADE_ADJUST)) -
            (left_grad >> (TRI_SHIFT - SHADE_ADJUST));
        if (gstep) 
            gstep = ((right_igrad << SHADE_ADJUST) -
                     (left_igrad << SHADE_ADJUST))/gstep;
        for (; height2; height2--)
        {
            int w;
            int xl;

            xl = leftx  >> TRI_SHIFT;
            w  = (rightx >> TRI_SHIFT) - xl;
            if (w > 0)
                gdraw_span(colour, screenp + xl, w, lefti, gstep);
            screenp += DestPr->w_;
            rightx  += right_grad;
            leftx   += left_grad;
            lefti   += left_igrad;
            /*      wsize   += wstep;*/
        }
    }
    else
    {
        left_grad  = m3;
        right_grad = m1;
        left_igrad  = mi3;
        right_igrad = mi1;

        gstep = (right_grad >> (TRI_SHIFT - SHADE_ADJUST)) -
            (left_grad >> (TRI_SHIFT - SHADE_ADJUST));
        if (gstep) 
            gstep = ((right_igrad << SHADE_ADJUST) -
                     (left_igrad << SHADE_ADJUST))/gstep;
        for (; height1; height1--)
        {
            int w;
            int xl;
            xl = leftx  >> TRI_SHIFT;
            w  = (int)(rightx >> TRI_SHIFT) - xl;
            if (w > 0)
                gdraw_span(colour, screenp + xl, w, lefti, gstep);
            screenp += DestPr->w_;
            rightx  += right_grad;
            leftx   += left_grad;
            lefti   += left_igrad;
        }

        right_grad  = m2;
        rightx      = x2 << TRI_SHIFT;
        right_igrad = mi2;

        gstep = (right_grad >> (TRI_SHIFT - SHADE_ADJUST)) -
            (left_grad >> (TRI_SHIFT - SHADE_ADJUST));
        if (gstep) 
            gstep = ((right_igrad << SHADE_ADJUST) -
                     (left_igrad << SHADE_ADJUST))/gstep;
        for (; height2; height2--)
        {
            int w;
            int xl;

            xl = leftx  >> TRI_SHIFT;
            w  = (int)(rightx >> TRI_SHIFT) - xl;
            if (w > 0)
                gdraw_span(colour, screenp + xl, w, lefti, gstep);
            screenp += DestPr->w_;
            rightx  += right_grad;
            leftx   += left_grad;
            lefti   += left_igrad;
        }
    }
}
#endif // SUPPORT_GOURAUD
