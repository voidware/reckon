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

#ifndef __utils_h__
#define __utils_h__

/* declare functions defined in assembler */

extern unsigned char* GetVRAMPtr();

/*
  col returns the keyboard matrix column 
  row returns the keyboard matrix row 
  type_of_waiting and timeout are the SDK's parameters 
  P5 must be set to 0 
  keycode returns a keycode, if it has been previously injected with
  syscall 0x910, only. It does not return a manually entered keycode. 
  the function's return value is 1 if a key has been hit, else 0. 

  NB: the keyboard matrix codes are different on standard FX9860 and slim! 
*/

extern int KBD_GetKeyWait(int* col, int* row,
                          int type_of_waiting,
                          int timeout,
                          int P5,
                          unsigned short* keycode);

extern int OSVersion(char *a, char *b, short int *c, short int *d); 
extern void OSPowerOff(int displayLogo);
extern void OSInjectKey(int a, int b, int key, int c);


#endif // __utils_h__


