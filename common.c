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

#include "common.h"
#include "fxlib.h"
#include "utils.h"
#include "defs.h"
#include "timer.h"
#include "customchars.h"
#include "cutils.h"

Pos pos;

#define _Print(_s) Print((const unsigned char*)(_s))
#define _PrintC(_s) PrintC((const unsigned char*)(_s))
#define _PrintMini(_x, _y, _s, _t) PrintMini(_x, _y, (const unsigned char*)(_s), _t)

void displayInfo()
{
    unsigned short c = 0xe59e;
    unsigned int k;
    PopUpWin(4);
    locate(3,2);
    _Print("Reckon " VERSION);
    locate(3,3);
    _Print("(");
    _PrintC(&c);
    _Print(") Voidware 2011");
    _PrintMini(13, 24, "www.voidware.com/reckon", MINI_OVER);
    GetKey(&k);
}

static const KeyMap TranslateKeys[] =
{
    { KEY_CHAR_EXP, CUSTOM_EXP_CHAR },
    { KEY_CHAR_PLUS, '+' },
    { KEY_CHAR_MINUS, '-' },
    { KEY_CHAR_MULT, '*' },
    { KEY_CHAR_DIV, '/' },
    { KEY_CHAR_LPAR, '(' },
    { KEY_CHAR_RPAR, ')' },
    { KEY_CHAR_POW, '^' },
    { KEY_CHAR_EQUAL, '=' },
    { KEY_CHAR_STORE, '=' },
    { KEY_CTRL_XTT, 'X' },
    { KEY_CTRL_VARS, '#' },
    { KEY_CHAR_IMGNRY, 'i' }, // use fancy letter later
    { KEY_CHAR_PI, CUSTOM_PI_CHAR }, // fancy pi.
    { KEY_CHAR_THETA, CUSTOM_DEG_CHAR },
    { KEY_CHAR_ANGLE, CUSTOM_ANG_CHAR },
};


// n >= 0 => arity
// -1 => constant
// -2 => special

// flags
// 0 => any
// 1 => alg
// 2 => RPN

static const KeyWord ExpandKeys[] =
{
    { KEY_CHAR_ANS, "Ans", -1 },
    { KEY_CHAR_LOG, "Log(", 1 },
    { KEY_CHAR_LN, "Ln(", 1 },
    { KEY_CHAR_SIN, "Sin(", 1 },
    { KEY_CHAR_COS, "Cos(", 1 },
    { KEY_CHAR_TAN, "Tan(", 1 },
    { KEY_CHAR_SQUARE, "^2", -2, 1 }, // ALG
    { KEY_CHAR_SQUARE, "Sq(", 1, 2 }, // RPN
    { KEY_CHAR_CUBEROOT, "^(1/3)", -2, 1 }, // ALG
    { KEY_CHAR_CUBEROOT, "CubeRoot(", 1, 2 }, // RPN
    { KEY_CHAR_EXPN10, "Alog(", 1 },
    { KEY_CHAR_EXPN, "Exp(", 1 },
    { KEY_CHAR_ASIN, "ASin(", 1 },
    { KEY_CHAR_ACOS, "ACos(", 1 },
    { KEY_CHAR_ATAN, "ATan(", 1 },
    { KEY_CHAR_ROOT, "Sqrt(", 1 },
    { KEY_CHAR_RECIP, "Inv(", 1 },
    { KEY_CTRL_FD, "flt(", 1 },
    { KEY_CTRL_F3, "Plot3D(", 5 },
    { KEY_CTRL_F4, "Plot(", 3 },
    { KEY_CTRL_F5, "Solve(", 3 },
    { KEY_CHAR_MAT, "[[", -2 },
    { KEY_CHAR_FRAC, "neg(", 1 },
    { KEY_CTRL_FRACCNVRT, "abs(", 1 },
    { KEY_CHAR_POWROOT, "nRoot(", 2 },
    { '!', "Factorial(", 1, 2 }, // RPN
    { KEY_CTRL_CLIP, "Purge(", 1},
    { CUSTOM_DEG_CHAR, "DmsRad(", 1, 2}, // RPN
    { CUSTOM_ANG_CHAR, "RadDms(", 1, 2}, // RPN
    { KEY_CTRL_SHIFT_ARROW, "T(", 1, 2}, // RPN
};

