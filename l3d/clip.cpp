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

#include "view.h"
#include "drawpoly.h"
#include "os.h"
#include "render.h"
#include "clip.h"

#define MAX_CLIP        12

static int clippos;
static Point3 clips[MAX_CLIP];
static Point3* clipin[MAX_CLIP];
static Point3* clipout[MAX_CLIP];

static Point3* clip_top(Point3 *p1, Point3 *p2)
{
    int clip = 0;
    Point3* newp = clips + clippos++;
    int x1 = p1->tx + (p2->tx - p1->tx) * - p1->ty / (p2->ty - p1->ty);

    if( x1 >= DestPr->w_) SET_FLAG(clip, CLIP_RIGHT);
    if( x1      < 0 ) SET_FLAG(clip, CLIP_LEFT);
    newp->tx    = x1;
    newp->ty    = 0;
    newp->clip = clip;
    newp->refs = p1->refs > p2->refs ? p1->refs : p2->refs;
    newp->tz = 1; // clip flag
    return newp;
}

static Point3* clip_bottom(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int clip = 0;
    Point3* newp = clips + clippos++;

    x1  = x1 + (x2 - x1) * (DestPr->h_ -1 -y1) / (y2 - y1);

    if( x1 >= DestPr->w_) SET_FLAG(clip, CLIP_RIGHT);
    if( x1      < 0 ) SET_FLAG(clip, CLIP_LEFT);
    newp->tx    = x1;
    newp->ty    = DestPr->h_ - 1;
    newp->clip = clip;
    newp->refs = p1->refs > p2->refs ? p1->refs : p2->refs;
    newp->tz = 1; // clip flag
    return newp;
}

static Point3* clip_right(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int clip = 0;
    Point3* newp = clips + clippos++;
    
    y1 = y1 + (y2 - y1) * (DestPr->w_ - 1 - x1) / (x2 -x1);

    if( y1   < 0 )    SET_FLAG(clip, CLIP_TOP);
    if( y1 >= DestPr->h_) SET_FLAG(clip, CLIP_BOTTOM);
    newp->ty = y1;
    newp->tx = DestPr->w_ - 1;
    newp->clip = clip;
    newp->refs = p1->refs > p2->refs ? p1->refs : p2->refs;
    newp->tz = 1; // clip flag
    return newp;
}

static Point3* clip_left(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int clip = 0;
    Point3* newp = clips + clippos++;

    y1 = y1 + (y2 - y1) * -x1 / (x2 - x1);

    if( y1   < 0 )    SET_FLAG(clip, CLIP_TOP);
    if( y1 >= DestPr->h_) SET_FLAG(clip, CLIP_BOTTOM);
    newp->ty = y1;
    newp->tx = 0;                
    newp->clip = clip;
    newp->refs = p1->refs > p2->refs ? p1->refs : p2->refs;
    newp->tz = 1; // clip flag
    return newp;
}

static int clip_poly_top(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];               /* s is last vertex */
    for (j = 0; j < n; j++)
    {
        p = inp[j];                     /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_TOP)) 
        {
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_TOP)) 
            {
                /* if s is inside */
                outp[outn++] = clip_top(p, s);
            }
        } 
        else
        { /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_TOP)) {     /* and s is outside */
                outp[outn++] = clip_top(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
}              
  
static int clip_poly_bottom(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j];                      /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_BOTTOM))
        { 
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_BOTTOM)) {    /* if s is inside */
                outp[outn++] = clip_bottom(p, s);
            }
        } else {                          /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_BOTTOM)) {     /* and s is outside */
                outp[outn++] = clip_bottom(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
} 

static int clip_poly_right(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                     /* s is last vertex */
    for (j = 0; j < n; j++) 
    {            
        p = inp[j];                   /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_RIGHT))
        {    
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_RIGHT)) {    /* if s is inside */
                outp[outn++] = clip_right(p, s);
            }
        } else {                          /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_RIGHT)) {     /* and s is outside */
                outp[outn++] = clip_right(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
} 
   
static int clip_poly_left(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j];                      /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_LEFT))
        {   
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_LEFT)) {    /* if s is inside */
                outp[outn++] = clip_left(p, s);
            }
        } else {                            /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_LEFT)) {     /* and s is outside */
                outp[outn++] = clip_left(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                  /* advance to next pair of vertices */
    }
    return outn;
}          

