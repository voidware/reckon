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

#include        <stdio.h>  // XX debug
#include        <string.h>
#include        <stdlib.h> // malloc
#include        "types.h"
#include        "calc.h"
#include        "big.h"
#include        "bigs.h"
#include        "cutils.h"

#ifdef _WIN32
#include "oswin.h"
#else
extern "C"
{
#include "common.h" // escape key
}
#endif

static Big* constZero;
static Big* constOne;

Big* one() 
{
    return constOne;
}

Big* zero()
{
    return constZero;
}


/* Forward Decls */
static Big* _create(int s);
static bool _roundFrac(BigFrac* f);
static unsigned int _bitlength(Big*);

#define FRACOK(_f) ((_f)->x_ && (_f)->y_)

/** BigFrac, methods *************************************************/

BigFrac::BigFrac(Big* b)
{
    x_ = b;
    y_ = constOne;
}

BigFrac::BigFrac(long v) 
{
    x_ = createBig(v);
    y_ = constOne;
}


/** Other, ***********************************************************/

void InitBig()
{
    constZero = _create(1);
    constZero[1] = 0;

    constOne = _create(1);
    constOne[1] = 1;
}

void _destroy(Big* b)
{
    delete b;
}

Big* _create(int s)
{
    Big* b;
    if (((s<<2)+s) <= Calc::theCalc->_maxDecimalDigits)
    {
        b = new Big[s + 1];
        SET_HEAD(b, 0, s);
    }
    else
        b = 0;

    return b;
}

void finishBig()
{
    _destroy(constZero);
    _destroy(constOne);
}

unsigned int _bitlength(Big* b)
{
    /* The size of `b' in binary */
    unsigned int n = DIGITS(b);
    unsigned int t;
    unsigned int c;

    t = b[n];
    c = 0;
    while (t) 
    {
        ++c;
        t >>= 1;
    }
    return c + (n-1)*BASEBITS;
}

unsigned int log2(Big* b)
{
    unsigned int l = 0;
    if (b) l =  _bitlength(b);
    if (l) --l;
    return l;
}

void destroyBig(Big* b)
{
    if (b && b != constZero && b != constOne)
        _destroy(b);
}

Big* parseBig(const char** sp)
{
    /* Expect `*sp' points to -?[0-9]+ or -?[0]*[Xb][0-9A-F]+ */
       
    const char* s = *sp;
    Big* r = constZero;
    int neg;
    int base = 10;

    if (*s == '-') 
    {
        neg = 1;
        ++s;
    }
    else neg = 0;

    // skip prefix
    while (*s == '0') ++s;

    if (*s == 'x' || *s == 'X')
    {
        /* hex number */
        base = 16;
        ++s;
    }
    else if (*s == 'b')
    {
        // binary
        base = 2;
        ++s;
    }

    while (*s) 
    {
        Big* t;
        int d;

        char c = u_toupper(*s);

        if (u_isdigit(c))
        {
            d = c - '0';
            if (d >= base) break; // illegal
        }
        else if (c >= 'A' && c <= 'F' && base == 16)
        {
            d = c - 'A' + 10;
        }
        else break;

        ++s;
        t = mulint(r, base); destroyBig(r);
        r = addint(t, d); destroyBig(t);
        if (!r) return 0; // bail number got too large
    }

    /* update callers pointer */
    *sp = s;

    if (!ISZERO(r)) SET_SIGNIF(r, neg);
    return r;
}

Big* createBig(long v)
{
    /* Big from signed int */
    Big* b;
    long l, h;
    int neg;

    if (!v) return constZero;
    if (v == 1) return constOne;

    neg = (v < 0);
    if (neg) v = -v;
    
    h = v >> BASEBITS;
    l = v & (BBASE-1);

    if (h) {
        b = _create(2);
        b[2] = (Big)h;
    }
    else b = _create(1);
    b[1] = (Big)l;

    if (neg) NEGATEBIT(b);

    return b;
}

Big* createBigu(unsigned long u)
{
    /* Big from unsigned int */

    Big* b;
    long l, h;

    if (!u) return constZero;
    if (u == 1) return constOne;

    h = u >> BASEBITS;
    l = u & (BBASE-1);

    if (h) 
    {
        b = _create(2);
        b[2] = (Big)h;
    }
    else b = _create(1);
    b[1] = (Big)l;
    return b;
}

Big* addu(Big* a, Big* b)
{
    /* a + b, ignore sign bits. */

    Big* r;
    Big* r0;

    unsigned long v, c;
    unsigned int n;
    unsigned int m;

    if (DIGITS(a) < DIGITS(b)) 
    {
        Big* t = a;
        a = b;
        b = t;
    }

    r0 = _create(DIGITS(a) + 1);
    if (r0)
    {
        c = 0;
        r = r0;
        n = DIGITS(b);
        m = DIGITS(a) - n;

        while (n) 
        {
            --n;
            v = *++a + *++b + c;
            c = v >> BASEBITS;
            *++r = v;
        }
    
        while (m) 
        {
            --m;
            v = *++a + c;
            c = v >> BASEBITS;
            *++r = v;
        }

        if (c) *++r = c;
        else --*r0;
    }
    return r0;
}

Big* addint(Big* a, int b)
{
    if (!a) return 0;
    
    if (b > 0) return addint(a, (unsigned int)b);
    else if (b < 0) return subint(a, (unsigned int)-b);
    return copyBig(a);
}

Big* addint(Big* a, unsigned int b)
{
    /* a + b, ignore sign bit. assume 0 <= b < BBASE */
    Big* r;
    Big* r0;

    unsigned long v;
    unsigned int n;
    unsigned int c;

    if (!a) return 0;

    n = DIGITS(a);
    r0 = _create(n + 1);
    if (r0)
    {
        r = r0;
        c = b;
        while (n) 
        {
            --n;
            v = *++a + c;
            c = v >> BASEBITS;
            *++r = v;
        }
    
        if (c) *++r = c;
        else --*r0;
    }
    return r0;
}

Big* subint(Big* a, unsigned int b)
{
    /* a - b, ignore sign bit. assume a >= b, 0 <= b < BBASE */

    Big* r;
    Big* r0;
    unsigned long v, c;
    unsigned int n;

    if (!a) return 0;

    n = DIGITS(a);
    r0 = _create(n);
    if (r0)
    {
        r = r0;
        v = BBASE + *++a - b;
        c = v >> BASEBITS;
        *++r = v;
        while (--n) 
        {
            v = (BBASE-1) + *++a + c;
            c = v >> BASEBITS;
            *++r = v;
        }

        while (!*r && r-1 != r0) 
        {
            --r;
            --*r0;
        }
    }
    return r0;
}