const KeyWord* findExpandKey(unsigned int k, unsigned int flags)
{
    int i;
    for (i = 0; i < DIM(ExpandKeys); ++i)
    {
        KeyWord* kw = ExpandKeys + i;
        if (kw->keycode == k 
            && (!kw->flags || kw->flags == flags))
            return kw;
    }
    return 0;
}

void AC()
{
    Bdisp_AllClr_DDVRAM();
    pos.x = 1;
    pos.y = 1;
    locate(pos.x, pos.y);
    Bdisp_PutDisp_DD();
}

void clearLine(unsigned int l)
{
    if (l < 8)
    {
        unsigned char* d = GetVRAMPtr();
        memset(d + 128*l, 0, 128); // erase line    
        pos.x = 1; pos.y = l + 1;
        locate(pos.x, pos.y);
        Bdisp_PutDisp_DD();
    }
}

void scroll1()
{
    // scroll 1 line
    unsigned char* d = GetVRAMPtr();
    unsigned char* p = d + 128;  // 128 bytes per line
    memmove(d, p, 128*7); // move 7 lines
    memset(d + 128*7, 0, 128); // erase last line    

    pos.x = 1; 
    pos.y = 8;
    locate(pos.x, pos.y);
    Bdisp_PutDisp_DD();
}

void newline()
{
    pos.x = 1;
    if (++pos.y == 9)
        scroll1();
    else
        locate(pos.x, pos.y);
}


int _printc(char* c)
{
    int cc = 1;
    if (*c == '\b')
    {
        if (pos.x > 1) --pos.x;
        else if (pos.y > 1)
        {
            --pos.y;
            pos.x = 21;
        }
        else return cc;

        locate(pos.x, pos.y);
        *c = ' ';
        PrintC((unsigned char*)c);
        locate(pos.x, pos.y);
    }
    else if (*c == '\n')
        newline();
    else 
    {
        PrintC((unsigned char*)c);
        if (ESC_CHAR(*c)) ++cc;
        if (++pos.x == 22) newline();
            
    }
    return cc;
}

int printc(char* c)
{
    int s = _printc(c);
    Bdisp_PutDisp_DD();
    return s;
}

void print(char* s)
{
    while (*s)
        s += _printc(s);
    Bdisp_PutDisp_DD();
}

void printAt(const char* s, int x, int y)
{
    pos.x = x; 
    pos.y = y;
    locate(pos.x, pos.y);
    print(s);
}

unsigned int translateKey(unsigned int k)
{
    int i;
    for (i = 0; i < DIM(TranslateKeys); ++i)
    {
        if (TranslateKeys[i].keycode == k)
            return TranslateKeys[i].keyval;
    }
    return k;
}

unsigned int waitForKey()
{
    unsigned int k;
    k = GetKeyNonblocking(1, 0); // block!
    return k;
}

void refreshScreen()
{
    Bdisp_PutDisp_DD();
}

static unsigned long cuTime;
static void timeHandler(void) { ++cuTime; }


void StartTimer()
{
    cuTime = 0;
    SetTimer(ID_USER_TIMER1, 1, timeHandler);
}

unsigned long ReadTimer()
{
    // return time elapsed in ms
    return cuTime * 25UL;
}

void StopTimer()
{
    KillTimer(ID_USER_TIMER1);
}


// ===========================================================================
// BEGIN Section
// ===========================================================================

/* This section of code was taken from "revolution-fx" source code
 * with the following attribute:
 */

