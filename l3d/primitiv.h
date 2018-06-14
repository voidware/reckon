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

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#define CLIP_TOP    1
#define CLIP_BOTTOM 2
#define CLIP_RIGHT  4
#define CLIP_LEFT   8
#define CLIP_Z     16

#include "l3defs.h"
#include "2d.h"

void  clear_pix(Bitmap2D*, int);
void  draw_polygon(int, int, Point3**);
void  clip_polygon(Plane*);

void  vert_line(Bitmap2D *pr, char col, int x, int y1, int y2);
void  hori_line(Bitmap2D *pr, char col, int x1, int x2, int y);
void  plot(Bitmap2D*, int, int, int);
void  line(Bitmap2D* pr, int col, int x1, int y1, int x2, int y2);
void  draw_box(Bitmap2D*, int, int, int, int, int);
void  draw_cross(Bitmap2D*, int col, int, int, int, int);

/* some more usable primitives as they invert and scale their
   coordinates first.
*/
void  h_line(Bitmap2D *pr, char col, int x1, int x2, int y);
void  v_line(Bitmap2D *pr, char col, int x, int y1, int y2);

void       fill_rect(Bitmap2D *pr, char col,
                     int x, int y, int width, int height );
void  scrn_fill_rect(Bitmap2D *pr, char col,
                     int x, int y, int width, int height );

#endif

