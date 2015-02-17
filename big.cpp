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

#include "big.h"

static void _gcdinv(const BigInt& x, const BigInt& y, BigInt& iy, BigInt& g)
{
    // x >= y >= 0, x > 0
    BigInt b, v, w;
    
    b = 0;
    g = x;
    v = 1;
    w = y;
    while (w > 0)
    {
        BigInt q = g/w;
        Big* t;

        t = (b - q*v).give();
        b = v.give();
        v = t;
        
        t = (g - q*w).give();
        g = w.give();
        w = t;
    }

    if (b.isNeg())
        b += x;
    else if (b >= x)
        b %= x;

    iy = b.give();
}

Big* invert(const BigInt& a, const BigInt& n, BigInt& g)
{
    BigInt ia;
    _gcdinv(n, mod(a,n), ia, g);
    return ia.give();  // avoid copy
}