Big* subu(Big* a, Big* b)
{
    /* a - b, ignore sign bits. assume |A| >= |B| */

    Big* r;
    Big* r0;

    unsigned long v, c;
    unsigned int n;
    unsigned int m;

    r0 = _create(DIGITS(a));
    if (r0)
    {
        r = r0;
        n = DIGITS(b);
        m = DIGITS(a) - n;

        c = 1;
        while (n) 
        {
            --n;
            v = (BBASE-1) + *++a - *++b + c;
            c = v >> BASEBITS;
            *++r = v;
        }
        while (m) 
        {
            --m;
            v = (BBASE-1) + *++a + c;
            c = v >> BASEBITS;
            *++r = v;
        }

        while (!*r) 
        {
            if (--r == r0) break;
            --*r0;
        }
    }
    return r0;
}

int compare(Big* a, Big* b)
{
    if (a == b || !a || !b) return 0;

    int sa = SIGNBIT(a);
    int sb = SIGNBIT(b);

    if (sa && !sb) return -1;  // a < b
    if (!sa && sb) return 1; // a > b

    int v = compareu(a, b);
    if (sa)
    {
        // both negative
        return -v;
    }
    else
    {
        // both positive
        return v;
    }
}

int compareu(Big* a, Big* b)
{
    /* compare magnitudes of a and b (ignore signs).
     * if |A| > |B| return >0;
     * if |A| < |B| return <0;
     * if |A| = |B| return 0;
     */

    int la = DIGITS(a);
    int lb = DIGITS(b);
    
    int d = la - lb;
    if (!d) 
    {
        /* same length */
        a += la;
        b += la;
        do {
            d = *a - *b;
            if (d) break;
            --a;
            --b;
        } while (--la);
    }
    return d;
}


Big* addBig(Big* a, Big* b)
{
    /* a + b, signed */
    int sa, sb;
    Big* r = 0;

    if (a && b)
    {
        sa = SIGNBIT(a);
        sb = SIGNBIT(b);

        if (sa == sb) 
        {
            r = addu(a, b);
            if (r) SET_SIGNIF(r, sa);
        }
        else 
        {
            int agb = compareu(a, b);
            if (agb > 0) 
            {
                r = subu(a, b);
                if (r) SET_SIGNIF(r, sa);
            }
            else if (agb < 0) 
            {
                r = subu(b, a);
                if (r) SET_SIGNIF(r, sb);
            }
            else 
                r = constZero;
        }
    }
    return r;
}

Big* subBig(Big* a, Big* b)
{
    /* a - b, signed */
    int sa, sb;
    Big* r = 0;

    if (a && b)
    {
        sa = SIGNBIT(a);
        sb = SIGNBIT(b);

        if (sa != sb) 
        {
            r = addu(a, b);
            if (r) SET_SIGNIF(r, sa);
        }
        else 
        {
            int agb = compareu(a, b);
            if (agb > 0) 
            {
                r = subu(a, b);
                if (r) SET_SIGNIF(r, sa);
            }
            else if (agb < 0) 
            {
                r = subu(b, a);
                if (r) SET_SIGNIF(r, !sb);
            }
            else 
                r = constZero;
        }
    }
    return r;
}

Big* negateBig(Big* b)
{
    /* DESTRUCTIVE, change sign of `b' */
    if (b && !ISZERO(b)) 
    {
        if (b == constOne) b = createBig(-1);
        else NEGATEBIT(b);
    }
    return b;
}

Big* lshiftn(Big* b, unsigned int n)
{
    /* b*2^n */
    Big* r;
    unsigned int d;

    if (!n || !b) return copyBig(b);

    d = n & (BASEBITS-1);
    if (d) 
    {
        r = lshiftBig(b, true);  // DUP
        while (r && --d) 
            r = lshiftBig(r, false);

    }
    else r = copyBig(b);
    
    if (r)
    {
        d = n >> BASEBITSBITS;
        if (d) 
        {
            unsigned int c = DIGITS(r);
            Big* t = _create(c + d);
            if (t)
            {
                int i;
                for (i = 1; i <= d; ++i)
                    t[i] = 0;
                
                int j = 1;
                while (j <= c)
                    t[i++] = r[j++];
                        
                SET_SIGNOF(t, b);
            }

            destroyBig(r);
            r = t;
        }
    }
    return r;
}


Big* lshiftBig(Big* b, int dup)
{
    /* double b,
     * !dup => DESTRUCTIVE.
     */
    unsigned int n;
    Big* r0;
    Big* r;
    Big* b0 = b;
    unsigned long v;
    unsigned int c;

    if (!b || ISZERO(b)) return b;
    if (b == constOne) return createBig(2);
    
    n = DIGITS(b);
    c = (b[n] & (1<<(BASEBITS-1))) != 0;  // top bit set?

    if (dup || c) 
    {
        /* need to reallocate */
        r0 = _create(n + c);
        if (r0) SET_SIGNOF(r0, b);
    }
    else r0 = b;

    if (r0)
    {
        c = 0;
        r = r0;
        while (n) 
        {
            n--;
            v = (*++b << 1) + c;
            c = v >> BASEBITS;
            *++r = v;
        }
        if (c) 
            *++r = c;
    }

    if (!dup && c)
    {
        // !DUP && carry => r0 is new
        // NB: r0 can be 0
        destroyBig(b0);
    }

    return r0;
}

Big* copyBig(Big* b)
{
    /* duplicate b, including sign */
    unsigned int n;
    Big* r0 = 0;
    Big* r;

    if (b)
    {
        if (b == constZero || b == constOne) return b;

        n = DIGITS(b);
        r0 = _create(n);
        if (r0)
        {
            r = r0;
            for (;;) 
            {
                *r++ = *b++;
                if (!n) break;
                --n;
            } 
        }
    }
    return r0;
}

Big* mulint(Big* a, int b)
{
    /* multiply `a' by b where b < BBASE */
    int neg;
    unsigned int n;
    unsigned long c, v;
    Big* r0;
    Big* r;

    if (!a) return 0;

    neg = SIGNBIT(a);
    if (b < 0) 
    {
        b = -b;
        neg ^= 1;
    }

    c = 0;
    n = DIGITS(a);
    r0 = _create(n + 1);
    if (r0)
    {
        r = r0;
        while (n) 
        {
            n--;
            v = *++a * b + c;
            c = v >> BASEBITS;
            *++r = v;
        }
    
        if (c) *++r = c;
        else --*r0;
        SET_SIGNIF(r0, neg);
    }
    return r0;
}

