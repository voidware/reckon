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

#ifndef __bcdfloat_h__
#define __bcdfloat_h__


#define int4 int
#define uint4 unsigned int
#define SMALL_INTxx

#define NEG 0x8000
#define P 7
#define BASE 10000
#define EXPLIMIT (BASE/4)
#define EXPMASK  0x1fff
#define EXPMASKNEG (NEG|EXPMASK)

/**
 * Here is the BCD float class.
 *
 * the BCD float is made of `P' 4dec digits. a 4dec digit is a chunk
 * of 4 decimal digits stored a a machine `short'. rather than use
 * a nibble per digit and base 10, we are using base 10,000 and thus
 * work with 4 decimal digits in one go.
 *
 * this is much more efficient because, not only is it faster, but
 * we are able to leverage the 16*16->32 and 32/16->16 multiply and
 * divide integer operations found on most modern processors as well
 * as 16 bit add & subtract.
 *
 * the exponent is stored in the last slot (ie P+1) and ranges from 
 * -10000 to +9999. the mantissa is stored unsigned and the sign is the
 * top bit of the exponent.
 *
 * there are a number of important considerations for this representation:
 *
 * the position of the radix must always correspond with the end of a 4dec
 * word. therefore up to one extra 4dec is wasted in the representation.
 * for example, the number 12345.67 will store as [0001][2345].[6700] and
 * utilise 3 4decs rather than two. this is the drawback of using 4decs.
 * bytes would waste up to 1 nibble and nibbles don't waste anything which
 * is why they are normally chosen for short digit (eg 6) representations.
 * for long digit (eg 20) this method is preferable.
 *
 * whence we take P=6 which always has the capacity for 21 decimal digits
 * of mantissa (and up to 24 when it gets lucky) and at P=7 we have
 * 1 spare 4dec word.
 *
 * the trick of storing the mantissa as the P+1'th slot is cunning.
 * most of the arithmetic proceeds by extracting the sign and exponent
 * before working with the mantissa. thus for the purposes of the internal
 * calculations, we can let the mantissa spill out into the last `digit'
 * and effectively have one extra digit of working precision. this is
 * especially useful for making sure we get the correct rounding at the
 * end when we put back the exponent. and also for accommodating any overall
 * numeric carry overflow before the number is shifted down.
 *
 * it has been suggested that, rather than use an extra 16 bit word, 5 words
 * are used for 20 digits without waste. this is possible but it involves
 * multiplying all 5 terms by 10, 100 or 1000 with carry, in order to align
 * the radix for operation. i didn't want this overhead, but it is true that
 * it would result in smaller representation. but don't forget that the
 * overall representation in size will be rounded up to a multiple of 4 bytes
 * anyway.
 */

#include "customchars.h"

#ifdef CUSTOM_EXP_CHAR
#define ISEXP(_c) \
((_c) == 'e' || (_c) == 'E' || (_c) == CUSTOM_EXP_CHAR)
#else
#define ISEXP(_c) \
    ((_c) == 'e' || (_c) == 'E')
#endif


struct BCDFloatData
{
    /* store P 4dec `digits', equivalent to P*4 decimal digits.
     * the last place is the exponent.
     */
    unsigned short      _d[P+1];  
};

#define MAX_P        2*(P+1)
#define POS_INF_EXP  0x2000
#define NEG_INF_EXP  0xA000
#define NAN_EXP      0x4000
#define GET_EXP(_d, _p) (((short)((_d)[_p] << 3)) >> 3)
#define SET_EXP(_d, _p, _v) ((_d)[_p] = (_v) & EXPMASK)
#define CLEAR_SIGN(_d, _p) ((_d)[_p] &= ~NEG)
#define NEGATE_SIGN(_d, _p) ((_d)[_p] ^= NEG)
#define GET_SPECIAL(_d, _p) ((_d)[_p]&0x6000)
#define GET_NAN(_d, _p) ((_d)[_p]&NAN_EXP)
#define GET_INF(_d, _p) ((_d)[_p]&0x2000)
#define GET_NEG_BIT(_d, _p) ((_d)[_p]&NEG)
#define SET_NEG_BIT(_d, _p) (_d[_p] |= NEG)
#define DDIGITS(_pn)    (((_pn)<<2)-3)

#define COPY(__d, __s, __p)                     \
{                                               \
    unsigned short* __dp = __d;                 \
    const unsigned short* __sp = __s;           \
    int __n = __p;                              \
    while (__n) { --__n; *__dp++ = *__sp++; }   \
}

#define CLEAR(__d, __p)                         \
{                                               \
    unsigned short* __dp = __d;                 \
    int __n = __p;                              \
    while (__n) { --__n; *__dp++ = 0; }         \
}


