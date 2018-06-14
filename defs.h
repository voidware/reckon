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

#ifndef __defs_h__ 
#define __defs_h__

#define DIM(_X)                 (sizeof(_X)/sizeof((_X)[0]))

#define VERSION "v2.2"

#ifdef __cplusplus

#ifdef _WIN32
typedef unsigned __int64 uint64_t;
#define INT64(_a,_b)   ((((uint64_t)(_a))<<32)|(_b))
#else

#ifdef __GNUC__
typedef unsigned long long uint64_t;
#define INT64(_a,_b)   ((((uint64_t)(_a))<<32)|(_b))
#else

// no 64 bit definition
#include "int64.h"
typedef Int64 uint64_t;
#define INT64(_a,_b)   Int64(_a,_b)
#endif
#endif
 
#endif // __cplusplus

#endif /* __defs_h__ */
