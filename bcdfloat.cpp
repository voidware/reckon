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

extern "C"
{
#include <stdio.h>
#include <string.h>
#include "cutils.h"
};

#include "bcdfloat.h"

#ifdef _WIN32
#include <stdio.h>
void dump(const BCDFloat& v)
{
    int i;
    printf("{ ");
    for (i = 0; i < P; ++i) 
        printf("%d, ", v._d[i]);

    printf("%d },\n", v._d[P]);
}

void print(const BCDFloat& v)
{
    char buf[64];
    //v.asString(buf);
    v.asStringFmt(buf, BCDFloat::format_normal, 4);
    printf("%s", buf);
}

#endif // win32


int BCDDecade[4] = { 1000, 100, 10, 1 };

unsigned short BCDFloat::posInfD_[P+1] = { 0, 0, 0, 0, 0, 0, 0, POS_INF_EXP };
unsigned short BCDFloat::negInfD_[P+1] = { 0, 0, 0, 0, 0, 0, 0, 0xA000 };
unsigned short BCDFloat::nanD_[P+1] =    { 0, 0, 0, 0, 0, 0, 0, 0x4000 };

int bcd_bup(unsigned short* d, int pn) 
{
    //  bump to successor
    int i = pn-1;
    int v = d[i] + 1;
    while (v >= BASE)
    {
        d[i] = (unsigned short)(v - BASE);
        if (!i)
        {
            // shift
            for (i = pn; i > 0; --i) d[i] = d[i-1];
            d[0] = 1;
            return 1;
        }
        v = d[--i]+1;
    }
    d[i] = v;
    return 0;
}

int bcd_round(unsigned short* d, int pn)
{
    // round d_[P] into the mantissa
    int res = 0;
    if (d[pn] >= 5000)
        res = bcd_bup(d, pn);

    return res;
}

int bcd_round25(unsigned short* d, int pn) 
{
    // round _d[P] into the mantissa and mask off digits after 25.
    int i = pn-1;
    int v;
    if (d[0] < 10)
        v = d[i] + (d[pn] >= 5000);
    else if (d[0] < 100)
        v = ((((int4) d[i]+5)*3277)>>15)*10;
    else if (d[0] < 1000)
        v = ((((int4) d[i]+50)*5243)>>19)*100;
    else
        v = ((((int4) d[i]+500)*8389)>>23)*1000;

    while (v >= BASE)
    {
        d[i] = (unsigned short)(v - BASE);
        if (!i)
        {
            // shift
            for (i = pn; i > 0; --i) d[i] = d[i-1];
            d[0] = 1;
            return 1;
        }
        v = d[--i]+1;
    }
    d[i] = v;
    return 0;
}


void bcd_op_prep(bcd_op* op,
                 const unsigned short* a, const unsigned short* b,
                 unsigned short* c,
                 int pa, int pb)
{
    op->a = a;
    op->b = b;
    op->c = c;
    op->pa = pa;
    op->pb = pb;
    op->pc = pa;

    op->ea = GET_EXP(a, pa);
    op->eb = GET_EXP(b, pb);
    op->ec = op->ea;
}

#define SWAP(_a, _b) { n = op->_a; op->_a = op->_b; op->_b = n; }
static void bcd_op_swap(bcd_op* op)
{
    const unsigned short* t = op->a;
    op->a = op->b;
    op->b = t;

    int n;
    SWAP(pa, pb);
    SWAP(ea, eb);

    op->pc = op->pa;
    op->ec = op->ea;

    // swap range (not needed for BCD)
    SWAP(ar, br);
}
#undef SWAP

void bcd_op_fin(bcd_op* op)
{
    if (op->ec > EXPLIMIT)
    {
        // overflow
        CLEAR(op->c, op->pc);
        op->ec = POS_INF_EXP;
    }
    else if (op->ec <= -EXPLIMIT) 
    {
        /* underflow */
        CLEAR(op->c, op->pc);
        //op->ec = 0;
    }

    int neg = GET_NEG_BIT(op->c, op->pc);
    if (IS_ZERO(op->c, op->pc))
    {
        op->ec = 0;
        neg = false; // no neg zero
    }
    
    SET_EXP(op->c, op->pc, op->ec);
    if (neg) SET_NEG_BIT(op->c, op->pc);
}

static int bcd_op_uadd(bcd_op* op)
{
    // `c' has size `pa'
    // result will have size pa
    // return an upper bound for first insignificant digit
    // ASSUME ea >= eb

    int rd = 0;
    int ca;
    int v;
    int d = op->ea - op->eb;
    int j = op->pa-d;
    int i = op->pa;

    // first insignificant digit of `b' used by final rounding
    if (j < op->pb)
    {
        if (j >= 0) rd = op->b[j];

        // scan remainder of `b' for rd upper bound
        int k = j+1;
        if (k < 0) k = 0;
        while (k < op->pb)
        {
            if (op->b[k])
            {
                ++rd;
                break;
            }
            ++k;
        }
    }

    // clear pc+1 term where exponent lives
    op->c[i--] = 0;

    // nb: if j == bp, then both are the same size, so no
    // hidden digit to add nor rounding value

    // copy over `a' terms beyond `b', if b is shorter
    while (j > op->pb)
    {
        op->c[i] = op->a[i];
        --j;
        --i;
    }

    ca = 0;
    while (j > 0) // perform addition of overlapping terms
    {
        v = op->a[i] + op->b[--j] + ca;
        ca = 0;
        if (v >= BASE)
        {
            v -= BASE;
            ca = 1;
        }
        op->c[i] = v;
        --i;
    }

    while (i >= 0) // remainder non-overlap terms
    {
        v = op->a[i] + ca;
        ca = 0;
        if (v >= BASE)
        {
            v -= BASE;
            ca = 1;
        }
        op->c[i] = v;
        --i;
    }

    if (ca)
    {
        // overall carry, shift down
        if (rd) rd = 1;
        if (op->c[op->pc-1]) rd = op->c[op->pc-1];

        for (i = op->pc; i > 0; --i) op->c[i] = op->c[i-1];
        op->c[0] = ca;
        ++op->ec;
    }
    return rd;
}

