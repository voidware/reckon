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

#include        "int64.h"

unsigned int muldvd(unsigned int a, 
                    unsigned int b,
                    unsigned int c,
                    unsigned int* rp)
{ /* multiply by parts */
    unsigned int middle,middle2;
    unsigned int q,r;
    unsigned short am,al,bm,bl;
    am=(unsigned short)(a>>16);
    al=(unsigned short)a;
    bm=(unsigned short)(b>>16);
    bl=(unsigned short)b;
    /* form partial products */
    r= (unsigned int)al*bl;  
    q= (unsigned int)am*bm;
    middle=(unsigned int)al*bm;
    middle2=(unsigned int)bl*am;
    middle+=middle2;                        /* combine them - carefully */
    if (middle<middle2) q+=((unsigned int)1<<16);
    r+=(middle << 16);
    if ((r>>16)<(unsigned short)middle) q++;
    q+=(middle>>16);
    r+=c;
    if (r<c) q++;
    *rp=r;
    return q;
}   

unsigned int muldvm(unsigned int a,
                   unsigned int c,
                   unsigned int m,
                   unsigned int* rp)
{ /* modified Blakely-Sloan */
    int i,carry;
    unsigned int q=0,r;
    r=a;
    for (i=8;i>0;i--)
    { /* do it bit by bit */
        carry=0;
        if ((int)r<0) carry=1;
        r<<=1;
        if ((int)c<0) r++;
        c<<=1;
        q<<=1;
        if (carry || r>=m) { r-=m; q++; }
        carry=0;
        if ((int)r<0) carry=1;
        r<<=1;
        if ((int)c<0) r++;
        c<<=1;
        q<<=1;
        if (carry || r>=m) { r-=m; q++; }
        carry=0;
        if ((int)r<0) carry=1;
        r<<=1;
        if ((int)c<0) r++;
        c<<=1;
        q<<=1;
        if (carry || r>=m) { r-=m; q++; }
        carry=0;
        if ((int)r<0) carry=1;
        r<<=1;
        if ((int)c<0) r++;
        c<<=1;
        q<<=1;
        if (carry || r>=m) { r-=m; q++; }
    }
    *rp=r;
    return q;
}
