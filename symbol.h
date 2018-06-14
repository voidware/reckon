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

#ifndef __symbol_h__
#define __symbol_h__

#define MAX_SYMBOLS             256

/* Forward Decls */
struct Symbol;

struct SymbolTable
{
    // Constructors
                                SymbolTable() { n_ = 0; }
                                ~SymbolTable();

    Symbol*                     find(const char*, unsigned int);
    Symbol*                     intern(const char*, unsigned int l);
    bool                        remove(const char*);
    Symbol*                     insert(const char* s, unsigned int l);
    static SymbolTable&         global() { return theSymbolTable_; }

private:

    unsigned int                _find(const char*, unsigned int);

    unsigned int                n_;
    Symbol*                     symbol_[MAX_SYMBOLS];
    static SymbolTable          theSymbolTable_;
};

#endif // symbol_h__
