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

#ifndef __rand_h__
#define __rand_h__

#include "defs.h"

/* provide alternative versions of rand() and srand() to 
 * link with that are known.
 * 
 * these implementations come from Numerical Recipes 3rd edition.
 * this is the simpler version p351.
 */

class Ranq1
{
public:
    
    mutable uint64_t v;

    Ranq1(uint64_t j = 0U) 
    //: v(4101842887655102017LL)
        : v(INT64(955034719U,3005552193U))
    {
        v ^= j;
        v = int64();
    }

    void seed(unsigned int s)
    {
        v = s;
    }

    void reset()
    {
        v = INT64(955034719U,3005552193U);
    }

    uint64_t int64() const
    {
        v ^= v >> 21;
        v ^= v << 35;
        v ^= v >> 4;
        //return v*2685821657736338717LL;
        return v*INT64(625341585U,1332534557U);
    }

    unsigned int int32() const { return (unsigned int)int64(); }

};

#undef RAND_MAX
#define RAND_MAX  0xffffffffUL
#define RAND_MAXLL  0xffffffffffffffffLL


#endif // __rand_h__
