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

#ifndef __bcd2_h__
#define __bcd2_h__

#include "bcdfloat2.h"
#include "bcd.h"

struct BCD2
{

    // Constructors
    BCD2() {}
    BCD2(int4 v) : _v(v) {}
    BCD2(uint4 v) : _v(v) {}

    BCD2(const BCDFloatData2& bf) : _v(bf) {}

    BCD2(const BCDFloatData& bf) : _v(bf) {}
    BCD2(const BCD& b) : _v(b._v) {}

    int                 exponent() const { return _v.exp(); }
    void                setExponent(int v) { _v.exp(v); }
    int                 digit(int n) const { return _v._d[n]; }
    void                digit(int n, int v)  { _v._d[n] = v; }
    BCD                 asBCD() const
    {
        BCD v;
        _v.asBCD(&v._v);
        return v;
    }

    // Arithmetic
    friend BCD2          operator+(const BCD2& a, const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::add(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD2          operator-(const BCD2& a, const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::sub(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD2          operator*(const BCD2& a, const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::mul(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD2          operator/(const BCD2& a, const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::div(&a._v, &b._v, &c._v);
        return c;
    }
    void                operator+=(const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::add(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator-=(const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::sub(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator*=(const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::mul(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator/=(const BCD2& b)
    {
        BCD2 c;
        BCDFloat2::div(&_v, &b._v, &c._v);
        _v = c._v;
    }
    BCD2                 operator-() const
    {
        BCD2 c(*this);
        c.negate();
        return c;
    }
    void                operator++() { *this += 1; }
    void                operator--() { *this -= 1; }

    friend int4         ifloor(const BCD2& a)
                        { return BCDFloat2::ifloor(&a._v); }
    friend int4         itrunc(const BCD2& a)
                        { return BCDFloat2::itrunc(&a._v); }
    friend BCD2          floor(const BCD2& a)
    {
        /* floor, largest integer <= a.
         * eg floor(2.1) = 2.
         *    floor(-2.1) = -3.
         */
        BCD2 t;
        if (a.isSpecial()) return a;
        BCDFloat2::floor(&a._v, &t._v);
        return t;
    }

    friend BCD2          trunc(const BCD2& a)
    {
        /* truncate towards zero.
         * trunc(2.1) = 2.
         * trunc(-2.1) = -2
         */
        if (a.isSpecial()) return a;
        BCD2 t;
        BCDFloat2::trunc(&a._v, &t._v);
        return t;
    }

    friend BCD2          fabs(const BCD2& a) { return (a.isNeg()) ? -a : a; }
    friend BCD2          frac(const BCD2& a)
    {
        if (a.isSpecial()) return a;
        return a - trunc(a);
    }
    static BCD2          epsilon(int n)
    {
        BCD2 v;
        BCDFloat2::epsilon(n, &v._v);
        return v;
    }

    bool                isZero() const { return _v.isZero(); }
    bool                isNeg() const { return _v.neg(); }
    bool                isSpecial() const
                                { return _v.isSpecial(); }
    bool                isInf() const
                                { return  _v.isInf(); }
    bool                isNan() const
                                { return _v.isNan(); }
    bool                isInteger() const { return _v.isInteger(); }

    void                negate() { _v.negate(); }

    void                makeInf() { _v.makeInf(); }


    // Comparision
    friend bool         operator==(const BCD2& a, const BCD2& b)
                        { return BCDFloat2::equal(&a._v, &b._v); }
    friend bool         operator!=(const BCD2& a, const BCD2& b)
                        { return !BCDFloat2::equal(&a._v, &b._v); }
    friend bool         operator<(const BCD2& a, const BCD2& b)
                        { return BCDFloat2::lt(&a._v, &b._v); }
    friend bool         operator<=(const BCD2& a, const BCD2& b)
                        { return BCDFloat2::le(&a._v, &b._v); }
    friend bool         operator>(const BCD2& a, const BCD2& b)
                        { return BCDFloat2::gt(&a._v, &b._v); }
    friend bool         operator>=(const BCD2& a, const BCD2& b)
                        { return BCDFloat2::ge(&a._v, &b._v); }

    friend BCD2 pow(const BCD2& a, int4 n)
    {
        int4 m;
        if (n == 0) return 1;
        m = (n < 0) ? -n : n;

        BCD2 s;
        if (m > 1) 
        {
            BCD2 r = a;
            s = 1;
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
        if (n < 0) 
            return 1/s;

        return s;
    }

    BCDFloat2            _v;
};

inline bool operator==(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a == *(BCD2*)&b;
}

inline bool operator<(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a < *(BCD2*)&b;
}

inline bool operator<=(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a <= *(BCD2*)&b;
}

inline bool operator>(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a > *(BCD2*)&b;
}

inline bool operator>=(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a >= *(BCD2*)&b;
}

inline BCD2 operator-(const BCDFloatData2& a, const BCDFloatData2& b)
{
    return *(BCD2*)&a - *(BCD2*)&b;
}

inline BCD2 sqrt(const BCD2& a)
{
    BCD2 c;
    if (!BCDFloat2::sqrt(&a._v, &c._v))
        c._v.makeNAN();
    return c;
}

#endif // __bcd2_h__