void clip_polygon(Plane* pln)
{
    int n, outn, reject, accept;     
    Point3** in_p;
    Point3** out_p;
    Point3** op;
    Point3** tmp;
    int trans = pln->color == TRANSPARENT_COLOUR;

    clippos = 0;
    op = &(pln->p1);

    reject = 0;
    accept = 0;
    outn = pln->num_verts;

    for (n = 0; n < outn; n++) 
    {
        int clip  = (*op)->clip;
        reject &= clip;
        accept |= clip;
        if (reject) return;       /* triangle completely off screen */

        // reset z coordinate, no longer used
        // we use it as a clip flag.
        (*op)->tz = 0;

        clipin[n] = *op++;
    }

    if (accept == 0)
    {
        // poly completely on screen.
        if (!trans)
            draw_polygon(pln->pixel, outn, &pln->p1);
        else
            draw_wire_polygon(outn, &pln->p1, pln);
        return;
    }

    if(CHECK_FLAG(accept, CLIP_Z))
        return;   /* do_z_clip()*/

    in_p  = clipin;
    out_p = clipout;
    
    if (CHECK_FLAG(accept, CLIP_TOP)) 
    {
        outn = clip_poly_top(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;

        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }
    if (CHECK_FLAG(accept, CLIP_BOTTOM)) 
    {
        outn = clip_poly_bottom(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;
        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }
    if (CHECK_FLAG(accept, CLIP_RIGHT))
    {
        outn = clip_poly_right(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;

        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }             
    if (CHECK_FLAG(accept, CLIP_LEFT))
    {
        outn = clip_poly_left(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;
        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }

    if (outn < 3) return;

    if (!trans)
        draw_polygon(pln->pixel, outn, in_p);
    else
        draw_wire_polygon(outn, in_p, pln);
} 


#ifdef SUPPORT_GOURAUD
static Point3* gclip_top(Point3 *p1, Point3 *p2)
{
    int x1;
    int i1;
    int clip = 0;
    Point3* newp = clips + clippos++;

    x1 = p1->tx + (p2->tx - p1->tx) * - p1->ty / (p2->ty - p1->ty);
    i1 = p1->ti + (p2->ti - p1->ti) * - p1->ty / (p2->ty - p1->ty);

    if( x1 >= DestPr->w_) SET_FLAG(clip, CLIP_RIGHT);
    if( x1      < 0 ) SET_FLAG(clip, CLIP_LEFT);
    newp->tx    = x1;
    newp->ty    = 0;
    newp->ti   = i1;                
    newp->clip = clip;
    return newp;
}

static Point3* gclip_bottom(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int i1 = p1->ti;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int i2 = p2->ti;
    int clip = 0;
    Point3* newp = clips + clippos++;

    x1  = x1 + (x2 - x1) * (DestPr->h_ -1 -y1) / (y2 - y1);
    i1  = i1 + (i2 - i1) * (DestPr->h_ -1 -y1) / (y2 - y1);

    if( x1 >= DestPr->w_) SET_FLAG(clip, CLIP_RIGHT); 
    if( x1      < 0 ) SET_FLAG(clip, CLIP_LEFT);
    newp->tx    = x1;
    newp->ty    = DestPr->h_ - 1;
    newp->ti   = i1;                
    newp->clip = clip;
    return newp;
}

static Point3* gclip_right(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int i1 = p1->ti;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int i2 = p2->ti;
    int clip = 0;
    Point3* newp = clips + clippos++;
    
    y1 = y1 + (y2 - y1) * (DestPr->w_ - 1 - x1) / (x2 -x1);
    i1 = i1 + (i2 - i1) * (DestPr->w_ - 1 - x1) / (x2 -x1);

    if( y1   < 0 )    SET_FLAG(clip, CLIP_TOP);
    if( y1 >= DestPr->h_) SET_FLAG(clip, CLIP_BOTTOM);

    newp->tx    = DestPr->w_ - 1;
    newp->ty    = y1;
    newp->ti   = i1;                
    newp->clip = clip;
    return newp;
}

static Point3* gclip_left(Point3 *p1, Point3 *p2)
{
    int x1 = p1->tx;
    int y1 = p1->ty;
    int i1 = p1->ti;
    int x2 = p2->tx;
    int y2 = p2->ty;
    int i2 = p2->ti;
    int clip = 0;
    Point3* newp = clips + clippos++;

    y1 = y1 + (y2 - y1) * -x1 / (x2 - x1);
    i1 = i1 + (i2 - i1) * -x1 / (x2 - x1);

    if( y1   < 0 )    SET_FLAG(clip, CLIP_TOP);
    if( y1 >= DestPr->h_) SET_FLAG(clip, CLIP_BOTTOM);

    newp->tx    = 0;                
    newp->ty    = y1;
    newp->ti   = i1;                
    newp->clip = clip;
    return newp;
}

static int gclip_poly_top(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];               /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j]; 
        if (CHECK_FLAG(p->clip, CLIP_TOP))
        {
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_TOP))
            {   
                /* if s is inside */
                outp[outn++] = gclip_top(p, s);
            }
        }
        else
        {                       /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_TOP))
            {  
                /* and s is outside */
                outp[outn++] = gclip_top(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
}              
  
static int gclip_poly_bottom(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j];                      /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_BOTTOM))
        { 
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_BOTTOM))
            { 
                /* if s is inside */
                outp[outn++] = gclip_bottom(p, s);
            }
        }
        else
        { 
            /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_BOTTOM))
            {
                /* and s is outside */
                outp[outn++] = gclip_bottom(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
} 

static int gclip_poly_right(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                     /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j];                   /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_RIGHT))
        {    
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_RIGHT))
            {
                /* if s is inside */
                outp[outn++] = gclip_right(p, s);
            }
        } 
        else
        {
            /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_RIGHT))
            {
                /* and s is outside */
                outp[outn++] = gclip_right(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                    /* advance to next pair of vertices */
    }
    return outn;
} 
   
