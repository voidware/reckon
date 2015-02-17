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

// ===========================================================================
// Dependencies
// ===========================================================================

#include "2d.h"
#include "2di.h"

// ===========================================================================
// Implementation
// ===========================================================================


Bitmap2D* CreateBitmap2D(Ord w, Ord h, Ord depth)
{
    unsigned int stride;
    unsigned long amt;
    Bitmap2D* bm;

    stride = (w*depth + 7) >> 3; /* round to bytes */
    amt = sizeof(Bitmap2D) - sizeof(Pix) + h*stride;
    bm = (Bitmap2D*)malloc(amt);
    if (bm)
    {
        bm->w_ = w;
        bm->h_ = h;
        bm->d_ = depth;
        bm->stride_ = stride;
    }
    return bm;
}

/* ASSUME the depths are 1, 2, 4, 8 or 16 */

#if 0
// here are some specific 1-bit versions

// NB: ignore c !!
#define SETPIXEL(_bm, _x, _y, _c)                               \
{                                                               \
    Pix* p = (_bm)->pix_ + (_y)*(_bm)->stride_;                 \
    p += (_x)>>3;                                               \
    *p |= 1<<((_x)&7);                                          \
}

#define GETPIXEL(_bm, _x, _y, _c)                               \
{                                                               \
    Pix* p = (_bm)->pix_ + (_y)*(_bm)->stride_;                 \
    p += (_x)>>3;                                               \
    _c = (*p >> ((_x)&7)) & 1;                                  \
}

#else

// these are the generic version
#define SETPIXEL(_bm, _x, _y, _c)                               \
{                                                               \
    Pix* p = (_bm)->pix_ + (_y)*(_bm)->stride_;                 \
    unsigned int __d = (_bm)->d_;                               \
    if (__d == 16) { ((unsigned short*)p)[_x] = (short)_c; }    \
    else {                                                      \
    unsigned int __t = (_x)*__d;                                \
    unsigned int __m;                                           \
    p += __t>>3;                                                \
    __t &= 7;                                                   \
    __m = (~(-1 << __d)) << __t;                                \
    *p = (Pix)((_c<<__t) & __m) | (*p & ~__m);                  \
    }                                                           \
}

#define GETPIXEL(_bm, _x, _y, _c)                               \
{                                                               \
    Pix* p = (_bm)->pix_ + (_y)*(_bm)->stride_;                 \
    unsigned int __d = (_bm)->d_;                               \
    if (__d == 16) { _c = ((unsigned short*)p)[_x]; }           \
    else {                                                      \
    unsigned int __t = (_x)*__d;                                \
    unsigned int __m;                                           \
    p += __t>>3;                                                \
    __m = ~(-1 << __d);                                         \
    _c = (*p >> (__t&7)) & __m;                                 \
    }                                                           \
}
#endif

static void _setPixel(Bitmap2D* bm, unsigned int x, unsigned int y, Col2D c)
{
    /* perform clip testing */
    if (x < bm->w_ && y < bm->h_) 
        SETPIXEL(bm, x, y, c);
}

void SetPixel2D(DC2D* dc, Ord x, Ord y, Col2D c)
{
    Bitmap2D* bm = dc->dobj_;
    if (bm) 
        _setPixel(bm, x, y, c);
}

Col2D GetPixel2D(DC2D* dc, Ord x, Ord y)
{
    Bitmap2D* bm = dc->dobj_;
    Col2D c = 0;
    if (bm) 
    {
        if (x < bm->w_ && y < bm->h_) 
            GETPIXEL(bm, x, y, c);
    }
    return c;
}

#define SWAP(_a, _b)                                    \
{                                                       \
    int t = _a;                                         \
    _a = _b;                                            \
    _b = t;                                             \
}

#define ORDER(_a, _b) if (_b < _a) SWAP(_a, _b)

void DrawHLine2D(DC2D* dc, int x1, int y1, int x2, Col2D c)
{
    /* [x1,x2] inclusive */
    Bitmap2D* bm = dc->dobj_;
    if (!bm) return;

    /* clip values */
    if (y1 < 0 || y1 >= bm->h_) return; 
    ORDER(x1, x2);
    if (x2 < 0 || x1 >= bm->w_) return;
    CLIPMIN(x1, 0);
    CLIPMAX(x2, bm->w_-1);

#ifdef REGIONS
    {
        /* update change region */
        Rect2D cr;
        cr.tl.x = x1;
        cr.tl.y = y1;
        cr.br.x = x2+1;
        cr.br.y = y1+1;
        Region2DAdd(&dc->change_, &cr);
    }
#endif

    for (;;) 
    {
        SETPIXEL(bm, x1, y1, c);
        if (x1 == x2) break;
        ++x1;
    }
}