/*
	Revolution-FX - Hardware Library for the fx-9860SD, fx-9860G, fx-9860AU, Graph85 and Graph85SD
	Written by kucalc
	
	Current Version: v0.3
	Contact: kucalc@gmail.com
	Homepage: http://revolution-fx.sourceforge.net
	Wiki: http://revolution-fx.sourceforge.net/wiki
	
	This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 License. 
	To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a 
	letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*/

struct st_cpg {                                         /* struct CPG   */
              union {                                   /* FRQCR        */
                    unsigned short WORD;                /*  Word Access */
                    struct {                            /*  Bit  Access */
                           unsigned short      :3;      /*              */
                           unsigned short CKOEN:1;      /*    CKOEN     */
                           unsigned short      :2;      /*              */
                           unsigned short STC  :2;      /*    STC       */
                           unsigned short      :2;      /*              */
                           unsigned short IFC  :2;      /*    IFC       */
                           unsigned short      :2;      /*              */
                           unsigned short _PFC :2;      /*    PFC       */
                           }       BIT;                 /*              */
                    }           FRQCR;                  /*              */
};                                                      /*              */

struct st_wdt {                                         /* struct WDT   */
              union {                                   /* WTCNT        */
                    unsigned char  READ;                /*  Read  Access*/
                    unsigned short WRITE;               /*  Write Access*/
                    }           WTCNT;                  /*              */
              union {                                   /* WTCSR        */
                    union {                             /*  Read  Access*/
                          unsigned char BYTE;           /*   Byte Access*/
                          struct {                      /*   Bit  Access*/
                                 unsigned char TME :1;  /*    TME       */
                                 unsigned char WTIT:1;  /*    WT/IT     */
                                 unsigned char RSTS:1;  /*    RSTS      */
                                 unsigned char WOVF:1;  /*    WOVF      */
                                 unsigned char IOVF:1;  /*    IOVF      */
                                 unsigned char CKS :3;  /*    CKS       */
                                 }      BIT;            /*              */
                         }         READ;                /*              */
                    unsigned short WRITE;               /*  Write Access*/
                    }           WTCSR;                  /*              */
};

#define CPG    (*(volatile struct st_cpg   *)0xFFFFFF80)/* CPG   Address*/
#define WDT    (*(volatile struct st_wdt   *)0xFFFFFF84)/* WDT   Address*/

void CPUSpeedNormal()
{
    WDT.WTCSR.WRITE = 96;
    WDT.WTCNT.WRITE = 0;
    CPG.FRQCR.BIT._PFC = 1;
    CPG.FRQCR.BIT.STC = 0;
}

void CPUSpeedFast()     // double
{
	WDT.WTCSR.WRITE = 96;
	WDT.WTCNT.WRITE = 0;
	CPG.FRQCR.BIT._PFC = 1;
	CPG.FRQCR.BIT.STC = 1;
}

// ===========================================================================
// END Section
// ===========================================================================

// are we the Slim edition?

#if 1

#define PORT_E_DR 0xA4000128 

char IsSlim()
{
    return !( *(char*)PORT_E_DR & 0x08 ); 
} 

static int IsEmulator()
{
    // at 0x8000FFD0 the calculator's 8 byte fingerprint is stored. 
    // these bytes are zero on the emulator. 
    return !( *(int*)0x8000FFD0 ); 
} 

#else

// old version for OS 1.x
int IsSlim()
{
    char a, b;
    short c, d;

    OSVersion(&a, &b, &c, &d);
    return a == 1 && b >= 10;
}
#endif

#define NROWS 10
#define NCOLS 7

// ===========================================================================
// HERE are the key layout matrices for non-slim model
// ===========================================================================

