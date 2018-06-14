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

#ifndef __nran_h__
#define __nran_h__

#include "rand.h"

#define MAX_TOKENS      16

class NRan
{
    // Generate "non-random" numbers.

public:
    
    // Constructors
    NRan() { _init(); }

    // assign the random engine
    void ran(Ranq1* r) { _ran = r; }

    unsigned int nran(unsigned int n, unsigned int maxp)
    {
        unsigned int v = MAX_TOKENS;

        if (n >= MAX_TOKENS) n = MAX_TOKENS-1;
        if (!n) return 0;
        
        // look for forced ommission
        unsigned int i;
        unsigned int imin = 0;
        for (i = 1; i < n; ++i)
            if (_stats[i] > _stats[imin])
                imin = i;
        
        if (_stats[imin] + 1 >= maxp)
        {
            // forced
            v = imin;
        }
        else
        {
            // otherwise choose randomly
            v = _ran->int32() % n;
        }

        // adjust stats
        for (i = 0; i < n; ++i)
        {
            if (i == v)
                _stats[i] = 0;
            else
                ++_stats[i];
        }
        
        return v + 1;
    }

    void seed(unsigned int s)
    {
        _init();
        _ran->seed(s);
    }

private:

    void _init()
    {
        memset(_stats, 0, sizeof(unsigned int)*MAX_TOKENS);
    }
    
    Ranq1*        _ran;
    unsigned int _stats[MAX_TOKENS];
};

#endif // __nran_h__
