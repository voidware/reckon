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

#ifndef __bcd_h__
#define __bcd_h__

#include "bcdfloat.h"

struct BCD
{
    typedef BCDFloat::Format Format;

    // Constructors
    BCD() {}
    BCD(const char* s) : _v(s) {}
    BCD(int4 v) : _v(v) {}
    BCD(uint4 v) : _v(v) {}
    BCD(const BCDFloatData& bf) : _v(bf) {}

#ifdef SMALL_INT
    BCD(int v) : _v(v) {}
    BCD(unsigned int v) : _v(v) {}
#endif

    BCD                 asBCD() const { return *this; }

    int                 exponent() const { return _v.exp(); }
    void                setExponent(int v) { _v.exp(v); }
    int                 digit(int n) const { return _v._d[n]; }
    void                digit(int n, int v)  { _v._d[n] = (unsigned short)v; }

    // Arithmetic
    friend BCD          operator+(const BCD& a, const BCD& b)
    {
        BCD c;
        BCDFloat::add(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD          operator-(const BCD& a, const BCD& b)
    {
        BCD c;
        BCDFloat::sub(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD          operator*(const BCD& a, const BCD& b)
    {
        BCD c;
        BCDFloat::mul(&a._v, &b._v, &c._v);
        return c;
    }
    friend BCD          operator/(const BCD& a, const BCD& b)
    {
        BCD c;
        BCDFloat::div(&a._v, &b._v, &c._v);
        return c;
    }
    void                operator+=(const BCD& b)
    {
        BCD c;
        BCDFloat::add(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator-=(const BCD& b)
    {
        BCD c;
        BCDFloat::sub(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator*=(const BCD& b)
    {
        BCD c;
        BCDFloat::mul(&_v, &b._v, &c._v);
        _v = c._v;
    }
    void                operator/=(const BCD& b)
    {
        BCD c;
        BCDFloat::div(&_v, &b._v, &c._v);
        _v = c._v;
    }
    BCD                 operator-() const
    {
        BCD c(*this);
        c.negate();
        return c;
    }
    void                operator++() { *this += 1; }
    void                operator--() { *this -= 1; }

    friend int4         ifloor(const BCD& a)
                        { return BCDFloat::ifloor(&a._v); }
    friend int4         itrunc(const BCD& a)
                        { return BCDFloat::itrunc(&a._v); }
    friend BCD          floor(const BCD& a)
    {
        /* floor, largest integer <= a.
         * eg floor(2.1) = 2.
         *    floor(-2.1) = -3.
         */
        BCD t;
        if (a.isSpecial()) return a;
        BCDFloat::floor(&a._v, &t._v);
        return t;
    }

    friend BCD          trunc(const BCD& a)
    {
        /* truncate towards zero.
         * trunc(2.1) = 2.
         * trunc(-2.1) = -2
         */
        if (a.isSpecial()) return a;
        BCD t;
        BCDFloat::trunc(&a._v, &t._v);
        return t;
    }

    friend BCD          fma(const BCD& a, const BCD& b, const BCD& c)
    {
        BCD r;
        BCDFloat::fma(&a._v, &b._v, &c._v, &r._v);
        return r;
    }

    friend BCD          cross(const BCD& a, const BCD& b,
                              const BCD& c, const BCD& d)
    {
        // a*d - b*c
        BCD w = -b*c;
        return fma(a,d,w) - fma(b,c,w);
    }

    friend BCD          fabs(const BCD& a) { return (a.isNeg()) ? -a : a; }

    friend BCD          frac(const BCD& a)
    {
        if (a.isSpecial()) return a;
        return a - trunc(a);
    }
    const char*         asString() const
    {
        _v.asString(_buf);
        return _buf;
    }

    static BCD          epsilon(int n)
    {
        BCD v;
        BCDFloat::epsilon(n, &v._v);
        return v;
    }

    static BCD          pow10(int n)
    {
        // generate 10^n, n >= 0
        BCD v;
        BCDFloat::pow10(n, &v._v);
        return v;
    }

    const char*         asStringFmt(Format fmt, int precision) const
    {
        _v.asStringFmt(_buf, fmt, precision);
        return _buf;
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
    BCD                 round(int digits) const
    {
        BCD rv;
        _v._roundDigits(BCDFloat::format_normal, digits, &rv._v);
        return rv;
    }

    static BCD          inf() { return BCDFloat::posInf(); }


    // Comparison
    friend bool         operator==(const BCD& a, const BCD& b)
                        { return BCDFloat::equal(&a._v, &b._v); }
    friend bool         operator!=(const BCD& a, const BCD& b)
                        { return !BCDFloat::equal(&a._v, &b._v); }
    friend bool         operator<(const BCD& a, const BCD& b)
                        { return BCDFloat::lt(&a._v, &b._v); }
    friend bool         operator<=(const BCD& a, const BCD& b)
                        { return BCDFloat::le(&a._v, &b._v); }
    friend bool         operator>(const BCD& a, const BCD& b)
                        { return BCDFloat::gt(&a._v, &b._v); }
    friend bool         operator>=(const BCD& a, const BCD& b)
                        { return BCDFloat::ge(&a._v, &b._v); }

    BCDFloat            _v;
    static char         _buf[40];
};

BCD powInt(const BCD& a, int4 n);
BCD sqrt(const BCD&);

inline BCD pow(const BCD& a, int4 n)
{
    return powInt(a, n);
}

inline bool operator==(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a == *(BCD*)&b;
}

inline bool operator<(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a < *(BCD*)&b;
}

inline bool operator<=(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a <= *(BCD*)&b;
}

inline bool operator>(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a > *(BCD*)&b;
}

inline bool operator>=(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a >= *(BCD*)&b;
}

inline BCD operator-(const BCDFloatData& a, const BCDFloatData& b)
{
    return *(BCD*)&a - *(BCD*)&b;
}

inline BCD frexp(const BCD& v, int* e)
{
    BCD n(v);
    *e = n.exponent()-1;
    n.setExponent(1);
    return n;
}

inline BCD ldexp(const BCD& v, int e)
{
    BCD n(v);
    n.setExponent(e);
    return n;
}
#endif // __bcd_h__
