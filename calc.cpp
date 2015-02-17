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
#include "calc.h"

/** Calc ************************************************************/

Calc* Calc::theCalc;

Calc::~Calc()
{
    clearContext();
    lastAnswerSymbol_ = 0;
    delete inBuf;
}

void Calc::start()
{
    _maxDecimalDigits = 512; // approx!

    InitSymbols();
    InitFunctions();
    InitBig();
    setPrecision(16);

    // XX console mode hack
    setPrecision(25); 

    inBufSize = 64;
    inBuf = new char[inBufSize]; 

    /* create initial symbol table */
    tc_.push();

    /* make a special symbol to represent the last answer */
    lastAnswerSymbol_ = tc_.internSymbol("Ans", 3);
    trials_ = tc_.internSymbol("trials", 6);

    // add in the physical constants
    _setupConstants();
}

void Calc::clearContext()
{
    delete tc_.pop();
    tc_.push();
}

void Calc::asString(TermRef& t, TermRef& str)
{
    if (t)
        t->asString(str, &_dispFormat);
}

bool Calc::eval(const char* p, TermRef& resStr, TermRef& errStr)
{
    TermRef o(Term::parse(&p, tc_));

    if (*p) 
    {
        STRING(errStr)->append("syntax error");
        return false;
    }

    if (!o)
    {
        // revaluate last expression
        o = lastAnswerSymbol_;
    }

    TermRef res = eval(o);

    if (res.valid()) 
    {
        // stringify result
        res->asString(resStr, &_dispFormat);
    }
    else 
    {
        STRING(errStr)->append("failed to reduce");
        return false;
    }
    return true;
}

TermRef Calc::eval(TermRef& o)
{
    TermRef res;
    if (o) 
    {
        TermRef r = o;
        for (;;)
        {
            bool f = r->reduce(res, tc_);
            if (!f || !res) break;
            r = res;
        }

        /* remember the last answer.        
         * unless we are evaluating "ans", which case case dont
         */
        if (res.valid() && o != lastAnswerSymbol_)
            SYMBOL(lastAnswerSymbol_)->assign(*res);
    }

    return res;
}

TermRef Calc::approximate(TermRef& t)
{
    if (t) 
    {
        if (ISRATIONAL(t)) 
        {
            TermRef s;
            if (t->convert(s, Float::mytype())) 
                return s;
        }
        else if (ISFUNCTION(t)) 
        {
            Function* f = FUNCTION(t);
            unsigned int i;
            for (i = 0; i < f->size(); ++i) 
            {
                TermRef s = ARG(f, i);
                f->changeArg(i, *approximate(s));
            }
        }
    }
    return t;
}

TermRef Calc::parse(const char** ep)
{
    const char* expr = *ep;

    TermRef res;
    if (expr && *expr) {
        res = Term::parse(ep, tc_);
    }
    return res;
}

bool Calc::storeVar(const char* var, TermRef& t)
{
    Symbol* s = tc_.internSymbol(var, strlen(var));
    if (s) {
        s->assign(*t);
        return true;
    }
    return false;
}

TermRef Calc::recallVar(const char* var)
{
    TermRef val;

    Symbol* s = tc_.internSymbol(var, strlen(var));
    if (s) {
        val = s->v_;
    }
    return val;
}

typedef unsigned short Dig[P+1];

struct ConstDef
{
    const char* name;
    Dig d;
};

