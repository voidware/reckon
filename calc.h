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

#ifndef __calc_h__
#define __calc_h__

#include "types.h"
#include "symbol.h"


struct Calc
{
    // Constructors

    // Destructor
    ~Calc();

    // Features
    void                start();
    void                clearContext();
    bool                eval(const char* p, TermRef& resStr, TermRef& errStr);
    TermRef             eval(TermRef& o);
    TermRef             parse(const char** expr);
    void                setPrecision(unsigned int p)
    {
        _dispFormat.precision(p);
    }
    
    TermRef             approximate(TermRef&);
    void                asString(TermRef& term, TermRef& str);

    bool                storeVar(const char* var, TermRef& t);
    TermRef             recallVar(const char* var);

    void                _init();
    void                _setupConstants();

    TermRef             lastAnswerSymbol_;
    TermRef             trials_;

    TermContext         tc_;
    DispFormat          _dispFormat;
    int                 _maxDecimalDigits;

    char*               inBuf;     // for text input
    size_t              inBufSize;
    
    static Calc*        theCalc;
};

#endif // calc_h__
