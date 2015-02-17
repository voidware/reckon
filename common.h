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

#ifndef __common_h__
#define __common_h__

typedef struct
{
    int x;
    int y;
} Pos;

extern Pos pos;

typedef struct
{
    unsigned short keycode;
    unsigned short keyval;
} KeyMap;

typedef struct
{
    unsigned short keycode;
    char* keyword;
    short nargs;
    short flags;
} KeyWord;

#define ESC_CHAR(_c) ((_c) == 0x7f || ((unsigned char)(_c)) > 0xE0)

/* NOTE: this keycode is not defined in the system header
 * i made it up! There is no code for the OFF key.
 */
#define KEY_CTRL_OFF    30200
#define KEY_CTRL_SHIFT_ARROW 30201
#define KEY_CTRL_HELP 30202

extern void displayInfo();
extern void AC();
extern void scroll1();
extern void newline();
extern int printc(char*);
extern void print(char*);
extern unsigned int translateKey(unsigned int);
extern void clearLine(unsigned int l);
extern void printAt(const char* s, int x, int y);
extern const KeyWord* findExpandKey(unsigned int, unsigned int);
extern unsigned int waitForKey();
extern void refreshScreen();
extern unsigned int GetKeyNonblocking(int block, int repeat);
extern int EscapeKeyPressed();

extern void CPUSpeedNormal();
extern void CPUSpeedFast();

extern void StartTimer();
extern unsigned long ReadTimer();
extern void StopTimer();
extern void Update3DScreen();

#endif // __common_h__