Big* mulBig(Big* a, Big* b)
{
    /* a * b, signed */
    Big* r0;
    Big* r;
    int neg;
    unsigned long c, v;
    unsigned int j;

    r0 = 0;
    if (a && b)
    {
        unsigned int la = DIGITS(a);
        unsigned int lb = DIGITS(b);

        if (ISZERO(a) || ISZERO(b)) return constZero;
        neg = SIGNBIT(a) ^ SIGNBIT(b);

        r0 = _create(la + lb);
        if (r0)
        {
            r = r0;
            for (j = 1; j <= lb; ++j) r[j] = 0;
            
            while (la) 
            {
                la--;
                c = 0;
                if (*++a)
                {
                    for (j = 1; j <= lb; ++j) 
                    {
                        v = *a * b[j] + r[j] + c;
                        c = v >> BASEBITS;
                        r[j] = v;
                    }
                }
                else
                    j = lb+1;

                r[j] = c;
                ++r;
            }

            if (!r[lb]) --*r0;
            SET_SIGNIF(r0, neg);
        }
    }
    return r0;
}

Big* sqrBig(Big* a)
{
    /* a^2 */
    Big* r = 0;
    if (a)
    {
        if (ISZERO(a)) return constZero;
        unsigned int t = DIGITS(a);
        unsigned int t2 = t<<1;
        r = _create(t2);
        if (r)
        {
            int i, j;
            for (i = 1; i <= t2; ++i) r[i] = 0;

            Big* ai = a + 1;
            for (i = 0; i < t; ++i)
            {
                unsigned int c;
                unsigned int c2;
                Big* rij = r + i + i + 1;
                unsigned int uv = *rij + (*ai)*(*ai);
                *rij = uv;
                c = uv >> BASEBITS;
                for (j = i+2; j <= t; ++j)
                {
                    //uv = r[i+j] + a[j]*(*ai)*2 + c;
                    c2 = 0;
                    uv = a[j]*(*ai);
                    if (uv & 0x80000000)
                    {
                        // will carry when doubled.
                        c2 = 1<<BASEBITS;
                    }
                    uv <<= 1;
                    c += *++rij;
                    uv += c;
                    if (uv < c)
                    {
                        // carried
                        c2 += 1<<BASEBITS;
                    }
                    
                    *rij = uv;
                    c = (uv >> BASEBITS) + c2;
                }

                c += *++rij;
                *rij = c;
                c >>= BASEBITS;
                if (c)
                    *++rij = c;

                ++ai;
            }
            if (!r[t2]) --*r;
        }
    }

#if 0
    {
        // XXX check answer is correct!
        Big* c = mulBig(a, a);
        if (compare(c, r))
        {
            printf("square doesn't match\n");
            destroyBig(r);
            r = c;
        }
        else
            destroyBig(c);
    }
#endif
    return r;
}


Big* mulBigMod(Big* a, Big* b, Big* m)
{
    /* signed a * b (mod m) */
    Big* v = mulBig(a, b);
    if (v)
    {
        int s = compareu(v, m);
        if (s > 0) 
        {
            Big* r;
            destroyBig(divBig(v, m, &r));
            destroyBig(v);
            v = r;
        }
    }
    return v;
}

Big* rshiftn(Big* b, unsigned int n)
{
    Big* a = copyBig(b);
    if (n && a)
    {
        int d = n & (BASEBITS-1);
        while (d)
        {
            --d;
            rshiftBig(a);
        }

        d = n >> BASEBITSBITS;
        if (d)
        {
            // drop `d' digits
            int c = DIGITS(a);
            int neg = NEGATIVE(a) != 0;
            int i = 1 + d;
            int j = 1;
            c -= d;
            SET_HEAD(a, neg, c);
            while (c)
            {
                --c;
                a[j++] = a[i++];
            }
        }
    }
    return a;
}

Big* rshiftBig(Big* b)
{
    /* DESTRUCTIVE, halve b*/
    unsigned int n;
    unsigned long c, v;
    Big* p;

    if (!b || ISZERO(b)) return b;
    if (ISONE(b)) return constZero;
    
    n = DIGITS(b);
    p = b + n;
    c = 0;
    for (;;) 
    {
        v = *p;
        *p = (Big)((v >> 1) | (c << (BASEBITS-1)));
        --p;

        if (p == b) break;
        c = v & 1;
    }

    if (!b[n]) --*b;
    return b;
}

