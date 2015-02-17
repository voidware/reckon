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

#include "fxlib.h"
#include "stdlib.h"
#include "stdio.h"
#include "defs.h"
#include "utils.h"
#include "common.h"

static int GetLine(char* buf, unsigned int size)
{
    unsigned int k;
    char* p = buf;
    int i;
    const KeyWord* kw;
    char* q;

    while (p - buf < size-1)
    {
        k = GetKeyNonblocking(1, 0); // block
        if (k == KEY_CTRL_HELP || k == KEY_CTRL_F1)
        {
            displayInfo();
            return -1;
        }
        if (k == KEY_CTRL_EXE) break;
        if (k == KEY_CTRL_AC || k == KEY_CTRL_OPTN)
        {
            return k; // escape
        }

        // translate manually, since different for RPN.
        if (k == KEY_CHAR_PMINUS) k = '-';

        k = translateKey(k);
        
        // check the key words expansion
        kw = findExpandKey(k, 1); // ALG mode
        if (kw)
        {
            const char* s = kw->keyword;
            while (*s && p - buf < size-1)
            {
                *p = *s++;
                printc(p);
                ++p;
            }
            k = 0;
        }
        
        if (k == KEY_CTRL_DEL)
        {
            // delete key
            if (p > buf)
            {
                *--p = '\b';
                printc(p);
                if (p > buf && ESC_CHAR(p[-1]))
                    --p;
            }
        }
        else if (k > 0)
        {
            q = p;
            if (k > 0xff) // extended
                *p++ = k >> 8;
            *p++ = k;
            printc(q);
        }
    }
    *p = 0;
    return 0;
}

extern void eval(const char* exp, char** const result);
extern void eval_init();
extern void eval_end();
extern void do_rpn();

//****************************************************************************
//  AddIn_main (Sample program main function)
//
//  param   :   isAppli   : 1 = This application is launched by MAIN MENU.
//                        : 0 = This application is launched by a strip in eACT application.
//
//              OptionNum : Strip number (0~3)
//                         (This parameter is only used when isAppli parameter is 0.)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;
    static char buf[64];
    const char* res;

    eval_init();

    Bdisp_AllClr_DDVRAM();
    AC();

    print("Reckon " VERSION);
    newline();

    for (;;)
    {

        print("> ");

        key = GetLine(buf, sizeof(buf));
        if (!key)
        {
            eval(buf, &res);
            newline();
            print(res);
            newline();
        }
        else
        {
            if (key == KEY_CTRL_OPTN)
            {
                do_rpn();
            }
            AC();
        }
    }

    eval_end();

    return 1;
}




//****************************************************************************
//**************                                              ****************
//**************                 Notice!                      ****************
//**************                                              ****************
//**************  Please do not change the following source.  ****************
//**************                                              ****************
//****************************************************************************


#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

//****************************************************************************
//  InitializeSystem
//
//  param   :   isAppli   : 1 = Application / 0 = eActivity
//              OptionNum : Option Number (only eActivity)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section

