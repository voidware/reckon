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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "types.h"
#include "symbol.h"
#include "dlist.h"
#include "dpdmath.h"
#include "plot.h"
#include "solve.h"
#include "mat.h"
#include "plot3d.h"
#include "finance.h"
#include "bigs.h"
#include "nran.h"
#include "cutils.h"

#ifdef _WIN32
#include "oswin.h"
#else
extern "C"
{
#include "common.h"
}
#endif


// see `theOperatorTable' ensure this is the correct index of
// multiply.
#define MULTIPLY_OPERATOR_INDEX         0

using namespace Utils;

static Operator* theOperatorTable[MAX_OPERATORS];
static unsigned int operatorCount;
static FnRegistry theFnRegistry;
static char textBuf[128];
static Ranq1 ranq;
static NRan nran;

/* global variable set by `reduce' when calling implementation
 * functions. only valid within those functions. nearly all functions
 * do not need the term context, so better than having an additional
 * formal parameter.
 */
TermContext* currentTC;

/** TermContext ******************************************/

TermContext::TermContext()
{
    sTab_ = 0;
}

Symbol* TermContext::internSymbol(const char* name, unsigned int l)
{
    /* if we have a local symbol table. look there first,
     * otherwise check the global table (later we might have
     * more tables). if not there either, create it in the 
     * local table (or global if not local).
     *
     * NOTE: possible result null if tables full.
     */

    Symbol* s = 0;
    if (sTab_) s = sTab_->find(name, l);

    if (!s) {
        /* check global */
        s = SymbolTable::global().find(name, l);
    }

    if (!s) {
        /* need to create it */
        if (sTab_) {
            s = sTab_->insert(name, l);
        }
        else {
            s = SymbolTable::global().insert(name, l);
        }
    }
    return s;
}

void TermContext::push()
{
    /* push a symbol table */
    sTab_ = new SymbolTable;
}

SymbolTable* TermContext::pop()
{
    SymbolTable* t = sTab_;
    sTab_ = 0;
    return t;
}

/** Term *************************************************/

void Term::destroy(Term* t)
{
    delete t;
}

bool Term::convert(TermRef& res, Type b) const
{
    /* can we convert to type `b'. */
       
    int t1 = type();
    RegInfo* cf =
        theFnRegistry.findConverter(t1, b);
    if (cf) {
        (*(FnImpl1*)cf->impl_)(res, (Term*)this);
        return true;
    }
    return false;
}

RegInfo* Term::findAnyFunction(Symbol* symbol)
{
    /* find any of the functions regered under the given symbol.
     * this is currently used in the UI to verify parameters.
     */

    unsigned int i;
    for (i = 0; i < theFnRegistry.size_; ++i) {
        RegInfo* ri = theFnRegistry.fn_[i];
        if (*SYMBOL(ri->symbol_) == *symbol) {
            return ri;
        }
    }
    return 0;
}

static Term* rebindLeft(Term* r, Function* f)
{
    Term* o = 0;

    if (ISFUNCTION(r) && ((Function*)r)->nargs_) 
    {
        Function* rf = (Function*)r;
        if (rf->isOperator())
        {
            if (rf->prec_ <= f->prec_) 
            {
                o = rebindLeft(ARG(rf,0), f);
                if (!o) 
                {
                    Term* t = ARG(rf, 0);
                    rf->setArg(0, f);
                    ARG(f, 1) = t;
                    o = r;
                }
            }
        }
    }
    return o;
}

Term* Term::parse(const char** s, TermContext& tc)
{
    const char* p = *s;
    while (u_isspace(*p)) ++p;

    Term* o = 0;

    if (*p == '(') 
    {
        ++p;
        o = Term::parse(&p, tc);
        while (u_isspace(*p)) ++p;
        if (*p == ')') ++p;

        if (o) 
        {
            *s = p;
            if (ISFUNCTION(o)) 
            {
                Function* f = (Function*)o;
                f->prec_ = PREC_BRACKETS; /* set precedence high */
            }
        }
    }

    if (!o) 
        /* array? */
        o = Array::parse(&p, tc);

    if (!o) 
        /* try for a number */
        o = Number::parse(&p);


    if (!o) 
        /* a string? */
        o = String::parse(&p);

    if (!o) 
        /* try for a function */
        o = Function::parse(&p, tc);

    if (!o) 
        /* try for a symbol */
        o = Symbol::parse(&p, tc);

    if (!o) 
    {
        /* try for a prefix operator */
        o = Operator::parse(&p, FUNC_PREFIX);

        if (o) 
        {
            *s = p;
            while (u_isspace(*p)) ++p;            
            Term* r = Term::parse(&p, tc);
            if (r) 
            {
                /* we have a prefix operation */
                Function* f = Function::create(1);
                f->symbol_ = ((Operator*)o)->symbol_;
                f->prec_ = ((Operator*)o)->prec_;
                f->flags_ = ((Operator*)o)->flags_;
                o = 0;

                /* if we bind tighter, need to split
                 * eg -x+y
                 */
                if (ISFUNCTION(r)) 
                {
                    Function* rf = (Function*)r;
                    if (f->prec_ >= rf->prec_) 
                    {
                        Term* t = ARG(rf, 0);
                        rf->setArg(0, f);
                        ARG(f, 0) = t;
                        o = r;
                    }
                }

                if (!o) 
                {
                    f->setArg(0, r);
                    o = f;
                }
                *s = p;
            }
        }
    }

    if (o) 
    {
        /* possible postfix operator.
         */
        while (u_isspace(*p)) ++p;
        if (*p) 
        {
            Operator* op = Operator::parse(&p, FUNC_POSTFIX);
            if (op) 
            {
                Function* f = Function::create(1);
                f->symbol_ = ((Operator*)op)->symbol_;
                f->prec_ = ((Operator*)op)->prec_;
                f->flags_ = ((Operator*)o)->flags_;

                f->setArg(0, o);
                o = f;
                *s = p;
            }
        }
    }

    if (o) 
    {
        *s = p;
        while (u_isspace(*p)) ++p;
        if (*p) 
        {
            Operator* op = Operator::parse(&p, FUNC_INFIX);

            if (!op)
            {
                // if what comes next looks like a symbol, then 
                // it might be a symbol or a function.

                const char* p1 = p;
                if (*p1 == '(' || Symbol::scan(&p1))
                {
                    // insert implied multipluy here and let OP take
                    // care of what comes next.
                    op = theOperatorTable[MULTIPLY_OPERATOR_INDEX];
                }
            }

            if (op) 
            {
                Term* r = Term::parse(&p, tc);
                if (r) 
                {
                    /* we have a binop functional */
                    Function* f = Function::create(2);
                    f->symbol_ = op->symbol_;
                    f->setArg(0, o);
                    f->prec_ = op->prec_;
                    f->flags_ = op->flags_;
                    if (rebindLeft(r, f)) o = r;
                    else 
                    {
                        f->setArg(1, r);
                        o = f;
                    }
                    *s = p;
                }
            }
        }
    }
    return o;
}

/** TermRef **************************************************/

TermRef& TermRef::operator=(const TermRef& r)
{
    if (ref_ != r.ref_) {
        if (r.ref_) r.ref_->incRef();
        purge();
        ref_ = r.ref_;
    }
    return *this;
}

void TermRef::purge() 
{
    if (ref_ && !ref_->decRef()) 
        Term::destroy(ref_);
    ref_ = 0;
}

/** Number **************************************************/

Number* Number::parse(const char** s)
{
    Number* o = 0;
    const char* p = *s;

    if (*p == 'i' && !u_isalnum(p[1])) 
    {
        /* interpret as `i' */
        o = ComplexN::parse(s);

        /* manually skip */
        ++p;
    }
    else 
    {
        bool v = Real::scan(&p);
        if (v) 
        {
            /* might be real or complex */
            if (*p == 'i') 
            {
                /* try for complex */
                ++p; // skip manually.
                o = ComplexN::parse(s);
            }
            if (!o) 
                /* otherwise try for a real */
                o = Real::parse(s);
        }

        if (!o) 
        {
            /* try for an integer */
            p = *s;
            v = Rational::scan(&p);
            if (v) 
            {
                if (*p == 'i') 
                {
                    /* integer complex, treat as float complex */
                    ++p; // manual
                    o = ComplexN::parse(s);
                }
                if (!o) 
                    o = Rational::parse(s);
            }
        }
    }
    *s = p;
    return o;
}

/** ComplexN **************************************************/

static void streamChar(void* stream, char c)
{
    TermRef* sp = (TermRef*)stream;
    STRING(*sp)->append(c);
}

ComplexN* ComplexN::parse(const char **s)
{
    /* NOTE: doesnt adjust s */
    ComplexN* c = 0;
    const char* p = *s;

    if (*p == 'i') {
        /* assume single `i' */
        Complex cmf(BCD(0), BCD(1));
        c = ComplexN::create(cmf);
    }
    else {
        /* assume a real already scanned and we expect an `i'
         * to follow.
         */
        Complex cmf(BCD(0), BCD(p));
        c = ComplexN::create(cmf);
    }
    return c;
}

void ComplexN::asString(TermRef& s, DispFormat* df) const
{
    bool r = cmf_.isReal();
    bool i = cmf_.isImaginary();

    unsigned int l = sizeof(textBuf);
    BufStreamer b(textBuf, l);

    if (r || !i) 
    {
        b.put(cmf_.r_.asStringFmt(df->_format, df->_precision));
    }
    if (!r) 
    {
        if (!i)  // not pure imaginary
        {
            //b.put('\n');
            if (!cmf_.i_.isNeg()) b.put('+');
        }

        if (cmf_.i_ != 1) 
            b.put(cmf_.i_.asStringFmt(df->_format, df->_precision));
        
        b.put("i");
    }
    STRING(s)->append(b.finish());
}

/** Real ********************************************************/