Big* divBig(Big* a,Big* b,Big** rem)
{
    /* a / b, if (rem) *rem = remainder, signed.
     *
     * USE Knuth Algorithm D.
     */
    unsigned int nm,n,m,d;
    Big* u;
    Big* v;
    Big* quo;
    int neg;
    unsigned int l, j;
    Big* dr;
    Big* dp;
    Big* dp2;
    int carry;
    int ts;
    unsigned int tu;
    unsigned int q;
    int agb;
    Big* r;

    if (rem) *rem = 0;

    if (!a || !b)
        return 0; // bail

    if (ISZERO(b)) 
    {
        if (rem) *rem = constZero;
        return constZero;
    }

    neg = SIGNBIT(a) ^ SIGNBIT(b);

    agb = compareu(a, b);
    if (!agb) 
    {
        quo = constOne;
        if (neg) quo = negateBig(quo);
        if (rem) *rem = constZero;
        return quo;
    }
    else if (agb < 0) 
    {
        quo = constZero;
        if (rem) *rem = copyBig(a);
        return quo;
    }

    nm = DIGITS(a);
    n = DIGITS(b);
    m = nm-n;

    if (rem) 
    {
        r = _create(n);
        if (!r) 
            return 0; // bail
    }
    
    quo = _create(m + 1);
    if (!quo) 
    {
        destroyBig(r);
        return 0;  // bail
    }

    SET_SIGNIF(quo, neg);

    if (n == 1) 
    {
        /* proceed manually for single digit divides */
        d = b[1];
        l = nm;
        dr = quo + l;
        dp = a + nm;
        carry = 0;
        while (l--) 
        {
            tu = (carry<<BASEBITS) + (*dp--);
            *dr = tu/d;
            carry = tu - (*dr--)*d;
        }
        if (rem) r[1] = carry;
    }
    else 
    {
        /* Normalise,
         *
         * d  := base/(V1+1),
         * (U0U1U2... Um+n) <- d * (U1U2.. Um+n),
         * (V1V2.. Vn) <- d * (V1V2.. Vn).
         */

        u = _create(nm + 1);
        v = _create(n);

        if (!u || !v)
        {
            // bail.
            destroyBig(quo);
            destroyBig(r);
            return 0;
        }

        d = BBASE/(b[n]+1);

        dp = a + 1;
        l = nm;
        carry =0;
        dr = u + 1;
        while (l)
        {
            tu = (*dp++)*d + carry;
            carry = tu>>BASEBITS;
            *dr++ = tu;
            l--;
        }

        *dr = carry;

        dp = b + 1;
        dr = v + 1;
        l = n;
        carry = 0;
        while (l) 
        {
            tu = (*dp++)*d + carry;
            carry = tu >> BASEBITS;
            *dr++ = tu;
            l--;
        }

        for (j = 0; j <= m; ++j) 
        {
    
            /* calculate normq,
             *
             * if ( Uj == V1)   normq <- base -1
             * else
             * normq <- (Uj*base + U(j+1)) / V1
             * while (V2*normq > (Uj*base + U(J+1) - normq*V1)*base + U(j+2))
             * normq <- normq - 1
             */               

            tu = (u[nm-j+1]<<BASEBITS)+u[nm-j];
            if (u[nm-j+1] == v[n]) q = BBASE-1;
            else q = tu/v[n];

            for (;;)
            {
                unsigned int ta = v[n-1]*q;
                unsigned int tb = tu-q*v[n];
                if (tb>>BASEBITS) break;

                if (ta > ((tb<<BASEBITS) + u[nm-j-1])) q--;
                else break;
            }
    
            if (!q) 
            {
                quo[m - j + 1] = 0; /* and this digit is also zero */
                continue;      /* no need since all zero */
            }
    
            /* multiply & subtract,
             *
             * (UjU(j+1)... U(j+n)) <- (Uj..U(j+n)) - normq * (V1.. Vn)
             */
            dp = &u[nm-n-j+1];
            l = n;
            dp2 = v + 1;
            carry = 0;
            while (l)
            {
                unsigned int t1 = (*dp2++)*q;
                unsigned int cm = -carry;

                unsigned int t2 = t1 + cm;
                int ov1 = 0;
                int ov2 = 0;

                if (t2 < t1 || t2 < cm) {
                    ov1 = 1;
                }

                if (*dp < t2) ov2 = 1;

                ts =  (*dp) - t2;

                if (ov1 || ov2) {

                    if (!ov1) {
                        carry = (ts>>BASEBITS);
                        carry |= 0xffff0000;
                    }
                    else {
                        if (ov2) {
                            carry = (ts>>BASEBITS);
                            carry -= BBASE;
                        }
                        else {
                            ts = -ts;
                            carry = (ts>>BASEBITS);
                        }
                    }
                }
                else carry = 0;
                *dp++ = ts;
                l--;
            }
            ts = *dp + carry;
            if (ts < 0) 
                carry = 1;

            else carry = 0;
            *dp = ts;

            /* any 'carry' left over is the net borrow 
             *
             * test remainder,
             * Qj <- normq
             * if (step_d4 >= 0 ) continue
             */
            quo[m-j+1] = q;
            if (carry) 
            {
                quo[m-j+1]--;
                dp = &u[nm-j-n+1];
                l = n;
                dp2 = v+1;
                carry = 0;
                while (l--) 
                {
                    tu = (*dp) + (*dp2++) + carry;
                    carry = tu>>BASEBITS;
                    *dp++ = tu;
                }
                tu = (*dp) + carry;
                tu -= BBASE;
                *dp = tu;
            }
        }
    
        /* unnormalize,
         * remainder <- (U(m+1)... U(m+n)) / d
         */

        if (rem) 
        {
            l = n;
            carry = 0;
            dp = &u[nm - m];
            dr = r + n;
            while (l--) {
                ts = (carry<<BASEBITS) + (*dp--);
                *dr = ts/d;
                carry = ts - (*dr--) * d;
            }
        }

        destroyBig(u);
        destroyBig(v);
    }

    dp = quo + m + 1;
    while (!*dp--) --*quo;
    if (rem) 
    {
        dp = r + n;
        while (!*dp && dp-1 != r) 
        {
            --*r;
            --dp;
        }

        if (ISZERO(r)) 
        {
            destroyBig(r);
            *rem = constZero;
        }
        else
        {
            SET_SIGNOF(r, a);
            *rem = r;
        }
    }

#if 0
    /* check */
    if (rem) {
        Big* t = mulBig(quo, b);
        Big* tt = addBig(t, *rem);
        int v;
        destroyBig(t);
        v = compareu(a, tt);
        if (v) {
            printf("divide got it wrong\n");
        }
    }
#endif

    return quo;
}

Big* gcdBigu(Big* a, Big* b)
{
    /* (|a|,|b|), |a| >= |b|, b != 0 */

    Big* r;
    Big* u = 0;
    Big* v;

    if (ISONE(b)) return constOne;

    destroyBig(divBig(a, b, &r));
    if (r)
    {
        u = b;
        v = r;

        if (ISZERO(v)) u = copyBig(u);
        else 
        {
            destroyBig(divBig(u, v, &r));
            u = 0;
            if (r)   // else destroy v and bail
            {
                u = v;
                v = r;

                while (!ISZERO(v)) 
                {
                    destroyBig(divBig(u, v, &r));
                    destroyBig(u); u = 0;
                    if (!r)
                        break;  // destroy v and bail.

                    u = v;
                    v = r;
                }
            }
        }
        destroyBig(v);
    }
    return u;
}


Big* gcdBig(Big* a, Big* b)
{
    /* (|a|,|b|) */

    Big* u = 0;
    Big* v;

    if (a && b)
    {
        int agb = compareu(a, b);
        if (agb >= 0) 
        {
            u = a;
            v = b;
        }
        else 
        {
            u = b;
            v = a;
        }

        if (ISZERO(v)) u = copyBig(u);
        else 
            u = gcdBigu(u, v);

        if (u && SIGNBIT(u)) u = negateBig(u);
    }
    return u;
}

unsigned int _decimalLength(Big* b)
{
    unsigned int l = _bitlength(b);
    unsigned int dl;
    
    if (l < 142676) {
        dl = (l*30103)/100000 + 1;
    }
    else {
        dl = l;
    }

    return dl;
}

