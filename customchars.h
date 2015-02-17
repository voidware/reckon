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

/* Definitions of character to be accepted as well as normal ascii */

#ifndef __customchars_h__
#define __customchars_h__

#ifndef _WIN32
#define CUSTOM_EXP_CHAR   0x0F
#endif

#define CUSTOM_PI_CHAR    0xE54F
#define CUSTOM_PI_STRING  "\xE5\x4f"

#define CUSTOM_DEG_CHAR   0x9C
#define CUSTOM_DEG_STRING "\x9C"

#define CUSTOM_ANG_CHAR  0x7f54
#define CUSTOM_ANG_STRING "\x7f\x54"

// compare char* to a value, bytewise to avoid alignment problems
#define COMPARE_CUSTOM(_p, _v) \
(((const unsigned char*)(_p))[0] == (((unsigned int)(_v)) >> 8) &&  \
 ((const unsigned char*)(_p))[1] == ((_v) & 0xff))

#endif // __customchars_h__