void bcd_uadd(const unsigned short* a, const unsigned short* b,
             unsigned short* c,
             int pa, int pb)
{
    bcd_op op;
    bcd_op_prep(&op, a, b, c, pa, pb);
    int rd = bcd_op_uadd(&op);

    // NB: `uadd' never changes pc
    
    // round to nearest
    if (rd >= 5000)
        if (bcd_bup(op.c, op.pc))
            ++op.ec;
    
    bcd_op_fin(&op);
}


static int bcd_op_usub(bcd_op* op)
{
    // `c' has size `pa'
    // result size is pa, but pc <= pa to indicate shorter size
    // return an upper bound for first insignificant digit

    bool neg = false;
    int rd;
    int d = op->ea - op->eb;
    int ca;
    int v;
    int j = op->pa-d;
    int i = op->pa;

    ca = 0;
    v = 0;

    // find the first insignificant digit
    if (j < op->pb)
    {
        if (j >= 0) v = op->b[j];

        // scan remainder of `b', round up to 1 
        int k = j+1;
        if (k < 0) k = 0;
        while (k < op->pb)
        {
            if (op->b[k])
            {
                // a further digit. add 1 to first insig digit
                ++v;
                break;
            }
            ++k;
        }

        if (v)
        {
            v = BASE - v;
            ca = 1;
        }
    }

    // overwrite exponent slot
    op->c[i--] = v;

    // copy over `a' terms beyond `b', if b is shorter
    while (j > op->pb)
    {
        op->c[i] = op->a[i];
        --j;
        --i;
    }

    while (j > 0)
    {
        v = op->a[i] - op->b[--j] - ca;
        ca = 0;
        if (v < 0) 
        {
            ca = 1;
            v += BASE;
        }
        op->c[i] = v;
        --i;
    }

    // copy remaining digits with any borrow
    while (i >= 0)
    {
        v = op->a[i] - ca;
        ca = 0;
        if (v < 0) 
        {
            ca = 1;
            v += BASE;
        }
        op->c[i] = v;
        --i;
    }
    
    if (ca)
    {
        /* overall borrow, need to complement number */
        for (i = op->pc; i >= 0; --i) 
        {
            v = BASE-1 - op->c[i] + ca;
            ca = 0;
            if (v >= BASE) 
            {
                ca = 1;
                v -= BASE;
            }
            op->c[i] = v;
        }
        neg = true;
    }

    rd = op->c[op->pc];

    // deal with cancellation. initial digits may be zero
    // NB: `rd' remains correct when we change both size and exponent
    i = 0;
    while (op->c[i] == 0 && i <= op->pc) i++;

    if (i > 0) 
    {
        if (i >= op->pc)
        {
            /* is zero */
            op->ec = 0;
            op->pc = 1; 
        }
        else 
        {
            op->ec -= i;

            int j;
            for (j = 0; j <= op->pc - i; j++) op->c[j] = op->c[j + i];
            for (; j <= op->pc; j++) op->c[j] = 0;

            // indicate that c can be shorter, but work with size pa
            op->pc -= i;
        }
    }
 
    if (neg) NEGATE_SIGN(op->c, op->pa); // use PA
    return rd;
}

int bcd_op_addsub(bcd_op* op, bool sub)
{
    // `c' has size `pa'
    // result size is pa, but pc <= pa to indicate shorter size
    // return an upper bound for first insignificant digit
    // ASSUME ea >= eb

    int rd;
    if (GET_SPECIAL(op->a, op->pa) || GET_SPECIAL(op->b, op->pb))
    {
        CLEAR(op->c, op->pc);
        op->c[op->pc] = NAN_EXP;

        /* inf + inf = inf
         * -inf + (-inf) = -inf
         */
        if (!GET_NAN(op->a,op->pa) && !GET_NAN(op->b,op->pb))
        {
            if (GET_INF(op->a,op->pa))
            {
                // add: b not inf or inf + inf or (-inf) + (-inf)
                // sub: b not inf or inf - inf or (-inf) - (-inf)
                if (!GET_INF(op->b,op->pb) || (op->a[op->pa] == op->b[op->pb]))
                {
                    // inf +/- x = inf
                    op->c[op->pc] = op->a[op->pa];
                }
            }
            else
            {
                // a not inf, b is +/- inf
                op->c[op->pc] = op->b[op->pb];

                if (sub)
                    NEGATE_SIGN(op->c, op->pc);
            }
        }
        rd = -1; 
    }
    else
    {
        rd = 0;
        if (GET_ZERO_NORM(op->a, op->pa))
        {
            // a == 0, ASSUME pc >= pb
            int pc = op->pb; 
            COPY(op->c, op->b, pc); // copy mant of b
            op->pc = pc; // signal size (if different)
            op->ec = op->eb;
            op->c[op->pa] = op->b[pc];  // put copy of exponent at pa
            if (sub) NEGATE_SIGN(op->c, op->pa);
            return rd;
       }
       if (GET_ZERO_NORM(op->b,op->pb))
       {
           // copy of a. NB: ec = ea
           COPY(op->c, op->a, op->pa+1); // include exponent
           return rd;
       }
       
        // need ea >= eb, otherwise swap
        // if same, prefer shorter value
        bool s = (op->ea < op->eb || (op->ea == op->eb && op->pa > op->pb));
        
        if (s)
            bcd_op_swap(op);

        bool na = GET_NEG_NORM(op->a,op->pa);
        bool nb = GET_NEG_NORM(op->b,op->pb);

        if (sub)
        {
            if (na == nb) 
            {
                rd = bcd_op_usub(op);

                // (-a)-(-b) = -(a-b)
                if (na ^ s) NEGATE_SIGN(op->c,op->pa); // NB: invert if swap
            }
            else
            {
                rd = bcd_op_uadd(op);

                // (-a)-b = -(a+b)
                if (na) NEGATE_SIGN(op->c,op->pa); 
            }
        }
        else
        {
            // add
            if (na == nb) 
            {
                // same sign
                rd = bcd_op_uadd(op);
            }
            else 
            {
                // different sign
                rd = bcd_op_usub(op);
            }

            // (-a)+b = -(a-b)
            // (-a)+(-b) = -(a+b)
            if (na) NEGATE_SIGN(op->c,op->pa);
        }
    }
    return rd;
}