unsigned int log10(Big* b)
{
    unsigned int l = 0;
    if (b) _decimalLength(b);
    if (l) --l;
    return l;
}

void asString(Big* b, streamFn sf, void* stream)
{
    /* convert b to decimal string */
    
    if (ISZERO(b)) (*sf)(stream, '0');
    else {
        Big* tens = createBig(10000);
        Big* n;
        Big* m;
        Big* r;
        unsigned int dl;
        char* p;
        char* buf;
        
        dl = _decimalLength(b) + 4;
        p = buf = (char*)malloc(dl);
        
        if (NEGATIVE(b)) (*sf)(stream, '-');

        n = b;
        for (;;) {
            int done;
            unsigned int t, d;

            m = divBig(n, tens, &r);
            
            if (n != b) destroyBig(n);
            n = m;

            done = ISZERO(n);
            
            d = r[1];
            t = d/1000; d -= t*1000;
            p[3] = t + '0';
            t = d/100; d -= t*100;
            p[2] = t + '0';
            t = d/10; d -= t*10;
            p[1] = t + '0';
            p[0] = d + '0';
            
            p += 4;
            while (done && p[-1] == '0') --p;
            
            destroyBig(r);

            if (done) break;
        } 
        destroyBig(n);
        destroyBig(tens);


        do {
            (*sf)(stream, *--p);
        } while (p != buf);

        free(buf);
    }
}

Big* powerBig(Big* b, unsigned int n)
{
    /* b^n, n >= 0 */
    Big* y;
    Big* z;
    Big* t;

    if (!n) return constOne;
    if (n == 1) return copyBig(b);
    
    y = constOne;
    z = b;
    
    for (;;) 
    {
        unsigned int c = n & 1;
        n >>= 1;
        if (c) 
        {
            t = mulBig(z, y);
            destroyBig(y);
            y = t;
            
            if (!y || !n) break;
        }

        t = mulBig(z, z);
        if (z != b)
            destroyBig(z);
        z = t;

        if (!z) 
        {
            destroyBig(y); y = 0;
            break;
        }
    }

    if (z != b) 
        destroyBig(z); 

    return y;
}

Big* powerBigMod(Big* b, Big* n0, Big* m)
{
    /* b^ n (mod m), b < m */
    Big* y;
    Big* z;
    Big* t;
    Big* n;

    if (!b || !n0 || !m) return 0;
    
    if (ISZERO(n0)) return constOne;
    if (ISONE(n0)) return copyBig(b);
    
    y = 0;
    z = b;
    n = copyBig(n0);
    if (n)
    {
        y = constOne;
        for (;;) 
        {
            unsigned int c = ISODD(n);
            n = rshiftBig(n);
            if (c) 
            {
                t = mulBigMod(z, y, m);
                destroyBig(y);
                y = t;
                if (!y || ISZERO(n)) break;
            }

            t = mulBigMod(z, z, m);
            if (z != b) destroyBig(z);
            z = t;
            if (!z)
            {
                destroyBig(y); y = 0;
                break;
            }
        }

        if (z != b) destroyBig(z);
        destroyBig(n);
    }
    return y;
}

Big* powerBigintMod(Big* b, unsigned int n, Big* m)
{
    /* b^ n (mod m), b < m */
    Big* y;
    Big* z;
    Big* t;
    
    if (!n) return constOne;
    if (n == 1) return copyBig(b);
    
    y = constOne;
    z = b;

    for (;;) 
    {
        unsigned int c = n&1;
        n >>= 1;
        if (c) 
        {
            t = mulBigMod(z, y, m);
            destroyBig(y);
            y = t;
            if (!y || !n) break;
        }

        t = mulBigMod(z, z, m);
        if (z != b) destroyBig(z);
        z = t;
        if (!z)
        {
            destroyBig(y); y = 0;
            break;
        }
    }

    if (z != b) destroyBig(z);
    return y;
}

Big* rootBig(Big* n)
{
    int t2gx;
    Big* x;
    Big* t1;
    Big* t2;

    if (!n || isNeg(n)) return 0;
    if (ISONE(n)) return constOne;
    if (ISZERO(n)) return constZero;

    x = copyBig(n);
    if (x) 
    {
        x = rshiftBig(x);
        while (x)
        {
            t1 = divBig(n, x, 0);
            t2 = addBig(t1, x);
            destroyBig(t1);
            if (t2)
            {
                t2 = rshiftBig(t2);
                t2gx = compareu(t2, x);
                if (t2gx >= 0) 
                {
                    destroyBig(t2);
                    break;
                }
            }
            destroyBig(x);
            x = t2;
        }
    }
    return x;
}

int bigAsInt(Big* b)
{
    unsigned int d = DIGITS(b);
    int v;
    
    if (d > 2) {
        if (NEGATIVE(b)) v = 0x80000000;
        else v = 0x7fffffff;
    }
    else {
        unsigned int u = b[1];
        if (d > 1) {
            u += ((unsigned int)b[2]) << BASEBITS;
        }
        if (!NEGATIVE(b)) {
            if (u > 0x7fffffff) u = 0x7fffffff;
            v = (int)u;
        }
        else {
            if (u > 0x80000000) u = 0x80000000;
            v = (int)u;
            v = -v;
        }
    }
    return v;
}


unsigned int bigAsUint(Big* b)
{
    unsigned int d = DIGITS(b);
    unsigned int u;
    
    if (NEGATIVE(b)) u = 0;
    else 
    {
        if (d > 2) u = 0xffffffff;
        else {
            u = b[1];
            if (d > 1) u += ((unsigned int)b[2]) << BASEBITS;
        }
    }
    return u;
}

/** BigFrac ******************************************************/

#if 0
static BigFrac* _createFrac(Big* u,Big* v)
{
    BigFrac* f = (BigFrac*)malloc(sizeof(BigFrac));
    f->x_ = u;
    f->y_ = v;
    return f;
}
#endif

static BigFrac* _normFrac(BigFrac* f)
{
    /* DESTRUCTIVE, f=x/y, x & y divided by (x,y) */
    Big* t;
    if (!ISONE(f->y_)) {
        Big* d = gcdBig(f->x_, f->y_);
        if (!ISONE(d)) {
            t = divBig(f->x_, d, 0);
            destroyBig(f->x_);
            f->x_ = t;

            t = divBig(f->y_, d, 0);
            destroyBig(f->y_);
            f->y_ = t;
        }
        destroyBig(d);
    }
    return f;
}

