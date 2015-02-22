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

#include "model.h"
#include "matrix.h"


void set_model_colour(Model* m, int col)
{                 
    int n = m->num_planes;
    Plane *pln  = m->planes;

    for (; n; n--) 
    {
        pln->color = col;
        pln->area = VISIBLE;
        pln++;
    }
    shade_planes(m);

#ifdef SUPPORT_GOURAUD
    shade_vertices(m);
#endif
}

void flip_plane(Plane *pln)
{
    Point3 *tmp;
    int n = pln->num_verts;
    if (n == 3) 
    {
        tmp = pln->p1;
        pln->p1 = pln->p2;
        pln->p2 = tmp;
    }
    else if (n == 4) 
    {
        tmp = pln->p1;
        pln->p1 = pln->p4;
        pln->p4 = tmp;
        tmp = pln->p2;
        pln->p2 = pln->p3;
        pln->p3 = tmp;
    }

    pln->normx = -pln->normx;
    pln->normy = -pln->normy;
    pln->normz = -pln->normz;
}

void flip_planes(Model *m)
{
    /* this flips all a models polygons turning it inside-out */
    int n = m->num_planes;
    Plane *pln = m->planes;
    for (; n; n--) flip_plane(pln++);
}

void set_shading_type(Model *m, int type)
{
    int n = m->num_planes;
    Plane *pln = m->planes;

    for (; n; n--) 
    {
        int t = pln->num_verts == 3;
        pln->type = t ? (TRIANGLE_TYPE<<SHADING_BITS) | type :
            (POLYGON_TYPE<<SHADING_BITS | type);
        pln++;
    }
}
        

