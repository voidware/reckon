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

#include "calc.h"
#include "types.h"

extern "C" 
{
    void eval(const char* exp, const char** result);
    void eval_init();
    void eval_end();
}


void eval_init()
{
    Calc::theCalc = new Calc();
    Calc::theCalc->start();
}

void eval_end()
{
    delete Calc::theCalc;
    Calc::theCalc = 0;
}


void eval(const char* expr, const char** result)
{
    static TermRef resStr = String::create();
    static TermRef errStr = String::create();

    STRING(resStr)->purge();
    STRING(errStr)->purge();
            
    bool v = Calc::theCalc->eval(expr, resStr, errStr);
    if (v) 
        *result = STRING(resStr)->s_;                
    else 
        *result = STRING(errStr)->s_; 
}

