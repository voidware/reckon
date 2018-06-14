/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : [ProjectName].c                                 */
/*                                                               */
/*   Copyright (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/
#include "fxlib.h"
#include "fio.h"
#include "../casio3d.h"

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
    int select = 0;

    const char* shapes[] =
    {
        "TORUS",
        "DUMBELL"
    };

    Bdisp_AllClr_DDVRAM();

    // init casio specific stuff
    CasioInit();

    print("Press 1 or 2\n");
    while (!select)
    {
        GetKey(&key);
        switch (key)
        {
        case KEY_CHAR_1:
            select = 1;
            break;
        case KEY_CHAR_2:
            select = 2;
            break;
        }
    }
    
    Bdisp_AllClr_DDVRAM();

    if (Casio3DStart(shapes[select-1]))
    {
        for (;;)
        {
            Casio3DUpdate();
            if (IsKeyDown(KEY_CTRL_EXIT)) break;
        }
    }

    while (1) GetKey(&key);
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