bool Real::scan(const char **s)
{
    /* allowable forms:
     * [-]?[0-9]*\.[0-9]*(e[-]?[0-9]+)?
     * [-]?[0-9]+e-[0-9]+
     * 
     * also -?Inf
     * 
     */

    const char* p = *s;
    int n;
    int point = 0;

    if (*p == '-') ++p;
    n = 0;
    while (u_isdigit(*p)) { ++n; ++p; }
    if (!n && *p != '.') 
    {
        // no digits and no .
        // check for Inf.
        if (!u_strnicmp(p, "inf", 3) && !u_isalnum(p[3]))
        {
            // buy inf
            *s = p + 3;
            return true;
        }
        return false;
    }
    if (*p == '.') 
    {
        ++p;
        point = 1;
        while (u_isdigit(*p)) { ++n; ++p; }
    }

#if USE_DPD
    if (*p == ':')
    {
        // range field?
        ++p;
        while (u_isdigit(*p)) ++p;
    }
#endif
    
    if (ISEXP(*p))
    {
        if (!n) return false;
        ++p;
        if (*p == '+' || *p == '-') ++p;
        n = 0;
        while (u_isdigit(*p)) { ++n; ++p; }
        if (!n) return false;
    }

#ifndef USE_DPD
    // when no dpd, only real when point exists 
    else if (!point) return false;
#endif

    *s = p;
    return true;
}

Real* Real::parse(const char** s)
{
    /* NOTE: doesnt adjust s */
    return Float::parse(s);
}

/** Float *******************************************************/

Float* Float::parse(const char** s)
{
    /* ASSUME we know a float is parsable.
     * doesnt adjust `s'.
     */
    Float* m;
    m = Float::create();
    m->v_ = DPD(*s);
    return m;
}


void Float::asString(TermRef& s, DispFormat* df) const
{
    BufStreamer b(textBuf, sizeof(textBuf));
    b.put(v_.asStringFmt(df->_format, df->_precision));
    STRING(s)->append(b.finish());
}

/** Function, methods ********************************************/

Function::~Function()
{
    unsigned int i;
    for (i = 0; i < nargs_; ++i) dropArg(i);
}

void Function::init()
{
    prec_ = PREC_FUNCTION;
    binding_ = 0;
    cosmeticBrackets_ = 0;
    flags_ = 0;
}

Function* Function::create(int nargs)
{
    Function* f = 0;
    if (nargs < MAX_FNARGS) 
    {
        f = new (nargs) Function(nargs);
        for (int i = 0; i < nargs; ++i) ARG(f, i) = 0;
    }
    return f;
}

bool Function::scan(const char** s, const char** args, int* nargsp)
{
    /* see if the form matches a function.
     * allow:
     *  [a-z]+[1-9]*(
     */

    const char* p = *s;
    int n = 0;
    while (u_isalpha(*p)) { ++n; ++p; }
    if (!n) return false;
    while (u_isalnum(*p)) { ++p; }
    if (*p != '(') return false;
    *args = ++p;

    /* scan for matching bracket */
    n = 1;
    int nargs = 0;
    while (*p) 
    {
        if (*p == '[')
        {
            // eat everything to end ']'
            int n = 1;
            while (n && *++p)
            {
                if (*p == '[') ++n;
                else if (*p == ']') --n;
            }
        }

        if (*p == '(') { ++n; }
        else if (*p == ',' && n == 1) ++nargs;
        else if (*p == ')') 
        {
            if (!--n) { ++p; break; }
        }
        else if (!nargs) ++nargs;
        if (*p) ++p;
    }
    *s = p;
    *nargsp = nargs;
    return true;
}

Function* Function::parse(const char** s, TermContext& tc)
{
    Function* f = 0;
    const char* p = *s;
    const char* args;
    int nargs;

    if (scan(&p, &args, &nargs))
    {
        f = create(nargs);
        f->nargs_ = nargs;

        const char* a = *s;
        Symbol* sym = Symbol::parse(&a, tc);
        if (sym) 
        {
            f->symbol_ = sym;
            a = args;
            int i = 0;
            while (nargs)
            {
                Term* o = Term::parse(&a, tc);
                if (!o) break;
                f->setArg(i++, o);
                --nargs;
                while (u_isspace(*a)) ++a;
                if (nargs && *a != ',') break; /* syntax error */
                ++a;
            }
        }
        if (!nargs) *s = p;
    }
    return f;
}

void Function::asString(TermRef& s, DispFormat* df) const
{
    int bc = 0;
    if (prec_ == PREC_BRACKETS) bc = 1; // calc3 
    if (cosmeticBrackets_) bc = cosmeticBrackets_;

    int b;
    for (b = 0; b < bc; ++b) STRING(s)->append('(');                

    if (isOperator()) 
    {
        if (nargs_ == 2) 
        {
            /* assume infix */
            ARG(this, 0)->asString(s, df);
            symbol_->asString(s, df);
            ARG(this, 1)->asString(s, df);

        }
        else 
        {
            /* assume nargs == 1 */
            //Operator* op = asOperator();
            bool pre = (flags_ & FUNC_PREFIX) != 0;

            if (pre) symbol_->asString(s, df);                
            ARG(this, 0)->asString(s, df);
            if (!pre) symbol_->asString(s, df);                
        }
    }
    else 
    {
        symbol_->asString(s, df);

        TermRef a = String::create();

        if (nargs_ > 1) STRING(a)->append('(');
        if (nargs_) 
        {
            for (int i = 0; i < nargs_-1; ++i) 
            {
                ARG(this, i)->asString(a, df);
                STRING(a)->append(',');
            }
            ARG(this, nargs_-1)->asString(a, df);
        }
        if (nargs_ > 1) STRING(a)->append(')');
        
        const char* p = STRING(a)->s_;
        if (p) 
        {
            bool b = (p[0] == '(');
            if (!b) STRING(s)->append('(');
            STRING(s)->append(p);
            if (!b) STRING(s)->append(')');
        }
    }
    for (b = 0; b < bc; ++b) STRING(s)->append(')');                
}

static bool generalType(Type a, Type b)
{
    /* true iff a is a is a more general (or equal) type
     * than b.
     */
    return (b % a) == 0;
}

static int typeMatchDistance(Type a, Type b)
{
    /* what is the conversion distance from b to a.
     * -1 => cant convert.
     */

    if (generalType(a, b)) return 0;

    int d = -1;
    if (a == Float::mytype()) 
    {
        if (b == Rational::mytype()) d = 1;
    }
    else if (a == ComplexN::mytype()) 
    {
        if (b == Rational::mytype()) d = 2;
        else if (b == Float::mytype()) d = 1;
    }
    return d;
}

struct BindCandidate: public DListRec
{
    int         dist_;
    RegInfo*    binding_;
};

void Function::bind(DList& blist, Term** args)
{
    /* make a list of binding candidates ordered by distance */
    unsigned int i;
    bool v = symbol_;

    if (v) for (i = 0; i < nargs_; ++i) 
           {
               v = args[i] != 0;
               if (!v) break;
           }

    if (v) 
    {
        BindCandidate bc;

        /* args are valid. now bind Function */
        for (i = 0; i < theFnRegistry.size_; ++i) 
        {
            RegInfo* ri = theFnRegistry.fn_[i];
            
            if (ri->nargs_ == nargs_ &&
                *SYMBOL(ri->symbol_) == *SYMBOL(symbol_)) 
            {
                /* name & arg counts match. check types */

                int j;
                bc.dist_ = 0;
                for (j = 0; j < nargs_; ++j) 
                {
                    int d =
                        typeMatchDistance(ri->argType_[j], args[j]->type());
                    
                    if (d < 0)
                        break;
                    bc.dist_ += d;
                }
                   
                if (j == nargs_) 
                {
                    /* we have a match subject to available
                     * conversion.
                     */
                    bc.binding_ = ri;

                    /* make a dynamic copy */
                    BindCandidate* abc = new BindCandidate;
                    *abc = bc;

                    /* insert into candidate list ordered by distance */
                    DListIterator bi(blist);
                    BindCandidate* t;
                    for (;;) 
                    {
                        t = (BindCandidate*)*bi;
                        if (!t) 
                        {
                            blist.append(abc);
                            break;
                        }
                        if (t->dist_ > bc.dist_) 
                        {
                            /* put before this one */
                            bi.insert(abc);
                            break;
                        }
                        ++bi;
                    }
                }
            }
        }
    }
}

bool Function::_reduce(TermRef& res, RegInfo* binding, Term** argBuf)
{
    // reduce using the given binding

    int i;
    for (i = 0; i < nargs_; ++i) 
    {
        Type t1 = argBuf[i]->type();
        Type t2 = binding->argType_[i];
                
        if (!generalType(t2, t1)) 
        {
            TermRef res;
            if (argBuf[i]->convert(res, t2)) 
            {
                DROP_ARG(argBuf[i]);
                SET_ARG(argBuf[i], *res);
            }
            else return false; // fail
        }
    }

    res = 0;

    /* converted all arguments */
    switch (nargs_) 
    {
    case 0:
        (*(FnImpl0*)binding->impl_)(res);
        break;
    case 1:
        (*(FnImpl1*)binding->impl_)(res, argBuf[0]);
        break;
    case 2:
        (*(FnImpl2*)binding->impl_)(res, argBuf[0], argBuf[1]);
        break;
    case 3:
        (*(FnImpl3*)binding->impl_)(res,
                                         argBuf[0],
                                         argBuf[1],
                                         argBuf[2]);
        break;
    case 4:
        (*(FnImpl4*)binding->impl_)(res,
                                         argBuf[0],
                                         argBuf[1],
                                         argBuf[2],
                                         argBuf[3]);
        break;
    case 5:
        (*(FnImpl5*)binding->impl_)(res,
                                         argBuf[0],
                                         argBuf[1],
                                         argBuf[2],
                                         argBuf[3],
                                         argBuf[4]);
        break;
    case 6:
        (*(FnImpl6*)binding->impl_)(res,
                                         argBuf[0],
                                         argBuf[1],
                                         argBuf[2],
                                         argBuf[3],
                                         argBuf[4],
                                         argBuf[5]);
        break;
    }

    return true;
}


