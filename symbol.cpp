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

#include <string.h>
#include "types.h"
#include "symbol.h"
#include "cutils.h"

/** SymbolTable ********************************************/

SymbolTable SymbolTable::theSymbolTable_;

SymbolTable::~SymbolTable()
{
    if (this != &theSymbolTable_) {
        for (unsigned int i = 0; i < n_; ++i) {
            if (!symbol_[i]->decRef()) {
                    Term::destroy(symbol_[i]);
            }
        }
    }
}

unsigned int SymbolTable::_find(const char* s, unsigned int l)
{
    /* return slot+1 */
    unsigned int i, j;
    Symbol** sym = &symbol_[0];
    i = n_;
    j = 1;
    while (i) {
        const char* sn = (*sym)->name_;
        if (strlen(sn) == l && !u_strnicmp(sn, s, l)) return j;
        ++sym;
        --i;
        ++j;
    }
    return 0;
}

Symbol* SymbolTable::find(const char* name, unsigned int l)
{
    Symbol* s = 0;
    unsigned int i = _find(name, l);
    if (i) {
        s = symbol_[i-1];
    }
    return s;
}

Symbol* SymbolTable::insert(const char* s, unsigned int l)
{
    /* NOTE: only use when not already present */
    Symbol* sym = 0;
    if (n_ < MAX_SYMBOLS-1) {
        sym = Symbol::create(s, l);
        sym->table_ = this;
        symbol_[n_++] = sym;

        /* NOTE: symbol table references are not counted
         * in the global table.
         */
        if (this != &theSymbolTable_) {
            sym->incRef();
        }
    }
    return sym;
}

Symbol* SymbolTable::intern(const char* s, unsigned int l)
{
    Symbol* sym = find(s, l);
    if (!sym) sym = insert(s, l);
    return sym;
}

bool SymbolTable::remove(const char* s)
{
    unsigned int i = _find(s, strlen(s));
    if (i) {
        symbol_[i-1] = symbol_[n_-1];
        symbol_[--n_] = 0;
        return true;
    }
    return false;
}
