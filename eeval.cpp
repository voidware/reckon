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

#include "eeval.h"
#include "calc.h"

bool ExprEvaluator::findVars(Term* t, varCB* cb, void* ctx)
{
    bool res = false;
    if (t)
    {
        if (ISFUNCTION(t))
        {
            Function* f = (Function*)t;
            unsigned int i;
            unsigned int n = f->nargs_;
            for (i = 0; i < n; ++i) 
            {
                Term* a = ARG(f, i);
                if (ISSYMBOL(a)) 
                {
                    if (!((Symbol*)a)->isBound())
                    {
                        // if true, we're done
                        res = (*cb)(a, ctx);
                        if (res) break; // done.
                    }
                }
                else
                {
                    res = findVars(a, cb, ctx);
                    if (res) break;
                }
            }
        }
        else if (ISSYMBOL(t) && !((Symbol*)t)->isBound())
            res = (*cb)(t, ctx);
    }
    return res;
}

bool ExprEvaluator::eval(const BCD& x, BCD& val)
{
    if (!_adFn) return _eval(x, val);
    return (*_adFn)(x, val, *this);
}

bool ExprEvaluator::_evalReduce(BCD& val)
{
   // eval function
    TermRef res;
    _expr->reduce(res, Calc::theCalc->tc_);
            
    // convert to float
    res = Calc::theCalc->approximate(res);
            
    bool v = ISFLOAT(res);
    if (v)
    {
        val = FLOAT(res)->v_.asBCD();
        v = !val.isSpecial();
    }
    return v;
}
