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

#ifndef __bcdfloat2_h__
#define __bcdfloat2_h__

#include "bcdfloat.h"

#define P2 (2*P+1)

struct BCDFloatData2
{
    /* store P 4dec `digits', equivalent to P*4 decimal digits.
     * the last place is the exponent.
     */
    unsigned short      _d[P2+1];  
};

struct BCDFloat2: public BCDFloatData2
{
    BCDFloat2() {} // Warning: not initialised.
    BCDFloat2(int4 v)
    {
        _init();
        if (v)
        {
            bool neg = v < 0;
            if (neg)
                v = -v;
            _fromUInt((uint4)v);
            if (neg) negate();
        }
    }

    BCDFloat2(uint4 v)
    {
        _init();
        if (v) _fromUInt(v);
    }

    BCDFloat2(const BCDFloatData2& d) { *this = *(BCDFloat2*)&d; }

    BCDFloat2(const BCDFloatData& d)
    {
        int i;
        for (i = 0; i < P; ++i) _d[i] = d._d[i];
        while (i < P2) _d[i++] = 0;
        _d[P2] = d._d[P];
    }

    // Features
    int                 exp() const { return ((short)(_d[P2] << 3)) >> 3; }
    void                exp(int v) { _d[P2] = v & EXPMASK; }
    bool                neg() const
    { return (_d[P2]& NEG) && (_d[0] != 0 || isInf()); }
            
    void                setSign() { _d[P2] |= NEG; }
    void                clearSign() { _d[P2] &= ~NEG; }
    void                negate() { _d[P2] ^= NEG; }
    bool                isSpecial() const { return (_d[P2]&0x6000) != 0; } 

    bool                isZero() const
                        { return _d[0] == 0 && !isSpecial(); }

    bool                isNan() const { return (_d[P2]&0x4000) != 0; }
    bool                isInf() const { return (_d[P2]&0x2000) != 0; }
    bool                isInteger() const ;

    void                asBCD(BCDFloat* v) const
    {
        for (int i = 0; i <= P; ++i) v->_d[i] = _d[i];
        int e = _d[P2];
        if (v->_round25()) ++e; // XX overflow?
        v->_d[P] = e;
    }

    void                ldexp(unsigned int mant, int e)
    {
        // load the exp as a 4-block
        _init();
        _fromUInt(mant);
        exp(e);
    }

    static void         add(const BCDFloat2* a, const BCDFloat2* b, BCDFloat2* c)
    {
        bcd_add(a->_d, b->_d, c->_d, P2, P2);
    }
    static void         sub(const BCDFloat2* a, const BCDFloat2* b, BCDFloat2* c)
    {
        bcd_sub(a->_d, b->_d, c->_d, P2, P2);
    }

    static void         mul(const BCDFloat2* a, const BCDFloat2* b, BCDFloat2* c)
    {
        unsigned short tmp[P2*2];
        bcd_mul(a->_d, b->_d, c->_d, tmp, P2, P2);
    }

    static void         div(const BCDFloat2* a, const BCDFloat2* b, BCDFloat2* c)
    {
        unsigned short tmp[P2*2+5];
        bcd_div(a->_d, b->_d, c->_d, tmp, P2, P2);
    }

    void                makeInf()
    {
        _init();
        _d[P2] = POS_INF_EXP;
    }

    void                makeNAN()
    {
        _init();
        _d[P2] = NAN_EXP;
    }

    static bool         sqrt(const BCDFloat2* a, BCDFloat2* ra) ;
    static bool         lt(const BCDFloat2* a, const BCDFloat2* b)
    {
        /* true iff a < b */
        return bcd_cmp(a->_d, b->_d, P2, P2) < 0;
    }

    static bool         le(const BCDFloat2* a, const BCDFloat2* b)
    {
        /* true iff a <= b */
        return bcd_cmp(a->_d, b->_d, P2, P2) <= 0;
    }

    static bool         gt(const BCDFloat2* a, const BCDFloat2* b)
    {
        /* true iff a > b */
        return bcd_cmp(a->_d, b->_d, P2, P2) > 0;
    }

    static bool         ge(const BCDFloat2* a, const BCDFloat2* b)
    {
        /* true iff a >= b */
        return bcd_cmp(a->_d, b->_d, P2, P2) >= 0;
    }

    static bool         equal(const BCDFloat2* a, const BCDFloat2* b)
    {
        return bcd_cmp(a->_d, b->_d, P2, P2) == 0;
    }

    static int4         ifloor(const BCDFloat2* x) 
    {
        BCDFloat2 a;
        floor(x, &a);
        return a.asInt();
    }

    static int4         itrunc(const BCDFloat2* x) 
    {
        BCDFloat2 a;
        trunc(x, &a);
        return a.asInt();
    }

    static bool         floor(const BCDFloat2* a, BCDFloat2* c) 
    {
        *c = *a;
        bcd_floor(c->_d, P2);
        return true;
    }

    static bool         trunc(const BCDFloat2* a, BCDFloat2* c) ;

    void                _init() ;
    int                 _round25()
    {
        return bcd_round25(_d, P2);
    }
    void                _fromUInt(uint4 v) { bcd_fromUInt(_d, P2, v); }


    static void         epsilon(int n, BCDFloat2* v)
    {
        // generate 10^-n, 
        int m = BCDDecade[(n-1) & 3];
        v->ldexp(m, -(n>>2));
    }

    int4                asInt() const;
};

#endif 