void DrawVLine2D(DC2D* dc, int x1, int y1, int y2, Col2D c)
{
    Bitmap2D* bm = dc->dobj_;
    if (!bm) return;

    /* clip values */
    if (x1 < 0 || x1 >= bm->w_) return;
    ORDER(y1, y2);
    if (y2 < 0 || y1 >= bm->h_) return;
    CLIPMIN(y1, 0);
    CLIPMAX(y2, bm->h_-1);

#ifdef REGIONS
    {
        Rect2D cr;
        /* update change region */
        cr.tl.x = x1;
        cr.tl.y = y1;
        cr.br.x = x1+1;
        cr.br.y = y2+1;
        Region2DAdd(&dc->change_, &cr);
    }
#endif
    
    for (;;) 
    {
        SETPIXEL(bm, x1, y1, c);
        if (y1 == y2) break;
        ++y1;
    }
}

void DrawLineUni2D(DC2D* dc, int x1, int y1, int x2, int y2, Col2D c)
{
    /* (x1,y1) to (x2,y2) inclusive */
    int dx, dy, e, i;
    int x, y;
    int inc;

    Bitmap2D* bm = dc->dobj_;
    if (!bm) return;

    if (x1 > x2) 
    {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    dx = x2 - x1;
    dy = y2 - y1;

    inc = 1;
    if (dy < 0) 
    {
        dy = -dy;
        inc = -1;
    }

    if (dx == 0) 
    {
        DrawVLine2D(dc, x1, y1, y2, c);
        return;
    }
    
    if (dy == 0) 
    {
        DrawHLine2D(dc, x1, y1, x2, c);        
        return;
    }

    if (dx >= dy) 
    {
        e = 2*dy - dx;
        x = x1;
        y = y1;
        for (i = 0; i <= dx; ++i) 
        {
            _setPixel(bm, x, y, c);
            if (e > 0) {
                y += inc;
                e += 2*(dy - dx);
            }
            else {
                e += 2*dy;
            }
            ++x;
        }
    }
    else {
        e = 2*dx - dy;
        x = x1;
        y = y1;
        for (i = 0; i <= dy; ++i) 
        {
            _setPixel(bm, x, y, c);
            if (e > 0) {
                ++x;
                e += 2*(dx - dy);
            }
            else {
                e += 2*dx;
            }
            y += inc;
        }
    }
}

void DrawLine2D(DC2D* dc, int x1, int y1, int x2, int y2, Col2D c)
{
    /* (x1,y1) to (x2,y2) inclusive */
    int dx, dy, e, i;
    int x, y;
    int inc;

    Bitmap2D* bm = dc->dobj_;
    if (!bm) return;

    if (x1 == x2)
    {
        DrawVLine2D(dc, x1, y1, y2, c);
        return;
    }

    if (y1 == y2)
    {
        DrawHLine2D(dc, x1, y1, x2, c);
        return;
    }

    dx = x2 - x1;
    dy = y2 - y1;

    inc = 1;
    if (dy < 0) 
    {
        dy = -dy;
        inc = -1;
    }

    x = x1; y = y1;
    if (x1 > x2) 
    {
        dx = -dx;
        if (dx >= dy) 
        {
            e = 2*dy - dx;
            for (i = 0; i <= dx; ++i) 
            {
                _setPixel(bm, x, y, c);
                if (e > 0)
                {
                    y += inc;
                    e += 2*(dy - dx);
                }
                else
                    e += 2*dy;
                --x;
            }
        }
        else 
        {
            e = 2*dx - dy;
            for (i = 0; i <= dy; ++i) 
            {
                _setPixel(bm, x, y, c);
                if (e >= 0)
                {
                    --x;
                    e += 2*(dx - dy);
                }
                else 
                    e += 2*dx;
                y += inc;
            }
        }
    }
    else
    {
        if (dx >= dy) 
        {
            e = 2*dy - dx;
            for (i = 0; i <= dx; ++i) 
            {
                _setPixel(bm, x, y, c);
                if (e > 0)
                {
                    y += inc;
                    e += 2*(dy - dx);
                }
                else
                    e += 2*dy;
                ++x;
            }
        }
        else 
        {
            e = 2*dx - dy;
            for (i = 0; i <= dy; ++i) 
            {
                _setPixel(bm, x, y, c);
                if (e >= 0)
                {
                    ++x;
                    e += 2*(dx - dy);
                }
                else 
                    e += 2*dx;
                y += inc;
            }
        }
    }
}

void DrawBox2D(DC2D* dc, int x1, int y1, int x2, int y2, Col2D c)
{
    x2--;
    y2--;
    if (x1 > x2 || y1 > y2) return;
    DrawHLine2D(dc, x1, y1, x2, c);
    DrawHLine2D(dc, x1, y2, x2, c);
    DrawVLine2D(dc, x1, y1, y2, c);
    DrawVLine2D(dc, x2, y1, y2, c);
}


/* Use when `m' in above is 2^d-1 and pass in `d' instead */
#define SUBMIX2(_v, _d, _cold, _cnew, _max, _c)                 \
{                                                               \
    _c = (_cold) - ((((_max)-(_cnew))*(_v)*(_d) + 0x7fff)>>16); \
    if ((_c) < 0) (_c) = 0;                                     \
}

#define COLMIX(_v, _d, _cold, _cnew, _inv, _c)          \
    _c = ((((_cold)*(_d) + (_v)*((_cnew) - (_cold)))*(_inv)) + 0x7fff)>>16;



#if 0
/* optimised version for 16 bit target */
void BitBlt2D4To16Blend(DC2D* dcDest, int dx1, int dy1, int w, int h,
                        DC2D* dcSrc, int sx1, int sy1)
{
    int dx;
    Pix* sp0;
    Pix* sp;
    Pix* dp0;
    short* dp;

    Bitmap2D* sbm = dcSrc->dobj_;    
    Bitmap2D* dbm = dcDest->dobj_;    
    Rect2D cr;
    Col2D c1, c2;

    DESTCLIP(dx1, dy1, 0, 0, dbm->w_, dbm->h_, sx1, sy1, w, h);

    if (!w || !h) return;

    cr.tl.x = dx1;
    cr.tl.y = dy1;
    cr.br.x = dx1+w;
    cr.br.y = dy1+h;
    Region2DAdd(&dcDest->change_, &cr);

    sp0 = sbm->pix_ + sy1*sbm->stride_;
    dp0 = dbm->pix_ + dy1*dbm->stride_;
    while (h) {
        sp = sp0;
        dp = (short*)dp0;
        sp += sx1>>1;
        dp += dx1;

        dx = 0;

        if (sx1 & 1) {
            c2 = (*sp++ >> 4) & 0xf;
            goto oddstart;
        }

        while (dx < w) {
            int dr, dg, db;
            int sr, sg, sb;

            /* source is treated as an intensity */
            c1 = *sp++;
            c2 = (c1 >> 4) & 0xf;
            c1 &= 0xf;

            if (c1) {
                UNPACK_COLOR16(dr, dg, db, *dp);
                UNPACK_COLOR16(sr, sg, sb, dcDest->fgCol_);
                SUBMIX2(c1, 4369, dr, sr, MAXR, sr);
                SUBMIX2(c1, 4369, dg, sg, MAXG, sg);
                SUBMIX2(c1, 4369, db, sb, MAXB, sb);
                PACK_COLOR16(sr, sg, sb, *dp);
            }

            ++dp;
            ++dx;

        oddstart:

            if (c2 && dx < w) {
                UNPACK_COLOR16(dr, dg, db, *dp);
                UNPACK_COLOR16(sr, sg, sb, dcDest->fgCol_);
                SUBMIX2(c2, 4369, dr, sr, MAXR, sr);
                SUBMIX2(c2, 4369, dg, sg, MAXG, sg);
                SUBMIX2(c2, 4369, db, sb, MAXB, sb);
                PACK_COLOR16(sr, sg, sb, *dp);
            }
            ++dp;
            ++dx;
        }
        ++sy1;
        ++dy1;
        --h;
        sp0 += sbm->stride_;
        dp0 += dbm->stride_;
    }
}
#endif // 0

void BitBlt2D(DC2D* dcDest, int dx1, int dy1, int w, int h,
              DC2D* dcSrc, int sx1, int sy1, Rop2D rop)
{
    /* 1 bit deep version */

    /* ASSUME: everything is byte aligned !!
     */

    Bitmap2D* sbm = dcSrc->dobj_;    
    Bitmap2D* dbm = dcDest->dobj_;  

    unsigned char* sp;
    unsigned char* dp;
    unsigned int n;

    sx1 &= ~7; 
    dx1 &= ~7;
    n = w >> 3;

    sp = sbm->pix_ + sy1*sbm->stride_ + (sx1>>3);
    dp = dbm->pix_ + dy1*dbm->stride_ + (dx1>>3);
    
    for (int i = 0; i < h; ++i)
    {
        memcpy(dp, sp, n);
        sp += sbm->stride_;
        dp += dbm->stride_;
    }
}


#if 0
static const int divTable[] = 
{
    0,
    65536,
    21845,
    9362,
    4369,
    2114,
    1040,
    516,
    257
};

static Col2D mapcol(Col2D c, int d1, int d2)
{
    while (d1 < d2) {
        c = (c << d1) + c;
        d1 <<= 1;
    }
    while (d1 > d2) {
        d1 >>= 1;
        /* c /= (1<<d1)+1; */
        c = (c >> d1);
    }
    return c;
}

void BitBlt2D(DC2D* dcDest, int dx1, int dy1, int w, int h,
              DC2D* dcSrc, int sx1, int sy1, Rop2D rop)
{
    int sx2, sy2;
    int dx, dy;
    int cx1, cy1, cx2, cy2;

    Bitmap2D* sbm = dcSrc->dobj_;    
    Bitmap2D* dbm = dcDest->dobj_;    

    if (!sbm || !dbm) return;

    DESTCLIP(dx1, dy1, 0, 0, dbm->w_, dbm->h_, sx1, sy1, w, h);

    if (!w || !h) return;

    sx2 = sx1 + w;
    sy2 = sy1 + h;

    /* now clip the source */
    cx1 = sx1;
    cy1 = sy1;
    cx2 = sx2;
    cy2 = sy2;
    HWCLIP(cx1, cy1, sbm->w_, sbm->h_);
    HWCLIP(cx2, cy2, sbm->w_, sbm->h_);

    /* now readjust the dest with clips from the source */
    dx = cx1 - sx1;
    dy = cy1 - sy1;
    dx1 += dx; w -= dx;
    dy1 += dy; h -= dy;

    dx = sx2 - cx2;
    dy = sy2 - cy2;
    w -= dx;
    h -= dy;    

    if (!w || !h) return;

#ifdef REGIONS
    {
        Rect2D cr;
        cr.tl.x = dx1;
        cr.tl.y = dy1;
        cr.br.x = dx1+w;
        cr.br.y = dy1+h;
        Region2DAdd(&dcDest->change_, &cr);
    }
#endif 

    for (dy = 0; dy < h; ++dy) 
    {
        for (dx = 0; dx < w; ++dx) 
        {
            Col2D c1, c2;

            switch (rop) 
            {
            case Rop2DBlack:
                c1 = BLACK;
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DInvert:
                {
                    int d = dbm->d_;
                    GETPIXEL(dbm, dx1+dx, dy1+dy, c2);
                    switch (d) {
                    case 16:
                        {
                            int dr, dg, db;
                            int fr, fg, fb;
                            int br, bg, bb;
                            int v;
                            int d;

                            /* turn the destination colour into an intensity,
                             * then use it to mix the fg colour with the
                             * bg colour. BUT use MAX-intensity in order
                             * to give an intersion effect.
                             */

                            UNPACK_COLOR16(fr, fg, fb, dcDest->fgCol_);
                            UNPACK_COLOR16(br, bg, bb, dcDest->bgCol_);
                            UNPACK_COLOR16(dr, dg, db, c2);

                            /* dont be too picky about how to turn components
                             * into intensity. although, in general, each
                             * is weighted, just add them together as it
                             * will be good enough for the effect. and
                             * subtract from max to get an inversion
                             * multiplier.
                             */
                            v = (MAXR+MAXG+MAXB) - (dr + dg + db);

                            d = 65536/(MAXR+MAXG+MAXB); /* CONSTANT */
                            
                            SUBMIX2(v, d, fr, br, MAXR, dr);
                            SUBMIX2(v, d, fg, bg, MAXG, dg);
                            SUBMIX2(v, d, fb, bb, MAXB, db);

                            PACK_COLOR16(dr, dg, db, c1);
                            // PACK_COLOR16(~dr, ~dg, ~db, c1);
                        }
                        break;
                    default:
                        /* otherwise treat the pixel value as an intensity
                         * and subtract from its max value.
                         */
                        c1 = dbm->d_ - c2 - 1;
                    }
                }
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DAnd:
                GETPIXEL(sbm, sx1+dx, sy1+dy, c1);
                GETPIXEL(dbm, dx1+dx, dy1+dy, c2);
                c1 = mapcol(c1, sbm->d_, dbm->d_);
                c1 &= c2;
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DOr:
                GETPIXEL(sbm, sx1+dx, sy1+dy, c1);
                GETPIXEL(dbm, dx1+dx, dy1+dy, c2);
                c1 = mapcol(c1, sbm->d_, dbm->d_);
                c1 |= c2;
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DCopy:
                GETPIXEL(sbm, sx1+dx, sy1+dy, c1);
                c1 = mapcol(c1, sbm->d_, dbm->d_);
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DSet:
                {
                    int d = dbm->d_;
                    /* source is treated as an intensity */
                    GETPIXEL(sbm, sx1+dx, sy1+dy, c1);
                    if (c1) {
                        GETPIXEL(dbm, dx1+dx, dy1+dy, c2);
                        switch (d) {
                        case 16:
                            {
                                int dr, dg, db;
                                int sr, sg, sb;
                                int m;
                                int d;
                                UNPACK_COLOR16(dr, dg, db, c2);
                                UNPACK_COLOR16(sr, sg, sb, dcDest->fgCol_);

                                d = (1<<sbm->d_)-1;
                                m = divTable[sbm->d_];
                                COLMIX(c1, d, dr, sr, m, sr);
                                COLMIX(c1, d, dg, sg, m, sg);
                                COLMIX(c1, d, db, sb, m, sb);
                                PACK_COLOR16(sr, sg, sb, c1);
                            }
                            break;
                        default:
                            /* this treats the colour values as channel
                             * intensities of black against white.
                             */
                            {
                                int m = (1<<dbm->d_)-1;
                                c1 = mapcol(c1, sbm->d_, dbm->d_);
                                COLMIX(c1, m, c2, dcDest->fgCol_, 
                                       divTable[dbm->d_], c1);
                            }
                        }
                        SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                    }
                }
                break;
            case Rop2DWhite:
                c1 = WHITE;
                SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                break;
            case Rop2DBlend:
                {
                    int d = dbm->d_;
                    /* source is treated as an intensity */
                    GETPIXEL(sbm, sx1+dx, sy1+dy, c1); 
                    if (c1) {
                        GETPIXEL(dbm, dx1+dx, dy1+dy, c2);
                        switch (d) {
                        case 16:
                            {
                                int dr, dg, db;
                                int sr, sg, sb;
                                int m;
                                UNPACK_COLOR16(dr, dg, db, c2);
                                UNPACK_COLOR16(sr, sg, sb, dcDest->fgCol_);

                                m = divTable[sbm->d_];
                                SUBMIX2(c1, m, dr, sr, MAXR, sr);
                                SUBMIX2(c1, m, dg, sg, MAXG, sg);
                                SUBMIX2(c1, m, db, sb, MAXB, sb);
                                PACK_COLOR16(sr, sg, sb, c1);
                            }
                            break;
                        default:
                            /* this treats the colour values as channel
                             * intensities of black against white.
                             */
                            {
                                int m = (1<<dbm->d_)-1;
                                c1 = mapcol(c1, sbm->d_, dbm->d_);
                                SUBMIX2(c1, divTable[dbm->d_], c2, dcDest->fgCol_, m, c1);
                            }
                        }
                        SETPIXEL(dbm, dx1+dx, dy1+dy, c1);
                    }
                }
                break;
            }
        }
    }
}
#endif

void FillRect2D(DC2D* dc, int x1, int y1, int w, int h, Col2D col)
{
    int x, y;
    Bitmap2D* bm = dc->dobj_;
    if (!bm) return;

    if (x1 < 0) { w += x1; x1 = 0; }
    else if (x1 >= bm->w_) return;

    if (y1 < 0) { h += y1; y1 = 0; }
    else if (y1 >= bm->h_) return;

    if (w <= 0 || h <= 0) return;
    CLIPMAX(w, bm->w_);
    CLIPMAX(h, bm->h_);

#ifdef REGIONS
    {
        Rect2D cr;
        cr.tl.x = x1;
        cr.tl.y = y1;
        cr.br.x = x1+w;
        cr.br.y = y1+h;
        Region2DAdd(&dc->change_, &cr);
    }
#endif
    
    if (bm->d_ == 16) 
    {
        /* special case 16bpp */
        Pix* p0 = bm->pix_ + y1*bm->stride_ + (x1<<1);
        int j;
        for (j = 0; j < h; ++j) 
        {
            memset(p0, col, w*2);
            p0 += bm->stride_;
        }
    }
    else 
    {
        /* general case */
        for (y = 0; y < h; ++y) 
            for (x = 0; x < w; ++x) 
                SETPIXEL(bm, x1+x, y1+y, col);
    }
}

int GetExtent2D(DC2D* dc, int* w, int* h, int* d)
{
    Bitmap2D* bm = dc->dobj_;
    if (!bm) return 0;

    *w = bm->w_;
    *h = bm->h_;
    if (d) *d = bm->d_;
    return 1;
}

