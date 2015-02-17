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

#ifndef __int64_h__
#define __int64_h__

extern unsigned int muldvd(unsigned int a, 
                           unsigned int b,
                           unsigned int c,
                           unsigned int* rp);

extern unsigned int muldvm(unsigned int a,
                           unsigned int c,
                           unsigned int m,
                           unsigned int* rp);

class Int64
{
public:
    
    // Constructors
    Int64() {}
    Int64(unsigned int v) { _lo = v; _hi = 0; }
    Int64(int v)
    {
        _lo = v;
        if (v < 0) 
            _hi = (unsigned int)-1;
        else _hi = 0;
    }

    Int64(const Int64& ll) { *this = ll; }
    Int64(unsigned int hi,
          unsigned int lo) : _lo(lo), _hi(hi) {}

    // Accessors
    unsigned int        lo() const { return _lo; }
    unsigned int        hi() const { return _hi; }

    // Modifiers
    void                lo(unsigned int v) { _lo = v; }
    void                hi(unsigned int v) { _hi = v; }

    // Features
    Int64&           operator+=(const Int64& ll)
    {
        unsigned int t = _lo + ll._lo;
        _hi += ll._hi;
        if (t < _lo) ++_hi;
        _lo = t;
        return *this;
    }

    Int64&           operator-=(const Int64& ll)
    {
        unsigned int t = _lo - ll._lo;
        _hi -= ll._hi;
        if (t > _lo) --_hi;
        _lo = t;
        return *this;
    }

    Int64&           operator*=(unsigned int v)
    {
        unsigned int c = muldvd(_lo, v, 0, &_lo);
        muldvd(_hi, v, c, &_hi);
        return *this;
    }

    Int64&           operator*=(int v)
    {
        if (v >= 0) return operator*=((unsigned int)v);
        operator*=((unsigned int)-v);
        negate();
        return *this;
    }

    Int64&           operator*=(const Int64& v)
                        { return *this = *this * v; }

    friend Int64           operator*(const Int64& a, const Int64& b)
    {
        Int64 v;
        unsigned int c = muldvd(a._lo, b._lo, 0, &v._lo);
        muldvd(a._hi, b._lo, c, &v._hi);
        muldvd(a._lo, b._hi, v._hi, &v._hi);
        return v;
    }

    Int64&           operator/=(unsigned int v)
    {
        if (v > 1) 
        {
            unsigned int q, r;
            q = _hi / v;
            r = _hi - q * v;
            _hi = q;
            _lo = muldvm(r, _lo, v, &r);
        }
        return *this;
    }

    Int64&           operator/=(int v)
    {
        if (v > 1) return operator/=((unsigned int)v);
        operator/=((unsigned int)-v);
        negate();
        return *this;
    }

    Int64&           operator+=(unsigned int v)
    {
        unsigned int t = _lo + v;
        if (t < _lo) ++_hi;
        _lo = t;
        return *this;
    }

    Int64&           operator-=(unsigned int v)
    {
        unsigned int t = _lo - v;
        if (t > _lo) --_hi;
        _lo = t;
        return *this;
    }

    Int64&           operator+=(int v)
    {
        if (v >= 0) return operator+=((unsigned int)v);
        else return operator-=((unsigned int)-v);
    }

    Int64&           operator-=(int v)
    {
        if (v >= 0) return operator-=((unsigned int)v);
        else return operator+=((unsigned int)-v);
    }

    Int64&           operator^=(const Int64& v)
    {
        _hi ^= v._hi;
        _lo ^= v._lo;
        return *this;
    }

    Int64            operator>>(int n) const
    {
        Int64 v;
        if (n >= 32)
        {
            v._hi = 0;
            v._lo = _hi >> (n-32);
        }
        else
        {
            v._lo = (_lo >> n) | ((_hi & (1<<n)-1) << (32-n));
            v._hi = _hi >> n;
        }
        return v;
    }

    Int64            operator<<(int n) const
    {
        Int64 v;
        if (n >= 32)
        {
            v._lo = 0;
            v._hi = _lo << (n-32);
        }
        else
        {
            v._hi = (_hi << n) | (_lo >> (32-n));
            v._lo = _lo << n;
        }
        return v;
    }