// NON-SHIFTED PORTRAIT LAYOUT
static const unsigned short keyRowColMatrix[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_AC,0,0,0,0,0,0,
    // row2
    0,0,KEY_CTRL_EXE,KEY_CHAR_PMINUS,KEY_CHAR_EXP,'.','0',
    // row3
    0,0,'-','+','3','2','1',
    // row4
    0,0,'/','*','6','5','4',
    // row5
    0,0,0,KEY_CTRL_DEL,'9','8','7',
    // row6
    0,KEY_CHAR_STORE,',',')','(',KEY_CTRL_FD,KEY_CHAR_FRAC,
    // row7
    0,KEY_CHAR_TAN,KEY_CHAR_COS,KEY_CHAR_SIN,KEY_CHAR_LN,KEY_CHAR_LOG,'X', // XTT
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_EXIT,'^',KEY_CHAR_SQUARE,KEY_CTRL_ALPHA,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CTRL_MENU,KEY_CTRL_VARS,KEY_CTRL_OPTN,KEY_CTRL_SHIFT,
        // row10
    0, KEY_CTRL_F6, KEY_CTRL_F5, KEY_CTRL_F4, KEY_CTRL_F3, KEY_CTRL_F2, KEY_CTRL_F1,
};

// SHIFTED PORTRAIT LAYOUT
static const unsigned short keyRowColMatrixShift[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_OFF,0,0,0,0,0,0,
    // row2
    0,0,KEY_CHAR_CR,KEY_CHAR_ANS,KEY_CHAR_PI,'=',KEY_CHAR_IMGNRY,
    // row3
    0,0,']','[','!',KEY_CHAR_MAT,KEY_CHAR_LIST,
    // row4
    0,0,'}','{','r','q',KEY_CTRL_CATALOG,
    // row5
    0,0,0,KEY_CTRL_INS,KEY_CTRL_PASTE,KEY_CTRL_CLIP,KEY_CTRL_CAPTURE,
    // row6
    0,KEY_CTRL_SHIFT_ARROW,'\'',KEY_CHAR_RECIP,KEY_CHAR_CUBEROOT,'h',KEY_CTRL_FRACCNVRT,
    // row7
    0,KEY_CHAR_ATAN,KEY_CHAR_ACOS,KEY_CHAR_ASIN,KEY_CHAR_EXPN,KEY_CHAR_EXPN10,KEY_CHAR_ANGLE,
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_QUIT,KEY_CHAR_POWROOT,KEY_CHAR_ROOT,KEY_CTRL_ALPHA,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CTRL_MENU,KEY_CTRL_VARS,KEY_CTRL_OPTN,KEY_CTRL_SHIFT,
        // row10
    0, KEY_CTRL_F6, KEY_CTRL_F5, KEY_CTRL_F4, KEY_CTRL_F3, KEY_CTRL_F2, KEY_CTRL_F1,
};

// ALPHA PORTRAIT LAYOUT
static const unsigned short keyRowColMatrixAlpha[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_AC,0,0,0,0,0,0,
    // row2
    0,0,KEY_CHAR_CR,KEY_CHAR_ANS,'"',' ','Z',
    // row3
    0,0,'Y','X','W','V','U',
    // row4
    0,0,'T','S','R','Q','P',
    // row5
    0,0,0,KEY_CTRL_DEL,'O','N','M',
    // row6
    0,'L','K','J','I','H','G',
    // row7
    0,'F','E','D','C','B','A',
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_QUIT,KEY_CHAR_THETA,KEY_CHAR_VALR,KEY_CTRL_ALPHA,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CTRL_MENU,KEY_CTRL_VARS,KEY_CTRL_OPTN,KEY_CTRL_SHIFT,
        // row10
    0, KEY_CTRL_F6, KEY_CTRL_F5, KEY_CTRL_F4, KEY_CTRL_F3, KEY_CTRL_F2, KEY_CTRL_F1,
};

// ===========================================================================
// HERE are the key layout matrices for slim model
// ===========================================================================

