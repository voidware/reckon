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

#include "solve.h"

#ifdef _WIN32
#include "oswin.h"
#else
extern "C"
{
#include "common.h"
}
#endif

using namespace bcdmath;


#define MAX_ITERATIONS  100

bool SolveN(TermRef& res, Term* t, const BCD& xmin, const BCD& xmax)
{
    Solver solver;

    if (solver.setTerm(t))
    {
        solver.setMinMax(xmin, xmax);
        if (solver.ridderSolve())
            res = Float::create(DPD(solver.root()));
    }
    
    return false;
}

/** Solver **************************************************************/

bool Solver::_searchOut()
{
    // widen the seatch until bracket a root, else false.

    BCD dx = _x1 - _x0;
    if (dx == 0) _x1 = _x0 + 1; // kick start

    _f0 = _eval(_x0);
    _f1 = _eval(_x1);

    for (int i = 0; i < 20; ++i)
    {
        if ((_f0 < 0) != (_f1 < 0))
            return true;
        
        if (fabs(_f0) < fabs(_f1))
        {
            _x0 -= 2*(_x1 - _x0);
            _f0 = _eval(_x0);
        }
        else
        {
            _x1 += 2*(_x1 - _x0);
            _f1 = _eval(_x1);
        }
    }
    return false;
}

bool Solver::ridderSolveOnly()
{
    // only within min/max
    if (_x0 == _x1) return false;
    
    _f0 = _eval(_x0);
    _f1 = _eval(_x1);

    if (_f0.isNeg() == _f1.isNeg()) return false;
    
    return _ridderRoot();
}

bool Solver::_ridderRoot()
{
    BCD x2, t;
    BCD f2, ff;
    int i;
    BCD x0, x1;

    if (_f0 == 0)
    {
        _root = _x0;
        return true;
    }
    else if (_f1 == 0)
    {
        _root = _x1;
        return true;
    }

    CPUSpeedFast();
    for (i = 0; i < MAX_ITERATIONS; ++i) 
    {
        x2 = (_x1+_x0)/2;
        f2 = _eval(x2);

        t = f2*f2-_f0*_f1;
        if (t == 0) break;
        t = f2*(x2-_x0)/sqrt(t);

        if (_f0 > _f1) _root = x2+t;
        else _root = x2-t;
        
        ff = _eval(_root);
        
        if ((f2 < 0) != (ff < 0)) 
        {
            _x0 = x2;
            _f0 = f2;
            _x1 = _root;
            _f1 = ff;
        }
        else if ((_f0 < 0) != (ff < 0)) 
        {
            _x1 = _root;
            _f1 = ff;
        }
        else 
        {
            _x0 = _root;
            _f0 = ff;
        }
        if (fabs(_x1-_x0) < _eps*fabs(_x1)) break;
    }
    CPUSpeedNormal();
    return true;
}

bool Solver::derivAdapter(const BCD& x, BCD& val, ExprEvaluator& eeval)
{
    // XX HACKY
    bool res = false;
    BCD eps = BCD::epsilon(8);
    BCD t = x + eps;
    BCD h = t - x;
    
    BCD f1, f2;
    if (eeval._eval(x + h, f1) && eeval._eval(x - h, f2))
    {
        val = (f1 - f2)/(2*h);
        res = true;
    }
    return res;
}