bool Function::reduce(TermRef& res, TermContext& tc)
{
    Term* argBuf[MAX_FNARGS];
    Term** argBufp = &ARG(this, 0);
    int i;
    bool reduced = false; // did we reduce

    bool eval = (flags_ & FUNC_NOEVAL) == 0;
    if (eval)
    {
        for (i = 0; i < nargs_; ++i) 
        {
            TermRef a;
            ARG(this, i)->reduce(a, tc);
            SET_ARG(argBuf[i], *a);
        }
        argBufp = argBuf;
    }


    /* set up the tc in case any functions need it */
    currentTC = &tc;

    // try to use last binding (if any)
    if (!binding_ || !_reduce(res, binding_, argBufp) || !res)
    {        
        DList blist;
        bind(blist, argBufp);

        if (blist.size() == 1)
        {
            // whether or not this binding works, it is the only
            // one. so store it with our function for next time.
            binding_ = ((BindCandidate*)blist.first())->binding_;
        }

        BindCandidate* bc;
        while ((bc = (BindCandidate*)blist.extractFirst()) != 0)
        {
            /* operate on each candidate. try to convert arguments
             * and call the Function. 
             *
             * note: for the time being assume that its better to convert
             * arguments from the last set converted rather than from the
             * original datatype.
             */

            RegInfo* binding = bc->binding_;
            delete bc;

            if (!_reduce(res, binding, argBufp)) continue;

            if (res) 
            {
                /* delete any remaining candidates and finish */
                while ((bc = (BindCandidate*)blist.extractFirst()) != 0)
                    delete bc;

                reduced = true;
                break;
            }
        }
    }

    if (!res)
    {
        // did not reduce, make the result a copy of ourself with
        // any reduced parameters.
        if (!eval)
        {
            // params are the same
            res = this;
        }
        else
        {
            Function* fc = create(nargs_);
            *fc = *this;  // copy guts

            // steal terms directly without adjusting their counts
            for (i = 0; i < nargs_; ++i) 
                ARG(fc, i) = argBuf[i];

            res = fc;
        }
    }
    else if (eval)
    {
        /* drop argbuf */
        for (i = 0; i < nargs_; ++i) 
            DROP_ARG(argBuf[i]);
    }

    return eval && reduced;
}

TermRef Function::clone()
{
    Function* f = create(nargs_);
    f->symbol_ = symbol_;
    f->prec_ = prec_;

    unsigned int i;
    for (i = 0; i < nargs_; ++i) 
        f->setArg(i, *ARG(this,i)->clone());

    return f;
}

void Function::_fixPrecisions()
{
    unsigned int i;
    for (i = 0; i < nargs_; ++i) 
    {
        if (ISFUNCTION(ARG(this, i)))
        {
            Function* f = (Function*)ARG(this, i);
            int bc;
            if (prec_ > f->prec_) 
            {
                /* we bind tighter that subexpressions, need to
                 * show brackets whether cosmetic or not.
                 */
                 bc = 1;
            }
            else 
            {
                /* XX assume PREC_BRACKETS == 8 */
                //bc = ((f->prec_>>3) - (prec_>>3));
                bc = (f->prec_ == PREC_BRACKETS) && (prec_ != PREC_BRACKETS);
            }
            f->_fixPrecisions();
            f->cosmeticBrackets_ = bc;
        }
    }
}

void Function::fixPrecisions()
{
    int bc = prec_ == PREC_BRACKETS;
    _fixPrecisions();
    cosmeticBrackets_ = bc;
}

/** Rational ************************************************/

Rational::~Rational()
{
    destroyBig(rat_.x_);
    destroyBig(rat_.y_);
}

bool Rational::scan(const char** s)
{
    /* only scan integers. allow forms:
     * -?[0-9]+
     * allow 0[Xb][0-9A-F]+
     */

    const char* p = *s;
    int n;
    int base = 10;

    if (*p == '-') ++p;
    n = 0;
    if (*p == '0')
    {
        if (u_toupper(p[1]) == 'X') { base = 16; p += 2; }
        else if (p[1] == 'b') { base = 2; p += 2; }
    }

    char c;
    while ((u_isdigit(*p) && (*p - '0') < base) ||
           (base == 16 && (c = u_toupper(*p)) >= 'A' && c <= 'F')) { ++p; ++n; }
    
    if (!n) return false;
    *s = p;
    return true;
}

Rational* Rational::parse(const char** s)
{
    Rational* o = 0;
    const char* p = *s;

    Big* b = parseBig(&p);
    if (b) 
    {
        o = create();
        o->rat_ = BigFrac(b);
        *s = p;
    }
    return o;
}

void Rational::asString(TermRef& s, DispFormat*) const
{
    asStringFrac(&rat_, streamChar, &s);
}

/** symbol **************************************************/

Symbol::~Symbol()
{
    /* remove from symbol table */
    if (table_) 
        table_->remove(name_);
    
    delete name_;
}

Symbol* Symbol::create(const char* name, unsigned int l)
{
    Symbol* s = new Symbol;

    /* make a copy of the name text for the symbol. */
    s->name_ = new char[l+1];
    memcpy(s->name_, name, l);
    s->name_[l] = 0;
    return s;
}

static bool iscustom(const char** p)
{
    bool res = false;
    if (COMPARE_CUSTOM(*p, CUSTOM_PI_CHAR))
    {
        (*p) += 2;
        res = true;
    }
    return res;
}

bool Symbol::scan(const char** s)
{
    const char* p = *s;

    /* allow
     * #?[a-z]+[a-z0-9]*
     * 
     * also allow custom symbols
     */

    int n = 0;
    if (*p == '#') ++p;
    while (u_isalpha(*p)) { ++n; ++p; }
    while (iscustom(&p)) ++n;
    if (n) 
    {
        while (u_isalnum(*p)) { ++p; }
        *s = p;
    }
    return n > 0;
}

Symbol* Symbol::parse(const char** s, TermContext& tc)
{
    const char* p = *s;
    Symbol* o = 0;

    if (scan(&p))
    {
        o = tc.internSymbol(*s, p-(*s));
        *s = p;
    }
    return o;
}

bool Symbol::assignOnly(Term* o) 
{
    /* dont re-assign */
    if (v_.valid()) 
    {
        if (*v_ == o) return true;  // if the same!
        return false;
    }
    v_ = o;
    return true;
}

void Symbol::assign(Term* o) 
{
    v_ = o;
}

bool Symbol::reduce(TermRef& res, TermContext&)
{
    /* symbols evaluate to their values.
     */

    bool f = v_.valid();
    
    if (f)
        res = v_;
    else res = this;
    return f;
}

Operator* Symbol::isOperator() const
{
    Operator* op = 0;
    if (v_ && ISOPERATOR(*v_)) op = OPERATOR(v_);
    return op;
}

/** String **************************************************/

void String::init()
{
    s_ = 0;
    size_ = 0;
    space_ = 0;
}

String* String::create()
{
    String* s = new String;
    s->init();
    return s;
}

void String::set(const char* s, unsigned int l)
{
    enoughFor(l);
    if (space_ < l+1) l = space_-1;
    memcpy(s_, s, l);
    s_[l] = 0;
}

void String::enoughFor(unsigned int l)
{
    ++l;
    if (space_ < l) {

        int d = l - space_;
        if (d < 32) l += 32;

        char* t = new char[l];
        if (t) {
            if (s_) memcpy(t, s_, size_ + 1);
            space_ = l;
            delete s_;
            s_ = t;
        }
    }
}

void String::append(const char* s)
{
    if (s) {
        unsigned int l = strlen(s);
        enoughMore(l);
        unsigned int m = space_ - size_ - 1;
        if (m < l) l = m;
        memcpy(s_ + size_, s, l);
        size_ += l;
        s_[size_] = 0;
    }
}

void String::append(char c)
{
    enoughMore(1);
    s_[size_++] = c;
    s_[size_] = 0;
}

String* String::parse(const char** s)
{
    const char* p = *s;
    if (*p != '"') return 0;
    ++p;
    while (*p) if (*p++ == '"') break;
    
    String* o = String::create();
    o->set((*s)+1, p-(*s)-2);
    *s = p;
    return o;
}

void String::asString(TermRef& s, DispFormat* df) const 
{
    STRING(s)->append('"');
    STRING(s)->append(s_);
    STRING(s)->append('"');
}

TermRef String::clone()
{
    String* s = create();
    if (size_) {
        set(s_, size_);
    }
    return s;
}

void String::truncate(int n)
{
    /* truncate to `n' characters */
    if (n >= 0 && size_ > n) {
        memset(s_ + n, 0, size_ - n);
        size_ = n;
    }
}

/** Label ************************************************/

Label* Label::create()
{
    Label* s = new Label;
    s->init();

    s->drawBrackets_ = 0;
    return s;
}

void Label::asString(TermRef& s, DispFormat* df) const 
{
    int i;
    for (i = 0; i < drawBrackets_; ++i) STRING(s)->append('(');
    STRING(s)->append(s_);    
    for (i = 0; i < drawBrackets_; ++i) STRING(s)->append(')');
}

TermRef Label::clone()
{
    Label* s = create();
    if (size_) {
        set(s_, size_);
    }
    return s;
}

/** operator *********************************************/

Operator* Operator::parse(const char** s, int opmask)
{
    /* `opmask' allows certain operator types */
    unsigned int i;
    const char* p = *s;
    Operator* o = 0;
    for (i = 0; i < operatorCount; ++i) 
    {
        Operator* op = theOperatorTable[i];
        if (op->flags_ & opmask) 
        {
            const char* name = SYMBOL(op->symbol_)->name_;
            int l = strlen(name);
            if (!strncmp(name, p, l)) 
            {
                p += l;
                *s = p;
                o = op;
                break;
            }
        }
    }
    return o;
}

/** TermList ****************************************************/

void TermList::init()
{
    size_ = 0;
    space_ = 0;
    terms_ = 0;
}

void TermList::sizeFor(unsigned int n)
{
    if (space_ < n) {
        unsigned int newSpace = (110 * (n+1))/100;
        if (newSpace < 4) newSpace = 4;
        
        TermRef* p = new TermRef[newSpace];
        for (unsigned int i = 0; i < size_; ++i) 
            p[i] = terms_[i];
        
        delete [] terms_;
        terms_ = p;
        space_ = newSpace;
    }
}