void bcd_add(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             int pa, int pb)
{
    bcd_op op;
    bcd_op_prep(&op, a, b, c, pa, pb);
    int rd = bcd_op_addsub(&op, false);

    // prevent size change
    op.pc = op.pa;

    if (rd >= 0)
    {
        // round to nearest
        if (rd >= 5000)
            if (bcd_bup(op.c, op.pc))
                ++op.ec;
    }
    
    bcd_op_fin(&op);
}


void bcd_sub(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             int pa, int pb)
{
    bcd_op op;
    bcd_op_prep(&op, a, b, c, pa, pb);
    int rd = bcd_op_addsub(&op, true);

    // prevent size change
    op.pc = op.pa;

    if (rd >= 0)
    {
        // round to nearest
        if (rd >= 5000)
            if (bcd_bup(op.c, op.pc))
                ++op.ec;
    }
    
    bcd_op_fin(&op);
}

int bcd_op_mul(bcd_op* op)
{
    // pa <= pb
    // size of result driven by `pc' which is normally set to 'pa'
    // size can be <= pc.
    // return an upper bound for first insignificant digit
    // ASSUME op->ws is pa+pb workspace

    int rd;

    // zero flags assuming not special
    bool az = GET_ZERO_NORM(op->a,op->pa);
    bool bz = GET_ZERO_NORM(op->b,op->pb);

    // neg flag assuming non-zero and non-special
    int na = GET_NEG_BIT(op->a,op->pa);
    int nb = GET_NEG_BIT(op->b,op->pb);

    if (GET_SPECIAL(op->a,op->pa) || GET_SPECIAL(op->b,op->pb))
    {
        /* all others -> nan */
        CLEAR(op->c, op->pc);
        op->c[op->pc] = NAN_EXP;

        if (!GET_NAN(op->a,op->pa) && !GET_NAN(op->b,op->pb))
        {
            if ((GET_INF(op->a,op->pa) && (GET_INF(op->b,op->pb) || !bz))
                || !az)
            {
                // inf * inf -> inf
                // inf * non-zero -> inf
                // non-zero * inf -> inf
                op->c[op->pc] = POS_INF_EXP;
                if (na != nb) NEGATE_SIGN(op->c, op->pc);
            }
        }
        rd = -1;
    }
    else if (!az && !bz)
    {
        int ca;
        int i, j;
        int4 u, v;

        // will build the product in the `op' workspace.
        // this must be enough for pa + pb terms

        unsigned short* w = op->ws;
        int pb = op->pb;
        i = op->pa;
        w += i;
        CLEAR(w, pb);  // need only clear pb terms

        int nw = i + pb;

        --i;
        for (;;)
        {
            ca = 0;
            u = op->a[i];
            if (u)
            {
                for (j = pb-1; j >= 0; --j) 
                {
                    v = op->b[j]*u + w[j] + ca;
                    ca = 0;
                    if (v >= BASE) 
                    {
                        ca = v / BASE;
                        v = v - ca*BASE;
                    }
                    w[j] = v;
                }
            }

            *--w = ca;

            if (!i) break;
            --i;
        }

        // `w' now points to start of product

        // skip possible 0 term
        if (!w[0])
        {
            --op->ec;
            --nw;
            ++w;
        }
        op->ec += op->eb;

        // limit by what we have
        if (op->pc > nw) op->pc = nw;

        // copy into c, `pc' terms requested
        COPY(op->c, w, op->pc);

        // clear exponent
        op->c[op->pc] = 0;

        // first ignored term
        rd = 0;
        i = op->pc;
        if (i < nw)
        {
            rd = w[i];

            // examine remaining digits
            while (++i < nw)
            {
                if (w[i])
                {
                    // add 1 to ignored term to make upper bound
                    ++rd;
                    break;
                }
            }
        }
        
        /* fix sign */
        if (na != nb) NEGATE_SIGN(op->c,op->pc);
    }
    else
    {
        // result is zero
        rd = 0;
        CLEAR(op->c, op->pc+1); // including exp
    }
    return rd;
}

void bcd_mul(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             unsigned short* tmp, // buf size pa+pb
             int pa, int pb)
{
    bcd_op op;
    bcd_op_prep(&op, a, b, c, pa, pb);

    // assign workspace
    op.ws = tmp;

    // `op.pc' is the requested size
    int rd = bcd_op_mul(&op);
    
    if (rd >= 0)
    {
        // round to nearest
        if (rd >= 5000)
            if (bcd_bup(op.c, op.pc))
                ++op.ec;
    }
    
    bcd_op_fin(&op);
}


