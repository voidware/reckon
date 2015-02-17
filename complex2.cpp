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

#include <stdio.h>
#include "complex2.h"

Complex2 operator*(const Complex2& a, const Complex2& b)
{
    Complex2 c;
    c.r_ = a.r_*b.r_ - a.i_*b.i_;
    c.i_ = a.i_*b.r_ + a.r_*b.i_;
    return c;
}

Complex2 operator/(const Complex2& a, const Complex2& b)
{
    Complex2 c;
    if (b.i_.isZero())
    {
        // divide by real
        c.r_ = a.r_ / b.r_;
        c.i_ = a.i_ / b.r_;
    }
    else if (b.r_.isZero())
    {
        // divide by imaginary
        c.r_ = a.i_ / b.i_;
        c.i_ = -a.r_ / b.i_;
    }
    else
    {
        // see Numerical recipes, 3rd p226
        BCD2 t, s;
        if (fabs(b.r_) >= fabs(b.i_))
        {
            t = b.i_/b.r_;                       // d/c
            s = b.r_ + b.i_*t;                   // c + d(*d/c)
            c.r_ = (a.r_+a.i_*t)/s;               // (a + b*t)/s
            c.i_ = (a.i_-a.r_*t)/s;              // (b-a*t)/s
        }
        else
        {
            t = b.r_/b.i_;                       // c/d
            s = b.r_*t+b.i_;                     // c*t+d
            c.r_ = (a.r_*t + a.i_)/s;             // (a*t + b)/s
            c.i_ = (a.i_*t - a.r_)/s;            // (b*t - a)/s
        }
    }
    return c;
}

Complex2 square(const Complex2& a)
{
    Complex2 c;
    c.r_ = a.r_*a.r_ - a.i_*a.i_;
    c.i_ = 2*a.i_*a.r_;
    return c;
}

Complex2 inv(const Complex2& a)
{
    // 1 / (c + i d) = c / (c*c + d*d) - i d / (c*c + d*d)
    Complex2 c;
    BCD2 d = a.r_*a.r_ + a.i_*a.i_;
    c.r_ = a.r_/d;
    c.i_ = -a.i_/d;
    return c;
}

Complex2 sqrt(const Complex2& a)
{
    Complex2 b;
    if (a.i_.isZero())
    {
        if (a.r_.isNeg())
        {
            b.r_ = 0;
            b.i_ = sqrt(-a.r_);
        }
        else
        {
            b.r_ = sqrt(a.r_);
            b.i_ = 0;
        }
    }
    else
    {
        // see Numerical Recipes, 3rd, p226
        BCD2 fc = fabs(a.r_);
        BCD2 fd = fabs(a.i_);
        BCD2 w, t;
        if (fc >= fd)
        {
            t = a.i_/a.r_;
            w = sqrt(fc)*sqrt((sqrt(1 + t*t) + 1)/2);
        }
        else
        {
            if (a.r_.isZero())
                w = sqrt(fd/2);
            else
            {
                t = a.r_/a.i_;
                w = sqrt(fd)*sqrt((fabs(t) + sqrt(1 + t*t))/2);
            }
        }
        
        if (w.isZero())
        {
            b.r_ = 0;
            b.i_ = 0;
        }
        else if (!a.r_.isNeg())
        {
            b.r_ = w;
            b.i_ = a.i_/(2*w);
        }
        else 
        {
            b.r_ = fd/(2*w);
            b.i_ = w;
            if (a.i_.isNeg())
                b.conjugate();
        }        
    }
    return b;
}

BCD2 fabs(const Complex2& a)
{
    // XXX need to use stable formula
    return sqrt(a.r_*a.r_ + a.i_*a.i_);
}