    Int64            operator*(unsigned int v) const
                            { Int64 t(*this); return t *= v; }       
    Int64            operator*(int v) const
                            { Int64 t(*this); return t *= v; }       
    Int64            operator/(unsigned int v) const
                            { Int64 t(*this); return t /= v; }       
    Int64            operator/(int v) const
                            { Int64 t(*this); return t /= v; }       
    Int64            operator+(unsigned int v) const
                            { Int64 t(*this); return t += v; }
    Int64            operator+(int v) const
                            { Int64 t(*this); return t += v; }
    Int64            operator-(unsigned int v) const
                            { Int64 t(*this); return t -= v; }
    Int64            operator-(int v) const
                            { Int64 t(*this); return t -= v; }

    int                 operator>(unsigned int v) const
    {
        if ((int)_hi < 0) return 0;
        return _hi || _lo > v;
    }

    int                 operator>(int v) const
    {
        if (v >= 0) return operator>((unsigned int)v);
        if ((int)_hi >= 0) return 1;
        if (_hi != (unsigned int)-1) return 0;
        return _lo > (unsigned int)v;
    }

    int                 operator>=(unsigned int v) const
    {
        if ((int)_hi < 0) return 0;
        return _hi || _lo >= v;
    }

    int                 operator>=(int v) const
    {
        if (v >= 0) return operator>=((unsigned int)v);
        if ((int)_hi >= 0) return 1;
        if (_hi != (unsigned int)-1) return 0;
        return _lo >= (unsigned int)v;
    }

    int                 operator<(unsigned int v) const
    {
        if ((int)_hi < 0) return 1;
        return !_hi && _lo < v;
    }

    int                 operator<(int v) const
    {
        if (v >= 0) return operator<((unsigned int)v);
        if ((int)_hi >= 0) return 0;
        if (_hi != (unsigned int)-1) return 1;
        return _lo < (unsigned int)v;
    }

    
    int                 operator<=(unsigned int v) const
    {
        if ((int)_hi < 0) return 1;
        return !_hi && _lo <= v;
    }

    int                 operator<=(int v) const
    {
        if (v >= 0) return operator<=((unsigned int)v);
        if ((int)_hi >= 0) return 0;
        if (_hi != (unsigned int)-1) return 1;
        return _lo <= (unsigned int)v;
    }

    Int64&           operator=(const Int64& ll)
    {
        _hi = ll._hi;
        _lo = ll._lo;
        return *this;
    }

    Int64&           operator=(unsigned int v) 
    { _hi = 0; _lo = v; return *this; }

    int                 operator==(const Int64& ll) const
    { return _hi == ll._hi && _lo == ll._lo; }

    int                 operator==(unsigned int v) const
    { return _lo == v && _hi == 0; }

    int                 operator==(int v) const
    {
        if (_lo == (unsigned int)v) 
        {
            if (v >= 0) return _hi == 0;
            else return _hi == (unsigned int)-1;
        }
        return 0;
    }

    int                 operator!=(unsigned int v) const
    {   return !(*this == v); }

    int                 operator!=(int v) const
    { return !(*this == v); }

    int                 operator!=(const Int64& ll) const
                        { return _hi != ll._hi || _lo != ll._lo; }

    operator unsigned int() const
    {
        return _lo;
    }

    void                negate()
    {
        _lo = ~_lo;
        _hi = ~_hi;
        if (!++_lo) ++_hi;
    }

    int                 sgn() const
    {
        if ((int)_hi < 0) return -1;
        return _hi || _lo;
    }

    // Conversions
    //                        operator int() const { return _lo; }
    

private:

    unsigned int        _lo;
    unsigned int        _hi;

};

inline Int64 operator+(const Int64& a, const Int64& b)
{
    Int64 c(a);
    return c += b;
}

inline Int64 operator-(const Int64& a, const Int64& b)
{
    Int64 c(a);
    return c -= b;
}



inline int operator>(const Int64& a, const Int64& b)
{
    return (a - b).sgn() == 1;
}

inline int operator<(const Int64& a, const Int64& b)
{
    return (b - a).sgn() == 1;
}

inline int operator>=(const Int64& a, const Int64& b)
{
    return (a - b).sgn() >= 0;
}

inline int operator<=(const Int64& a, const Int64& b)
{
    return (b - a).sgn() >= 0;
}

#endif /* __longlong_h__ */

