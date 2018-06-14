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

#ifndef __mi_h__
#define __mi_h__

#include "bcd.h"

typedef unsigned short Big;
typedef BCD MF;

#define BASEBITS                        16
#define BASEBITSBITS                    4
#define BBASE                           0x10000L

#define DIGITS(_x)                      ((_x)[0] & 0x7fff)
#define NEGATIVE(_x)                    ((_x)[0] & 0x8000)
#define ASINT(_x)                       (NEGATIVE(_x) ? -(_x)[1] : (_x)[1])
#define ISZERO(_x)                      ((_x)[0] == 1 && (_x)[1] == 0)
#define ISONE(_x)                       ((_x)[0] == 1 && (_x)[1] == 1)
#define ISTWO(_x)                       ((_x)[0] == 1 && (_x)[1] == 2)
#define ISODD(_x)                       ((_x)[1] & 1)

#define SIGNBIT(_x)                     ((_x)[0] >> 15)
#define SET_HEAD(_x, _s, _d)            ((_x)[0] = ((_s) << 15) | (_d))
#define SET_SIGNIF(_x, _s)              ((_x)[0] |= ((_s) << 15))
#define SET_SIGNOF(_x, _y)              ((_x)[0] |= ((_y)[0] & 0x8000))
#define RESET_SIGN(_x)                  ((_x)[0] & 0x7fff)
#define NEGATEBIT(_x)                   ((_x)[0] ^= 0x8000)

void    InitBig();
void    finishBig();

/* Forward Decls */
struct BigFrac;

typedef void (*streamFn)(void* stream, char);

/* creative */
Big*    parseBig(const char** s);
Big*    createBig(long);
Big*    createBigu(unsigned long);
Big*    addBig(Big*,Big*);
Big*    subBig(Big*,Big*);
Big*    mulBig(Big*,Big*);
Big*    divBig(Big*, Big*, Big** rem);
Big*    copyBig(Big*);
Big*    gcdBig(Big*, Big*);
Big*    powerBig(Big*, unsigned int);
Big*    lshiftn(Big* b, unsigned int n);
Big*    rshiftn(Big* b, unsigned int n);
Big*    rootBig(Big*);
Big*    mulBigMod(Big* a, Big* b, Big* m);
Big*    powerBigMod(Big* b, Big* n, Big* m);
Big*    powerBigintMod(Big* b, unsigned int n, Big* m);
Big*    addint(Big* a, unsigned int b);
Big*    subint(Big* a, unsigned int b);
Big*    mulint(Big*, int);
Big*    addint(Big* a, int b);
int     bigAsInt(Big* b);
Big*    one();
Big*    zero();
Big*    sqrBig(Big*);
unsigned int bigAsUint(Big* b);
unsigned int log2(Big*);
unsigned int log10(Big*);
inline bool    isNeg(Big* a) { return a && SIGNBIT(a) != 0; }
inline bool    isZero(Big* a) { return !a || ISZERO(a); }

/* destructive */
Big*    negateBig(Big*);
Big*    lshiftBig(Big*, int dup);

Big*    rshiftBig(Big* b);
void    destroyBig(Big*);

/* features */
void            asString(Big*, streamFn, void*);
int             compare(Big* a, Big* b);
int             compareu(Big* a, Big* b);

/* BigFrac */
// BigFrac*        createFrac(Big* u,Big* v);
// BigFrac*        createFracInt(int v);
bool            copyFrac(BigFrac* f, BigFrac* c);
bool            addFrac(BigFrac*,BigFrac*, BigFrac* c);
bool            subFrac(BigFrac*,BigFrac*, BigFrac* c);
bool            mulFrac(BigFrac*,BigFrac*, BigFrac* c);
bool            divFrac(BigFrac*,BigFrac*, BigFrac* c);
bool            modFrac(BigFrac*,BigFrac*, BigFrac* c);
bool            powerFracInt(BigFrac* b, unsigned int n, BigFrac* c);
bool            powerFrac(BigFrac*,BigFrac*, BigFrac*);
bool            rootFrac(BigFrac*, BigFrac* c);
bool            nRootFrac(BigFrac* a, unsigned int k, BigFrac* c);
unsigned int    fracAsUint(BigFrac*);
void            asStringFrac(const BigFrac*, streamFn, void*);
bool            sameFrac(BigFrac* a, BigFrac* b);

/* destructive */
void            destroyFrac(BigFrac*);
void            invertFrac(BigFrac*);

struct BigFrac
{
    // Constructors
                        BigFrac() {}
                        BigFrac(long v);
                        BigFrac(Big* b);

    // Features
    bool                isZero() const { return ISZERO(x_); }
    
    Big*        x_;
    Big*        y_;
};

/* conversions */
bool BigToMF(Big* b, MF*);
bool BigFracToMF(BigFrac* b, MF*);
bool BCDToFrac(const BCD& a, const BCD& b, BigFrac* c);

unsigned long randomN();
void setrandomN(unsigned long v);

/* Other functions */
Big*                    pollardRho(Big* n,
                                   unsigned int pw,
                                   unsigned int* trials);
bool                    factorialFrac(const BigFrac* a, BigFrac* c);
bool                    factorFrac(const BigFrac* a, BigFrac* c);
bool                    parallelFrac(BigFrac* a, BigFrac* b, BigFrac* c);

#endif /* __mi__h */
