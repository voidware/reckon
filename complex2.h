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

#ifndef __complex2_h__
#define __complex2_h__

#include "bcd2.h"
#include "complex.h"

/* forward decls */
struct Complex2;

BCD2 fabs(const Complex2&);

Complex2 operator*(const Complex2& a, const Complex2& b);
Complex2 operator/(const Complex2& a, const Complex2& b);

struct Complex2
{
    // Constructors
                        Complex2() {}
               explicit Complex2(int v) : r_(v), i_(0) {}
               explicit Complex2(const Complex& z) : r_(z.r_), i_(z.i_) {}
                        Complex2(const BCD2& r) : r_(r), i_(0) {}
                        Complex2(const BCD2& r, const BCD2& i) : r_(r), i_(i) {}

    // Operators
    bool                operator==(const Complex2& c) const
                                { return r_ == c.r_ && i_ == c.i_; }
    bool                operator!=(const Complex2& c) const
                                { return r_ != c.r_ || i_ != c.i_; }

    bool                operator==(const BCD2& v) const
                                { return r_ == v && i_.isZero(); }
    bool                operator!=(const BCD2& v) const
                                { return r_ != v || !i_.isZero(); }

    void                operator+=(const Complex2& c);
    void                operator-=(const Complex2& c);
    void                operator*=(const Complex2& b)
                                { *this = *this * b; }
    void                operator/=(const Complex2& b)
                                { *this = *this/b; }
    
    void                operator+=(const BCD2& a) { r_ += a; }
    void                operator-=(const BCD2& a) { r_ -= a; }
    void                operator*=(const BCD2& a) { r_ *= a; i_ *= a; }
    void                operator/=(const BCD2& a) { r_ /= a; i_ /= a; }

    Complex2             operator-() const { return Complex2(-r_, -i_); }

    // Features
    Complex2             conj() const { return Complex2(r_, -i_); }
    bool                isReal() const { return i_.isZero(); }
    bool                isImaginary() const { return r_.isZero(); }
    bool                isZero() const { return r_.isZero() && i_.isZero(); }

    void                conjugate() { i_.negate(); }
    
    BCD2                 modulus() const { return fabs(*this); }
    //BCD2                 arg() const { return atan2(i_, r_); }

    Complex             asComplex() const
                        { return Complex(r_.asBCD(), i_.asBCD()); }

    BCD2                 r_;
    BCD2                 i_;
};

inline Complex2 operator+(const Complex2& c1, const Complex2& c2)
{
    return Complex2(c1.r_ + c2.r_, c1.i_ + c2.i_);
}

inline Complex2 operator+(const Complex2& c1, const BCD2& c2)
{
    return Complex2(c1.r_ + c2, c1.i_);
}

inline Complex2 operator-(const Complex2& c1, const Complex2& c2)
{
    return Complex2(c1.r_ - c2.r_, c1.i_ - c2.i_);
}

inline Complex2 operator-(const Complex2& c1, const BCD2& c2)
{
    return Complex2(c1.r_ - c2, c1.i_);
}

inline void Complex2::operator+=(const Complex2& c)
{ *this = *this + c; }

inline void Complex2::operator-=(const Complex2& c)
{ *this = *this - c; }

inline Complex2 operator*(const Complex2& a, const BCD2& b)
{
    return Complex2(a.r_ * b, a.i_ * b);
}

inline Complex2 operator*(const BCD2& a, const Complex2& b)
{
    return Complex2(a * b.r_, a * b.i_);
}

inline Complex2 operator/(const Complex2& a, const BCD2& b)
{
    return Complex2(a.r_ / b, a.i_ / b);
}

Complex2 sqrt(const Complex2&);
Complex2 square(const Complex2&);
Complex2 inv(const Complex2&);




#endif // complex2_h__