void TermList::add(const TermRef& t)
{
    sizeFor(size_ + 1);
    terms_[size_++] = t;
}

void TermList::insert(unsigned int at, const TermRef& t)
{
    sizeFor(size_ + 1);
    unsigned int i;
    for (i = size_; i > at; --i) terms_[i] = terms_[i-1];
    terms_[i] = t;
    ++size_;
}

void TermList::empty()
{
    delete [] terms_;
    init();
}


/** Array ****************************************************/

void* Array::operator new(size_t amt, int n)
{
    return new char[amt + (n-1)*sizeof(TermRef)];
}

Array* Array::create(int n)
{
    Array* a = new(n) Array();
    a->size_ = n;
    memset(&a->elts_, 0, sizeof(TermRef)*n);
    return a;
}

TermRef Array::clone()
{
    Array* newa = create(size_);
    for (int i = 0; i < size_; ++i)
        newa->elts_[i] = elts_[i]->clone();
    newa->_initFlags();
    return newa;
}

Array* Array::parse(const char** s, TermContext& tc)
{
    Array* a = 0;
    const char* p = *s;
    if (*p == '[') 
    {
        TermList tl;
        ++p;
        for (;;) 
        {
            while (u_isspace(*p)) ++p;
            if (*p == ',') 
            {
                // allow commas as delimiters as well
                ++p;
                while (u_isspace(*p)) ++p;
            }
            if (*p == ']') 
            {
                ++p;
                break;
            }
            Term* t = Term::parse(&p, tc);
            if (!t) break;
            tl.add(t);
        }

        /* now allocate our array exactly the right size */
        a = create(tl.size_);
        for (unsigned int i = 0; i < tl.size_; ++i) 
            a->elts_[i] = tl.terms_[i];

        // set flags
        a->_initFlags();
        *s = p;
    }
    return a;
}

void Array::asString(TermRef& s, DispFormat* df) const
{
    STRING(s)->append('[');
    for (unsigned int i = 0; i < size_; ++i) 
    {
        // XXX hack to make matrices smaller for now..
        DispFormat d1 = *df;
        d1._precision = 6; 

        elts_[i]->asString(s, &d1);
        if (i < size_-1) 
        {
            if (flags_ == array_realmatrix)
                STRING(s)->append("\n ");
            else
                STRING(s)->append(' ');
        }
    }
    STRING(s)->append(']');
}

void Array::_initFlags()
{
    flags_ = array_normal;
    nCols_ = 0;
    
    // if we are a list of real numbers, we are a realvector

    // if we are a list of lists of real numbers, all the same size
    // we are a realmatrix
    int i;

    // 0  => might be matrix, dont know yet
    // -1 => not a matrix
    int colSize = 0; 
    int isVec = 0;  // same as above
    bool isComplex = false;

    for (i = 0; i < size_; ++i)
    {
        TermRef& t = _at(i);
        if (ISARRAY(t))
        {
            bool ok = false;

            // will already have flags
            int f = ARRAY(t)->flags_;
            if (f & array_realvector) // real or complex vector?
            {
                int c = ARRAY(t)->size_;
                if (colSize == 0 || colSize == c)
                {
                    colSize = c;
                    ok = true;
                }
                if (f == array_complexvector) isComplex = true;
            }
            
            if (!ok)
            {
                colSize = -1; // cant be a matrix
                break;
            }
        }
        else 
            colSize = -1; // definitely not a matrix

        bool c = ISCOMPLEX(t);
        
        if (ISFLOAT(t) || ISRATIONAL(t) || c)
        {
            if (isVec >= 0) ++isVec;
            if (c) isComplex = true;
        }
        else
            isVec = -1;  // not a real vector
    }

    if (colSize > 0 && size_ > 1)
    {
        // we are a real matrix with this number of columns
        // NB: cant have just one row, eg [[1]]
        flags_ = isComplex ? array_complexmatrix : array_realmatrix;
        nCols_ = colSize;
    }

    if (isVec > 0)  // valid vector?
    {
        flags_ = isComplex ? array_complexvector : array_realvector;
        nCols_ = size_;
        // convert all elements to floats
        for (i = 0; i < size_; ++i)
        {
            TermRef& t = _at(i);
            if (ISRATIONAL(t)) 
            {
                TermRef s;
                if (t->convert(s, 
                               (isComplex ? ComplexN::mytype() : Float::mytype())))
                    t = s;
            }
        }
    }
}

bool Array::reduce(TermRef& res, TermContext& tc)
{
    res = this;

    for (unsigned int i = 0; i < size_; ++i) 
    {
        TermRef r;
        elts_[i]->reduce(r, tc);
        elts_[i] = r;
    }

    // recalc flags
    _initFlags();
    return false; // finished reducing
}

/** RegInfo **************************************************/

RegInfo* RegInfo::create(Symbol* s, Type rt, int nargs, ...)
{
    va_list vl;
    RegInfo* ri = new (nargs) RegInfo(nargs);

    ri->symbol_ = s;
    ri->rType_ = rt;

    va_start(vl, nargs);

    for (int i = 0; i < nargs; ++i) 
    {
        Type t = va_arg(vl, Type);
        ri->argType_[i] = t;
    }

    va_end(vl);
    
    return ri;
}

RegInfo* RegInfo::createN(Symbol* s, Type rt, int nargs, const Type* narg0)
{
    RegInfo* ri = new (nargs) RegInfo(nargs);

    ri->symbol_ = s;
    ri->rType_ = rt;

    for (int i = 0; i < nargs; ++i) 
        ri->argType_[i] = narg0[i];

    return ri;
}

void FnRegistry::purge()
{
    for (unsigned int i = 0; i < size_; ++i) {
        delete fn_[i];
        fn_[i] = 0;
    }
}

bool FnRegistry::intern(RegInfo* ri)
{
    if (size_ < MAX_FUNCTIONS-1) {
        fn_[size_++] = ri;
        return true;
    }
    return false;
}

RegInfo* FnRegistry::findConverter(Type a, Type b) const
{
    /* look for a conversion Function from type a to type b
     */
    unsigned int i;
    for (i = 0; i < size_; ++i) 
    {
        RegInfo* ri = fn_[i];
        if (ri->convFn() && ri->nargs_ == 1 &&
            ri->rType_ == b &&
            ri->argType_[0] == a) {
            return ri;
        }
    }
    return 0;
}

/****************************************************/

struct OpInfoRec
{
    const char*         name_;
    unsigned int        nargs_;
    unsigned int        flags_;
    unsigned int        prec_;
};

static const OpInfoRec InitialOperatorsTable[] = 
{
    // XX NB: put this first in table to be at known place to 
    // index for finding it for implied multiply.
    { "*", 2, FUNC_INFIX, PREC_OPMULDIV },

    { "-", 1, FUNC_PREFIX, PREC_UNARYMINUS },
    { "'", 1, FUNC_PREFIX | FUNC_NOEVAL, PREC_QUOTE },
    { "=", 2, FUNC_INFIX | FUNC_NOEVAL, PREC_ASSIGN },
    { "//", 2, FUNC_INFIX, PREC_OPPPARALLEL },
    { "+", 2, FUNC_INFIX, PREC_OPADDSUB },
    { "-", 2, FUNC_INFIX, PREC_OPADDSUB },
    { "/", 2, FUNC_INFIX, PREC_OPMULDIV },
    { "^", 2, FUNC_INFIX, PREC_OPPOWER },
    { "!", 1, FUNC_POSTFIX, PREC_OPPOSTFIX },
    { CUSTOM_DEG_STRING, 1, FUNC_POSTFIX, PREC_OPPOSTFIX },
    { CUSTOM_ANG_STRING, 1, FUNC_POSTFIX, PREC_OPPOSTFIX },
};

void InitSymbols()
{
    /* put the initial Operator symbols into the global table */
    SymbolTable& st = SymbolTable::global();

    int i;
    for (i = 0; i < DIM(InitialOperatorsTable); ++i) 
    {
        const OpInfoRec* ir = InitialOperatorsTable + i;
        Symbol* s = st.intern(ir->name_, strlen(ir->name_));
        Operator* op = Operator::create();
        op->nargs_ = ir->nargs_;
        op->flags_ = ir->flags_;
        op->prec_ = ir->prec_;

        /* Operators have a backlink to the symbol */
        op->symbol_ = s;

        int j;
        if (operatorCount)
            for (j = operatorCount-1; j >= 0; --j)
            {
                if (!strcmp(SYMBOL(theOperatorTable[j]->symbol_)->name_,
                            ir->name_))
                {
                    op->similar_ = theOperatorTable[j];
                    break;
                }
            }
        
        theOperatorTable[operatorCount++] = op; op->incRef();
        s->assign(op);
    }
}

/** EVALUATION *****************************************************/


/** Float ******************************************************/

void parallelFloat(TermRef& res, Float* a, Float* b)
{
    DPD one(1);
    res = Float::create(one/(one/a->v_ + one/b->v_));
}

void addFloat(TermRef& res, Term* a, Term* b)
{ res = Float::create(F(a)->v_ + F(b)->v_); }

void subFloat(TermRef& res, Float* a, Float* b)
{ res = Float::create(a->v_ - b->v_); }

void mulFloat(TermRef& res, Float* a, Float* b)
{ res = Float::create(a->v_ * b->v_); }

void divFloat(TermRef& res, Float* a, Float* b)
{
    res = Float::create(a->v_ / b->v_);
}

void powFloat(TermRef& res, Float* a, Float* b)
{
    DPD c = pow(a->v_, b->v_);
    if (!c.isNan() || a->v_.isZero())
        res = Float::create(c);
}

void modFloat(TermRef& res, Float* a, Float* b)
{
    res = Float::create(fmod(a->v_, b->v_));
}

void asFloatRational(TermRef& res, Term* a)
{
    BCD v;
    BigFracToMF(&R(a)->rat_, &v);
    res = Float::create(DPD(v));
}