#if 0
BigFrac* createFrac(Big* u,Big* v)
{
    /* create new frac, normalise */
    BigFrac* f = _createFrac(u, v);
    _roundFrac(f);
    return _normFrac(f);
}

BigFrac* createFracInt(int v)
{
    /* create frac from signed int */
    return _createFrac(createBig(v), constOne);
}
#endif

void destroyFrac(BigFrac* f)
{
    destroyBig(f->x_); f->x_ = 0;
    destroyBig(f->y_); f->y_ = 0;
}

bool  _roundFrac(BigFrac* f)
{
    /* DESTRUCTIVE, f=x/y, ensure y > 0 
     * also check both parts are valid.
     */

    if (!f->x_ || !f->y_)
    {
        destroyFrac(f);
        return false;
    }
    else
    {
        if (SIGNBIT(f->y_)) 
        {
            NEGATEBIT(f->y_);
            f->x_ = negateBig(f->x_);
        }
        
        if (ISONE(f->y_)) 
        {
            destroyBig(f->y_);
            f->y_ = constOne;
        }
    }
    return true;
}

bool copyFrac(BigFrac* f, BigFrac* c)
{
    /* duplicate frac */
    c->x_ = copyBig(f->x_);
    c->y_ = copyBig(f->y_);
    return _roundFrac(c);
}

static bool addsubFrac(BigFrac* u,BigFrac* v, Big* (*addsubfn)(Big*,Big*),
                       BigFrac* c)
{
    /* u +/- v, signed */
    Big* d1;
    Big* t1;
    Big* t2;
    Big* t3;
    Big* t4;
    Big* x = 0;
    Big* y = 0;

    d1 = gcdBig(u->y_, v->y_);
    if (d1)
    {
        if (ISONE(d1)) 
        {
            t1 = mulBig(u->x_, v->y_);
            t2 = mulBig(u->y_,v->x_);

            x = (*addsubfn)(t1,t2);

            destroyBig(t1);
            destroyBig(t2);

            y = mulBig(u->y_, v->y_);
        }
        else 
        {
            t2 = divBig(v->y_, d1, 0);
            t1 = mulBig(t2, u->x_);
            destroyBig(t2);

            t2 = divBig(u->y_, d1, 0);
            t3 = mulBig(t2, v->x_);

            t4 = (*addsubfn)(t1, t3);
            destroyBig(t3);
            destroyBig(t1);
        
            t3 = gcdBig(t4, d1);
        
            x = divBig(t4, t3, 0);
            destroyBig(t4);

            t1 = divBig(v->y_, t3, 0);
            destroyBig(t3);
            y = mulBig(t2, t1);
            destroyBig(t1);
            destroyBig(t2);
        }
        destroyBig(d1);
    }

    c->x_ = x;
    c->y_ = y;
    return _roundFrac(c);
}

bool addFrac(BigFrac* u,BigFrac* v, BigFrac* c)
{
    /* u + v, signed */
    return addsubFrac(u, v, addBig, c);
}

bool subFrac(BigFrac* u,BigFrac* v, BigFrac* c)
{
    /* u - v, signed */
    return addsubFrac(u, v, subBig, c);
}

bool mulFrac(BigFrac* u,BigFrac* v, BigFrac* c)
{
    /* u * v, signed */
    Big* d1;
    Big* d2;
    Big* t1;
    Big* t2;
    Big* x;
    Big* y;

    d1 = gcdBig(u->x_, v->y_);
    d2 = gcdBig(u->y_, v->x_);
    
    t1 = divBig(u->x_, d1, 0);
    t2 = divBig(v->x_, d2, 0);
    
    x = mulBig(t1, t2);
    destroyBig(t1);
    destroyBig(t2);

    t1 = divBig(u->y_, d2, 0);
    destroyBig(d2);
    t2 = divBig(v->y_, d1, 0);
    destroyBig(d1);

    y = mulBig(t1, t2);
    destroyBig(t1);
    destroyBig(t2);

    c->x_ = x;
    c->y_ = y;
    return _roundFrac(c);
}

bool divFrac(BigFrac* u,BigFrac* v, BigFrac* c)
{
    /* u / v, signed */
    Big* d1;
    Big* d2;
    Big* t1;
    Big* t2;
    Big* x = 0;
    Big* y = 0;

    if (v->x_ && !ISZERO(v->x_)) 
    {
        d1 = gcdBig(u->x_, v->x_);
        d2 = gcdBig(u->y_, v->y_);
    
        t1 = divBig(u->x_, d1, 0);
        t2 = divBig(v->y_, d2, 0);
    
        x = mulBig(t1, t2);
        destroyBig(t1);
        destroyBig(t2);

        t1 = divBig(u->y_, d2, 0);
        destroyBig(d2);
        t2 = divBig(v->x_, d1, 0);
        destroyBig(d1);

        y = mulBig(t1, t2);
        destroyBig(t1);
        destroyBig(t2);
    }
    
    c->x_ = x;
    c->y_ = y;
    return _roundFrac(c);
}

bool modFrac(BigFrac* u,BigFrac* v, BigFrac* c)
{
    bool res = false;
    BigFrac a;
    if (divFrac(u, v, &a))
    {
        Big* q = divBig(a.x_, a.y_, 0);
        destroyFrac(&a);
        if (q)
        {
            a.x_ = mulBig(q, v->x_);
            a.y_ = copyBig(v->y_);
            destroyBig(q);
            if (_roundFrac(&a))
            {
                if (_normFrac(&a) && _roundFrac(&a))
                    res = subFrac(u, &a, c);
                destroyFrac(&a);
            }
        }
    }
    return res;
}


void invertFrac(BigFrac* f)
{
    /* DESTRUCTIVE, 1/f */
    Big* t = f->x_;
    if (t)
    {
        f->x_ = f->y_;
        f->y_ = t;
        _roundFrac(f);
    }
}

bool powerFracInt(BigFrac* f, unsigned int n, BigFrac* c)
{
    /* f^n, n >= 0 */
    bool res = true;
    if (!n) 
    {
        c->x_ = constOne;
        c->y_ = constOne;
    }
    else if (n == 1) 
        res = copyFrac(f, c);
    else 
    {
        BigFrac y;
        BigFrac z;
        BigFrac t;
           
        y.x_ = constOne;
        y.y_ = constOne;

        copyFrac(f, &z);
        
        for (;;) 
        {
            unsigned int c = n & 1;
            n >>= 1;
            if (c) {
                mulFrac(&z, &y, &t);
                destroyFrac(&y);
                y = t;
            
                if (!FRACOK(&y) || !n) break;
            }

            mulFrac(&z, &z, &t);
            destroyFrac(&z);
            z = t;
            if (!FRACOK(&z)) 
            {
                destroyFrac(&y);
                break;
            }
        }

        destroyFrac(&z);
        *c = y;
        res = _roundFrac(c);
    }
    return res;
}