void bcd_fma(const unsigned short* a,
             const unsigned short* b,
             const unsigned short* d,
             unsigned short* c,
             int pn)
{

    // zero flags assuming not special
    bool az = GET_ZERO_NORM(a,pn);
    bool bz = GET_ZERO_NORM(b,pn);
    bool dz = GET_ZERO_NORM(d,pn);

    if (dz)
    {
        unsigned short tmp[MAX_P*2];
        bcd_mul(a,b,c,tmp, pn, pn);
        return;
    }

    // neg flag assuming non-zero and non-special
    int na = GET_NEG_BIT(a,pn);
    int nb = GET_NEG_BIT(b,pn);

    int i;
    for (i = 0; i <= pn; ++i) c[i] = 0;

    if (GET_SPECIAL(a,pn) | GET_SPECIAL(b,pn) | GET_SPECIAL(d,pn))
    {
        /* all others -> nan */
        c[pn] = NAN_EXP;

        if (!(GET_NAN(a,pn) | GET_NAN(b,pn) | GET_NAN(d,pn)))
        {
            if ((GET_INF(a,pn) && (GET_INF(b,pn) || !bz)) || !az)
            {
                // inf * inf -> inf
                // inf * non-zero -> inf
                // non-zero * inf -> inf
                c[pn] = POS_INF_EXP;
                if (na != nb) NEGATE_SIGN(c, pn);
            }
        }
        return;
    }

    if (!az && !bz)
    {        
        int ca;
        int i, j;
        int4 u, v;

        int ea = GET_EXP(a,pn);
        int eb = GET_EXP(b,pn);
        int ed = GET_EXP(d,pn);

        // after multiply, effective exponent is ea+eb
        ea += eb;

        if (ed - pn <= ea) // otherwise d makes a*b small
        {
            // accumulator for double precision multiply terms
            unsigned short acc[MAX_P*2];
            unsigned short dex[MAX_P*2];
            unsigned short res[MAX_P*2];

            int pn2 = 2*pn-1;

            // clear acc 
            // acc is shifted by 1 to allow carry
            for (i = pn2; i >= 0; --i) acc[i] = 0;

            i = pn-1;
            for (;;)
            {
                u = a[i];
                if (u)
                {
                    ca = 0;
                    for (j = pn; j > 0; --j) 
                    {
                        v = b[j-1]*u + acc[i+j] + ca; // acc is +1 for carry
                        ca = 0;
                        if (v >= BASE) 
                        {
                            ca = v / BASE;
                            v = v - ca*BASE;
                        }
                        acc[i+j] = v;
                    }
                    acc[i] = ca; // acc carry
                }
            
                if (!i) break;
                --i;
            }

            /// find first significant digit
            int a0 = 0;
            if (!acc[0]) 
            {
                // one less digit in a*b
                ++a0;
                --pn2;
                --ea;
            }

            // expand d for double precision add
            for (i = 0; i < pn; ++i)
                dex[i] = d[i];
            
            // clear remainder
            while (i < pn2) dex[i++] = 0;
            
            // final exponent
            dex[pn2] = d[pn];
            
            // put exponent onto a*b
            SET_EXP(acc+a0,pn2,ea);
            if (na != nb) NEGATE_SIGN(acc + a0,pn2);

            // perform add with normalise
            bcd_add(acc + a0, dex, res, pn2, pn2);

            if (!GET_SPECIAL(res, pn2))
            {
                // get result exponent
                ea = GET_EXP(res, pn2);
                int nres = GET_NEG_BIT(res,pn2);

                // copy significant digits of res to c
                for (i = 0; i <= pn; ++i) c[i] = res[i];
            
                if (bcd_round25(c,pn)) ++ea;
            
                if (ea > EXPLIMIT)
                    c[pn] = POS_INF_EXP;  // overflow           
                else 
                    SET_EXP(c,pn,ea);

                if (nres) NEGATE_SIGN(c,pn);
            }
            else
            {
                // just copy over flags & code from exponent
                c[pn] = res[pn2];
            }
            return;
        }
    }
    
    // a or b is zero or d is large, return d
    for (i = 0; i <= pn; ++i) c[i] = d[i];
}

int bcd_op_div(bcd_op* op)
{
    // pa >= pb
    // op->ws, space for max(pa,pb)+2+pb+1
    // driven by `pc' digits. pc <= pa
    // return an upper bound for first insignificant digit

    int rd;
    int pa = op->pa;
    int pb = op->pb;

    int as = GET_SPECIAL(op->a,pa);
    int bs = GET_SPECIAL(op->b,pb);
        
    // zero flags assuming not special
    int az = GET_ZERO_NORM(op->a,pa) && !as;
    int bz = GET_ZERO_NORM(op->b,pb) && !bs;

    // neg assuming non-special
    int na = GET_NEG_BIT(op->a,pa) && !az;
    int nb = GET_NEG_BIT(op->b,pb) && !bz;
    int i;

    if (as || bs || az || bz)
    {
        CLEAR(op->c, op->pc);

        /* all others -> nan */
        op->c[op->pc] = NAN_EXP;

        if (!GET_NAN(op->a,pa) && !GET_NAN(op->b,pb))
        {
            if (GET_INF(op->a,pa) || bz)
            {
                // inf/inf -> nan
                if (!GET_INF(op->b,pb) && !az)
                {
                    // inf/x -> inf
                    if (na == nb) op->c[op->pc] = POS_INF_EXP;
                    else op->c[op->pc] = NEG_INF_EXP;
                }
            }
            else if (GET_INF(op->b,op->pc) | az)
            { 
                // x/inf -> 0
                op->c[op->pc] = 0;
            }
        }
        rd = -1;
    }
    else
    {
        int4 u, v;
        int ca;
        int j;
        int4 q;

        // size of accumulator is max(pa,pb)+2
        int pac = pa;
        if (pb > pac) pac = pb;
        pac += 2;
        
        unsigned short* acc = op->ws;
        unsigned short* b1 = acc + pac; // pb+1 digits

        u = BASE/(op->b[0]+1);

        // ensure j+2 is clear
        acc[2] = 0;
        
        if (u != 1) 
        {
            /* prenormialise `a' and move into acc using spare digit */
            ca = 0;
            for (i = pa; i > 0; --i) 
            {
                v = op->a[i-1]*u + ca;
                ca = 0;
                if (v >= BASE) 
                {
                    ca = v/BASE;
                    v -= ca*BASE;
                }
                acc[i] = v;
            }
            acc[0] = ca;

            /* prenormalise `b' into b1 (no carry) */
            ca = 0;
            for (i = pb-1; i >= 0; --i) 
            {
                v = op->b[i]*u + ca;
                ca = 0;
                if (v >= BASE) 
                {
                    ca = v/BASE;
                    v -= ca*BASE;
                }
                b1[i] = v;
            }
        }
        else 
        {
            /* u is often 1 */
            COPY(acc+1,op->a,pa);
            COPY(b1,op->b,pb);
            acc[0] = 0;
        }

        // slot not used unless pb=1
        b1[pb] = 0;

        // clear accumulator excess of pa+1
        if (pac > pa)
            CLEAR(acc + pa + 1, pac - pa - 1);
        
        j = 0;
        rd = 0;
        for (;;)
        {
            if (acc[0] == b1[0]) q = BASE-1;
            else 
            {
                v = acc[0]*((int4)BASE) + acc[1];
                q = v/b1[0];
                while (b1[1]*q > ((v - q*b1[0])*BASE + acc[2])) --q;
            }

            if (!q && !j) 
            {
                /* first quotient digit is zero. can gain extra
                 * accuracy by ignoring this and adjusting exponent.
                 */
                --op->ec;
            }
            else 
            {
                if (j == op->pc) 
                {
                    rd = q+1;  // additional digit (upper bound)
                    break;
                }

                ca = 0;
                for (i = pb; i > 0; --i) 
                {
                    v = acc[i] - b1[i-1]*q - ca;
                    ca = 0;
                    if (v < 0) 
                    {
                        ca = (-v + BASE-1)/BASE;
                        v += ca*BASE;
                    }
                    acc[i] = v;
                }

                v = acc[0] - ca;
                //acc[0] = v;

                if (v < 0)
                {
                    /* the infamous add back correction */
                    ca = 0;
                    for (i = pb; i > 0; --i) 
                    {
                        v = acc[i] + b1[i-1] + ca;
                        ca = 0;
                        if (v >= BASE) 
                        {
                            ca = 1;
                            v -= BASE;
                        }
                        acc[i] = v;
                    }
                    --q;
                    //acc[0] = 0;
                }
                if (q == 0 && j == 0)
                    --op->ec;
                else
                    op->c[j++] = q;
            }

            // shift accumulator
            for (i = 0; i < pac-1; ++i) acc[i] = acc[i+1]; 
            acc[pac-1] = 0;
        }
        
        op->c[op->pc] = 0; // clear exp
        if (na != nb) NEGATE_SIGN(op->c, op->pc);
        op->ec -= op->eb - 1;
    }
    return rd;
}

