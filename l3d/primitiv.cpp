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

#define DEBUG

#include <string.h>
#include "model.h"
#include "fill.h"

extern Bitmap2D *ScreenBitmap2D;
extern Bitmap2D *DestPr;  

void line(Bitmap2D* pr, int col, int x1, int y1, int x2, int y2)
{
    DC2D dc;
    dc.dobj_ = pr;
    DrawLineUni2D(&dc, x1, y1, x2, y2, col);
}

void plot(Bitmap2D* pr, int col, int x, int y)
{
    DC2D dc;
    dc.dobj_ = pr;
    SetPixel2D(&dc, x, y, col);
}
