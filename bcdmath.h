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

#ifndef __bcdmath_h__
#define __bcdmath_h__

#include "bcd.h"

namespace bcdmath
{

BCD pi();
BCD pi2(); // 2pi
BCD ln10constant();
BCD halfConstant();
BCD sin(const BCD&);
BCD cos(const BCD&);
BCD tan(const BCD&);
BCD exp(const BCD&);
BCD log(const BCD&);
BCD atan(const BCD&);
BCD pow(const BCD&, const BCD&);
BCD atan2(const BCD& y, const BCD& x);
BCD asin(const BCD&);
BCD acos(const BCD&);
BCD modtwopi(const BCD&);
BCD log10(const BCD&);
BCD hypot(const BCD& a, const BCD& b);
BCD fmod(const BCD& a, const BCD& b);
BCD ln1p(const BCD&);
BCD expm1(const BCD&);
BCD gammaFactorial(const BCD&);
BCD gammaln(const BCD&);
BCD sinh(const BCD&);
BCD cosh(const BCD&);
BCD tanh(const BCD&);
BCD asinh(const BCD&);
BCD acosh(const BCD&);
BCD atanh(const BCD&);
BCD ceil(const BCD&);
BCD difftimeBCD(int a, int b);
void sincos(const BCD& v, BCD* sinv, BCD* cosv);
void sinhcosh(const BCD& a, BCD* sinha, BCD* cosha);
BCD erf(const BCD&);
BCD normalProbability(const BCD&);
BCD alog(const BCD& y);

BCD sinDeg(const BCD&);
BCD sinGrad(const BCD&);
BCD cosDeg(const BCD&);
BCD cosGrad(const BCD&);
BCD tanDeg(const BCD&);
BCD tanGrad(const BCD&);

BCD asinDeg(const BCD&);
BCD asinGrad(const BCD&);
BCD acosDeg(const BCD&);
BCD acosGrad(const BCD&);
BCD atanDeg(const BCD&);
BCD atanGrad(const BCD&);
BCD atan2Deg(const BCD& y, const BCD& x);
BCD atan2Grad(const BCD& y, const BCD& x);

bool rationalise(const BCD& a, const BCD& b, int4* num, int4* den);
bool rationalise2(BCD x, const BCD& w, BCD& n, BCD& d);

BCD npr(const BCD& n, const BCD& r);
BCD ncr(const BCD& n, const BCD& r);

inline BCD diffTimeBCD(const BCD& t1, const BCD& t2)
{
    // XXX dummy
    return t1;
}

}; // namespace

#endif // bcdmath