// zero assuming normal (non-special)
#define GET_ZERO_NORM(_d, _p) ((_d)[0] == 0)

// is zero generally
#define IS_ZERO(_d, _p)  \
    (GET_ZERO_NORM(_d, _p) && GET_SPECIAL(_d, _p) == 0)

#define IS_NEG(_d, _p) \
    (GET_NEG_BIT(_d,_p) && (!GET_ZERO_NORM(_d,_p) || GET_INF(_d,_p)))

// negative assuming non-special
#define GET_NEG_NORM(_d, _p) (GET_NEG_BIT(_d, _p) && !GET_ZERO_NORM(_d,_p))

int bcd_round(unsigned short* d, int pn);
int bcd_round25(unsigned short* d, int pn);
void bcd_uadd(const unsigned short* a,
              const unsigned short* b,
              unsigned short* c,
              int pa, int pb);

void bcd_add(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             int pa, int pb);
void bcd_sub(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             int pa, int pb);
void bcd_mul(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             unsigned short* tmp,
             int pa, int pb);
void bcd_div(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             unsigned short* tmp,
             int pa, int pb);
int bcd_cmp(const unsigned short* a, 
            const unsigned short* b,
            int pa, int pb);
void bcd_fma(const unsigned short* a,
             const unsigned short* b,
             const unsigned short* d,
             unsigned short* c,
             int pn);
void bcd_fromUInt(unsigned short* d, int pn, uint4 v);
void bcd_fromUShort(unsigned short* d, int pn, unsigned short v);
void bcd_floor(unsigned short* d, int pn);
void bcd_roundDigits(const unsigned short* d, int pn,
                     int fmt, unsigned int precision, unsigned short* v);
void bcd_asString(const unsigned short* d, int pn,
                  unsigned short* tmp,
                  char* buf, int fmt, int precision);
int bcd_fromString(unsigned short* d, int pn, const char* s);
unsigned int isqrt(unsigned int v);
extern int BCDDecade[4];


struct BCDFloat: public BCDFloatData
{
    enum Format
    {
        format_normal = 0,
        format_scimode = 1,
        format_rounding = 2,

        // precision is number of places after point
        format_precision_decimals = 4,
    };

    BCDFloat() {} // Warning: not initialised.
    BCDFloat(const char* s) 
    {
        bcd_fromString(_d, P, s);
    }

    BCDFloat(int4 v)
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

    BCDFloat(uint4 v)
    {
        _init();
        if (v) _fromUInt(v);
    }

#ifdef SMALL_INT
    BCDFloat(int v)
    {
        _init();
        bool neg = v < 0;
        if (neg) v = -v;
        bcd_fromUShort(_d, P, v); 
        if (neg) negate();
    }
    
    BCDFloat(unsigned int v)
    {
        _init();
        bcd_fromUShort(_d, P, v); 
    }
#endif // SMALL_INT

    BCDFloat(const BCDFloatData& d) { *this = *(BCDFloat*)&d; }

    // Features
    void                asString(char* buf) const
    { _asString(buf, format_normal, DDIGITS(P)); }

    void                asStringFmt(char* buf,
                                    int fmt,
                                    int precision) const
    {
        _asString(buf, fmt, precision); 
    }

    int                 exp() const { return GET_EXP(_d, P); }
    void                exp(int v) { SET_EXP(_d, P, v); }
    bool                neg() const { return IS_NEG(_d, P); }
            
    void                setSign() { SET_NEG_BIT(_d, P); }
    void                clearSign() { CLEAR_SIGN(_d, P); }
    void                negate() { NEGATE_SIGN(_d, P); }
    bool                isSpecial() const { return GET_SPECIAL(_d,P) != 0; } 
    bool                isZero() const { return IS_ZERO(_d, P); }

    bool                isNan() const { return GET_NAN(_d,P) != 0; }
    bool                isInf() const { return GET_INF(_d,P) != 0; }
    bool                isInteger() const ;

    void                ldexp(unsigned int mant, int e)
    {
        // load the exp as a 4-block
        _init();
        _fromUInt(mant);
        exp(e);
    }

    static void         add(const BCDFloat* a, const BCDFloat* b, BCDFloat* c)
    {
        bcd_add(a->_d, b->_d, c->_d, P, P);
    }

    static void         sub(const BCDFloat* a, const BCDFloat* b, BCDFloat* c)
    {
        bcd_sub(a->_d, b->_d, c->_d, P, P);
    }

    static void         mul(const BCDFloat* a, const BCDFloat* b, BCDFloat* c)
    {
        unsigned short tmp[2*P];
        bcd_mul(a->_d, b->_d, c->_d, tmp, P, P);
    }

