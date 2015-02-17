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

#ifndef __finance_h__
#define __finance_h__

#include "types.h"

void mjdFloat(TermRef& res, Float* a);
void dateFloat(TermRef& res, Float* a);
void bondPriceFloat(TermRef& res,
                    Float* settlement,
                    Float* maturity,
                    Float* cpn,
                    Float* y,
                    Float* call,
                    Float* m);
void bondMDurationFloat(TermRef& res,
                       Float* settlement,
                       Float* maturity,
                       Float* cpn,
                       Float* y,
                       Float* call,
                       Float* m);

#endif // __finance_h__