// NON-SHIFTED LANDSCAPE LAYOUT
static const unsigned short keyRowColMatrixSlim[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_AC,0,0,0,0,0,0,
    // row2
    0,'-',KEY_CTRL_EXE,'/',')','(',0,
    // row3
    0,KEY_CHAR_PMINUS,'+','*',KEY_CTRL_DEL,KEY_CTRL_FD,KEY_CTRL_EXIT,
    // row4
    0,KEY_CHAR_EXP,'3','6','9',KEY_CHAR_FRAC,KEY_CTRL_F6,
    // row5
    0,KEY_CHAR_DP,'2','5','8',KEY_CHAR_TAN,KEY_CTRL_F5,
    // row6
    0,'0','1','4','7',KEY_CHAR_COS,KEY_CTRL_F4,
    // row7
    0,0/*light*/,KEY_CTRL_HELP,KEY_CTRL_VARS,KEY_CHAR_STORE,KEY_CHAR_SIN,KEY_CTRL_F3,
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_OPTN,KEY_CHAR_COMMA,KEY_CHAR_LN,KEY_CTRL_F2,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CHAR_POW,KEY_CHAR_SQUARE,KEY_CHAR_LOG,KEY_CTRL_F1,
    // row10
    0,0,0,KEY_CTRL_ALPHA,KEY_CTRL_SHIFT,'X'/*XTT*/,KEY_CTRL_MENU,
};

// SHIFTED LANDSCAPE LAYOUT
static const unsigned short keyRowColMatrixShiftSlim[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_OFF,0,0,0,0,0,0,
    // row2
    0,']',KEY_CHAR_CR,'}',KEY_CHAR_RECIP,KEY_CHAR_CUBEROOT,0,
    // row3
    0,KEY_CHAR_ANS,'[','{',KEY_CTRL_INS,KEY_CTRL_FD,KEY_CTRL_QUIT,
    // row4
    0,KEY_CHAR_PI,'!','r',KEY_CTRL_PASTE,KEY_CTRL_FRACCNVRT,KEY_CTRL_F6,
    // row5
    0,'=',KEY_CHAR_MAT,'q',KEY_CTRL_CLIP,KEY_CHAR_ATAN,KEY_CTRL_F5,
    // row6
    0,KEY_CHAR_IMGNRY,KEY_CHAR_LIST,KEY_CTRL_CATALOG,KEY_CTRL_CAPTURE,KEY_CHAR_ACOS,KEY_CTRL_F4,
    // row7
    0,0/*light*/,KEY_CTRL_HELP,KEY_CTRL_VARS,KEY_CTRL_SHIFT_ARROW,KEY_CHAR_ASIN,KEY_CTRL_F3,
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_OPTN,'\'',KEY_CHAR_EXPN,KEY_CTRL_F2,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CHAR_POWROOT,KEY_CHAR_ROOT,KEY_CHAR_EXPN10,KEY_CTRL_F1,
    // row10
    0,0,0,KEY_CTRL_ALPHA,KEY_CTRL_SHIFT,KEY_CHAR_ANGLE,KEY_CTRL_MENU,
};


// ALPHA LANDSCAPE LAYOUT
static const unsigned short keyRowColMatrixAlphaSlim[NCOLS*NROWS] =
{ 
    // row1
    KEY_CTRL_AC,0,0,0,0,0,0,
    // row2
    0,'Y',KEY_CHAR_CR,'T','J','I',0,
    // row3
    0,KEY_CHAR_ANS,'X','S',KEY_CTRL_DEL,'H',KEY_CTRL_QUIT,
    // row4
    0,'"','W','R','O','G',KEY_CTRL_F6,
    // row5
    0,' ','V','Q','N','F',KEY_CTRL_F5,
    // row6
    0,'Z','U','P','M','E',KEY_CTRL_F4,
    // row7
    0,0/*light*/,KEY_CTRL_HELP,KEY_CTRL_VARS,'L','D',KEY_CTRL_F3,
    // row8
    0, KEY_CTRL_RIGHT,KEY_CTRL_DOWN,KEY_CTRL_OPTN,'K','C',KEY_CTRL_F2,
        // row9
    0, KEY_CTRL_UP,KEY_CTRL_LEFT,KEY_CHAR_THETA,KEY_CHAR_VALR,'B',KEY_CTRL_F1,
    // row10
    0,0,0,KEY_CTRL_ALPHA,KEY_CTRL_SHIFT,'A',KEY_CTRL_MENU,
};