    static void         div(const BCDFloat* a, const BCDFloat* b, BCDFloat* c)
    {
        unsigned short tmp[2*P+5];
        bcd_div(a->_d, b->_d, c->_d, tmp, P, P);
    }
    
    static bool         sqrt(const BCDFloat* a, BCDFloat* ra) ;

    static bool         lt(const BCDFloat* a, const BCDFloat* b)
    {
        /* true iff a < b */
        return bcd_cmp(a->_d, b->_d, P, P) < 0;
    }

    static bool         le(const BCDFloat* a, const BCDFloat* b)
    {
        /* true iff a <= b */
        return bcd_cmp(a->_d, b->_d, P, P) <= 0;
    }

    static bool         gt(const BCDFloat* a, const BCDFloat* b)
    {
        /* true iff a > b */
        return bcd_cmp(a->_d, b->_d, P, P) > 0;
    }

    static bool         ge(const BCDFloat* a, const BCDFloat* b)
    {
        /* true iff a >= b */
        return bcd_cmp(a->_d, b->_d, P, P) >= 0;
    }

    static bool         equal(const BCDFloat* a, const BCDFloat* b)
    {
        return bcd_cmp(a->_d, b->_d, P, P) == 0;
    }

    static int4         ifloor(const BCDFloat* x) 
    {
        BCDFloat a;
        floor(x, &a);
        return a.asInt();
    }

    static int4         itrunc(const BCDFloat* x) 
    {
        BCDFloat a;
        trunc(x, &a);
        return a.asInt();
    }

    static bool         floor(const BCDFloat* a, BCDFloat* c)
    {
        *c = *a;
        bcd_floor(c->_d, P);
        return true;
    }

    static bool         trunc(const BCDFloat* a, BCDFloat* c) ;

    void                _init() 
    {
        for (int i = 0; i <= P; ++i) _d[i] = 0;
    }

    static void         _uadd(const BCDFloat* a,
                              const BCDFloat* b,
                              BCDFloat* c)
    {
        bcd_uadd(a->_d, b->_d, c->_d, P, P);
    }

    static void         fma(const BCDFloat* a, const BCDFloat* b,
                            const BCDFloat* c, BCDFloat* d)
    {
        bcd_fma(a->_d, b->_d, c->_d, d->_d, P);
    }

    int                 _round25() { return bcd_round25(_d, P); }

    void                _asString(char* buf, int fmt, int precision) const
    {
        unsigned short tmp[P+1];
        bcd_asString(_d, P, tmp, buf, fmt, precision);
    }

    void                _fromUInt(uint4 v) { bcd_fromUInt(_d, P, v); }
    void                _roundDigits(int fmt, unsigned int precision, BCDFloat* v) const
    {
        bcd_roundDigits(_d, P, fmt, precision, v->_d);
    }

    static const BCDFloat& posInf() { return *(BCDFloat*)posInfD_; }
    static const BCDFloat& negInf() { return *(BCDFloat*)negInfD_; }
    static const BCDFloat& nan() { return *(BCDFloat*)nanD_; }

    static void         epsilon(int n, BCDFloat* v)
    {
        // generate 10^-n, 
        int m = BCDDecade[(n-1) & 3];
        v->ldexp(m, -(n>>2));
    }

    static void         pow10(int n, BCDFloat* v)
    {
        // generate 10^n, n >= 0
        int m = BCDDecade[3 - (n & 3)];
        v->ldexp(m, (n>>2)+1);
    }

    static void         mul2(const unsigned short* ad, int ea,
                             const unsigned short* bd, int eb,
                             unsigned short* cd, int& ec) ;

    int4                asInt() const;
    

    static unsigned short posInfD_[P+1];
    static unsigned short negInfD_[P+1];
    static unsigned short nanD_[P+1];
};

struct bcd_op
{
    const unsigned short*       a;
    const unsigned short*       b;
    unsigned short*             c;
    int                         pa;
    int                         pb;
    int                         pc;
    int                         ea;
    int                         eb;
    int                         ec;

    // dpd info
    int                         ar;
    int                         br;

    // workspace. 
    // always assumed to be enough
    unsigned short*             ws;
};

void bcd_op_prep(bcd_op* op,
                 const unsigned short* a, const unsigned short* b,
                 unsigned short* c,
                 int pa, int pb);
int bcd_op_addsub(bcd_op* op, bool sub);
int bcd_op_mul(bcd_op* op);
int bcd_op_div(bcd_op* op);
void bcd_op_fin(bcd_op* op);
int bcd_bup(unsigned short* d, int pn) ;

#ifdef _WIN32
extern void dump(const BCDFloat& v);
#endif

#endif 
