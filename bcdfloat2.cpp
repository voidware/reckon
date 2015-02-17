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

#pragma warning (disable : 4244) // int -> short

#include "bcdfloat2.h"

void BCDFloat2::_init()
{
    for (int i = 0; i <= P2; ++i) _d[i] = 0;
}

bool BCDFloat2::trunc(const BCDFloat2* a, BCDFloat2* c)
{
    /* truncate towards zero.
     * trunc(2.1) = 2.
     * trunc(-2.1) = -2
     */
    *c = *a;
    int e = c->exp();
    int i;
    for (i = P2-1; i >= 0; --i) 
        if (i >= e) c->_d[i] = 0;

    return true;
}

int4 BCDFloat2::asInt() const
{
    // if we fit in an int, return it otherwise 0
    int ea = exp();
    int4 v = 0;
    int i = 0;
    while (i < ea && i < P2) 
    {
        if (v > 214748L) return 0; // too large, bail out.
        v*= BASE;
        v += _d[i];
        ++i;
    }
    if (neg()) v = -v;
    return v;
}

bool BCDFloat2::isInteger() const
{
    if (isZero())
        return true;

    int e = exp();
    int i;
    for (i = P2-1; i >= 0; --i) 
        if (_d[i]) return e > i;
    return false;
}

bool BCDFloat2::sqrt(const BCDFloat2* a, BCDFloat2* r)
{
    // use single precision version as approx.
    BCDFloat a1;
    BCDFloat r1;
    a->asBCD(&a1);
    bool res = BCDFloat::sqrt(&a1, &r1);
    
    BCDFloat2 rr(r1);
    BCDFloat2 t, t2;
    BCDFloat2 two(2U);
    if (res)
    {
        if (!rr.isSpecial())
        {
            for (int i = 0; i < 3; ++i)
            {
                div(a,&rr,&t);              // t = a/x
                add(&t, &rr, &t2);          // t2 = (a/x) + x
                div(&t2, &two, &rr);        // x = (t2)/2
            }
        }
        *r = rr;
    }
    return res;
}