void bcd_div(const unsigned short* a,
             const unsigned short* b,
             unsigned short* c,
             unsigned short* tmp,
             int pa, int pb)
{
    bcd_op op;
    bcd_op_prep(&op, a, b, c, pa, pb);

    // assign workspace
    op.ws = tmp;

    int rd = bcd_op_div(&op);
    
    if (rd >= 0)
    {
        // round to nearest
        if (rd >= 5000)
            if (bcd_bup(op.c, op.pc))
                ++op.ec;
    }
    
    bcd_op_fin(&op);
}

int bcd_cmp(const unsigned short* a, 
            const unsigned short* b,
            int pa, int pb)
{
    if (GET_NAN(a,pa) || GET_NAN(b,pb))
    {
        // NaNs are not equal to anything, even themselves;
        // ALL comparisons involving them should return 'false'
        // (except '!=', which should return 'true').
        // So, I return a special value here.
        return 2;
    }
    
    // zero flags assuming not special
    int az = GET_ZERO_NORM(a,pa) && !GET_INF(a,pa);
    int bz = GET_ZERO_NORM(b,pb) && !GET_INF(b,pb);

    // neg 
    int na = GET_NEG_BIT(a,pa) && !az;
    int nb = GET_NEG_BIT(b,pb) && !bz;

    if (na != nb)
        return na ? -1 : 1;

    if (az)
        return bz ? 0 : -1;

    if (bz)
        return 1;

    // both positive or both negative, neither zero

    int res = 0;

    if (GET_INF(a, pa))
    {
        if (!GET_INF(b,pb)) res = 1; // inf < x. inf == inf
    }
    else if (GET_INF(b,pb)) res = -1; // x < inf
    else
    {
        int ea = GET_EXP(a,pa);
        int eb = GET_EXP(b,pb);

        res = ea - eb;
        if (!res)
        {
            int i;
            int pn = pa;
            if (pb < pn) pn = pb;
            for (i = 0; i < pn; i++) 
            {
                res = a[i] - b[i];
                if (res)
                    break;
            }
            if (!res)
            {
                if (i < pa) res = 1;
                else if (i < pb) res = -1;
            }
            
            // otherwise equal
        }
    }

    // if both negative, switch result
    return na ? -res : res;
}

#ifdef SMALL_INT
void bcd_fromUShort(unsigned short* d, int pn, unsigned short v)
{
    if (v < BASE) 
    {
        d[0] = v;
        d[pn] = 1;
    }
    else
    {
        d[0] = (unsigned short)(v/BASE);
        d[1] = (unsigned short)(v - d[0]*BASE);
        d[pn] = 2;
    }
}
#endif // SMALL_INT

void bcd_fromUInt(unsigned short* d, int pn, uint4 v)
{
    /* quicker to deal with cases separately */
    if (v < BASE) 
    {
        d[0] = v;
        d[pn] = 1;
    }
    else if (v < ((int4) BASE)*BASE) 
    {
        d[0] = (unsigned short)(v/BASE);
        d[1] = (unsigned short)(v - d[0]*BASE);
        d[pn] = 2;
    }
    else 
    {
        d[0] = (unsigned short)(v/(((int4) BASE)*BASE));
        v -= d[0]*(((int4) BASE)*BASE);
        d[1] = (unsigned short)(v/BASE);
        d[2] = (unsigned short)(v - d[1]*BASE);
        d[pn] = 3;
    }
}

void bcd_floor(unsigned short* c, int pn)
{
    /* floor, largest integer <= a.
     * eg floor(2.1) = 2.
     *    floor(-2.1) = -3.
     */
    if (!GET_SPECIAL(c,pn))
    {
        int e = GET_EXP(c,pn);
        int i;
        bool changed = false;
        for (i = pn-1; i >= 0; --i) 
        {
            if (i >= e) 
            {
                if (c[i]) changed = true;
                c[i] = 0;
            }
            else break;
        }
    
        if (GET_NEG_BIT(c,pn) && changed) 
        {
            /* need to subtract 1 */
            if (i >= 0)
            {
                int v = c[i] + 1;
                while (v >= BASE)
                {
                    c[i] = (unsigned short)(v - BASE);
                    if (!i) 
                    {
                        // shift right
                        for (i = pn-1; i > 0; --i) c[i] = c[i-1];
                        v = 1;

                        // bump exp
                        // exp will be +ve 
                        int e = GET_EXP(c,pn);

                        // NB: will not overflow
                        SET_EXP(c,pn,e+1);
                        NEGATE_SIGN(c,pn);
                        break;
                    }
                    v = c[--i]+1;
                }
                c[i] = v;
            }
            else
            {
                // result is -1
                c[0] = 1;
                SET_EXP(c,pn,1);
                NEGATE_SIGN(c,pn);
            }
        }
    }
}

