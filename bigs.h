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

#ifndef __bigs_h__
#define __bigs_h__

#include "big.h"
#include "bcdmath.h"

#define DEFAULT_PRIMETEST_LIMIT 50

struct SmallDivs
{
    // generator of small trial divisors
    // return small divisors > 7, not multiples of 2, 3, 5

    SmallDivs() { _ai = 0; _d = 7; }

    unsigned int next()
    {
        static unsigned char add[] = {4, 2, 4, 2, 4, 6, 2, 6};
        _d += add[_ai++];
        _ai &= 0x7;
        return _d;
    }
    unsigned int        _ai;
    unsigned int        _d;
};

struct CompState
{
    // state of `isComposite' so we can continue testing.
    CompState(const BigInt& n) : _n(n)
    { 
        _r = 0;
        _nm1 = (_n - 1).give();
        _s = _nm1;
        _dn = 0;
        _limit = 0;

        // factor out 2's
        while (_s.isEven())
        {
            _s >>= 1;
            ++_r;
        }
    }

#if 0
    void setReimannLimit()
    {
        // min witness = min(floor(2ln(n)^2), n-1)
        
        if (_n < 30)
            _limit = _nm1.asUInt();
        else
        {
            BCD v = log(_n.asBCD());
            _limit = ifloor(v*v*2);
        }
    }
#endif

    void setDefaultLimit()
    {
        _limit = _nm1.asUInt();
        
        // NB: if too big, limit = -1U and this test passes
        if (_limit > DEFAULT_PRIMETEST_LIMIT)
            _limit = DEFAULT_PRIMETEST_LIMIT;
    }

    bool                done() const { return _dn > _limit; }

    void                next()
    {
        static unsigned char primes[] = 
            { 2, 0, 3, 5, 0, 7 };

        if (_dn < 7)
            _dn = primes[_dn];
        else
            _dn = _divs.next();
    }

    const BigInt&       _n;
    unsigned int        _limit;
    BigInt              _nm1; // n-1
    BigInt              _s; // n without 2's
    unsigned int        _r;
    unsigned int        _dn;
    SmallDivs           _divs;
};

int isPrime(const BigInt&);
bool Lenstra(const BigInt& n, BigInt& f, int ntrials);
bool isOddComposite(CompState& st);
bool nextPrime(const BigInt& n0, BigInt& np);
bool prevPrime(const BigInt& n0, BigInt& np);
bool isOddCompositeAux(CompState& st);
BigInt nRoot(const BigInt& n, unsigned int k);

#endif // __bigs_h__