bool powerFrac(BigFrac* u, BigFrac* v, BigFrac* c)
{
    // u^v, v integral or 1/n 

    bool res;
    if (ISZERO(u->x_))
    {
        c->x_ = constOne;
        c->y_ = constOne;
        res = true;
    }
    else if (ISONE(v->x_))
    {
        // consider (a/b)^(1/q)
        unsigned int k = bigAsUint(v->y_);
        if (k > 0 && k != (unsigned int)-1)
            return nRootFrac(u, k, c);
    }
    else if (!ISONE(v->y_) || DIGITS(v->x_) > 2)
    {
        /* cant do it */
        res = false;
    }
    else 
    {
        int val = bigAsInt(v->x_);
        
        if (val < 0) val = -val;

        if (ISONE(u->y_)) 
        {
            c->x_ = powerBig(u->x_, val);
            c->y_ = constOne;
        }
        else 
            powerFracInt(u, val, c);

        if (NEGATIVE(v->x_))
            invertFrac(c);

        res = _roundFrac(c);
    }
    return res;
}

void asStringFrac(const BigFrac* f, streamFn sf, void* stream)
{
    /* convert frac to decimal ratio */

    if (!f->x_) return; 
    
    if (ISONE(f->y_)) {
        asString(f->x_, sf, stream);
    }
    else {
        int xgy = compareu(f->x_, f->y_);
        if (xgy > 0) {
            Big* r;
            Big* n;

            n = divBig(f->x_, f->y_, &r);

            asString(n, sf, stream);

            if (!NEGATIVE(r)) {
                (*sf)(stream, '+');
            }
            
            destroyBig(n);
            asString(r, sf, stream);
            destroyBig(r);
        }
        else {
            asString(f->x_, sf, stream);
        }

        (*sf)(stream, '/');
        asString(f->y_, sf, stream);
    }
}

bool rootFrac(BigFrac* n, BigFrac* c)
{
    c->x_ = rootBig(n->x_);
    if (ISONE(n->y_)) 
        c->y_ = constOne;
    else 
        c->y_ = rootBig(n->y_);
    
    return _roundFrac(c);
}

bool nRootFrac(BigFrac* a, unsigned int k, BigFrac* c)
{
    bool res = false;
    if (ISONE(a->y_))
    {
        BigInt n(a->x_);
        BigInt rt = nRoot(n, k);

        // check we are still rational
        if (pow(rt, k) == n)
        {
            c->x_ = rt.give();
            c->y_ = constOne;
            res = true;
        }
        n.give();
    }
    return res;
}


unsigned int fracAsUint(BigFrac* f)
{
    unsigned int v;

    if (!NEGATIVE(f->x_)) {
        if (ISONE(f->y_)) {
            v = bigAsUint(f->x_);
        }
        else {
            Big* d = divBig(f->x_, f->y_, 0);
            v = bigAsUint(d);
            destroyBig(d);
        }
    }
    else v = 0;

    return v;
}

bool sameFrac(BigFrac* a, BigFrac* b)
{
    /* are two Fracs the same numerator AND denominator */
    return !compare(a->x_, b->x_) && !compare(a->y_, b->y_);
}

bool parallelFrac(BigFrac* a, BigFrac* b, BigFrac* c)
{
    bool res;
    if (ISZERO(a->x_) || ISZERO(b->x_))
    {
        c->x_ = constZero;
        c->y_ = constOne;
        res = true;
    }
    else
    {
        BigFrac inva;
        BigFrac invb;
        res = copyFrac(a, &inva) && copyFrac(b, &invb);
        if (res)
        {
            invertFrac(&inva);  // destructive
            invertFrac(&invb);  // destructive
            res = addFrac(&inva, &invb, c);
            if (res) invertFrac(c);

            destroyFrac(&inva);
            destroyFrac(&invb);
        }
    }
    return res;
}


/************ Algorithms ***************************************/

static unsigned int smallFactorMethod(Big* n, unsigned int limit)
{
    // use algorithm published in HPCC Datafile V28n5 p26, December 2009.

    // ASSUME n > 1
    // if we reach `limit' return value > limit

    // are we even?
    if (!ISODD(n)) return 2;

    // check 3, 5, 7, 11, 13 directly
    int i;
    int cc = 0;
    for (i = 3; i <= 13; i += 2)
    {
        Big* r;
        int z;
        Big* d;

        if (i == 9) continue; 
        if (i > limit) return i;

        d = createBigu(i);
        destroyBig(divBig(n, d, &r));
        if (!r) return 0; // fail
        z = ISZERO(r);
        destroyBig(r);
        destroyBig(d);
        if (z) return i;


    }
    
    // now start the trial shifter 
    // first divide the number by 16's. we will need a slot
    // for every 16.
    int nd = (_bitlength(n) + 3)/4; // number of 16s
    int* div = new int[nd];
    unsigned int d = 0;
    int j;

    if (div)
    {
        int* pp = div;
        for (i = 1; i <= DIGITS(n); ++i)
        {
            int v = n[i];
            bool on = i < DIGITS(n);
            for (j = 0; j < 4; ++j)
            {
                if (v || on) { *pp++ = v & 0xf; v >>= 4; }
            }
        }

#if 0
        // debug print
        for (i = 0; i < nd; ++i)
            printf("%d ", div[i]);
        printf("\n");
#endif

        // slide to 17
        d = 17;
        for (i = nd-2; i >= 0; --i)
        {
            for (j = i; j < nd-1; ++j)
            {
                if ((div[j] -= div[j+1]) < 0)
                {
                    div[j] += d;
                    --div[j+1];
                }
            }
            if (!div[j]) --nd;
        }

        // not divisible by 17
        // generic slide by 2
        while (div[0])
        {
            d += 2;
            if (d > limit) 
                break;
            for (i = nd-2; i >= 0; --i)
            {
                for (j = i; j < nd-1; ++j)
                {
                    int v = div[j] - div[j+1] - div[j+1];
                    if (v < 0)
                    {
                        v += d;
                        --div[j+1];
                        if (v < 0)
                        {
                            v += d;
                            --div[j+1];
                        }
                    }
                    div[j] = v;
                }
                if (!div[j]) --nd;
            }

            cc = (cc + 1) & 0xffff;
            if (!cc && EscapeKeyPressed()) return 0; // fail
        }
        delete [] div;
    }
    return d;
}

