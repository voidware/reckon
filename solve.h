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

#ifndef __solve_h__
#define __solve_h__

#include "types.h"
#include "eeval.h"


// external interface
extern bool SolveN(TermRef& res, Term* t, const BCD& xmin, const BCD& xmax);

struct Solver: public ExprEvaluator
{
    
    Solver()
    {
        // default
        _eps = BCD::epsilon(18);
    }

    void setMinMax(const BCD& xmin, const BCD& xmax)
    {
        if (xmin <= xmax)
        {
            _x0 = xmin;
            _x1 = xmax;
        }
        else
        {
            _x1 = xmin;
            _x0 = xmax;
        }
    }

    const BCD& root() const { return _root; }
    
    void        eps(const BCD& e) { _eps = e; }
    bool        ridderSolve() { return _searchOut() && _ridderRoot(); }
    bool        ridderSolveOnly();

    bool        _ridderRoot();
    bool        _searchOut();

    BCD         _eval(const BCD& x)
    {
        BCD v;
        return eval(x, v) ? v : 0;
    }

    static bool derivAdapter(const BCD& x, BCD& val, ExprEvaluator& eeval);

protected:

    BCD         _x0;
    BCD         _x1;
    BCD         _root;
    BCD         _eps;
    BCD         _f0;
    BCD         _f1;
};





#endif // __solve_h__
