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

#ifndef DEFFS_H
#define DEFFS_H

#include "BCDh.h"
#include "misc.h"

// comment this out to remove Goraud support and save code
#define     SUPPORT_GOURAUD
                            
struct Matrix3
{
  long   _11, _12, _13, 
         _21, _22, _23, 
         _31, _32, _33;
};

struct Vector3
{
    int x;
    int y;
    int z;
};

#define DOT(_a, _b) ((_a).x*(_b).x + (_a).y*(_b).y + (_a).z*(_b).z)

#define VSUB(_d, _a, _b)                \
{                                       \
    (_d).x = (_a).x - (_b).x;           \
    (_d).y = (_a).y - (_b).y;           \
    (_d).z = (_a).z - (_b).z;           \
}

// 8 bit normal
struct norm8
{
    char x;
    char y;
    char z;
};

struct Point3
{
    BCDh                vx;
    BCDh                vy;
    BCDh                vz;
    int                 x;
    int                 y;
    int                 z;
    int                 tx;
    int                 ty;
    int                 tz;
    char                clip;
    unsigned char       refs;
#ifdef SUPPORT_GOURAUD
    int                 ti;
    norm8               norm;
#endif
};             

/* model shading types */
#define SHADING_SIMPLE   0
#define SHADING_GOURAUD  1
#define SHADING_WIRE     2
#define SHADING_BITS     2
#define SHADING_MASK     ((1<<SHADING_BITS)-1)

#define GET_COLOUR(_x)   ((_x)->color<<5)
#define PUT_COLOUR(_x,_v)((_x)->color = (_v)>>5)


/* poly types */

#define     TRIANGLE_TYPE        1
#define     POLYGON_TYPE         2

#define     TRIANGLE            ((TRIANGLE_TYPE<<SHADING_BITS)|SHADING_SIMPLE)
#define     POLYGON             ((POLYGON_TYPE<<SHADING_BITS)|SHADING_SIMPLE)
#define     GOURAUD_TRIANGLE    ((TRIANGLE_TYPE<<SHADING_BITS)|SHADING_GOURAUD)
#define     GOURAUD_POLYGON     ((POLYGON_TYPE<<SHADING_BITS)|SHADING_GOURAUD)
#define     WIRE_TRIANGLE       ((TRIANGLE_TYPE<<SHADING_BITS)|SHADING_WIRE)
#define     WIRE_POLYGON        ((POLYGON_TYPE<<SHADING_BITS)|SHADING_WIRE)


#define TRANSPARENT_COLOUR  255


struct Plane
{
    unsigned char       num_verts;
    unsigned char       color;
    unsigned char       area;
    unsigned char       type;
    unsigned char       pixel;          // shading

    signed char         normx;
    signed char         normy;
    signed char         normz;

    Point3*             p1;
    Point3*             p2;
    Point3*             p3;
    Point3*             p4;
};

#define DIVIDE(_dx, _dy)   ((_dy) ? ((_dx) << TRI_SHIFT)/(_dy) : ((_dx) << TRI_SHIFT))

#endif // DEFS_H
