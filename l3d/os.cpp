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
#include <stdlib.h>
#include "os.h"
#include "2d.h"

Bitmap2D* ScreenBitmap2D;

Bitmap2D* GetScreenBitmap2D()
{
    return ScreenBitmap2D;
}

void init_os(Bitmap2D* scr)
{
    ScreenBitmap2D = scr;
    clear_pix(scr, 0);
}

void clear_pix(Bitmap2D* pr, int colour)
{
    memset(pr->pix_, colour, pr->w_ * pr->h_);
}

long Time()
{
    return 0;
}
