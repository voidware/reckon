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

#ifndef __complex_h__
#define __complex_h__

#include "bcdmath.h"

/* forward decls */
struct Complex;

#define T BCD
#define SP bcdmath

bool power(const Complex& a, long b, Complex& c);
bool power(const Complex& a, const T& b, Complex& c);
bool power(const Complex& a, const Complex& b, Complex&);
T fabs(const Complex&);

struct Complex
{

    // Constructors
                        Complex() {}
                        explicit Complex(const char* s);
                        explicit Complex(unsigned int v) : r_(v), i_(0) {}
                        explicit Complex(int v) : r_(v), i_(0) {}
                        Complex(const T& r) : r_(r), i_(0) {}
                        Complex(const T& r, const T& i) : r_(r), i_(i) {}

    // Operators
    bool                operator==(const Complex& c) const
                                { return r_ == c.r_ && i_ == c.i_; }
    bool                operator!=(const Complex& c) const
                                { return r_ != c.r_ || i_ != c.i_; }

    bool                operator==(const T& v) const
                                { return r_ == v && i_.isZero(); }
    bool                operator!=(const T& v) const
                                { return r_ != v || !i_.isZero(); }

    void                operator+=(const Complex& c) { *this = *this + c; }
    void                operator-=(const Complex& c) { *this = *this - c; }
    void                operator*=(const Complex& b) { *this = *this * b; }
    void                operator/=(const Complex& b) { *this = *this/b; }
    
    void                operator+=(const T& a) { r_ += a; }
    void                operator-=(const T& a) { r_ -= a; }
    void                operator*=(const T& a) { r_ *= a; i_ *= a; }
    void                operator/=(const T& a) { r_ /= a; i_ /= a; }

    Complex             operator-() const { return Complex(-r_, -i_); }

    friend Complex operator+(const Complex& c1, const Complex& c2)
    { return Complex(c1.r_ + c2.r_, c1.i_ + c2.i_); }

    friend Complex operator+(const Complex& c1, const T& c2)
    {return Complex(c1.r_ + c2, c1.i_); }

    friend Complex operator+(const T& c1, const Complex& c2)
    {return Complex(c2.r_ + c1, c2.i_); }

    friend Complex operator-(const Complex& c1, const Complex& c2)
    { return Complex(c1.r_ - c2.r_, c1.i_ - c2.i_); }

    friend Complex operator-(const Complex& c1, const T& c2)
    { return Complex(c1.r_ - c2, c1.i_); }

    friend Complex operator-(const T& c1, const Complex& c2)
    { return Complex(c1 - c2.r_, -c2.i_); }

    friend Complex operator*(const Complex& a, const Complex& b);

    friend Complex operator*(const Complex& a, const T& b)
    { return Complex(a.r_ * b, a.i_ * b); }

    friend Complex operator*(const T& a, const Complex& b)
    { return Complex(a * b.r_, a * b.i_); }

    friend Complex operator/(const Complex& a, const Complex& b);

    friend Complex operator/(const Complex& a, const T& b)
    { return Complex(a.r_ / b, a.i_ / b); }

    friend bool operator<(const Complex& a, const Complex& b)
    { return a.r_ < b.r_; }

    friend bool operator>(const Complex& a, const Complex& b)
    { return a.r_ > b.r_; }

    friend bool operator<=(const Complex& a, const Complex& b)
    { return a.r_ <= b.r_; }

    friend bool operator>=(const Complex& a, const Complex& b)
    { return a.r_ >= b.r_; }

    friend Complex fmod(const Complex& z, const Complex& m)
    { return bcdmath::fmod(z.r_, m.r_); }

    friend Complex pow(const Complex& z, const Complex& p)
    {
        Complex c;
        power(z, p, c);
        return c;
    }

    // Features
    Complex             conj() const { return Complex(r_, -i_); }
    bool                isReal() const { return i_.isZero(); }
    bool                isImaginary() const { return r_.isZero(); }
    bool                isZero() const { return r_.isZero() && i_.isZero(); }
    bool                isInf() const { return r_.isInf() || i_.isInf(); }
    Complex             muli() const { return Complex(-i_, r_); } // mul by i

    void                conjugate() { i_.negate(); }
    void                negate() { r_.negate(); i_.negate(); }
    