void expMF(TermRef& res, Float* a)
{
    DPD c = exp(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void alogMF(TermRef& res, Float* a)
{
    res = Float::create(pow(DPD(10), a->v_));
}

void piMF(TermRef& res)
{
    res = Float::create(pi());
}

void logMF(TermRef& res, Float* a)
{
    DPD c = log(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void log10MF(TermRef& res, Float* a)
{
    DPD c = log10(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void sqrtMF(TermRef& res, Float* a)
{
    DPD c = sqrt(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void sinMF(TermRef& res, Float* a)
{ res = Float::create(sin(a->v_)); }

void cosMF(TermRef& res, Float* a)
{ res = Float::create(cos(a->v_)); }

void tanMF(TermRef& res, Float* a)
{
    DPD c = tan(a->v_);
    if (!c.isNan())
        res = Float::create(c);

}

void atanMF(TermRef& res, Float* a)
{
    DPD c = atan(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void atan2MF(TermRef& res, Float* y, Float* x)
{
    DPD c = atan2(y->v_, x->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void asinMF(TermRef& res, Float* a)
{
    DPD c = asin(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}
void acosMF(TermRef& res, Float* a)
{
    DPD c =acos(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}
void sinhMF(TermRef& res, Float* a)
{
    DPD c = sinh(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}
void coshMF(TermRef& res, Float* a)
{
    DPD c = cosh(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}
void tanhMF(TermRef& res, Float* a)
{
    DPD c = tanh(a->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void invMF(TermRef& res, Float* a)
{
    // allow 1/0
    res = Float::create(DPD(1)/a->v_);
}

void negMF(TermRef& res, Float* a) { res = Float::create(-a->v_); }

void factorialMF(TermRef& res, Float* a)
{ res = Float::create(gammaFactorial(a->v_)); }

void ln1pMF(TermRef& res, Float* a)
{ res = Float::create(ln1p(a->v_)); }

void expm1MF(TermRef& res, Float* a)
{ res = Float::create(expm1(a->v_)); }

void absFloat(TermRef& res, Float* a)
{ res = Float::create(fabs(a->v_)); }

void sqFloat(TermRef& res, Float* a)
{ res = Float::create(a->v_ * a->v_); }

void cubeRootFloat(TermRef& res, Float* a)
{
    DPD v = pow(a->v_, DPD(1)/3);
    if (!v.isNan())
        res = Float::create(v);
}

void nthRootFloat(TermRef& res, Float* a, Float* b)
{
    DPD n = 1/b->v_;
    if (n.isSpecial()) res = Float::create(DPDFloat::nan());
    else
    {
        DPD v = pow(a->v_, n);
        if (!v.isNan())
            res = Float::create(v);
    }
}

DPD hms(const DPD& h)
{
    return (90*h+100*floor(h)+floor(60*h))/250;
}

static DPD hr(const DPD& x)
{
    return (250*x-60*floor(x)-floor(100*x))/90;
}

void dmsToRadFloat(TermRef& res, Float* a)
{
    DPD v = hr(a->v_)*pi()/180;
    res = Float::create(v);
}

void radToDmsFloat(TermRef& res, Float* a)
{
    DPD v = hms(180*a->v_/pi());
    res = Float::create(v);
}

void ratFloat(TermRef& res, Float* a, Float* eps)
{
    BigFrac c;
    BCD v = a->v_.asBCD();
    if (BCDToFrac(v, eps->v_.asBCD(), &c))
        res = Rational::create(c.x_, c.y_);
}

void fmaFloat(TermRef& res, Float* x, Float* y, Float* z)
{
    DPD c = 0; // XXX HACK fma(x->v_, y->v_, z->v_);
    if (!c.isNan())
        res = Float::create(c);
}

void floorFloat(TermRef& res, Float* a)
{
    res = Float::create(floor(a->v_));
}

void ranFloat(TermRef& res, Float* a)
{
    res = Float::create(floor(a->v_)); // XXX
}

void erfFloat(TermRef& res, Float* a)
{
    res = Float::create(erf(a->v_));
}

void normFloat(TermRef& res, Float* a)
{
    res = Float::create(normalProbability(a->v_));
}

#ifdef _WIN32
void dumpFloat(TermRef& res, Float* a)
{
    dump(a->v_.asBCD()._v);
    res = Float::create(a->v_);
}
#endif

/** Rational ****************************************************/

void parallelRational(TermRef& res, Rational* a, Rational* b)
{ 
    BigFrac c;
    if (parallelFrac(&a->rat_, &b->rat_, &c))
        res = Rational::create(c.x_, c.y_);
}

void addRational(TermRef& res, Rational* a, Rational* b)
{ 
    BigFrac c;
    if (addFrac(&a->rat_, &b->rat_, &c))
        res = Rational::create(c.x_, c.y_);
}

void subRational(TermRef& res, Rational* a, Rational* b)
{ 
    BigFrac c;
    if (subFrac(&a->rat_, &b->rat_, &c))
        res = Rational::create(c.x_, c.y_);
}
void mulRational(TermRef& res, Rational* a, Rational* b)
{ 
    BigFrac c;
    if (mulFrac(&a->rat_, &b->rat_, &c))
        res = Rational::create(c.x_, c.y_);
}

void divRational(TermRef& res, Rational* a, Rational* b)
{
    if (!b->rat_.isZero())
    {
        BigFrac c;
        if (divFrac(&a->rat_, &b->rat_, &c))
            res = Rational::create(c.x_, c.y_);
    }
}

void invRational(TermRef& res, Rational* a)
{
    if (!a->rat_.isZero()) 
    {
        BigFrac c;
        BigFrac one(1L);
        if (divFrac(&one, &a->rat_, &c))
            res = Rational::create(c.x_, c.y_);
    }
}

void modRational(TermRef& res, Rational* a, Rational* b)
{
    if (!b->rat_.isZero())
    {
        BigFrac c;
        if (modFrac(&a->rat_, &b->rat_, &c))
            res = Rational::create(c.x_, c.y_);
    }
}

void negRational(TermRef& res, Rational* a)
{
    BigFrac c;
    copyFrac(&a->rat_, &c);
    c.x_ = negateBig(c.x_);
    res = Rational::create(c.x_, c.y_);
}

void powRational(TermRef& res, Rational* a, Rational* b)
{
    BigFrac c;
    if (powerFrac(&a->rat_, &b->rat_, &c)) 
    {
        /* valid answer */
        res = Rational::create(c.x_, c.y_);
    }
}

void factorialRational(TermRef& res, Rational* a)
{
    BigFrac c;
    if (factorialFrac(&a->rat_, &c)) 
        res = Rational::create(c.x_, c.y_);
}

void factorRational(TermRef& res, Rational* a)
{
    BigFrac c;
    CPUSpeedFast();
    bool v = factorFrac(&a->rat_, &c);
    CPUSpeedNormal();
    if (v)
        res = Rational::create(c.x_, c.y_);
}

void isPrimeRational(TermRef& res, Rational* a)
{
    int v = 0; // not prime
    if (ISONE(a->rat_.y_))
    {
        // dont allow this to be destroyed as it's owned 
        BigInt t(a->rat_.x_);
        v = isPrime(t);
        t.give(); // drop content
    }
    res = Rational::create(v);
}

void nextPrimeRational(TermRef& res, Rational* a)
{
    if (ISONE(a->rat_.y_))
    {
        // dont allow this to be destroyed as it's owned 
        BigInt t(a->rat_.x_);
        BigInt np;
        if (nextPrime(t, np))
            res = Rational::create(np.give());
        t.give(); // drop content
    }
}

void prevPrimeRational(TermRef& res, Rational* a)
{
    if (ISONE(a->rat_.y_))
    {
        // dont allow this to be destroyed as it's owned 
        BigInt t(a->rat_.x_);
        BigInt np;
        if (prevPrime(t, np))
            res = Rational::create(np.give());
        t.give(); // drop content
    }
}

void sqRational(TermRef& res, Rational* a)
{
    BigFrac c;
    if (ISONE(a->rat_.y_))
    {
        Big* b = sqrBig(a->rat_.x_);
        if (b)
            res = Rational::create(b);
    }
    else if (mulFrac(&a->rat_, &a->rat_, &c))
        res = Rational::create(c.x_, c.y_);
}

void sqrtRational(TermRef& res, Rational* a)
{
    BigFrac c;
    if (rootFrac(&a->rat_, &c))
    {
        /* check perfect square, otherwise fail */
        BigFrac v;
        if (mulFrac(&c, &c, &v))
        {
            if (sameFrac(&v, &a->rat_))
            {
                // same, was a perfecrt square!
                res = Rational::create(c.x_, c.y_);        
            }
            destroyFrac(&v);
        }
    }
}

void nRootRational(TermRef& res, Rational* a, Rational* r)
{
    if (ISONE(r->rat_.y_))
    {
        unsigned int k = bigAsUint(r->rat_.x_);
        if (k > 0 && k != (unsigned int)-1)
        {
            BigFrac c;
            if (nRootFrac(&a->rat_, k, &c))
                res = Rational::create(c.x_, c.y_);                   
        }
    }
}

void ranRational(TermRef& res, Rational* a)
{
    if (ISONE(a->rat_.y_))
    {
        unsigned int n = bigAsUint(a->rat_.x_);
        res = Rational::create((ranq.int32() % n) + 1);
    }
}

void sranRational(TermRef& res, Rational* a)
{
    if (ISONE(a->rat_.y_))
    {
        unsigned int n = bigAsUint(a->rat_.x_);
        nran.seed(n); // also seeds ranq
        res = Rational::create(n);
    }
}

void nranRational(TermRef& res, Rational* a, Rational* b)
{
    if (ISONE(a->rat_.y_) && ISONE(b->rat_.y_))
    {
        unsigned int n = bigAsUint(a->rat_.x_);
        unsigned int m = bigAsUint(b->rat_.x_);
        res = Rational::create(nran.nran(n, m));
    }
}

/** ComplexN ******************************************************/

void addComplex(TermRef& res, ComplexN* a, ComplexN* b)
{ 
    Complex c = a->cmf_ + b->cmf_;
    res = ComplexN::create(c);
}
void subComplex(TermRef& res, ComplexN* a, ComplexN* b)
{ 
    Complex c = a->cmf_ - b->cmf_;
    res = ComplexN::create(c);
}
void mulComplex(TermRef& res, ComplexN* a, ComplexN* b)
{ 
    Complex c = a->cmf_ * b->cmf_;
    res = ComplexN::create(c);
}
void divComplex(TermRef& res, ComplexN* a, ComplexN* b)
{
    if (!b->cmf_.isZero()) 
    {
        Complex c = a->cmf_ / b->cmf_;
        res = ComplexN::create(c);
    }
    else
        res = Float::create(DPDFloat::posInf());
}

void invComplex(TermRef& res, ComplexN* a)
{
    res = ComplexN::create(inv(a->cmf_));
}

void asComplexFloat(TermRef& res, Term* a)
{ res = ComplexN::create(Complex(F(a)->v_.asBCD())); }

void asComplexRational(TermRef& res, Term* a)
{
    asFloatRational(res, a);
    if (res) 
    {
        // XX leak!
        asComplexFloat(res, FLOAT(res));
    }
}

void negComplex(TermRef& res, ComplexN* a)
{
    res = ComplexN::create(Complex(-a->cmf_.r_, -a->cmf_.i_));
}

void absComplex(TermRef& res, ComplexN* a)
{
    res = Float::create(DPD(fabs(a->cmf_)));
}

void sqrtComplex(TermRef& res, ComplexN* a)
{
    Complex c = sqrt(a->cmf_);
    res = ComplexN::create(c);    
}

void logComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (log(a->cmf_, c)) {
        res = ComplexN::create(c);
    }
}

void log10Complex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (log10(a->cmf_, c)) res = ComplexN::create(c);
}

void expComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (exp(a->cmf_, c)) {
        res = ComplexN::create(c);
    }
}

void alogComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (alog(a->cmf_, c)) res = ComplexN::create(c);
}

void conjComplex(TermRef& res, ComplexN* a)
{
    Complex c(a->cmf_.r_, -a->cmf_.i_);
    res = ComplexN::create(c);
}

void powComplex(TermRef& res, ComplexN* a, ComplexN* b)
{
    Complex c;
    if (power(a->cmf_, b->cmf_, c)) {
        res = ComplexN::create(c);
    }
}
void sinComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (sin(a->cmf_, c)) {
        res = ComplexN::create(c);
    }
}
void cosComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (cos(a->cmf_, c)) {
        res = ComplexN::create(c);
    }
}

void tanComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (tan(a->cmf_, c)) {
        res = ComplexN::create(c);
    }
}

void asinComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (asin(a->cmf_, c)) 
        res = ComplexN::create(c);
}

void acosComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (acos(a->cmf_, c))
        res = ComplexN::create(c);
}

void atanComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (atan(a->cmf_, c))
        res = ComplexN::create(c);
}

void sinhComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (sinh(a->cmf_, c))
        res = ComplexN::create(c);
}

void coshComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (cosh(a->cmf_, c))
        res = ComplexN::create(c);
}

void tanhComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (tanh(a->cmf_, c))
        res = ComplexN::create(c);
}

void asinhComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (asinh(a->cmf_, c))
        res = ComplexN::create(c);
}

void acoshComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (acosh(a->cmf_, c))
        res = ComplexN::create(c);
}

void atanhComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (atanh(a->cmf_, c))
        res = ComplexN::create(c);
}

void varAssign(TermRef& res, Symbol* a, Term* b)
{
    b->reduce(res, *currentTC);
    a->assign(*res);
}

void gammaComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    if (gammaFactorial(a->cmf_, c))
        res = ComplexN::create(c);
}

void sqComplex(TermRef& res, ComplexN* a)
{ 
    res = ComplexN::create(square(a->cmf_));
}

void cubeRootComplex(TermRef& res, ComplexN* a)
{
    Complex c;
    power(a->cmf_, BCD(1)/3, c);
    res = ComplexN::create(c);
}

void nthRootComplex(TermRef& res, ComplexN* a, ComplexN* b)
{
    Complex c;
    if (power(a->cmf_, inv(b->cmf_), c))
        res = ComplexN::create(c);
}

void parallelComplex(TermRef& res, ComplexN* a, ComplexN* b)
{
    res = ComplexN::create(inv(inv(a->cmf_) + inv(b->cmf_)));
}

/** EXPRESSION *******************************************************/

void fltConvert(TermRef& res, Term* t)
{
    bool ok = false;
    if (ISRATIONAL(t))
    {
        t->convert(res, Float::mytype());
        ok = true;
    }
    else if (ISFLOAT(t))
    {
        // convert to fraction with a small epsilon
        Float* a = (Float*)t;
        BigFrac c;
        BCD v1 = a->v_.asBCD();
        if (BCDToFrac(v1, BCD::epsilon(6), &c))
        {
            // then convert back to a BCD 
            BCD v2;
            BigFracToMF(&c, &v2);

            if (fabs(v2 - v1) < BCD::epsilon(16)*fabs(v1))
            {
                // if we're all the digits accept!
                res = Rational::create(c.x_, c.y_);
                ok = true;
            }
            else
                destroyFrac(&c); // not good enough.
        }
    }

    if (!ok)
        res = t;
}

void plotExpr(TermRef& res, Term* t, Float* xmin, Float* xmax)
{
    BCD a = xmin->v_.asBCD();
    BCD b = xmax->v_.asBCD();
    PlotGraph(t, a, b);
    res = t;
}

void plot3DExpr(TermRef& res, Term* t,
                Float* xmin, Float* xmax,
                Float* ymin, Float* ymax)
{
    PlotGraph3D(t,
                xmin->v_.asBCD(), xmax->v_.asBCD(),
                ymin->v_.asBCD(), ymax->v_.asBCD(), 0);
    res = t;
}

void plot3DExpr2(TermRef& res, Term* t,
                 Float* xmin, Float* xmax)
{
    PlotGraph3D(t,
                xmin->v_.asBCD(), xmax->v_.asBCD(),
                xmin->v_.asBCD(), xmax->v_.asBCD(), 0);
    res = t;
}

void plot3DExpr3(TermRef& res, Term* t,
                 Float* xmin, Float* xmax, Float* np)
{
    PlotGraph3D(t,
                xmin->v_.asBCD(), xmax->v_.asBCD(),
                xmin->v_.asBCD(), xmax->v_.asBCD(), np->v_.asBCD());
    res = t;
}

void solveExpr(TermRef& res, Term* t, Float* xmin, Float* xmax)
{
    SolveN(res, t, xmin->v_.asBCD(), xmax->v_.asBCD());
}

void purgeExpr(TermRef& res, Term* t)
{
    if (ISSYMBOL(t))
    {
        ((Symbol*)t)->unbind();
    }
    res = t;
}

void quoteExpr(TermRef& res, Term* t)
{
    res = t;
}

/** Array *******************************************************/

void opArrayScalar(TermRef& res, Array* a, Number* b, TermRef sym)
{ 
    unsigned int i;
    if (sym) 
    {
        int n = 1;
        if (b) ++n;

        Array* newA = Array::create(a->size());
        for (i = 0; i < a->size(); ++i) 
        {
            Function* f = Function::create(n);
            TermRef fr(f);
            f->symbol_ = sym;
            f->setArg(0, *a->_at(i));
            if (b) f->setArg(1, b);

            TermRef v;
            f->reduce(v, *currentTC);
            newA->_at(i) = v ? v : fr;
        }
        newA->_initFlags();
        res = newA;
    }
}

void addArrayScalar(TermRef& res, Array* a, Number* b)
{ 
    TermRef sym = currentTC->internSymbol("+", 1);
    opArrayScalar(res, a, b, sym);
}

void addScalarArray(TermRef& res, Number* a, Array* b)
{ 
    addArrayScalar(res, b, a);
}

void mulArrayScalar(TermRef& res, Array* a, Number* b)
{ 
    TermRef sym = currentTC->internSymbol("*", 1);
    opArrayScalar(res, a, b, sym);
}

void mulScalarArray(TermRef& res, Number* a, Array* b)
{ 
    mulArrayScalar(res, b, a);
}

void subArrayScalar(TermRef& res, Array* a, Number* b)
{ 
    TermRef sym = currentTC->internSymbol("-", 1);
    opArrayScalar(res, a, b, sym);
}

void divArrayScalar(TermRef& res, Array* a, Number* b)
{ 
    TermRef sym = currentTC->internSymbol("/", 1);
    opArrayScalar(res, a, b, sym);
}

void negArray(TermRef& res, Array* a)
{
    TermRef sym = currentTC->internSymbol("-", 1);
    opArrayScalar(res, a, 0, sym);
}

void invArray(TermRef& res, Array* a)
{
    if (a->isRealVecMat())
    {
        Matrix mc;
        bool singular;
        if (invert(Matrix(a), mc, singular))
            res = mc._a;
        else if (singular)
            res = Float::create(DPDFloat::posInf());
    }
}

void subScalarArray(TermRef& res, Number* a, Array* b)
{ 
    TermRef t;
    negArray(t, b); /* t = -b */
    addArrayScalar(res, ARRAY(t), a); /* res = (-b)+a */
}

void divScalarArray(TermRef& res, Number* a, Array* b)
{ 
    if (b->isRealVecMat())
    {
        TermRef t;
        invArray(t, b);  /* t = 1/b */
        if (t)
        {
            if (ISARRAY(t)) mulArrayScalar(res, ARRAY(t), a);
            else res = t;
        }
    }
}

void mulArrayArray(TermRef& res, Array* a, Array* b)
{
    if (a->isRealVecMat() && b->isRealVecMat())
    {
        Matrix mc;
        if (mul(Matrix(a), Matrix(b), mc))
            res = mc._a;
    }
}

void addArrayArray(TermRef& res, Array* a, Array* b)
{
    if (a->isRealVecMat() && b->isRealVecMat())
    {
        Matrix mc;
        if (add(Matrix(a), Matrix(b), mc))
            res = mc._a;
    }
}

void subArrayArray(TermRef& res, Array* a, Array* b)
{
    if (a->isRealVecMat() && b->isRealVecMat())
    {
        Matrix mc;
        if (sub(Matrix(a), Matrix(b), mc))
            res = mc._a;
    }
}

void divArrayArray(TermRef& res, Array* a, Array* b)
{
    if (a->isRealVecMat() && b->isRealVecMat())
    {
        Matrix mc;
        if (div(Matrix(a), Matrix(b), mc))
            res = mc._a;
    }
}

void transposeArray(TermRef& res, Array* a)
{
    if (a->isRealVecMat())
    {
        Matrix mc;
        transpose(Matrix(a), mc);
        res = mc._a;
    }
}

void absArray(TermRef& res, Array* a)
{
    // compute determinant as ABS
    if (a->isRealVecMat())
    {
        BCD v;
        if (Matrix(a).determinant(v))
            res = Float::create(DPD(v));
    }
}

void prootArray(TermRef& res, Array* a)
{
    if (a->isRealVecMat())
    {
        Matrix mc;
        if (prootEigen(Matrix(a), mc))
            res = mc._a;
    }
}

/****************************************************************/

static const Fn1ImplRec InitialFn1ConvTable[] =
{
    { "_cmfrat", FLOAT_TYPE, asFloatRational, RATIONAL_TYPE },
    { "_cxmf", COMPLEX_TYPE, asComplexFloat, FLOAT_TYPE },
    { "_cxrat", COMPLEX_TYPE, asComplexRational, RATIONAL_TYPE },
};

static const Fn0ImplRec InitialFn0ImplTable[] =
{
    { "pi", FLOAT_TYPE, (FnImpl*)piMF },
};

static const Fn1ImplRec InitialFn1ImplTable[] =
{
    { "-", FLOAT_TYPE, (FnImpl1*)negMF, FLOAT_TYPE },
    { "-", COMPLEX_TYPE, (FnImpl1*)negComplex, COMPLEX_TYPE },
    { "-", ARRAY_TYPE, (FnImpl1*)negArray, ARRAY_TYPE },
    { "'", EXPRESSION_TYPE, (FnImpl1*)quoteExpr, EXPRESSION_TYPE },
    { "!", RATIONAL_TYPE, (FnImpl1*)factorialRational, RATIONAL_TYPE },
    { "!", FLOAT_TYPE, (FnImpl1*)factorialMF, FLOAT_TYPE },
    { CUSTOM_DEG_STRING, FLOAT_TYPE, (FnImpl1*)dmsToRadFloat, FLOAT_TYPE },
    { CUSTOM_ANG_STRING, FLOAT_TYPE, (FnImpl1*)radToDmsFloat, FLOAT_TYPE },
    { "dmsrad", FLOAT_TYPE, (FnImpl1*)dmsToRadFloat, FLOAT_TYPE },
    { "raddms", FLOAT_TYPE, (FnImpl1*)radToDmsFloat, FLOAT_TYPE },
    { "!", COMPLEX_TYPE, (FnImpl1*)gammaComplex, COMPLEX_TYPE },
    { "factorial", RATIONAL_TYPE, (FnImpl1*)factorialRational, RATIONAL_TYPE },
    { "factorial", FLOAT_TYPE, (FnImpl1*)factorialMF, FLOAT_TYPE },
    { "factorial", COMPLEX_TYPE, (FnImpl1*)gammaComplex, COMPLEX_TYPE },
    { "exp", FLOAT_TYPE, (FnImpl1*)expMF, FLOAT_TYPE },
    { "exp", COMPLEX_TYPE, (FnImpl1*)expComplex, COMPLEX_TYPE },
    { "ln", FLOAT_TYPE, (FnImpl1*)logMF, FLOAT_TYPE },
    { "ln", COMPLEX_TYPE, (FnImpl1*)logComplex, COMPLEX_TYPE },
    { "sqrt", RATIONAL_TYPE, (FnImpl1*)sqrtRational, RATIONAL_TYPE },
    { "sqrt", FLOAT_TYPE, (FnImpl1*)sqrtMF, FLOAT_TYPE },
    { "sqrt", COMPLEX_TYPE, (FnImpl1*)sqrtComplex, COMPLEX_TYPE },
    { "sin", FLOAT_TYPE, (FnImpl1*)sinMF, FLOAT_TYPE },
    { "sin", COMPLEX_TYPE, (FnImpl1*)sinComplex, COMPLEX_TYPE },
    { "cos", FLOAT_TYPE, (FnImpl1*)cosMF, FLOAT_TYPE },
    { "cos", COMPLEX_TYPE, (FnImpl1*)cosComplex, COMPLEX_TYPE },
    { "tan", FLOAT_TYPE, (FnImpl1*)tanMF, FLOAT_TYPE },
    { "tan", COMPLEX_TYPE, (FnImpl1*)tanComplex, COMPLEX_TYPE },
    { "asin", FLOAT_TYPE, (FnImpl1*)asinMF, FLOAT_TYPE },
    { "asin", COMPLEX_TYPE, (FnImpl1*)asinComplex, COMPLEX_TYPE },
    { "acos", FLOAT_TYPE, (FnImpl1*)acosMF, FLOAT_TYPE },
    { "acos", COMPLEX_TYPE, (FnImpl1*)acosComplex, COMPLEX_TYPE },
    { "atan", FLOAT_TYPE, (FnImpl1*)atanMF, FLOAT_TYPE },
    { "atan", COMPLEX_TYPE, (FnImpl1*)atanComplex, COMPLEX_TYPE },
    { "sinh", FLOAT_TYPE, (FnImpl1*)sinhMF, FLOAT_TYPE },
    { "sinh", COMPLEX_TYPE, (FnImpl1*)sinhComplex, COMPLEX_TYPE },
    { "cosh", FLOAT_TYPE, (FnImpl1*)coshMF, FLOAT_TYPE },
    { "cosh", COMPLEX_TYPE, (FnImpl1*)coshComplex, COMPLEX_TYPE },
    { "tanh", FLOAT_TYPE, (FnImpl1*)tanhMF, FLOAT_TYPE },
    { "tanh", COMPLEX_TYPE, (FnImpl1*)tanhComplex, COMPLEX_TYPE },
    { "asinh", COMPLEX_TYPE, (FnImpl1*)asinhComplex, COMPLEX_TYPE },
    { "acosh", COMPLEX_TYPE, (FnImpl1*)acoshComplex, COMPLEX_TYPE },
    { "atanh", COMPLEX_TYPE, (FnImpl1*)atanhComplex, COMPLEX_TYPE },
    { "inv", RATIONAL_TYPE, (FnImpl1*)invRational, RATIONAL_TYPE },
    { "inv", FLOAT_TYPE, (FnImpl1*)invMF, FLOAT_TYPE },
    { "inv", COMPLEX_TYPE, (FnImpl1*)invComplex, COMPLEX_TYPE },
    { "inv", EXPRESSION_TYPE, (FnImpl1*)invArray, ARRAY_TYPE },
    { "neg", FLOAT_TYPE, (FnImpl1*)negMF, FLOAT_TYPE },
    { "neg", RATIONAL_TYPE, (FnImpl1*)negRational, RATIONAL_TYPE },
    { "neg", COMPLEX_TYPE, (FnImpl1*)negComplex, COMPLEX_TYPE },
    { "neg", ARRAY_TYPE, (FnImpl1*)negArray, ARRAY_TYPE },
    { "log", FLOAT_TYPE, (FnImpl1*)log10MF, FLOAT_TYPE },
    { "log", COMPLEX_TYPE, (FnImpl1*)log10Complex, COMPLEX_TYPE },
    { "alog", FLOAT_TYPE, (FnImpl1*)alogMF, FLOAT_TYPE },
    { "alog", COMPLEX_TYPE, (FnImpl1*)alogComplex, COMPLEX_TYPE },
    { "ln1p", FLOAT_TYPE, (FnImpl1*)ln1pMF, FLOAT_TYPE },
    { "expm1", FLOAT_TYPE, (FnImpl1*)expm1MF, FLOAT_TYPE },
    { "flt", EXPRESSION_TYPE, (FnImpl1*)fltConvert, EXPRESSION_TYPE },
    { "fac", RATIONAL_TYPE, (FnImpl1*)factorRational, RATIONAL_TYPE },
    { "T", ARRAY_TYPE, (FnImpl1*)transposeArray, ARRAY_TYPE },
    { "abs", FLOAT_TYPE, (FnImpl1*)absFloat, FLOAT_TYPE },
    { "abs", FLOAT_TYPE, (FnImpl1*)absComplex, COMPLEX_TYPE },
    { "abs", FLOAT_TYPE, (FnImpl1*)absArray, ARRAY_TYPE },
    { "sq", FLOAT_TYPE, (FnImpl1*)sqFloat, FLOAT_TYPE },
    { "sq", COMPLEX_TYPE, (FnImpl1*)sqComplex, COMPLEX_TYPE },
    { "sq", RATIONAL_TYPE, (FnImpl1*)sqRational, RATIONAL_TYPE },
    { "cubeRoot", FLOAT_TYPE, (FnImpl1*)cubeRootFloat, FLOAT_TYPE },
    { "cubeRoot", COMPLEX_TYPE, (FnImpl1*)cubeRootComplex, COMPLEX_TYPE },
    { "purge", EXPRESSION_TYPE, (FnImpl1*)purgeExpr, EXPRESSION_TYPE },
    { "proot", ARRAY_TYPE, (FnImpl1*)prootArray, ARRAY_TYPE },
    { "mjd", FLOAT_TYPE, (FnImpl1*)mjdFloat, FLOAT_TYPE },
    { "date", FLOAT_TYPE, (FnImpl1*)dateFloat, FLOAT_TYPE },
    { "prime", RATIONAL_TYPE, (FnImpl1*)isPrimeRational, RATIONAL_TYPE },
    { "conj", COMPLEX_TYPE, (FnImpl1*)conjComplex, COMPLEX_TYPE },
    { "np", RATIONAL_TYPE, (FnImpl1*)nextPrimeRational, RATIONAL_TYPE },
    { "pp", RATIONAL_TYPE, (FnImpl1*)prevPrimeRational, RATIONAL_TYPE },
    { "floor", FLOAT_TYPE, (FnImpl1*)floorFloat, FLOAT_TYPE },
    { "int", FLOAT_TYPE, (FnImpl1*)floorFloat, FLOAT_TYPE },
    { "ran", RATIONAL_TYPE, (FnImpl1*)ranRational, RATIONAL_TYPE },
    { "sran", RATIONAL_TYPE, (FnImpl1*)sranRational, RATIONAL_TYPE },
    { "ran", FLOAT_TYPE, (FnImpl1*)ranFloat, FLOAT_TYPE },
    { "erf", FLOAT_TYPE, (FnImpl1*)erfFloat, FLOAT_TYPE },
    { "norm", FLOAT_TYPE, (FnImpl1*)normFloat, FLOAT_TYPE },

    // alias q to normal for legacy reasons
    { "q", FLOAT_TYPE, (FnImpl1*)normFloat, FLOAT_TYPE }, 
#ifdef _WIN32
    { "dump", FLOAT_TYPE, (FnImpl1*)dumpFloat, FLOAT_TYPE },
#endif
};

static const Fn2ImplRec InitialFn2ImplTable[] =
{
    { "//", FLOAT_TYPE, (FnImpl2*)parallelFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "+", FLOAT_TYPE, addFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "-", FLOAT_TYPE, (FnImpl2*)subFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "*", FLOAT_TYPE, (FnImpl2*)mulFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "/", FLOAT_TYPE, (FnImpl2*)divFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "^", FLOAT_TYPE, (FnImpl2*)powFloat, FLOAT_TYPE, FLOAT_TYPE },

    { "//", RATIONAL_TYPE, (FnImpl2*)parallelRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "//", COMPLEX_TYPE, (FnImpl2*)parallelComplex, COMPLEX_TYPE, COMPLEX_TYPE },
    { "+", RATIONAL_TYPE, (FnImpl2*)addRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "-", RATIONAL_TYPE, (FnImpl2*)subRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "*", RATIONAL_TYPE, (FnImpl2*)mulRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "/", RATIONAL_TYPE, (FnImpl2*)divRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "^", RATIONAL_TYPE, (FnImpl2*)powRational, RATIONAL_TYPE, RATIONAL_TYPE },

    { "+", COMPLEX_TYPE, (FnImpl2*)addComplex, COMPLEX_TYPE, COMPLEX_TYPE },
    { "-", COMPLEX_TYPE, (FnImpl2*)subComplex, COMPLEX_TYPE, COMPLEX_TYPE },
    { "*", COMPLEX_TYPE, (FnImpl2*)mulComplex, COMPLEX_TYPE, COMPLEX_TYPE },
    { "/", COMPLEX_TYPE, (FnImpl2*)divComplex, COMPLEX_TYPE, COMPLEX_TYPE },
    { "^", COMPLEX_TYPE, (FnImpl2*)powComplex, COMPLEX_TYPE, COMPLEX_TYPE },

    { "+", ARRAY_TYPE, (FnImpl2*)addArrayScalar, ARRAY_TYPE, NUMBER_TYPE },
    { "+", ARRAY_TYPE, (FnImpl2*)addScalarArray, NUMBER_TYPE, ARRAY_TYPE },
    { "+", ARRAY_TYPE, (FnImpl2*)addArrayArray, ARRAY_TYPE, ARRAY_TYPE },
    { "*", ARRAY_TYPE, (FnImpl2*)mulArrayScalar, ARRAY_TYPE, NUMBER_TYPE },
    { "*", ARRAY_TYPE, (FnImpl2*)mulScalarArray, NUMBER_TYPE, ARRAY_TYPE },
    { "*", ARRAY_TYPE, (FnImpl2*)mulArrayArray, ARRAY_TYPE, ARRAY_TYPE },
    { "-", ARRAY_TYPE, (FnImpl2*)subArrayScalar, ARRAY_TYPE, NUMBER_TYPE },
    { "-", ARRAY_TYPE, (FnImpl2*)subScalarArray, NUMBER_TYPE, ARRAY_TYPE },
    { "-", ARRAY_TYPE, (FnImpl2*)subArrayArray, ARRAY_TYPE, ARRAY_TYPE },
    { "/", ARRAY_TYPE, (FnImpl2*)divArrayScalar, ARRAY_TYPE, NUMBER_TYPE },
    { "/", ARRAY_TYPE, (FnImpl2*)divScalarArray, NUMBER_TYPE, ARRAY_TYPE },
    { "/", ARRAY_TYPE, (FnImpl2*)divArrayArray, ARRAY_TYPE, ARRAY_TYPE },

    { "mod", RATIONAL_TYPE, (FnImpl2*)modRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "mod", FLOAT_TYPE, (FnImpl2*)modFloat, FLOAT_TYPE, FLOAT_TYPE },

    { "=", EXPRESSION_TYPE,
      (FnImpl2*)varAssign, SYMBOL_TYPE, EXPRESSION_TYPE },    
    { "atan2", FLOAT_TYPE, (FnImpl2*)atan2MF, FLOAT_TYPE, FLOAT_TYPE },
    
    { "nRoot", RATIONAL_TYPE, (FnImpl2*)nRootRational, RATIONAL_TYPE, RATIONAL_TYPE },
    { "nRoot", FLOAT_TYPE, (FnImpl2*)nthRootFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "nRoot", COMPLEX_TYPE, (FnImpl2*)nthRootComplex, COMPLEX_TYPE, COMPLEX_TYPE },

    { "rat", EXPRESSION_TYPE, (FnImpl2*)ratFloat, FLOAT_TYPE, FLOAT_TYPE },
    { "nran", RATIONAL_TYPE, (FnImpl2*)nranRational, RATIONAL_TYPE, RATIONAL_TYPE },
};

static const FnnImplRec InitialFnnImplTable[] =
{
    { "plot", EXPRESSION_TYPE, (FnImpl*)plotExpr, 3,
      EXPRESSION_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },
    { "solve", EXPRESSION_TYPE, (FnImpl*)solveExpr, 3,
      EXPRESSION_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },

    { "fma", FLOAT_TYPE, (FnImpl*)fmaFloat, 3,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },

    { "plot3d", EXPRESSION_TYPE, (FnImpl*)plot3DExpr, 5,
      EXPRESSION_TYPE,
      FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE
    },

    { "plot3d", EXPRESSION_TYPE, (FnImpl*)plot3DExpr2, 3,
      EXPRESSION_TYPE,
      FLOAT_TYPE, FLOAT_TYPE
    },

    { "plot3d", EXPRESSION_TYPE, (FnImpl*)plot3DExpr3, 4,
      EXPRESSION_TYPE,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },

    { "price", FLOAT_TYPE, (FnImpl*)bondPriceFloat, 6,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },

    { "mduration", FLOAT_TYPE, (FnImpl*)bondMDurationFloat, 6,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE, FLOAT_TYPE
    },
#if 0
    // finance functions
    { "fv", FLOAT_TYPE, (FnImpl*)FinMFFv, 5,
      FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE,
      RATIONAL_TYPE },
    { "fv", FLOAT_TYPE, (FnImpl*)FinMFFv4, 4,
      FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE },
    { "pmt", FLOAT_TYPE, (FnImpl*)FinMFPmt, 5,
      FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE,
      RATIONAL_TYPE },
    { "pmt", FLOAT_TYPE, (FnImpl*)FinMFPmt4, 4,
      FLOAT_TYPE, FLOAT_TYPE,
      FLOAT_TYPE, FLOAT_TYPE },
    { "solve", EXPRESSION_TYPE, (FnImpl*)SolveFN, 3,
      EXPRESSION_TYPE, FLOAT_TYPE, FLOAT_TYPE },
#endif
};


void InitFunctions()
{
    int i;

    // assign random engine
    ranq.reset(); 
    nran.ran(&ranq);

    /* put the Function symbols into the global table */
    SymbolTable& st = SymbolTable::global();

    for (i = 0; i < DIM(InitialFn1ConvTable); ++i) 
    {
        const Fn1ImplRec* fr = InitialFn1ConvTable + i;
        Symbol* s = st.intern(fr->name_, strlen(fr->name_));
        RegInfo* ri = RegInfo::create(s, fr->rt_, 1, fr->t1_);
        ri->impl_ = (FnImpl*)fr->impl_;
        ri->setConvFn();
        theFnRegistry.intern(ri);
    }

    for (i = 0; i < DIM(InitialFn0ImplTable); ++i) 
    {
        const Fn0ImplRec* fr = InitialFn0ImplTable + i;
        Symbol* s = st.intern(fr->name_, strlen(fr->name_));
        RegInfo* ri = RegInfo::create(s, fr->rt_, 0);
        ri->impl_ = fr->impl_;
        theFnRegistry.intern(ri);
    }

    for (i = 0; i < DIM(InitialFn1ImplTable); ++i) 
    {
        const Fn1ImplRec* fr = InitialFn1ImplTable + i;
        Symbol* s = st.intern(fr->name_, strlen(fr->name_));
        RegInfo* ri = RegInfo::create(s, fr->rt_, 1, fr->t1_);
        ri->impl_ = (FnImpl*)fr->impl_;
        theFnRegistry.intern(ri);
    }

    for (i = 0; i < DIM(InitialFn2ImplTable); ++i) 
    {
        const Fn2ImplRec* fr = InitialFn2ImplTable + i;
        Symbol* s = st.intern(fr->name_, strlen(fr->name_));
        RegInfo* ri = RegInfo::create(s, fr->rt_, 2, fr->t1_, fr->t2_);
        ri->impl_ = (FnImpl*)fr->impl_;
        theFnRegistry.intern(ri);
    }

    for (i = 0; i < DIM(InitialFnnImplTable); ++i) 
    {
        const FnnImplRec* fr = InitialFnnImplTable + i;
        Symbol* s = st.intern(fr->name_, strlen(fr->name_));
        RegInfo* ri = RegInfo::createN(s, fr->rt_, fr->nargs_, fr->t_);
        ri->impl_ = fr->impl_;
        theFnRegistry.intern(ri);
    }
}