static const ConstDef ConstTable[] =
{
    CUSTOM_PI_STRING, { 3, 1415, 9265, 3589, 7932, 3846, 2643, 1 }, // pi

    "#g",  { 9, 8066, 5000, 0, 0, 0, 0, 1 },

    // length
    "#au", { 1495, 9787, 691, 0, 0, 0, 0, 3 },
    "#in", { 254, 0, 0, 0, 0, 0, 0, 0 },
    "#ft", { 3048, 0, 0, 0, 0, 0, 0, 0 },
    "#yd", { 9144, 0, 0, 0, 0, 0, 0, 0 },
    "#mi", { 1609, 3440, 0, 0, 0, 0, 0, 1 },
    "#nmi", { 1852, 0, 0, 0, 0, 0, 0, 1 },
    "#pc", { 3, 856, 7758, 1305, 7289, 5329, 1552, 5 }, // parsec = 648000/pi*au
    "#ly", { 9460, 5362, 706, 8016, 0, 0, 0, 4 }, // light year = #yr*#c
    "#moonr", { 173, 7100, 0, 0, 0, 0, 0, 2 }, // moon mean radius
    "#earthr", { 637, 1000, 0, 0, 0, 0, 0, 2 }, // earth mean radius
    "#sunr", { 6, 9600, 0, 0, 0, 0, 0, 3 }, // sun mean radius
    "#fathom", { 1, 8288, 0, 0, 0, 0, 0, 1 }, // 6 feet

    // speed
    "#mph", { 4470, 4000, 0, 0, 0, 0, 0, 0 },
    "#c", { 2, 9979, 2458, 0, 0, 0, 0, 3 }, 

    // time
    "#yr", { 3155, 6952, 0, 0, 0, 0, 0, 2 }, // calendar year
    "#sday", { 8, 6164, 910, 0, 0, 0, 0, 2 }, // siderial day
    "#lday", { 255, 1443, 0, 0, 0, 0, 0, 2 }, // lunar day = synodic period 29 days
    "#syr", { 3155, 8149, 7635, 4560, 0, 0, 0, 2 }, // siderial year

    "#GG", { 66, 7428, 0, 0, 0, 0, 0, 32766&EXPMASK }, // gravitational constant

    // mass & weight
    "#oz", { 283, 4952, 3125, 0, 0, 0, 0, 0 },
    "#ozt", { 311, 347, 6800, 0, 0, 0, 0, 0 },
    "#lb", { 4535, 9237, 0, 0, 0, 0, 0, 0 },
    "#me", { 91, 938, 2154, 5000, 0, 0, 0, 8185 }, // electron
    "#mp", { 16, 7262, 1637, 8300, 0, 0, 0, 8186 }, // proton
    "#mn", { 16, 7492, 7292, 8000, 0, 0, 0, 8186 }, // neutron
    "#earth", { 5, 9736, 0, 0, 0, 0, 0, 7 }, // earth mass
    "#moon", { 734, 7700, 0, 0, 0, 0, 0, 6 }, // moon mass
    "#sun", { 198, 8920, 0, 0, 0, 0, 0, 8 }, // sun mass
    "#ton", { 1016, 469, 880, 0, 0, 0, 0, 1 }, // long ton, 2240lb
    "#cwt", { 50, 8023, 4544, 0, 0, 0, 0, 1 }, // hundredweight, uk 112lb
    
    // area
    "#acre", { 4046, 8564, 2240, 0, 0, 0, 0, 1 }, 

    // volume
    "#pt", { 5, 6826, 1250, 0, 0, 0, 0, 0 },  // uk pint in m^3
    "#foz", { 2841, 3062, 5000, 0, 0, 0, 0, 8191 }, // uk floz
    "#gal", { 45, 4609, 0, 0, 0, 0, 0, 0 }, // uk gallon

    "#ptus", { 4, 7317, 6473, 0, 0, 0, 0, 0 }, // us pint
    "#fozus", { 2957, 3529, 5625, 0, 0, 0, 0, 8191 }, // us floz
    "#galus", { 37, 8541, 1784, 0, 0, 0, 0, 0 }, // us gallon

    // pressure
    "#atm", { 10, 1325, 0, 0, 0, 0, 0, 2 },  // 1 atomos N/m^2 (Pa)
    "#psi", { 6894, 7572, 9316, 8361, 3367, 2267, 3000, 1 }, // Pa

    // other
    "#k", { 13, 8065, 424, 0, 0, 0, 0, 8187 },     // boltzmann J/K

    // energy
    "#ev", { 16, 217, 6531, 4000, 0, 0, 0, 8188 },  // electron volt J

    "#h", { 662, 6068, 9633, 0, 0, 0, 0, 8184 }, // plank Js

    "#e", { 16, 217, 6487, 4000, 0, 0, 0, 8188 }, // electron charge C

    "#na", { 6022, 1415, 1000, 0, 0, 0, 0, 6 }, // avagadro mol^-1
    "#R", { 8, 3144, 7215, 0, 0, 0, 0, 1 }, // gas constant J/K/mol
    "#hp", { 745, 7000, 0, 0, 0, 0, 0, 1 }, // mechanical horspower Watts
    
#if 0
    // constants from old calculator TBD
    "gr", { 0 },
    "btu", { 0 },
    "fth", { 0 },
    "fath", { 0 },
    "fur", { 0 },
    "furlong", { 0 },
    "ch", { 0 },
    "chain", { 0 },
    "dr", { 0 },
    "dram", { 0 },
    "bar", { 0 },
    "galUS", { 0 },
    "bbl", { 0 },
    "barrel", { 0 },
    "cubit", { 0 },
    "gi", { 0 },
    "gill", { 0 },
    "ha", { 0 },
    "hectare", { 0 },
    "lbt", { 0 },
    "siday", { 0 },
    "vm", { 0 },
    "sun", { 0 },
    "lday", { 0 },
    "tonUK", { 0 },
    "ton", { 0 },
    "tonUS", { 0 },
#endif
};

void Calc::_setupConstants()
{
    int i;
    for (i = 0; i < DIM(ConstTable); ++i)
    {
        const ConstDef* c = ConstTable + i;
        Symbol* sym = tc_.internSymbol(c->name, strlen(c->name));
        if (sym)
        {
            // place refence so it will never be deleted.
            sym->incRef();
            BCD v(*(const BCDFloat*)c->d);
            sym->assign(Float::create(DPD(v)));
        }
    }
}

