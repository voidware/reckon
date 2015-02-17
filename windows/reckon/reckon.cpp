/**
 *
 * Portions Copyright (c) 2013 Voidware Ltd.  All Rights Reserved.
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
 */


/* this is the windows version */

#include <stdio.h>
#include "types.h"
#include "dpd.h"

extern "C" 
{
    void eval(const char* exp, const char** result);
    void eval_init();
    void eval_end();
}

// external interface
void PlotGraph(Term* t, BCD& xmin, BCD& xmax)
{
    // do nothing.
}

char* getline()
{
    static char line[128];
    int c;
    char* p = line;
    while ((c = getchar()) != EOF)
    {
        if (c == '\n') break;
        *p++ = c;
    }
    *p = 0;
    return line;
}

int main(int agrc, char** argv)
{
    printf("This is Reckon " VERSION "\n");
    printf("Copyright (c) Voidware Ltd. 2010-2015\n");
    printf("\nThis is the console version.\nhttp://www.voidware.com/reckon\n");
    printf("(empty line to exit)\n\n\n");

    eval_init();

    for (;;)
    {
        printf("> "); fflush(stdout);
        const char* p = getline();
        if (!*p) break;
        const char* res;
        eval(p, &res);
        printf("%s\n", res);
    }

    eval_end();

    return 0;
}


