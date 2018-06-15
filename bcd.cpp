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

#include <stdlib.h>

#pragma warning (disable : 4244) // int -> short

#include "bcdfloat.h"

#ifdef SMALL_INT
// keep BCD2 out of this
#include "bcd.h"
typedef BCD BCD2;
#else
#include "bcd2.h"
#endif

char BCD::_buf[40];

BCD sqrt(const BCD& a)
{
    BCD c;
    if (!BCDFloat::sqrt(&a._v, &c._v))
        c._v = BCDFloat::nan();
    return c;
}

BCD powInt(const BCD& a, int4 n)
{
    int4 m;
    if (n == 0) return 1U;
    m = (n < 0) ? -n : n;

    BCD2 s;
    if (m > 1) 
    {
        BCD2 r = a;
        s = 1U;
        /* Use binary exponentiation */
        for (;;) 
        {
            if (m & 1) s *= r;
            m >>= 1;
            if (!m) break;
            r *= r;
        }
    } else { s = a; }

    /* Compute the reciprocal if n is negative. */
    BCD s1 = s.asBCD();
    return n < 0 ? 1U/s1 : s1;
}