void bcd_roundDigits(const unsigned short* d, int pn,
                     int fmt, unsigned int precision, unsigned short* v) 
{
    // `v' is size `pn'

    if (!GET_SPECIAL(d, pn) && !GET_ZERO_NORM(d, pn) &&
        precision <= DDIGITS(pn))
    {
        bool n = IS_NEG(d, pn);
        int pr = precision;

        if (!(fmt & BCDFloat::format_precision_decimals))
        {
            // adjust precision to number of digits before 4radix.
            if (d[0] >= 10) --pr;
            if (d[0] >= 100) --pr;
            if (d[0] >= 1000) --pr;
            --pr; // units
        }

        int m = BCDDecade[pr & 3] * 5;
        int de = GET_EXP(d,pn) - (pr>>2) - 1;

        if (precision == DDIGITS(pn) && d[0] > 0 && d[0] < 10)
            m = 0; // the last digit is already rounded.

        // make a small number
        unsigned short rv[2];
        rv[0] = m;
        SET_EXP(rv, 1, de);

        // ignores signs
        bcd_uadd(d, rv, v, pn, 1);

        // restore sign if needed
        if (n) SET_NEG_BIT(v, pn);
    }
    else
        COPY(v, d, pn+1);
}

#define COUNT_ZERO_AS_DIGITxx

static bool bcd_asStringFits(const unsigned short* d, int pn,
                             char* buf, int fmt, int precision,
                             int* underzeros)
{
    // can we print out without rounding?
    // return true if perfect fit.
    
    // also, the number of zeros under the point
    // eg 0.001 is 2
    // eg 0.333 is 0
    // -1 if no chance
    //
    // work to `precision' digits not including sign, but including the 
    // "0" of the "0."

    bool res = false;
    char* p = buf;
    int i;
    bool scimode = (fmt & BCDFloat::format_scimode) != 0;
    if (IS_NEG(d, pn)) *p++ = '-';
    char* point = 0;
    int e = GET_EXP(d, pn);
    i = 0;

    if (underzeros) *underzeros = -1;

    if (!scimode && e <= 0 && (1-e)*4+1 < precision)
    {
        /* see if `precision' is enough to write the whole
         * number
         */

        int n = pn;
        while (!d[n-1] && n > i) --n;

        *p++ = '0';
        point = p;
        *p++ = '.';
        
        if (e < 0)
        {
            for (int j = 0; j < -e; ++j)
            {
                *p++ = '0';
                *p++ = '0';
                *p++ = '0';
                *p++ = '0';
            }
        }

        while (i < n)
        {
            sprintf(p, "%04d", d[i]); 
            p += 4;     
            ++i;
        }

        *p = 0;        

        /* tidy up */
        while (p > point && (p[-1] == '0' || p[-1] == '.')) *--p = 0;

        // calculate logical size
        int extra = 0;

#ifndef COUNT_ZERO_AS_DIGIT
        ++extra;
#endif

        if (IS_NEG(d, pn)) ++extra; // allow extra for sign
        if (strchr(buf, '.')) ++extra; // allow for point



        if (strlen(buf) <= precision + extra) // do we fit?
        {
            res = true;
        }
        else
        {
            buf[precision+extra] = 0;

            // tidy up again
            p = buf + strlen(buf);
            while (p > point && (p[-1] == '0' || p[-1] == '.')) *--p = 0;
        }

        if (underzeros)
        {
            // how many zeros below point
            // we might accept it anyway.
            // for example 0.3333333333333333 is sometimes preferred
            // over 3.33333333e-1
            
            *underzeros = 0;
            while (*++point == '0')
                ++*underzeros;
        }
    }
    return res;
}

static void bcd_asStringAux(const unsigned short* d, int pn,
                            char* buf, int fmt, int precision)
{
    char* p = buf;
    int i;
    bool scimode = (fmt & BCDFloat::format_scimode) != 0;
    if (IS_NEG(d, pn)) *p++ = '-';
    char* point = 0;
    int v;
    int eadj = 0;
    int e = GET_EXP(d, pn);
    int pr = 0;  // significant digits printed
    i = 0;
    if (e > 0) 
    {
        /* if the exponent is less than our digits, we can
         * print out the number like an integer.
         */
        if (!scimode)
        {
            v = d[i++];
            sprintf(p, "%d", v);         
            while (*p) { ++p; ++pr; }

            int ep = pr + (e-i)*4;
            if (ep <= precision)
            {
                // have enough space
                pr = ep;
                while (i < e && i < pn) 
                {
                    v = d[i];            
                    sprintf(p, "%04d", v);            
                    p += 4;
                    ++i;
                }
                while (i < e)
                {
                    // mantissa shorter than exponent
                    strcpy(p, "0000");
                    p += 4;
                    ++i;
                }
                e = 0;
            }
            else
            {
                // too big, resort to sci notation
                p -= pr;
                pr = 0;
                i = 0;
            }
        }
    }

    if (!pr)
    {
        /* otherwise we have a larger number (or small). print out
         * as scientific form.
         */
        v = d[0];
        char tb[8];
        sprintf(tb, "%d", v);
        char* q = tb;
        *p++ = *q++;
        ++pr;
        point = p; 
        *p++ = '.';
        while (*q) 
        {
            *p++ = *q++;
            ++pr;
            ++eadj;
        }
        ++i;
        --e;
    }


    // here we have i = number of 4digits printed
    // e = remains of exponent
    int n = pn;
    while (!d[n-1] && n > i) --n;
    if (fmt & BCDFloat::format_precision_decimals)
        pr = 0;

    if (i < n) 
    {
        if (!point) 
        {
            point = p;
            *p++ = '.';
        }

        for (; i < n; ++i) 
        {
            v = d[i];
            sprintf(p, "%04d", v); 
            pr += 4;
            p += 4;
            if (precision && pr >= precision)
            {
                int over = pr - precision;
                p -= over;
                *p = 0;
                break;
            }
        }
    }

    /* tidy up */
    if (point) 
        while (p > point && (p[-1] == '0' || p[-1] == '.')) *--p = 0;
    
    // print exponent
    if (!IS_ZERO(d, pn) && (e || scimode || eadj))
#ifdef CUSTOM_EXP_CHAR
            sprintf(p, "%c%d", CUSTOM_EXP_CHAR, e*4+eadj);
#else
            sprintf(p, "e%d", e*4+eadj);
#endif
    
}

