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

#ifndef __big_h__
#define __big_h__

#include "mi.h"

struct BigInt;

struct BigInt
{
    // constructors
    BigInt() { _big = 0; }
    BigInt(long v)  { _big = createBig(v); }
    BigInt(unsigned long v)  { _big = createBigu(v); }
    BigInt(int v)  { _big = createBig((long)v); }
    BigInt(unsigned int v)  { _big = createBigu((unsigned long)v); }
    BigInt(Big* b) : _big(b) {}
    BigInt(const BigInt& b) { _big = copyBig(b._big); }

    // destructor
    ~BigInt() { destroyBig(_big); }
    
    void operator=(const BigInt& b)
    {
        // ASSUME not self!
        _purge();
        _big = copyBig(b._big);
    }

    void operator=(Big* b)
    {
        _purge();
        _big = b;
    }

    void operator=(long v)
    {
        _purge();
        _big = createBig(v);
    }
    
    void operator=(unsigned long v)
    {
        _purge();
        _big = createBigu(v);
    }

    bool valid() const { return _big != 0; }
    bool isZero() const { return ::isZero(_big); }
    bool isNeg() const { return ::isNeg(_big); }

    void operator=(int v) { *this = (long)v; }
    void operator=(unsigned int v) { *this = (unsigned long)v; }

    // operators
    friend BigInt operator+(const BigInt& a, const BigInt& b)
    { return addBig(a._big, b._big); }

    friend BigInt operator+(const BigInt& a, int b)
    { return addint(a._big, b); }

    friend BigInt operator+(const BigInt& a, unsigned int b)
    { return addint(a._big, b); }

    friend BigInt operator-(const BigInt& a, const BigInt& b)
    { return subBig(a._big, b._big); }

    friend BigInt operator-(const BigInt& a, int b)
    { return addint(a._big, -b); }

    friend BigInt operator-(const BigInt& a, unsigned int b)
    { return subint(a._big, b); }

    friend BigInt operator*(const BigInt& a, const BigInt& b)
    { return mulBig(a._big, b._big); }

    friend BigInt operator*(const BigInt& a, int b)
    { return mulint(a._big, b); }

    friend BigInt operator/(const BigInt& a, const BigInt& b)
    { return divBig(a._big, b._big, 0); }

    friend Big* operator%(const BigInt& a, const BigInt& n)
    {
        Big* r;
        destroyBig(divBig(a._big, n._big, &r));
        return r;
    }

    friend Big* mod(const BigInt& a, const BigInt& n)
    {
        // as %, but does not allow negative
        Big* r;
        destroyBig(divBig(a._big, n._big, &r));
        if (::isNeg(r))
        {
            Big* m = addBig(r, n._big);
            destroyBig(r);
            r = m;
        }
        return r;
    }


    friend BigInt operator<<(const BigInt& a, unsigned int n)
    {
        return lshiftn(a._big, n);
    }

    friend BigInt operator>>(const BigInt& a, unsigned int n)
    {
        return rshiftn(a._big, n);
    }

    void operator>>=(unsigned int n) { *this = (*this >> n).give(); } 
    void operator%=(const BigInt& n) { *this = *this % n; } 
    void operator+=(const BigInt& b) { *this = (*this + b).give(); }
    void operator+=(int b) { *this = (*this + b).give(); }
    void operator+=(unsigned int b) { *this = (*this + b).give(); }

    void operator-=(const BigInt& b) { *this = (*this - b).give(); }
    void operator-=(int b) { *this = (*this - b).give(); }
    void operator-=(unsigned int b) { *this = (*this - b).give(); }

    void operator*=(const BigInt& b) { *this = (*this * b).give(); }
    void operator*=(int b) { *this = (*this * b).give(); }

    // Comparison
    friend bool operator<(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) < 0; }
    friend bool operator<=(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) <= 0; }
    friend bool operator==(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) == 0; }
    friend bool operator!=(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) != 0; }
    friend bool operator>(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) > 0; }
    friend bool operator>=(const BigInt& a, const BigInt& b)
    { return compare(a._big, b._big) >= 0; }

    friend BigInt fabs(const BigInt& b)
    {
        Big* a = copyBig(b._big);
        if (::isNeg(a))
            a = negateBig(a);
        return a;
    }

    friend BigInt sqrt(const BigInt& a)
    {
        return rootBig(a._big);
    }

    friend unsigned int log2(const BigInt& a) { return log2(a._big); }
    friend unsigned int log10(const BigInt& a) { return log10(a._big); }

    friend BigInt gcd(const BigInt& a, const BigInt& b)
    {
        // a, b >= 0
        return gcdBig(a._big, b._big);
    }

    friend BigInt pow(const BigInt& a, unsigned int n)
    {
        // n >= 0
        return powerBig(a._big, n);
    }

    friend BigInt powMod(const BigInt& a, const BigInt& n, 
                         const BigInt& m)
    {
        // a^n % m, a < m
        return powerBigMod(a._big, n._big, m._big);
    }

    friend BigInt powMod(const BigInt& a, unsigned int n, 
                         const BigInt& m)
    { return powerBigintMod(a._big, n, m._big); }

    friend Big* subMod(const BigInt& a, const BigInt& b, const BigInt& n)
    {
        Big* v = subBig(a._big, b._big);
        if (::isNeg(v))
        {
            Big* t = addBig(v, n._big);
            destroyBig(v);
            v = t;
        }
        return v;
    }

    friend BigInt sqr(const BigInt& a) { return sqrBig(a._big); }
    
    // invert mod n
    friend Big* invert(const BigInt& a, const BigInt& n, BigInt&);

    bool isOdd() const { return _big && ISODD(_big); }
    bool isEven() const { return !ISODD(_big); }

    BCD toBCD() const
    {
        BCD v;
        BigToMF(_big, &v);
        return v;
    }

    unsigned int asUInt() const
    {
        // will be zero if does not convert
        return _big ? bigAsUint(_big) : 0;
    }

    BCD         asBCD() const
    {
        BCD nf(0);
        if (_big) BigToMF(_big, &nf);
        return nf;
    }
    
    // destructive
    Big* give() const
    {
        Big* p = _big;
        ((BigInt*)this)->_big = 0; // const cast
        return p;
    }
    void negate() { _big = negateBig(_big); }
    
private:

    void _purge() { destroyBig(_big); _big = 0; }

    Big*        _big;
};

#endif
