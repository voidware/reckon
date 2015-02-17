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

#include "bigs.h"

#ifdef _WIN32
#include "oswin.h"
#else
extern "C"
{
#include "common.h" // escape key
}
#endif


/* Algorithms with big numbers */

bool isOddCompositeAux(CompState& st)
{
    /* ASSUME n > 1, is odd.
     * put n = 2^r*s + 1.
     */
    // false means not necessarily composite, may be prime
    // true, definitely composite

    // peform one test
    bool res = false;

    /* find a^s mod n */
    BigInt v = powMod(BigInt(st._dn), st._s, st._n);
    if (v != 1 && v != st._nm1)
    {
        /* for j = 1 .. r-1, find (a^s)^(2^j) mod n */
        unsigned int j;
        bool hit = false;
        for (j = 1; j < st._r; ++j)
        {
            v = sqr(v) % st._n;
            if (v == st._nm1) { hit = true; break; }
        }
        res = !hit;
    }
    return res;
}

bool isOddComposite(const BigInt& n)
{
    CompState st(n);
    st.setDefaultLimit();

    for (;;)
    {
        st.next();
        if (st.done()) break;
        if (isOddCompositeAux(st)) return true;
    }
    return false; // not composite
}

bool isComposite(const BigInt& n)
{
    // n > 2
    // true iff composite

    // make sure we're not even!
    if (n.isEven())
        return true;

    return isOddComposite(n);
}

int isPrime(const BigInt& n)
{
    // 1 => prime
    // 0 => not prime (composite unless 1)
    // -1 => dont know

    if (n == 1) return 0;
    if (n <= 0) return -1;
    if (n == 2) return 1;
    
    return !isComposite(n);
}

struct SmallPrimes
{
    // generate small, odd primes
    SmallPrimes()
    {
        _p = 1;
        _rp = 1;
        _rp2 = 4;
        _ai = 0;
    }

    unsigned int next()
    {
        // start after the number 7
        static int add[] = {4, 2, 4, 2, 4, 6, 2, 6};

        if (_p < 7)
        {
            // generate 3, 5, 7
            _p += 2;
        }
        else
        {
        again:
            // then > 7, not multiples of 2, 3, 5
            _p += add[_ai++];
            _ai &= 0x7;
        }
            
        // adjust root
        if (_p >= _rp2)
        {
            ++_rp;
            _rp2 = _rp + 1;
            _rp2 *= _rp2;
        }

        // now try small divisors up to sqroot
        SmallDivs sd;
        unsigned int d = 7;

        while (d <= _rp)
        {
            if (_p % d == 0) goto again;
            d = sd.next();
        }
        return _p;
    }
    
    unsigned int        _ai;
    unsigned int        _p;     // current prime
    unsigned int        _rp;    // floor(root p)
    unsigned int        _rp2;   // [floor(root p) + 1]^2
};

/// here define class needed for ECM
struct ECurve
{
    ECurve(const BigInt& n) : _n(n) {}

    const BigInt&       _n;
    BigInt              _a;
};


struct EPoint
{
    EPoint(ECurve& c) : _c(c) {}

    bool valid() const { return _x.valid() && _z.valid(); }

    friend void addh(const EPoint& p1, const EPoint& p2, const EPoint& p3,
                     EPoint& res)
    {
        const BigInt& n = p1._c._n;
        res._x = (p1._x*p2._x - p1._z*p2._z) % n;
        res._x = ((sqr(res._x) % n) * p3._z) % n;
        res._z = (p1._x*p2._z - p2._x*p1._z) % n;
        res._z = ((sqr(res._z) % n) * p3._x) % n;
    }

    friend void twiceh(const EPoint& p, EPoint& res)
    {
        // MODS:5, MUL:6, ADDS:3
        const BigInt& n = p._c._n;
        BigInt x2 = sqr(p._x);  // defer mod
        BigInt z2 = sqr(p._z);

        res._x = (x2 - z2) % n;
        res._x = sqr(res._x) % n;
        BigInt xz = p._x*p._z % n;
        res._z = (x2 + xz*p._c._a + z2) % n;
        res._z = ((res._z * xz)<<2) % n;
    }