void bcd_asString(const unsigned short* d, int pn,
                  unsigned short* tmp,
                  char* buf, int fmt, int precision)
{
    // `tmp' is workspace size `pn'
    if (GET_SPECIAL(d, pn))
    {
        char* p = buf;
        if (GET_NAN(d, pn)) strcpy(p, "NaN");
        else 
        { // inf
            if (IS_NEG(d, pn)) *p++ = '-';
            strcpy(p, "Inf");
        }
    }
    else
    {
        int underzeros;
        if (!bcd_asStringFits(d, pn, buf, fmt, precision, &underzeros))
        {
            const unsigned short* f = d;

            // prefer 0.0333333 to 3.3333e-2
            bool allowunder = (underzeros >= 0 && underzeros < 2);

            if (fmt & BCDFloat::format_rounding)
            {
                int pr = precision;
                if (allowunder) pr -= underzeros;
#ifdef COUNT_ZERO_AS_DIGIT
                // one less if we're counting the zero
                --pr; 
#endif
                bcd_roundDigits(d, pn, fmt, pr, tmp);

                // use version in `tmp'
                f = tmp;
            }

            bool done = false;

            if (allowunder)
            {
                // we've rounded so this time it might not fit anymore
                bcd_asStringFits(f, pn, buf, fmt, precision, &underzeros);

                // if same, then ok.
                done = (underzeros >= 0 && underzeros < 2);
            }

            if (!done)
                bcd_asStringAux(f, pn, buf, fmt, precision);

        }
    }
}

int bcd_fromString(unsigned short* d, int pn, const char* s)
{
    //  make a BCD float from a string.
    // if d == 0, return number of digits required

    int rd = 0;
    bool neg = false;
    if (*s == '-') 
    {
        neg = true;
        ++s;
    }

    int i;
    int j = 0;
    int e = 0;
    
    /* find the end of the input string */
    const char* endp = s;
    const char* startp = 0;
    const char* point = 0;

    while (*endp)
    {
        if (*endp == '.') 
        {
            if (point) break;
            point = endp;
            if (!startp) 
                startp = endp;
        }
        else 
        {
            if (!u_isdigit(*endp)) break;
            
            // skip leading zeros before point
            if (*endp != '0' && !point && !startp)
                startp = endp;
        }
        ++endp;
    }

    if (d) CLEAR(d, pn+1);

    if (!startp)
    {
        // check for Inf before fail
        if (!u_strnicmp(s, "inf", 3))
        {
            d[pn] = POS_INF_EXP;
            if (neg) NEGATE_SIGN(d, pn);
        }
    }
    else
    {
        bool eneg = false;
        if (startp) 
        {
            const char* ep = endp;
            if (*ep == ':')
            {
                ++ep;
                while (u_isdigit(*ep))
                {
                    rd = rd*10 + (*ep - '0');
                    ++ep;
                }
            }

            if (ISEXP(*ep)) 
            {
                ++ep;
                if (*ep == '-') 
                {
                    eneg = true;
                    ++ep;
                }
                else if (*ep == '+') ++ep;  // allow E+1234 

                e = 0;
                while (u_isdigit(*ep))
                    e = e*10 + (*ep++ - '0');
                
                if (eneg) e = -e;
            }

            /* represent the decimal point by adjusting the exponent */
            if (point) 
                e += (int)(point - startp);
            else 
                e += (int)(endp - startp);

            /* calculate the decade offset of the exponent remainder */
            i = (-e) & 3;
            e += i;

            /* convert to 4dec */
            e >>= 2;

            bool leading_zero = true;
            const char* p = startp;

            for (;;)
            {
                int di = 0;
                while (i < 4) 
                {
                    if (*p != '.') 
                    {
                        di += (*p - '0')*BCDDecade[i];
                        ++i;
                    }
                    ++p;
                    if (p == endp) break;
                }
                if (leading_zero && di == 0) 
                {
                    e--;
                    i = 0;
                    continue;
                }
                leading_zero = false;
                if (d) d[j] = di;

                if (++j > pn)
                    // full up.
                    break;

                if (p == endp) break;
                i = 0;
            }

            // i is number filled in last digit.
            i = (i - 1) & 3;
            rd *= BCDDecade[i];

            if (leading_zero)
                e = 0;

            if (d && bcd_round(d, pn)) e++;

            if (e <= -EXPLIMIT)
            {
                // underflow
                if (d) CLEAR(d, pn+1); 
                j = 1;
            }
            else 
            {
                if (e > EXPLIMIT)
                {
                    if (d)
                    {
                        CLEAR(d, pn);
                        d[pn] = POS_INF_EXP;
                    }
                    j = 0;
                }
                else if (d) SET_EXP(d, pn, e);
                if (neg && d) NEGATE_SIGN(d, pn);
            }
        }
    }
    return d ? rd : j;
}

unsigned int isqrt(unsigned int v)
{
    /* integer root */
    unsigned int x, t;
    if (v <= 1) return v;

    x = v >> 1;
    for (;;)
    {
        t = (x + v/x) >> 1;
        if (t >= x) break;
        x = t;
    }
    return x;
}

