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

#ifndef __cutils_h__
#define __cutils_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DIM
#define DIM(_x)  (sizeof(_x)/sizeof(_x[0]))
#endif

// define our own set here
#define u__tolower(_c)    ( (_c)-'A'+'a' )
#define u__toupper(_c)    ( (_c)-'a'+'A' )

#define u_tolower(_c)     (u_isupper(_c) ? u__tolower(_c) : _c)
#define u_toupper(_c)     (u_islower(_c) ? u__toupper(_c) : _c)

#define u_isalpha(_c)  (u_isupper(_c) || u_islower(_c))
#define u_isupper(_c)  (((_c) >= 'A') & ((_c) <= 'Z'))
#define u_islower(_c)  (((_c) >= 'a') & ((_c) <= 'z'))
#define u_isdigit(_c)  (((_c) >= '0') & ((_c) <= '9'))
#define u_isspace(_c)  (((_c) == ' ') | ((_c) == '\t') | ((_c) == '\n'))
#define u_isalnum(_c)  (u_isalpha(_c) || u_isdigit(_c))
#define u_isprint(_c)  (((_c) >= 0x20) & ((_c) <= 127))

int u_strnicmp(const char* s1, const char* s2, int n);

#ifdef __cplusplus
};
#endif



#endif // __cutils_h__
