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
#include "view.h"
#include "triangle.h"
#include "drawpoly.h"
#include "clip.h"

Bitmap2D *DestPr;
SCREEN_UNIT *DestMem;

void init_renderer(Bitmap2D *pr)
{
    DestPr   = pr;
    DestMem  = pr->pix_;
}

void render_model_planes(Model* m)
{
    Plane**    pln_pp = m->planes_order;
    int n = m->num_planes;
    while (n--)
    {
        Plane* pln = *pln_pp++;
        switch (pln->type)
        {
        case TRIANGLE :
            draw_triangle(pln);
            break;
        case POLYGON : 
            draw_polygon(pln->pixel, pln->num_verts, &pln->p1);
            break;
#ifdef SUPPORT_GOURAUD
        case GOURAUD_TRIANGLE :
            draw_gtriangle(pln);
            break;
        case GOURAUD_POLYGON  :
            draw_gpolygon(GET_COLOUR(pln), pln->num_verts, &pln->p1);
            break;
#endif
        }
    }
}

void render_clipped_model_planes(Model* m)
{
    /* Render the model's planes using the `model_plane_order' */
    Plane**    pln_pp = m->planes_order;
    int n = m->num_planes;
    while (n--)
    {
        Plane* pln = *pln_pp++;
        switch (pln->type & SHADING_MASK)
        {
        case SHADING_SIMPLE:
        case SHADING_WIRE:
            clip_polygon(pln);
            break;
#ifdef SUPPORT_GOURAUD
        case SHADING_GOURAUD:
            gclip_polygon(pln);
            break;
#endif
        }
    }
}



