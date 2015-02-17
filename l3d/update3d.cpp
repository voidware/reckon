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

#include "os.h"

extern "C"
{
#include "fxlib.h"
#include "../utils.h"

    void Update3DScreen();
};

void Update3DScreen()
{
    Bitmap2D* pr;
    unsigned char* dp;
    unsigned char* sp0;
    unsigned char* sp;
    int i;
    int j;
    int k;

    dp = GetVRAMPtr();
    pr = GetScreenBitmap2D();
    sp0 = pr->pix_;

    for (i = 0; i < pr->h_; ++i)
    {
        int v = 0;
        k = 8;
        sp = sp0;
        for (j = 0; j < pr->w_; ++j)
        {
            --k;
            if (*sp++ > 0x80) v |= (1<<k);
            if (!k)
            {
                *dp++ = v;
                v = 0;
                k = 8;
            }
        }
        sp0 += pr->stride_;
    }
    clear_pix(pr, 0);
    
    Bdisp_PutDisp_DD();
}
