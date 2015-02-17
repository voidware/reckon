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

#ifndef FILL_H
#define FILL_H

extern Bitmap2D* DestPr;

#define DRAW_SPAN(_sp, _col, _leftx, _w)		        \
{								\
    SCREEN_UNIT *dst;						\
    dst = (_sp) + (_leftx);					\
    while (_w--)						\
        *dst++ = _col;			         		\
}

#define DRAW_SPAN_UP(screenp, colour, height, leftx,     rightx,     \
					 left_grad, right_grad) \
for (; height; height--) {					\
    int xl =  (leftx+TRI_ROUND) >> TRI_SHIFT;                   \
    int w  = ((rightx+TRI_ROUND) >> TRI_SHIFT) - xl + 1;        \
    if (w > 0) DRAW_SPAN(screenp, colour, xl, w);               \
    screenp += DestPr->w_;					\
    rightx  += right_grad;					\
    leftx   += left_grad;					\
}							

#endif

