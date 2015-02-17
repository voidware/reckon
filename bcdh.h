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

#ifndef __bcdh_h__
#define __bcdh_h__

#include "bcdfloath.h"
#include "bcd.h"

struct BCDh
{

    // Constructors
    BCDh() {}
    BCDh(int4 v) : _v(v) {}
    BCDh(uint4 v) : _v(v) {}

    BCDh(const BCDFloatDataH& bf) : _v(bf) {}
    BCDh(const BCDFloatData& bf) : _v(bf) {}
    BCDh(const BCD& b) : _v(b._v) {}

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
    friend BCDh          operator+(const BCDh& a, const BCDh& b)
    {
        BCDh c;
        BCDFloatH::add(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCDh          operator-(const BCDh& a, const BCDh& b)
    {
        BCDh c;
        BCDFloatH::sub(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCDh          operator*(const BCDh& a, const BCDh& b)
    {
        BCDh c;
        BCDFloatH::mul(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCDh          operator/(const BCDh& a, const BCDh& b)
    {
        BCDh c;
        BCDFloatH::div(&a._v, &b._v, &c._v);
        return c;
    }
    void                operator+=(const BCDh& b)
    {
        BCDh c;
        BCDFloatH::add(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator-=(const BCDh& b)
    {
        BCDh c;
        BCDFloatH::sub(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator*=(const BCDh& b)
    {
        BCDh c;
        BCDFloatH::mul(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator/=(const BCDh& b)
    {
        BCDh c;
        BCDFloatH::div(&_v, &b._v, &c._v);
        _v = c._v;
    }
    BCDh                 operator-() const
    {
        BCDh c(*this);
        c.negate();
        return c;
    }
    void                operator++() { *this += 1; }
    void                operator--() { *this -= 1; }


    static BCDh          epsilon(int n)
    {
        BCDh v;
        BCDFloatH::epsilon(n, &v._v);
        return v;
    }

    friend int4         itrunc(const BCDh& a)
                        { return BCDFloatH::itrunc(&a._v); }
    friend BCDh         trunc(const BCDh& a)
    {
        /* truncate towards zero.
         * trunc(2.1) = 2.
         * trunc(-2.1) = -2
         */
        if (a.isSpecial()) return a;
        BCDh t;
        BCDFloatH::trunc(&a._v, &t._v);
        return t;
    }

    friend BCDh          fabs(const BCDh& a) { return (a.isNeg()) ? -a : a; }
    friend BCDh          frac(const BCDh& a)
    {
        if (a.isSpecial()) return a;
        return a - trunc(a);
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


    // Comparision
    friend bool         operator==(const BCDh& a, const BCDh& b)
                        { return BCDFloatH::equal(&a._v, &b._v); }
    friend bool         operator!=(const BCDh& a, const BCDh& b)
                        { return !BCDFloatH::equal(&a._v, &b._v); }
    friend bool         operator<(const BCDh& a, const BCDh& b)
                        { return BCDFloatH::lt(&a._v, &b._v); }
    friend bool         operator<=(const BCDh& a, const BCDh& b)
                        { return BCDFloatH::le(&a._v, &b._v); }
    friend bool         operator>(const BCDh& a, const BCDh& b)
                        { return BCDFloatH::gt(&a._v, &b._v); }
    friend bool         operator>=(const BCDh& a, const BCDh& b)
                        { return BCDFloatH::ge(&a._v, &b._v); }

    BCDFloatH            _v;
};

inline bool operator==(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a == *(BCDh*)&b;
}

inline bool operator<(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a < *(BCDh*)&b;
}

inline bool operator<=(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a <= *(BCDh*)&b;
}

inline bool operator>(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a > *(BCDh*)&b;
}

inline bool operator>=(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a >= *(BCDh*)&b;
}

inline BCDh operator-(const BCDFloatDataH& a, const BCDFloatDataH& b)
{
    return *(BCDh*)&a - *(BCDh*)&b;
}

#endif // __bcdh_h__