bool BCDFloat::sqrt(const BCDFloat* a, BCDFloat* r)
{
    if (a->neg() || a->isNan()) return false;
    if (a->isInf()) 
    {
        *r = *a;  // sqrt(inf) = inf
        return true;
    }

    int rs;
    int4 v;
    int rodd;
    int4 q;

    // acc, t and u get one more digit of workspace.
    unsigned short acc[P+2];
    int as;    

    unsigned short t[P+2];
    int ts;

    unsigned short u[P+2];
    int us = 0;

    r->_init();

    int e = a->exp();

    v = isqrt(a->_d[0]);

    rodd = !(e & 1);
    r->_d[0] = v;
    if (rodd) r->_d[0] *= 100;
    rs = 1;

    as = 0;
    acc[0] = a->_d[0] - v*v;
    if (acc[0]) ++as;

    int i;
    int j = 1;
    int4 ca;

    for (;;) 
    {
        /* bring in the next digit */
        acc[as] = j < P ? a->_d[j++] : 0;

        q = 0;
        if (acc[0]) 
        {
            ++as;
        
            /* t = 200*r if even, t=2*r if odd */
            int m = rodd ? 2 : 200;
            ca = 0;
            ts = rs;
            for (i = rs; i > 0; --i) 
            {
                v = ((int4) r->_d[i-1])*m + ca;
                ca = 0;
                if (v >= BASE) 
                {
                    ca = v/BASE;
                    v -= ca*((int4)BASE);
                }
                t[i] = v;
            }
            t[0] = ca;
            ++ts;

            while (!t[0])
            {
                for (i = 0; i < ts; ++i) t[i] = t[i+1];
                --ts;
            }

            if (ts > P+1) 
            {
                /* rarely, the temporary size can become bigger than
                 * we can handle. this can only happen on the last
                 * digit. if so, stop.
                 */
                break;
            }

            q = 0;
            if (ts == as) 
            {
                if (ts > 1)
                    q = (((int4) acc[0])*((int4)BASE) + acc[1])/(((int4) t[0])*((int4)BASE)+t[1]);
                else
                    q = ((int4)acc[0])/t[0];
            }
            else if (as > ts) 
            {
                q = (((int4) acc[0])*((int4)BASE) + acc[1])/t[0];
            }

            if (q) 
            {
                if (q > 99) q = 99;
        
                /* t = t + q */
                t[ts-1] += q;  // cant carry

                for (;;) 
                {
                    /* u = t*q */
                    ca = 0;
                    us = ts;
                    for (i = ts; i > 0; --i) 
                    {
                        v = t[i-1]*q + ca;
                        ca = 0;
                        if (v >= BASE) 
                        {
                            ca = v/BASE;
                            v -= ca*((int4)BASE);
                        }
                        u[i] = v;
                    }
                    u[0] = ca;
                    if (ca) ++us;
                    else 
                        for (i = 0; i < us; ++i) u[i] = u[i+1];
                
                    /* determine whether u > acc. if so then q was too
                     * big.
                     */
                    bool fail = us > as;
                    if (!fail && us == as)
                    {
                        for (i = 0; i < as; ++i) 
                        {
                            int d = u[i] - acc[i];
                            if (d > 0) 
                            {
                                fail = true;
                                break;
                            }
                            else if (d < 0)
                                break;
                        }
                    }

                    if (!fail) break;

                    /* decrease q by 1 and try again */
                    q -= 1;
                    --t[ts-1]; // adjust for new q
                }
            }
        }

        if (rodd) 
        {
            /* can accommodate 2 more digits in current size */
            r->_d[rs-1] += q;
            rodd = 0;
            if (rs == P+1) break;
        }
        else 
        {
            r->_d[rs++] = q*100;
            rodd = 1;
        }
        
        if (q) 
        {
            /* acc = acc - u.
             * wont borrow because u <= acc.
             */
            int k;
            ca = 0;
            k = us;
            for (i = as-1; i >= 0; --i) 
            {
                v = acc[i] - ca;
                if (k > 0) v -= u[--k];
                ca = 0;
                if (v < 0) 
                {
                    v += BASE;
                    ca = 1;
                }
                acc[i] = v;
            }

            while (!acc[0]) 
            {
                for (i = 0; i < as; ++i) acc[i] = acc[i+1];            
                if (!--as) break;
            }
        }
    }
    e = e >= -1 ? (e + 1) / 2 : e / 2;
    if (r->_round25()) ++e;
    r->exp(e);
    return true;
}

bool BCDFloat::trunc(const BCDFloat* a, BCDFloat* c)
{
    /* truncate towards zero.
     * trunc(2.1) = 2.
     * trunc(-2.1) = -2
     */
    *c = *a;
    int e = c->exp();
    int i;
    for (i = P-1; i >= 0; --i) 
        if (i >= e) c->_d[i] = 0;

    return true;
}

int4 BCDFloat::asInt() const
{
    // if we fit in an int, return it otherwise 0
    int ea = exp();
    int4 v = 0;
    int i = 0;
    while (i < ea && i < P) 
    {
        if (v > 214748L) return 0; // too large, bail out.
        v*= BASE;
        v += _d[i];
        ++i;
    }
    if (neg()) v = -v;
    return v;
}

bool BCDFloat::isInteger() const
{
    if (isZero())
        return true;

    int e = exp();
    int i;
    for (i = P-1; i >= 0; --i) 
        if (_d[i]) return e > i;
    return false;
}

void BCDFloat::mul2(const unsigned short* ad, int ea,
                    const unsigned short* bd, int eb,
                    unsigned short* cd, int& ec)
{
    int ca;
    int i, j;
    int4 u, v;

    unsigned short acc[2*P+1];

    for (i = 0; i < 2*P; ++i) cd[i] = 0;
    int cc = 0;
    for (i = 2*P-1; i >= 0; --i) 
    {
        for (j = 2*P; j > 0; --j) cd[j] = cd[j-1];
        cd[0] = cc; cc = 0;
        u = ad[i];
        if (!u) continue;

        ca = 0;
        for (j = 2*P; j > 0; --j) 
        {
            v = bd[j-1]*u + ca;
            ca = 0;
            if (v >= BASE) 
            {
                ca = v / BASE;
                v = v - ca*((int4)BASE);
            }
            acc[j] = v;
        }
        acc[0] = ca;

        /* now add acc into c */
        for (j = 2*P; j >= 0; --j) 
        {
            v = cd[j] + acc[j] + cc;
            cc = 0;
            if (v >= BASE) 
            {
                cc = 1;
                v -= BASE;
            }
            cd[j] = v;
        }
    }
    cd[0] += cc;  // carry?
    ec = ea + eb;
}
