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

#define GAMMA
#include <stdio.h>
#include <string.h> // strcat etc
#include "types.h"
#include "complex2.h"
#include "cutils.h"

using namespace bcdmath;

Complex operator*(const Complex& a, const Complex& b)
{
    Complex c;
    c.r_ = cross(a.r_, a.i_, b.i_, b.r_);
    c.i_ = cross(a.r_, a.i_, -b.r_, b.i_);
    return c;
}

Complex operator/(const Complex& a, const Complex& b)
{
    Complex c;
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
        BCD t, s;
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

Complex square(const Complex& a)
{
    Complex c;
    c.r_ = a.r_*a.r_ - a.i_*a.i_;
    c.i_ = 2*a.i_*a.r_;
    return c;
}

Complex inv(const Complex& a)
{
    // 1 / (c + i d) = c / (c*c + d*d) - i d / (c*c + d*d)
    Complex c(0);
    if (a.isReal())
        c.r_ = BCD(1U)/a.r_;
    else if (a.isImaginary())
        c.i_ = BCD(-1)/a.i_;
    else if (!a.isZero())
    {
        if (!a.isInf())
        {
            BCD d = a.r_*a.r_ + a.i_*a.i_;
            c.r_ = a.r_/d;
            c.i_ = -a.i_/d;
        }
    }
    else 
        c.r_ = c.i_ = BCD::inf();
    return c;
}

    
Complex sqrt(const Complex& a)
{
    Complex b;
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
        BCD fc = fabs(a.r_);
        BCD fd = fabs(a.i_);
        BCD w, t;
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

BCD fabs(const Complex& a)
{
    return BCD(hypot(a.r_, a.i_));
}

bool log(const Complex& z, Complex& c)
{
    /* use log(z) = log(r) + i(arg) */
    c.r_ = log(z.modulus());
    c.i_ = z.arg();
    return true;
}

bool log10(const Complex& z, Complex& c)
{
    /* use log10(z) = log10(r) + i(arg)/ln10 */
    c.r_ = log10(z.modulus());
    c.i_ = z.arg()/ln10constant();
    return true;
}

bool exp(const Complex& z, Complex& c)
{
    /* use exp(z) = exp(x)((cos(y) + i sin(y)) */

    BCD sinz, cosz, t;
    sincos(z.i_, &sinz, &cosz);
    t = exp(z.r_);
    c.r_ = t*cosz;
    c.i_ = t*sinz;
    return true;
}

bool alog(const Complex& z, Complex& c)
{
    /* alog(z) = 10^z */

    Complex a(BCD(10));
    return power(a, z, c);
}

bool power(const Complex& a, long bi, Complex& c)
{
    if (bi) 
    {
        long m;
        Complex r = a;
        m = (bi < 0) ? -bi : bi;
        if (m > 1) {
            c = Complex(BCD(1));
            /* Use binary exponentiation */
            for (;;) 
            {
                if (m & 1) c *= r;
                m >>= 1;
                if (!m) break;
                r *= r;
            }
        } else { c = r; }

        /* Compute the reciprocal if n is negative. */
        if (bi < 0) c = Complex(BCD(1))/c;
    }
    return true;
}

bool power(const Complex& a, const BCD& b, Complex& c)
{
    // c = a^b, b real.

    BCD r = a.modulus();
    BCD t;
    BCD sina, cosa;

    t = pow(r, b);
    sincos(a.arg()*b, &sina, &cosa);
    
    c.r_ = t*cosa;
    c.i_ = t*sina;
    return true;
}

bool power(const Complex& a, const Complex& b, Complex& c)
{
    Complex t;
    bool v = true;

    if (a.isZero()) 
    {
        c = a;
        return v;
    }
    else if (b.isZero()) 
    {
        c = BCD(1);
        return v;
    }
    else if (b.isReal()) 
    {

        if (b.r_.isInteger())
        {
            long n = ifloor(b.r_);
            /* if integral use power ladder */
            return power(a, n, c);
        }
        
        /* otherwise case for real b bv*/
        return power(a, b.r_, c);
    }

    /* this might fail if we contain zero. if so, then power
     * fails as we dont know enough precision.
     */
    
    /* use complex logs and exp */
    v = log(a, t);
    if (v) 
        v = exp(t*b, c);
    return v;
}

bool sin(const Complex& a, Complex& c)
{
    /* sin z = (exp(iz) - exp(-iz))/2i
     * but use expanded version so as to avoid errors when |z| is 
     * small.
     *
     * use sin(x+iy) = sin(x)*cosh(y) + icos(x)*sinh(y)
     *
     */
    
    BCD sina, cosa;
    BCD sinha, cosha;

    sincos(a.r_, &sina, &cosa);
    sinhcosh(a.i_, &sinha, &cosha);
    c.r_ = sina * cosha;
    c.i_ = cosa * sinha;
    return true;
}

bool cos(const Complex& a, Complex& c)
{
    /* use cos(x+iy) = cos(x)*cosh(y) - isin(x)*sinh(y) 
     */
    
    BCD sina, cosa;
    BCD sinha, cosha;

    sincos(a.r_, &sina, &cosa);
    sinhcosh(a.i_, &sinha, &cosha);
    c.r_ = cosa * cosha;
    c.i_ = -sina * sinha;
    return true;
}

bool sincos(const Complex& a, Complex* sa, Complex* ca)
{
    BCD sina, cosa;
    BCD sinha, cosha;

    sincos(a.r_, &sina, &cosa);

    if (a.isReal())
    {
        if (sa)
        {
            sa->r_ = sina;
            sa->i_ = 0;
        }
        if (ca)
        {
            ca->r_ = cosa;
            ca->i_ = 0;
        }
    }
    else
    {
        sinhcosh(a.i_, &sinha, &cosha);
        if (sa)
        {
            sa->r_ = sina * cosha;
            sa->i_ = cosa * sinha;
        }

        if (ca)
        {
            ca->r_ = cosa * cosha;
            ca->i_ = -sina * sinha;
        }
    }
    return true;

}

bool tan(const Complex& a, Complex& c)
{
    /* use tan(x+iy) = (sin(2x) + isinh(2y))/(cos(2x) + cosh(2y)) 
     *
     */
    
    BCD sa, ca;
    BCD shb, chb;
    BCD d;
    sincos(a.r_*2, &sa, &ca);
    sinhcosh(a.i_*2, &shb, &chb);
    d = ca + chb;
    c.r_ = sa/d;
    c.i_ = shb/d;
    return true;
}

bool asin(const Complex& a, Complex& c)
{
    c = pi()/2 - acos(a);
    return true;
}

bool acos(const Complex& a, Complex& c)
{
    if (a.isReal() && a.r_ <= 1)
    {
        c = acos(a.r_);
        return true;
    }

    // kahan 1987
    /*
     * acos(z) = -2i*log(sqrt((1+z)/2) + i*sqrt((1-z)/2))
     */
    c = sqrt((1+a)/2);
    c += sqrt((1-a)/2).muli();
    c = -2*log(c).muli();
    return true;
}

bool atan(const Complex& a, Complex& c)
{
    BCD t = a.r_*a.r_;
    BCD u = a.i_*a.i_;
    BCD w1 = (atan2(2*a.r_, 1-t-u))/2;
    BCD w2, v;
        
    u = a.i_+1; 
    v = a.i_-1; 
    w2 = log((t + u*u)/(t + v*v))/4;
        
    c.r_ = w1;
    c.i_ = w2;
    return true;
}

bool sinh(const Complex& a, Complex& c)
{
    Complex t(a.r_, -a.i_);
    cos(t, c);
    c.conjugate();
    return true;
}

bool cosh(const Complex& a, Complex& c)
{
    Complex t(-a.i_, a.r_);
    return cos(t, c);
}

bool tanh(const Complex& a, Complex& c)
{
    Complex t(-a.i_, a.r_);
    tan(t, c);
    c.conjugate();
    return true;
}

bool asinh(const Complex& a, Complex& c)
{
    Complex t(-a.i_, a.r_);
    asin(t, c);
    c.conjugate();
    return true;
}

bool acosh(const Complex& a, Complex& c)
{
    Complex t;
    acos(a, t);
    c.r_ = -t.i_;
    c.i_ = t.r_;
    return true;
}

bool atanh(const Complex& a, Complex& c)
{
    // a != +/-1
    c = log((1+a)/(1-a))/2;
    return true;
}

#ifdef GAMMA

#define K 12
#define GG 12

#if 0
/* This is not the version to use. it is the version that calculates
 * the coefficients as it goes. It is used to recalculate the constants
 * if we change K or GG.
 */
Complex _gammaFactorialSlow(const Complex& z)
{
    /* calculate gamma(z+1) = z!
     * using lanczos expansion.
     */

    Complex t1;
    Complex t2;
    Complex s;
    Complex t;
    Complex fgc[K+1];
    Complex pks;
    Complex aa;
    int a[K];
    int b[K+1];
    int tb;
    int i, j;
    BCD half = BCD(1)/BCD(2);

    a[0] = 1;
    b[0] = -1;
    b[1] = 2;

    fgc[0] = exp(GG+half)/sqrt(GG+half);
    s = fgc[0]*half;
    //print(s); // emit coefficient

    aa = half;
    t = Complex(1);
    i = 1;
    for (;;) 
    {
        t *= (z+(1-i))/(z + i);

        t1 = i + half;
        t2 = t1 + GG;

        fgc[i] = aa*exp(t2 - log(t2)*t1);
        aa *= t1;

        pks = Complex(0);
        for (j = 0; j <= i; ++j) 
            pks += b[j]*fgc[j];

        //print(pks); // emit coefficient
        s += pks*t;

        if (i == K) break;

        a[i] = 0;
        b[i+1] = 0;

        tb = b[0];
        for (j = 0; j <= i; ++j) 
        {
            a[j] = -a[j] + 2*tb;
            tb = b[j+1];
            b[j+1] = -b[j+1] + 2*a[j];
        }

        b[0] = -b[0];
        ++i;
    }

    t1 = z + half;
    t2 = t1 + GG;
    return 2*exp(t1*log(t2)-t2)*s;
}
#endif


namespace bcdmath
{
    extern const BCDFloat2* lanczConstants();
};

static void _gammaFactorialAux(const Complex& z,
                               Complex& t1, Complex& t2, Complex& s)
{
    /* calculate gamma(z+1) = z!
     * using lanczos expansion.
     */

    Complex2 t, s2;
    int i;

    const BCDFloat2* lancz = lanczConstants();
    s2 = Complex2(lancz[0]);

    t = Complex2(1);
    for (i = 1; i <= K; ++i)
    {
        t *= Complex2((z+(1-i))/(z + i));
        s2 += t*lancz[i];
    }

    s = s2.asComplex();
    s *= 2;
    
    t1 = z + halfConstant();
    t2 = t1 + GG;
}

static Complex _gammaFactorial(const Complex& z)
{
    Complex t1, t2, s;
    _gammaFactorialAux(z, t1, t2, s);
    return exp(t1*log(t2)-t2)*s;
}

bool gammaFactorial(const Complex& a, Complex& c)
{
    Complex v = a;
    if (v.isZero()) c = Complex(1);
    else 
    {
        if (v.r_.isNeg()) 
        {
            /* use reflection formula */
            Complex z1(-v.r_ , v.i_);
            Complex z2 = z1*pi();
            c = z2 / sin(z2)/ _gammaFactorial(z1);
        }
        else 
            c =_gammaFactorial(v);
    }
    return true;
}
#endif // GAMMA

Complex::Complex(const char* s)
{

    BCD r(s);
    
    // skip first number
    if (*s == '-') ++s;
    while (*s == '.' || u_isdigit(*s)) ++s;
    
    if (ISEXP(*s))
    {
        ++s;
        if (*s == '-' || *s == '+') ++s;
        while (u_isdigit(*s)) ++s;
    }

    if (*s == 'i')
    {
        // all imag
        r_ = 0;
        i_ = r;
    }
    else
    {
        r_ = r;
        i_ = BCD(s);
    }
}

char* Complex::asString(char* buf) const
{
    bool r = isReal();
    bool i = isImaginary();


    *buf = 0;
    if (r || !i) 
    {
        r_._v.asString(buf);
    }
    if (!r) 
    {
        if (!i)  // not pure imaginary
        {
            if (!i_.isNeg()) strcat(buf, "+");
        }

        if (i_ != BCD(1)) 
            i_._v.asString(buf + strlen(buf));

        strcat(buf, "i");
    }
    return buf;
}
