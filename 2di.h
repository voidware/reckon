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

/*
 * 2D internal definitions and macros.
 */

#ifndef __2di_h__
#define __2di_h__

/* clipping macros */
#define CLIPMIN(_x, _v) if (_x < _v) _x = _v;
#define CLIPMAX(_x, _v) if (_x > _v) _x = _v;

#define HWCLIP(_x, _y, _w, _h)                          \
{                                                       \
    CLIPMIN(_x, 0); CLIPMAX(_x, _w);                    \
    CLIPMIN(_y, 0); CLIPMAX(_y, _h);                    \
}

#define BOXCLIP(_x, _y, _x1, _y1, _x2, _y2)             \
{                                                       \
    CLIPMIN(_x, _x1); CLIPMAX(_x, _x2);                 \
    CLIPMIN(_y, _y1); CLIPMAX(_y, _y2);                 \
}

#define RECTBOXCLIP(_r1, _rclip)                        \
{                                                       \
    BOXCLIP((_r1).tl.x, (_r1).tl.y, (_rclip).tl.x, (_rclip).tl.y, (_rclip).br.x, (_rclip).br.y);        \
    BOXCLIP((_r1).br.x, (_r1).br.y, (_rclip).tl.x, (_rclip).tl.y, (_rclip).br.x, (_rclip).br.y);        \
}

#define DESTCLIP(_dx1, _dy1, _cx1, _cy1, _cx2, _cy2, _sx1, _sy1, _sw, _sh)    \
{                                                               \
    int __cx1, __cy1, __cx2, __cy2;                             \
    int __dx2, __dy2;                                           \
    int __dx, __dy;                                             \
    __dx2 = _dx1 + _sw;                                         \
    __dy2 = _dy1 + _sh;                                         \
    __cx1 = _dx1;                                               \
    __cy1 = _dy1;                                               \
    __cx2 = __dx2;                                              \
    __cy2 = __dy2;                                              \
    BOXCLIP(__cx1, __cy1, _cx1, _cy1, _cx2, _cy2);              \
    BOXCLIP(__cx2, __cy2, _cx1, _cy1, _cx2, _cy2);              \
    __dx = __cx1 - _dx1;                                        \
    __dy = __cy1 - _dy1;                                        \
    _sx1 += __dx; _sw -= __dx;                                  \
    _sy1 += __dy; _sh -= __dy;                                  \
    __dx = __dx2 - __cx2;                                       \
    __dy = __dy2 - __cy2;                                       \
    _sw -= __dx;                                                \
    _sh -= __dy;                                                \
    _dx1 = __cx1;                                               \
    _dy1 = __cy1;                                               \
}

#define EXTBOXY(_box, _y)                                       \
{                                                               \
    if ((_y) < (_box).tl.y) (_box).tl.y = _y;                   \
    else if ((_y) > (_box).br.y) (_box).br.y = _y;              \
}

#define EXTBOXX(_box, _x)                                       \
{                                                               \
    if ((_x) < (_box).tl.x) (_box).tl.x = _x;                   \
    else if ((_x) > (_box).br.x) (_box).br.x = _x;              \
}

#define EXTBOX(_box, _x, _y)                                    \
{                                                               \
    EXTBOXX(_box, _x)                                           \
    EXTBOXY(_box, _y)                                           \
}

#define BOXUNION(_ba, _bb, _br)                                         \
{                                                                       \
    (_br).tl.x = ((_ba).tl.x <= (_bb).tl.x) ? (_ba).tl.x : (_bb).tl.x;  \
    (_br).tl.y = ((_ba).tl.y <= (_bb).tl.y) ? (_ba).tl.y : (_bb).tl.y;  \
    (_br).br.x = ((_ba).br.x >= (_bb).br.x) ? (_ba).br.x : (_bb).br.x;  \
    (_br).br.y = ((_ba).br.y >= (_bb).br.y) ? (_ba).br.y : (_bb).br.y;  \
}

#define BOXINTERSECT(_ba, _bb, _br)     \
{                                       \
    (_br) = (_ba);                      \
    RECTBOXCLIP(_br, _bb);              \
}

#define BOXAREA(_b) ((_b).br.x - (_b).tl.x)*((_b).br.y - (_b).tl.x)

#define BOXOK(_box)     \
    ((_box).tl.x < (_box).br.x && (_box).tl.y < (_box).br.y)

#define POINTEQUAL(_pa, _pb) ((_pa).x == (_pb).x && (_pa).y == (_pb).y)

#define BOXEQUAL(_ba, _bb)  \
     (POINTEQUAL((_ba).tl,(_bb).tl) && POINTEQUAL((_ba).br,(_bb).br))

#define BOXTRANSLATE(_b, _dx, _dy)      \
{                                       \
    (_b).tl.x += (_dx);                 \
    (_b).tl.y += (_dy);                 \
    (_b).br.x += (_dx);                 \
    (_b).br.y += (_dy);                 \
}


#endif // __2di_h__