static int gclip_poly_left(Point3** inp, Point3** outp, int n)
{
    int j, outn = 0;     
    Point3 *s, *p;

    s = inp[n-1];                /* s is last vertex */
    for (j = 0; j < n; j++)
    {            
        p = inp[j];                      /* p is next vertex to check */
        if (CHECK_FLAG(p->clip, CLIP_LEFT))
        {   
            /* if flag is set point is outside this boundary */
            if (!CHECK_FLAG(s->clip, CLIP_LEFT))
            {    /* if s is inside */
                outp[outn++] = gclip_left(p, s);
            }
        }
        else 
        {
            /* if p is inside boundary */
            if (CHECK_FLAG(s->clip, CLIP_LEFT))
            {
                /* and s is outside */
                outp[outn++] = gclip_left(s, p);
            }
            outp[outn++] = p;
        }
        s = p;                  /* advance to next pair of vertices */
    }
    return outn;
}          

void gclip_polygon(Plane* pln)
{

    int n, outn, reject, accept;     
    Point3** in_p;
    Point3** out_p;
    Point3** op;
    Point3** tmp;

    clippos = 0;
    op = &(pln->p1);

    reject = 0;
    accept = 0;
    outn = pln->num_verts;

    for (n = 0; n < outn; n++) 
    {
        int clip  = (*op)->clip;
        reject &= clip;
        accept |= clip;
        if (reject) return;       /* triangle completely off screen */

        (*op)->tz = 0;
        clipin[n] = *op++;
    }

    if (accept == 0)
    {
        /*
         * poly completely on screen.
         */
        draw_gpolygon(GET_COLOUR(pln), outn, &pln->p1);
        return;
    }

    if(CHECK_FLAG(accept, CLIP_Z))
        return;   /* do_z_clip()*/

    in_p  = clipin;
    out_p = clipout;

    if (CHECK_FLAG(accept, CLIP_TOP)) 
    {
        outn = gclip_poly_top(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;

        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++) 
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }
    if (CHECK_FLAG(accept, CLIP_BOTTOM)) 
    {
        outn = gclip_poly_bottom(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;
        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }
    if (CHECK_FLAG(accept, CLIP_RIGHT)) 
    {
        outn = gclip_poly_right(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;

        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++) 
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }             
    if (CHECK_FLAG(accept, CLIP_LEFT)) 
    {
        outn = gclip_poly_left(in_p, out_p, outn);
        tmp   = in_p;
        in_p  = out_p;
        out_p = tmp;
        reject = 0;
        accept = 0;
        for (n = 0; n < outn; n++)
        {
            reject &= in_p[n]->clip;
            accept |= in_p[n]->clip;
        }
        if (reject) return;
    }

    if (outn < 3) return;

	draw_gpolygon(GET_COLOUR(pln), outn, in_p);
} 
#endif // SUPPORT_GOURAUD

