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

#ifndef __2d_h__
#define __2d_h__

// ===========================================================================
// Dependencies
// ===========================================================================

#define REGIONSxx

#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include "region.h"


/* HERE is how to set the 16bpp layout. Define exactly one of these */

#ifndef DSP_TARGET
#define RGB555
#else
#define BGR565
#endif

// ===========================================================================
// Interface
// ===========================================================================

typedef unsigned long Col2D;
typedef unsigned char Pix;

struct _FaceInfo;

#define ORD_MAXX  32767
#define ORD_MAXY  32767

typedef enum
{
    Rop2DBlack,
    Rop2DInvert,
    Rop2DAnd,
    Rop2DOr,
    Rop2DCopy,
    Rop2DSet,
    Rop2DWhite,
    Rop2DBlend,

} Rop2D;

typedef struct
{
    Ord         w_;
    Ord         h_;
    Ord         d_;             /* depth */
    Ord         stride_;
    Pix         pix_[1];         /* pix data, rest follow, NB: aligned */
} Bitmap2D;

typedef struct
{
    Bitmap2D*           dobj_;  /* draw object */
    int                 textFlags_;
    Col2D               fgCol_;
    Col2D               bgCol_;
#ifdef REGIONS
    Region2D            change_;  /* track region of change */
#endif

    struct _FaceInfo*   font_;

} DC2D;

#ifdef BGR565

#define MAXR  31
#define MAXG  63
#define MAXB  31

#define WHITE 0xffff
#define BLACK 0
#define BLUE16 (31<<11)
#define GREEN16 (63<<5)
#define RED16 31
#define YELLOW16 (RED16|GREEN16)
#define MAGENTA16 (RED16|BLUE16)
#define CIAN16 (GREEN16|BLUE16)

/* pack into 16 bit 5:6:5 */
#define PACK_COLOR16(_r, _g, _b, _c)        \
_c = (Col2D)(((_b) << 11) | (((_g)&0x3f) << 5) | ((_r)&0x1f))

/* pack from 24bit rgb into 5:6:5 */
#define PACK_COLOR3216(_r, _g, _b)        \
((Col2D)((((_b)&~7) << 8) | (((_g)&~3)<< 3) | ((_b)>>3)))

#define UNPACK_COLOR16(_r, _g, _b, _c)          \
{                                               \
    _r = (_c) & 31;                             \
    _g = ((_c) >> 5) & 63;                      \
    _b = (_c) >> 11;                            \
}
#endif

#ifdef RGB555

#define MAXR  31
#define MAXG  31
#define MAXB  31

#define WHITE 0x7fff
#define BLACK 0
#define RED16 (31<<10)
#define GREEN16 (31<<5)
#define BLUE16 31
#define YELLOW16 (RED16|GREEN16)
#define MAGENTA16 (RED16|BLUE16)
#define CIAN16 (GREEN16|BLUE16)

/* pack into 16 bit 5:5:5 */
#define PACK_COLOR16(_r, _g, _b, _c)        \
_c = (Col2D)(((_r) << 10) | (((_g)&0x1f) << 5) | ((_b)&0x1f))

/* pack from 24bit rgb into 5:5:5 */
#define PACK_COLOR3216(_r, _g, _b)        \
((Col2D)((((_r)&~7) << 7) | (((_g)&~7)<< 2) | ((_b)>>3)))

#define UNPACK_COLOR16(_r, _g, _b, _c)          \
{                                               \
    _b = (_c) & 31;                             \
    _g = ((_c) >> 5) & 31;                      \
    _r = (_c) >> 10;                            \
}
#endif


/* API */
inline void DrawEnd2D(DC2D*) {}

Bitmap2D* CreateBitmap2D(Ord w, Ord h, Ord depth);
Col2D GetPixel2D(DC2D*, Ord x, Ord y);
void SetPixel2D(DC2D*, Ord x, Ord y, Col2D c);
void DrawHLine2D(DC2D*, int x1, int y1, int x2, Col2D c);
void DrawVLine2D(DC2D*, int x1, int y1, int y2, Col2D c);
void DrawLine2D(DC2D*, int x1, int y1, int x2, int y2, Col2D c);
void DrawLineUni2D(DC2D*, int x1, int y1, int x2, int y2, Col2D c);
void DrawBox2D(DC2D*, int x1, int y1, int x2, int y2, Col2D c);
void BitBlt2D(DC2D* dcDest, int dx1, int dy1, int dx2, int dy2,
              DC2D* dcSrc, int sx1, int sx2, Rop2D);
void FillRect2D(DC2D*, int x1, int y1, int w, int h, Col2D);
int GetExtent2D(DC2D* dc, int* w, int* h, int* d);

//Handle SelectObject2D(DC2D*, Handle);
inline void DeleteObject2D(void* h)
{
    if (h) free(h);
}

inline void DeleteBitmap2D(Bitmap2D* bm)
{
    DeleteObject2D(bm);    
}


inline void SetFgCol(DC2D* dc, Col2D col)
{
    dc->fgCol_ = col;
}

inline Col2D GetFgCol(DC2D* dc)
{
    return dc->fgCol_;
}

inline void SetBgCol(DC2D* dc, Col2D col)
{
    dc->bgCol_ = col;
}

inline Col2D GetBgCol(DC2D* dc)
{
    return dc->bgCol_;
}

/* special cases */
void BitBlt2D4To16Blend(DC2D* dcDest, int dx1, int dy1, int w, int h,
                        DC2D* dcSrc, int sx1, int sy1);

inline void SelectStaticBitmap2D(DC2D* dc, Bitmap2D* bm)
{
    dc->dobj_ = bm;
}

inline int DrawBegin2D(DC2D* dc)
{
#ifdef REGIONS
    /* clear change region */
    Region2DEmpty(&dc->change_);
#endif
    return dc->dobj_ != 0;
}

inline void CreateDC2D(DC2D* dc)
{
    memset(dc, 0, sizeof(DC2D));
    dc->fgCol_ = BLACK;
    dc->bgCol_ = WHITE;

#ifdef REGIONS
    /* initialise change region */
    Region2DInit(&dc->change_);
#endif
}

 
#endif // 2d

