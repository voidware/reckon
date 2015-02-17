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

#ifndef __eeval_h__
#define __eeval_h__

#include "types.h"

struct ExprEvaluator
{
    typedef bool evalAdapter(const BCD& x, BCD& val, ExprEvaluator& eeval);
    typedef bool varCB(Term*, void* ctx);

    ExprEvaluator()
    {
        _adFn = 0;
        _nVars = 0;
    }

    ~ExprEvaluator()
    {
        if (_var) SYMBOL(_var)->unbind();
        if (_var2) SYMBOL(_var2)->unbind();
    }

    // Features
    bool setTerm(Term* t, int nVars = 1) 
    {
        _nVars = nVars; // expected vars

        bool res = findVars(t, _setVarCB, this);
        if (res) 
            _expr = t;
        return res;
    }

    void setTermAndVar(const TermRef& t, const TermRef& var)
    {
        _expr = t;
        _var = var;
    }

    void setAdapter(evalAdapter* af) { _adFn = af; }

    bool eval(const BCD& x, BCD& val);

    bool _eval(const BCD& x, BCD& val)
    {
        SYMBOL(_var)->assign(Float::create(DPD(x)));
        return _evalReduce(val);
    }

    bool _eval(const BCD& x, const BCD& y, BCD& val)
    {
        SYMBOL(_var)->assign(Float::create(DPD(x)));
        SYMBOL(_var2)->assign(Float::create(DPD(y)));
        return _evalReduce(val);
    }

    bool _setVar(Term* t)
    {
        // return true if done
        if (!_var.valid())
        {
            _var = t;
            if (_var.valid() && _nVars == 1)
                return true; // 1 var expected and set.
        }
        else if (!_var2.valid() && t != *_var)
        {
            _var2 = t;
            if (_var2.valid()) return true;
        }
        return false;
    }

    static bool _setVarCB(Term* t, void* ctx)
    {
        ExprEvaluator* eeval = (ExprEvaluator*)ctx;
        return eeval->_setVar(t);
    }

    bool findVars(Term* t, varCB*, void* ctx);

protected:

    bool _evalReduce(BCD& val);

    
    TermRef             _expr;          // expression to eval
    int                 _nVars;
    TermRef             _var;           // depdenent variable
    TermRef             _var2;          // second dimension
    evalAdapter*        _adFn;
    
};


#endif // __eeval_h__