    friend bool mulnh(const EPoint& p, unsigned int n, EPoint& res)    
    {
        // algorithm 7.2.7
        // ASSUME n > 2
        bool v = true;

        EPoint u = p;
        EPoint t(p._c); twiceh(p, t);

        // find high bit
        int i = 31;
        while ((n & 0x80000000) == 0)
        {
            n <<= 1;
            --i;
        }

        // ignore top bit
        n <<= 1;
        --i;

        EPoint tp(p._c);
        while (i)
        {
            if (n & 0x80000000)
            {

                addh(t, u, p, tp);
                u = tp;

                twiceh(t, tp);
                t = tp;
            }
            else
            {
                addh(u, t, p, tp);
                t = tp;
                    
                twiceh(u, tp);
                u = tp;
            }
            n <<= 1;
            --i;
        }

        if (n)
            addh(u, t, p, res);
        else
            twiceh(u, res);
        return v;
    }

    void operator=(const EPoint& p) {_x = p._x;  _z = p._z; }

    BigInt              _x;
    BigInt              _z; 
    ECurve&             _c;
};

bool Lenstra(const BigInt& n, BigInt& f, int ntrials)
{

    // n is odd
    // n is not divisible by 3
    // n not a power.

    // eliminate perfect square
    BigInt k = sqrt(n);
    if (k*k == n)
    {
        f = k;
        return true;
    }

    // eliminate powers of small primes
    SmallPrimes ps;
    unsigned int r;
    while ((r = ps.next()) <= 7)
    {
        BigInt rt = nRoot(n, r);
        if (pow(rt, r) == n)
        {
            f = rt;
            return true;
        }
    }
    
    // make sequence repeatable
    setrandomN(1);

    ECurve ec(n);
    int cc;
    unsigned int b1 = 500;

    EPoint p(ec);
    for (cc = 0; cc < ntrials; ++cc)
    {
        // check for escape
        if (EscapeKeyPressed()) return false;

#ifdef _WIN32
        //std::cout << "pass " << cc << std::endl;
#endif

        BigInt sg = randomN() + 6;
        if (sg >= n) sg %= n;
        
        BigInt u = (sqr(sg) - 5) % n;
        BigInt v = (sg<<2) % n;

        BigInt t = subMod(v, u, n);
        t = (((sqr(t)% n)*t % n) * ( u*3 + v)) % n;

        p._x = ((sqr(u)%n)*u) % n;  // u^3

        BigInt d = ((p._x* v)<<2) % n;
        BigInt g;
        d = invert(d, n, g);

        if (g != 1)
        {
            f = g;
            return true;
        }

        ec._a = mod(t*d - 2, n);
        p._z = ((sqr(v)%n) * v) % n;  // v^3

        // go through all the primes < b1
        SmallPrimes sp;
        unsigned int pi = 2;
        EPoint r(p._c);

        for (;;)
        {
            unsigned int pin = pi;
            
            for (;;)
            {
                unsigned int t = pin*pi;
                if (t > b1) break;
                pin = t;
            }
            
            mulnh(p, pin, r);

            if (!r.valid())
                return false; // arithmetic bailed
            p = r;

            pi = sp.next();
            if (pi > b1) break;
        }

        // invert z
        g = gcd(p._z, n);
        if (g != 1 && g != n)
        {
            f = g;
            return true;
        }
        
        // slightly increase b1 for each curve
        b1 += 50;
    }
    return false;
}

bool nextPrime(const BigInt& n0, BigInt& np)
{
    if (n0 < 1) np = 2;
    else
    {
        BigInt n;
        if (n0.isEven()) n = n0 + 1;
        else n = n0 + 2;
        while (isOddComposite(n))
            n += 2;
        np = n;
    }
    return true;
}

bool prevPrime(const BigInt& n0, BigInt& np)
{
    if (n0 > 3)
    {
        BigInt n;
        if (n0.isEven()) n = n0 - 1;
        else n = n0 - 2;
        while (isOddComposite(n))
            n -= 2;
        np = n;
    }
    else
    {
        if (n0 == 3) np = 2;
        else np = 0;
    }
    return true;
}

BigInt nRoot(const BigInt& n, unsigned int k)
{
    unsigned int b = (log2(n) + 1);
    unsigned int bx = (b + k - 1)/k;
    BigInt x = Big(1)<<bx;
    BigInt x1;
    for (;;)
    {
        x1 = ((k-1)*x + n/pow(x,k-1))/k;
        if (x1 >= x)
            break;

        x = x1;
    }
    return x;
}
