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

#ifndef __bcdfloath_h__
#define __bcdfloath_h__

#include "bcdfloat.h"

#define PH (P/2)

struct BCDFloatDataH
{
    unsigned short      _d[PH+1];  
};

struct BCDFloatH: public BCDFloatDataH
{
    BCDFloatH() {} // Warning: not initialised.
    BCDFloatH(int4 v)
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

    BCDFloatH(uint4 v)
    {
        _init();
        if (v) _fromUInt(v);
    }

    BCDFloatH(const BCDFloatDataH& d) { *this = *(BCDFloatH*)&d; }
    BCDFloatH(const BCDFloatData& d)
    {
        int i;
        for (i = 0; i <= PH; ++i) _d[i] = d._d[i];
        _round25();
        _d[PH] = d._d[P];
    }

    // Features
    int                 exp() const { return ((short)(_d[PH] << 3)) >> 3; }
    void                exp(int v) { _d[PH] = v & EXPMASK; }
    bool                neg() const
    { return (_d[PH]& NEG) && (_d[0] != 0 || isInf()); }
            
    void                setSign() { _d[PH] |= NEG; }
    void                clearSign() { _d[PH] &= ~NEG; }
    void                negate() { _d[PH] ^= NEG; }
    bool                isSpecial() const { return (_d[PH]&0x6000) != 0; } 

    bool                isZero() const
                        { return _d[0] == 0 && !isSpecial(); }

    bool                isNan() const { return (_d[PH]&0x4000) != 0; }
    bool                isInf() const { return (_d[PH]&0x2000) != 0; }
    bool                isInteger() const ;

    void                asBCD(BCDFloat* v) const
    {
        v->_init();
        for (int i = 0; i < PH; ++i) v->_d[i] = _d[i];
        v->_d[P] = _d[PH];
    }

    static void         add(const BCDFloatH* a, const BCDFloatH* b, BCDFloatH* c)
    {
        bcd_add(a->_d, b->_d, c->_d, PH, PH);
    }
    static void         sub(const BCDFloatH* a, const BCDFloatH* b, BCDFloatH* c)
    {
        bcd_sub(a->_d, b->_d, c->_d, PH, PH);
    }

    static void         mul(const BCDFloatH* a, const BCDFloatH* b, BCDFloatH* c)
    {
        unsigned short tmp[PH*2];
        bcd_mul(a->_d, b->_d, c->_d, tmp, PH, PH);
    }

    static void         div(const BCDFloatH* a, const BCDFloatH* b, BCDFloatH* c)
    {
        unsigned short tmp[PH*2+5];
        bcd_div(a->_d, b->_d, c->_d, tmp, PH, PH);
    }

    static bool         lt(const BCDFloatH* a, const BCDFloatH* b)
    {
        /* true iff a < b */
        return bcd_cmp(a->_d, b->_d, PH, PH) < 0;
    }

    static bool         le(const BCDFloatH* a, const BCDFloatH* b)
    {
        /* true iff a <= b */
        return bcd_cmp(a->_d, b->_d, PH, PH) <= 0;
    }

    static bool         gt(const BCDFloatH* a, const BCDFloatH* b)
    {
        /* true iff a > b */
        return bcd_cmp(a->_d, b->_d, PH, PH) > 0;
    }

    static bool         ge(const BCDFloatH* a, const BCDFloatH* b)
    {
        /* true iff a >= b */
        return bcd_cmp(a->_d, b->_d, PH, PH) >= 0;
    }

    static bool         equal(const BCDFloatH* a, const BCDFloatH* b)
    {
        return bcd_cmp(a->_d, b->_d, PH, PH) == 0;
    }

    static int4         itrunc(const BCDFloatH* x) 
    {
        BCDFloatH a;
        trunc(x, &a);
        return a.asInt();
    }

    static bool         trunc(const BCDFloatH* a, BCDFloatH* c)
    {
        /* truncate towards zero.
         * trunc(2.1) = 2.
         * trunc(-2.1) = -2
         */
        *c = *a;
        int e = c->exp();
        int i;
        for (i = PH-1; i >= 0; --i) 
            if (i >= e) c->_d[i] = 0;
        return true;
    }

    void                _init() 
    {
        for (int i = 0; i <= PH; ++i) _d[i] = 0;
    }

    int                 _round25()  { return bcd_round25(_d, PH); }
    void                _fromUInt(uint4 v) { bcd_fromUInt(_d, PH, v); }

    int4                asInt() const
    {
        // if we fit in an int, return it otherwise 0
        int ea = exp();
        int4 v = 0;
        int i = 0;
        while (i < ea && i < PH) 
        {
            if (v > 214748L) return 0; // too large, bail out.
            v*= BASE;
            v += _d[i];
            ++i;
        }
        if (neg()) v = -v;
        return v;
    }

    void                ldexp(unsigned int mant, int e)
    {
        // load the exp as a 4-block
        _init();
        _fromUInt(mant);
        exp(e);
    }

    static void         epsilon(int n, BCDFloatH* v)
    {
        // generate 10^-n, 
        int m = BCDDecade[(n-1) & 3];
        v->ldexp(m, -(n>>2));
    }
};

#endif 