// ===========================================================================
// 
// ===========================================================================

unsigned int keyMatrix(int row, int col, int shift, int alpha)
{
    int i = (row-1)*NCOLS + col - 1;
    unsigned short* tab;

    static int slimTested;
    static int isSlim;

    if (!slimTested)
    {
        slimTested = 1;
        isSlim = IsSlim();
    }

    if (isSlim)
    {
        tab = keyRowColMatrixSlim; // normal
        if (shift) tab = keyRowColMatrixShiftSlim;
        if (alpha) tab = keyRowColMatrixAlphaSlim;
    }
    else
    {
        tab = keyRowColMatrix; // normal
        if (shift) tab = keyRowColMatrixShift;
        if (alpha) tab = keyRowColMatrixAlpha;
    }

    return tab[i];
}

unsigned int GetKeyNonblocking(int block, int repeat)
{
    unsigned int k;
    int col, row;
    static int lastCol, lastRow;
    static int shift;
    static int alpha;
    static int alphaLock;
    int v;
    int wait;

 again:

    wait = KEYWAIT_HALTOFF_TIMEROFF; // nonblocking option
    if (block)
    {
        wait = KEYWAIT_HALTON_TIMEROFF;

        // clear to get every press
        lastRow = 0;
        lastCol = 0;
    }

    v = KBD_GetKeyWait(&col, &row, wait, 0, 0, &k);

    k = 0;
    if (v == KEYREP_KEYEVENT)  
    {
        // key is down
        if (repeat || (row != lastRow || col != lastCol))
        {
            k = keyMatrix(row, col, shift, alpha|alphaLock);

            if (shift)
                k = tolower(k);

#if 1
            // doesnt work
            if (k == KEY_CTRL_MENU)
            {
                OSInjectKey(-1,-1,KEY_CTRL_MENU,0);
                GetKey(&k);
                k = 0;
            }
#endif

            if (k == KEY_CTRL_OFF)
            {
                k = 0;
                OSPowerOff(1);
                // and then come back here!
            }

            if (k == KEY_CTRL_ALPHA)
            {
                if (shift)
                {
                    // alpha lock
                    alphaLock = 1; 
                    alpha = 1;
                    shift = 0;
                }
                else
                {
                    if (alphaLock)
                    {
                        alphaLock = 0;
                        alpha = 0;
                    }
                    else
                        alpha = !alpha;
                }
                k = 0; // dont return alpha
            }
            else if (k == KEY_CTRL_SHIFT)
            {
                shift = !shift;
                k = 0; // dont return shift
            }
            else
            {
                shift = 0;
                alpha = 0;
            }

            if (k == KEY_CTRL_VARS)
            {
                /* VARS currently sets alpha lock as it is the
                 * prefix key for entering constants.
                 * otherwise remove this bit.
                 */
                alphaLock = 1;
            }

            if (k == KEY_CTRL_AC)
            {
                // drop alphalock on EXE and clear.
                alphaLock = 0;
            }

            // XXX hacky.
            // want to ignore the effect of alphalock on CR so
            // that we can enter symbols in alpha lock, then press EXE
            if (alphaLock && k == KEY_CHAR_CR)
            {
                alphaLock = 0;
                k = KEY_CTRL_EXE;
            }

            lastRow = row;
            lastCol = col;
        }
    }
    else
    {
        // no key
        lastCol = 0;
        lastRow = 0;
    }

    if (!k && block) goto again;
    return k;
}

int EscapeKeyPressed()
{
    unsigned int k = GetKeyNonblocking(0, 0);
    if (k == KEY_CTRL_EXIT || k == KEY_CTRL_DEL) return 1;
    return 0;
}