static unsigned long randomNumber;

void setrandomN(unsigned long v)
{
    randomNumber = v;
}

unsigned long randomN()
{
    randomNumber = randomNumber*1664525UL + 1013904223UL;
    return randomNumber;
}


unsigned int modBig(Big* n, unsigned int m)
{
    unsigned int mod;

    Big* r;
    Big* d = createBigu(m);
    destroyBig(divBig(n, d, &r));
    destroyBig(d);
    mod = bigAsUint(r);
    destroyBig(r);
    return mod;
}

bool BigToMF(Big* b, MF* er)
{
    unsigned int d = DIGITS(b);
    unsigned int i;
    int v;

    MF base((uint4)BBASE);
    MF bp(1);
    MF res(0);

    for (i = 1; i <= d; ++i)
    {
        v = b[i];
        MF dv(v);
        res += (dv*bp);
        bp *= base;
    }

    if (NEGATIVE(b)) 
    {
        res = -res;
    }

    *er = res;
    return true;
}

bool BigFracToMF(BigFrac* b, MF* mf)
{
    BigToMF(b->x_, mf);
    if (!ISONE(b->y_)) {
        MF d;
        BigToMF(b->y_, &d);
        *mf /= d;
    }
    return true;
}

bool factorFrac(const BigFrac* a, BigFrac* c)
{
    bool res = false;
    if (ISONE(a->y_)) 
    {
        c->y_ = constOne;

        // eliminate 0 or 1
        if (ISONE(a->x_))
        {
            c->x_ = constOne;
            res = true;
        }
        else if (ISZERO(a->x_))
        {
            c->x_ = constZero;
            res = true;
        }
        else if (!isNeg(a->x_))  // only > 0 for now
        {
            Big* r = rootBig(a->x_);

            // since x > 1, sqrt(x) >= 1. If too large, rl will be maxuint
            unsigned int rl = bigAsUint(r);
            destroyBig(r);
            
            // max we're prepared to count to
            unsigned int trialMax = 1<<20; 
            bool clamp = false;

            if (rl > trialMax)
            {
                rl = trialMax;
                clamp = true;
            }
            
            unsigned int u = smallFactorMethod(a->x_, rl);

            if (u <= rl)
            {
                if (!u) return false; // bail

                // found a factor
                c->x_ = createBigu(u);
                res = true;
            }
            else if (!clamp)
            {
                // we didn't find a factor up to limit but this limit
                // was less than max, therefore we are prime.
                c->x_ = constOne;
                res = true;
            }
            else
            {
                // try simple prime test
                BigInt n(a->x_);
                CompState st(n);
                //st.setReimannLimit();
                st.setDefaultLimit();
                
                // perform 50 tests
                //unsigned int mx = 50;
                bool prime = false;
                bool composite = false;

                for (;;)
                {
                    st.next();
                    if (st.done())
                    {
                        // we are prime or XXX assume prime
                        c->x_ = constOne;
                        prime = true;
                        res = true;
                        break;
                    }
                    
                    if (isOddCompositeAux(st)) 
                    {
                        composite = true;
                        break;
                    }
                }

                if (!prime)
                {
                    // try a bit of lenstra
                    BigInt f;
                    bool v = Lenstra(n, f, 500);
                    if (v)
                    {
                        c->x_ = f.give();
                        res = true;
                    }
                }

                n.give();
            }
        }
    }
    if (res) res = _roundFrac(c); // perform valid checks
    return res;
}

bool factorialFrac(const BigFrac* a, BigFrac* c)
{
    if (!ISONE(a->y_) || SIGNBIT(a->x_)) return false;

    c->y_ = constOne;
    c->x_ = constOne;

    bool res = true;
    unsigned int a1 = bigAsUint(a->x_);
    if (a1 > 1) 
    {
        if (a1 == 0xffffffff) 
            return false;

        Big* b = constOne;
        while (--a1 > 0) 
        {
            Big* t;

            t = addint(b, 1U);
            destroyBig(b);
            b = t;
            
            t = mulBig(c->x_, b);
            destroyBig(c->x_);
            c->x_ = t;

            if (!FRACOK(c))
            {
                destroyFrac(c);
                res = false;
                break;
            }
        }
        destroyBig(b);
    }
    return res;
}

bool BCDToFrac(const BCD& a, const BCD& b, BigFrac* c)
{
    BigInt n, d;
    BigInt q, r;

    BigInt cn = 1, pn = 0;
    BigInt cd = 0, pd = 1;
    BigInt n0, d0;
    BigInt x, y;

    d0 = 1;

    BCD b0 = fabs(b);

    bool neg = a.isNeg();
    BCD a0 = neg ? -a : a;

    if (b < BCD::epsilon(26)*a) return false;

    unsigned int v = 0;
    if (a0 >= 1)
    {
        v = itrunc(a0);
        if (!v) return false;
        a0 -= v;
    }
    n0 = v;

    while (b0 < 1) 
    {
        d0*=100;
        b0*=100;

        a0*=100;
        n0*=100;
        if (a0 >= 1)
        {
            v = itrunc(a0);
            n0 += v;
            a0 -= v;
        }
    }

    n = n0;
    d = d0;
    
    for (;;) 
    {
        q = n/d; r = n-q*d;
        x = q*cn + pn;
        y = q*cd + pd;
        pn = cn;
        cn = x;
        pd = cd;
        cd = y;
        n = d;
        d = r;
        if (fabs(n0*y - d0*x).toBCD() <= b*(d0*y).toBCD()) break;
    }

    BigInt hi;
    BigInt mid;
    BigInt lo = 0;
    
    if (q > 1) 
    {
        hi = q;
        for (;;) 
        {
            mid = (lo+hi)/2;
            x = cn - pn*mid;
            y = cd - pd*mid;
            if (fabs(n0*y - d0*x).toBCD() <= b*(d0*y).toBCD()) 
                lo = mid;
            else 
                hi = mid;

            if (hi - lo <= 1) break;
        }
        
        x = cn - pn*lo;
        y = cd - pd*lo;
    }

    if (neg) x.negate();

    // donate results to return
    c->x_ = x.give();
    c->y_ = y.give();
    return _roundFrac(c);
}
