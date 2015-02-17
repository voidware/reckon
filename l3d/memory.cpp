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

#include	<stdlib.h>
#include	<stdarg.h>
#include	<stdio.h>
#include        "os.h"

typedef void* (*FatalHandler)(void*, size_t);

static void* mem_handler(void* ptr, size_t amt)
{
    //print("Out of Memory!");
    //exit(1);
    return 0;
}

FatalHandler fatal_mem_handler = mem_handler;

FatalHandler set_mem_handler(FatalHandler fn)
{
    FatalHandler old_fn = fatal_mem_handler;
    fatal_mem_handler = fn;
    return old_fn;
}

void* xmalloc(size_t amt) 
{
    void* mem = malloc(amt);
    if (!mem) mem = (*fatal_mem_handler)(NULL, amt);
    return mem;
}

void xfree(void* ptr)
{
    if (ptr) free(ptr);
}