    T                   modulus() const { return fabs(*this); }
    T                   modsqr() const { return r_*r_ + i_*i_; }
    T                   arg() const { return SP::atan2(i_, r_); }

    const char*         asString() const
    {
        static char buf[100];
        asString(buf);
        return buf;
    }
    
    char*               asString(char* buf) const;

    T                  r_;
    T                  i_;
};

Complex sqrt(const Complex&);
Complex square(const Complex&);
Complex inv(const Complex&);
bool log(const Complex& z, Complex& c);
bool log10(const Complex& z, Complex& c);
bool exp(const Complex& z, Complex& c);
bool alog(const Complex& z, Complex& c);

bool sincos(const Complex& a, Complex* sa, Complex* ca);

bool sin(const Complex& a, Complex& c);
bool cos(const Complex& a, Complex& c);
bool tan(const Complex& a, Complex& c);
bool asin(const Complex& a, Complex& c);
bool acos(const Complex& a, Complex& c);
bool atan(const Complex& a, Complex& c);
bool sinh(const Complex& a, Complex& c);
bool cosh(const Complex& a, Complex& c);
bool tanh(const Complex& a, Complex& c);
bool asinh(const Complex& a, Complex& c);
bool acosh(const Complex& a, Complex& c);
bool atanh(const Complex& a, Complex& c);
bool gammaFactorial(const Complex& a, Complex& c);

inline Complex log(const Complex& z) { Complex c; log(z, c); return c; }
inline Complex log10(const Complex& z) { Complex c; log10(z, c); return c; }
inline Complex exp(const Complex& z) { Complex c; exp(z, c); return c; }
inline Complex sin(const Complex& z) { Complex c; sin(z, c); return c; }
inline Complex cos(const Complex& z) { Complex c; cos(z, c); return c; }
inline Complex tan(const Complex& z) { Complex c; tan(z, c); return c; }
inline Complex asin(const Complex& z) { Complex c; asin(z, c); return c; }
inline Complex acos(const Complex& z) { Complex c; acos(z, c); return c; }
inline Complex atan(const Complex& z) { Complex c; atan(z, c); return c; }

inline Complex sinh(const Complex& z) { Complex c; sinh(z, c); return c; }
inline Complex cosh(const Complex& z) { Complex c; cosh(z, c); return c; }
inline Complex tanh(const Complex& z) { Complex c; tanh(z, c); return c; }
inline Complex asinh(const Complex& z) { Complex c; asinh(z, c); return c; }
inline Complex acosh(const Complex& z) { Complex c; acosh(z, c); return c; }
inline Complex atanh(const Complex& z) { Complex c; atanh(z, c); return c; }
inline Complex gammaFactorial(const Complex& z) { Complex c; gammaFactorial(z, c); return c; }

inline T real(const Complex& z)  { return z.r_; }
inline T imag(const Complex& z)  { return z.i_; }

inline Complex atan2(const Complex& y, const Complex& x)
{
    if (y.isReal() && x.isReal())
        return atan2(y.r_, x.r_);
    return atan(y/x);
}

inline Complex expm1(const Complex& c)
{
    if (c.isReal())
        return expm1(c.r_);
    return exp(c) - T(1);
}

inline Complex fma(const Complex& a, Complex& b, Complex& c)
{
    // a*b + c
    if (a.isReal() && b.isReal() && c.isReal())
        return fma(a.r_,b.r_,c.r_);
    return a*b + c;
}

inline Complex frexp(const Complex& c, int* e)
{
    return frexp(c.r_, e);
}

inline Complex ldexp(const Complex& a, int e)
{
    Complex c;
    c.r_ = ldexp(a.r_, e);
    c.i_ = 0;
    return c;
}

// methods that apply to reals
inline Complex ceil(const Complex& c) { return bcdmath::ceil(c.r_); }
inline Complex floor(const Complex& c) { return floor(c.r_); }
inline Complex trunc(const Complex& c) { return trunc(c.r_); }
inline Complex gammaln(const Complex& c) { return bcdmath::gammaln(c.r_); }
inline Complex normal(const Complex& c) { return normal(c.r_); }

#undef T
#undef SP

#endif // complex_h__

