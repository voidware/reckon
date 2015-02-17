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

#include "plot.h"
#include "solve.h"

extern "C"
{
#include "utils.h"
#include "common.h"
#include "fxlib.h"
}

// is 5 but overlaps 1 space pixel per char
#define MINIW  4
#define MINIH  7

void Plot::recalibrateShift(int n)
{
    _minBegun = false; // start tracking ymin/ymax
            
    BCD shift = _deltax * n;

    PlotPoint* pp;
    int i;
    BCD bb;

    // recycle points
    if (n > 0)
    {
        if (n < _nPoints)
        {
            // fix gx and refresh scale
            for (i = n; i < _nPoints; ++i)
            {
                pp = _points + i;
                if (pp->_gx >= 0) pp->_gx -= n;
                _points[i-n] = *pp;
                _updateScale(i-n);
            }

            // zap the new points
            bb = _x3;
            for (i = _nPoints - n; i < _nPoints; ++i)
            {
                bb += _deltax;
                pp = _points + i;
                pp->_gy = -1;
                pp->_x = bb;
            }
        }
    }
    else
    {
        n = -n;
        if (n < _nPoints)
        {
            int m = _nPoints - n;
            for (i = m-1; i >= 0; --i)
            {
                pp = _points + i;
                if (pp->_gx >= 0) pp->_gx += n;
                _points[i+n] = *pp;
                _updateScale(i+n);
            }

            // zap the new points
            bb = _x0;
            for (i = n-1; i >= 0; --i)
            {
                bb -= _deltax;
                pp = _points + i;
                pp->_gy = -1;
                pp->_x = bb;
            }
        }
    }

    _x0 += shift;
    _x1 += shift;
    _x2 += shift;
    _x3 += shift;

}

bool Plot::calibrate(DC2D* dc, BCD& x1, BCD& x2, int w, int h)
{
    _purge();
    _minBegun = false; // start tracking ymin/ymax

    // this is where we will draw
    _dc = dc;
            
    if (x1 <= x2)
    {
        _x1 = x1;
        _x2 = x2;
    }
    else
    {
        _x2 = x1;
        _x1 = x2;
    }

    // size of user cavas not including margins
    _width = w;
    _height = h;

    // roughly size margins at 10%
    // also make it multiple of 8
    _margin = (_width/10 + 7) & ~7;

    // calculate x width of each sample
    _deltax = (_x2 - _x1)/(_width - 1);
    _scalex = 1/_deltax;

    // sample twice margin + original width
    _nPoints = (_margin<<1) + _width;

    // allocate points
    _points = new PlotPoint[_nPoints];
    if (!_points) return false;  // bail

    PlotPoint* pp;
    int i;

    // set the start and end points to the user input
    pp = _points + _margin;
    pp->_gy = -1; // not yet eval.
    pp->_x = _x1;

    pp = _points + _nPoints - _margin - 1;
    pp->_gy = -1; // not yet eval.
    pp->_x = _x2;

    // walk right and left setting x for the margins
    _x0 = _x1;
    _x3 = _x2;
    for (i = 0; i < _margin; ++i)
    {
        _x3 += _deltax;
        _x0 -= _deltax;

        // go right
        pp = _points + _margin + _width + i;
        pp->_gy = -1; // not yet eval.
        pp->_x = _x3;

        // go left
        pp = _points + _margin - i - 1;
        pp->_gy = -1; // not yet eval.
        pp->_x = _x0;
    }

    // x0 and x3 are now total minx and maxx including margin

    // now set the other x points between x1 and x2
    BCD xl = _x1;
    for (i = 1; i < _width-1; ++i)
    {
        xl += _deltax;
        PlotPoint* pp = _points + _margin + i;
        pp->_gy = -1; // not yet eval.
        pp->_x = xl;
    }

    // now we must evaluate, at least the user x1 and x2 
    eval1(_margin);
    eval1(_margin + _width - 1);
    return true;
}

int Plot::update1()
{
    // find biggest gap, eval it 
    // return gap

    int i = 0;
    int bn = 0; // best length
    int bl; // best left

    for (;;)
    {
        int gl = -1;
        int gr;
        if (_points[i]._gy < 0)
        {
            if (gl < 0) 
            {
                gl = i;
                
                // scan foward to size gap
                gr = i + 1;
                while (gr < _nPoints && _points[gr]._gy < 0) ++gr;
                
                if (gr - gl > bn)
                {
                    // better!
                    bn = gr - gl;
                    bl = gl;
                }

                i = gr;
            }
        }
        if (++i >= _nPoints) break; // done
    }

    if (bn)
    {
        // fill in gap midpoint
        i = bl + (bn>>1);
        eval1(i);
    }
    return bn;
}


void Plot::_updateScale(int pt)
{
    PlotPoint* pp = _points + pt;
    if (pp->_gx >= 0 && pp->_gy >= 0)
    {
        bool inWindow = (pt >= _margin && pt < _nPoints - _margin);
        if (inWindow)
        {
            if (_minBegun)
            {
                if (pp->_fx > _ymax) _ymax = pp->_fx;
                else if (pp->_fx < _ymin) _ymin = pp->_fx;
            }
            else
            {
                _minBegun = true;
                _ymin = pp->_fx;
                _ymax = _ymin;
            }
        }
    }
}

void Plot::eval1(int pt)
{
    PlotPoint* pp = _points + pt;

    // mark as no longer uneval (even if fail)
    pp->_gy = 0;

    // mark undefined (incase fail)
    pp->_gx = -1; 

    BCD y;
    if (eval(pp->_x, y))
    {
        if (!y.isSpecial())  // nan inf etc.
        {
            // accept value
            pp->_fx = y;
            pp->_gx = pt;
            _updateScale(pt);
        }
    }
}

void Plot::scalePoints()
{
    // now calculate the scale

    _scaleymin = _ymin;
    if (_ymin == _ymax)
        _scaley = 0;
    else 
    {
        // widen the yscale just slightly to look neater
        BCD dy = _ymax - _ymin;
        BCD ddy = dy/100;
        _scaley = _height/(dy + ddy + ddy);
        _scaleymin -= ddy;
    }
        
    for (int i = 0; i < _nPoints; ++i)
    {
        PlotPoint* pp = _points + i;

        // if evaluated and not undefined scale it!
        if (pp->_gx >= 0 && pp->_gy >= 0)
            pp->_gy = _scaleY(pp->_fx);
    }

    // also calculate the position of the yaxis
    _zx = -1; // none
    if (_x1 <= 0 && _x2 >= 0) _zx = _scaleX(BCD(0));

    // truncate towards zero
    _zxmin = itrunc(_x1);
    _zxmax = itrunc(_x2);

    // and the xaxis
    _zy = -1; // none
    if (_scaleymin <= 0 && _ymax >= 0) _zy = _scaleY(BCD(0));

    _zymin = itrunc(_scaleymin);
    _zymax = itrunc(_ymax);
}

void Plot::plotPoints()
{
    int i;
    Ord lastx, lasty;
    bool begun = false;

    // NB: use fillrect later.
    memset(_dc->dobj_->pix_, 0, _dc->dobj_->stride_*_height);

    int n = _margin + _width;
    for (i = _margin; i < n; ++i)
    {
        PlotPoint* pp = _points + i;
        if (pp->_gx >= 0 && pp->_gy >= 0)
        {
            Ord x = pp->_gx - _margin;
            Ord y = _height - 1 - pp->_gy;
            if (!begun)
            {
                begun = true;
                SetPixel2D(_dc, x, y, WHITE);
            }
            else
                DrawLine2D(_dc, lastx, lasty, x, y, WHITE);
                    
            lastx = x;
            lasty = y;
        }
    }
}


void Plot::plotAxes()
{
    // now draw axes
    if (_zx >= 0)
    {
        DrawVLine2D(_dc, _zx, 0, _height-1, WHITE);
                
        // draw tick marks
        int d = _zymax - _zymin;
        if (d < _height/3)
        {
            for (int y = _zymin; y <= _zymax; ++y)
            {
                int t = _height-1 - _scaleY(BCD(y));
                SetPixel2D(_dc, _zx + 1, t, WHITE);
                SetPixel2D(_dc, _zx - 1, t, WHITE);
            }                
        }
    }

    if (_zy >= 0)
    {
        int zy = _height-1 - _zy;
        DrawHLine2D(_dc, 0, zy, _nPoints-1, WHITE);

        // draw tick marks
        int d = _zxmax - _zxmin;
        if (d < _nPoints/4)
        {
            for (int x = _zxmin; x <= _zxmax; ++x)
            {
                int t = _scaleX(BCD(x));
                SetPixel2D(_dc, t, zy+1, WHITE);
                SetPixel2D(_dc, t, zy-1, WHITE);
            }
        }
    }
}

static void updateScreen(Bitmap2D* bm)
{
    /* blit from our W2D screen to the calculator screen */
    unsigned char* sp = (unsigned char*)bm->pix_;
    unsigned char* dp = GetVRAMPtr();

    unsigned char rev[256];
    for (int i = 0; i < 256; ++i)
    {
        int r = 0;
        int x = i;
        for (int j = 0; j < 8; ++j)
        {
            r <<= 1;
            r += x & 1;
            x >>= 1;
        }
        rev[i] = r;
    }

    for (int i = 0; i < 1024; ++i)
        *dp++ = rev[*sp++];

    refreshScreen();
}

bool Plot::_poiMinMax(int p1, int p2)
{
    Solver sol;
    sol.eps(BCD::epsilon(6)); // 6 digits only
    sol.setTermAndVar(_expr, _var); // our expression and var
    sol.setAdapter(&Solver::derivAdapter);
    sol.setMinMax(_points[p1]._x, _points[p2]._x);
    bool res = sol.ridderSolveOnly();
    BCD fpm;

    if (res && sol.ExprEvaluator::_eval(sol.root(), fpm)) 
    {
        const char* s = fpm.asStringFmt(BCDFloat::format_normal, 5);
        unsigned int l = strlen(s) * MINIW;

        PlotPoint* pm = _points + (p1 + p2)/2;
        Ord x = pm->_gx - _margin - l/2;
        if (x < 0) x = 0;
        if (x + l > _width)
            x = _width - l;

        // shift y up or down to create space.
        Ord y = pm->_gy;
        if (y + MINIH+3 < _height) y += MINIH+3; // 3 margin
        else y -= 5; // 5 margin

        y = _height - 1 - y;
        PrintMini(x, y, (const unsigned char*)s, MINI_OVER);
    }
    return res;
}

bool Plot::poi(Bitmap2D* scr)
{
    // find points of interest

    // find next zero crossing
    int pos = _poiZero;
    for (;;)
    {
        ++pos;
        if (pos < _margin) pos = _margin;
        
        if (pos >= _nPoints - _margin) 
        {
            pos = _margin;
            _poiLast = 0; // min/max doesnt wrap.
            if (_poiZero < 0) break; // did not have previous
        }

        if (pos == _poiZero) break; // wrapped
        
        PlotPoint* p1 = _points + pos;
        PlotPoint* p2 = p1 + 1;
        if (p1->_gx >= 0 && p1->_gy >= 0 && p2->_gx >= 0 && p2->_gy >= 0)
        {
            if (p1->_fx.isNeg() != p2->_fx.isNeg())
            {
                // found a zero crossing.
                Solver sol;
                sol.eps(BCD::epsilon(8)); // 8 digits only
                sol.setTermAndVar(_expr, _var); // our expression and var
                sol.setMinMax(p1->_x, p2->_x);
                _poiLast = 0;
                if (sol.ridderSolveOnly())
                {
                    _poiRoot = sol.root();
                    const char* s = _poiRoot.asStringFmt(BCDFloat::format_normal, 7);
                    unsigned int l = strlen(s) * MINIW;

                    Ord x = p1->_gx - _margin;
                    Ord y = _zy;
                    int r = 4;
                    int iy = _height-1;
                    DrawBox2D(_dc,
                              x - r, iy - y - r,
                              x + r, iy - y + r, WHITE);

                    x -= l/2;
                    if (x < 0) x = 0;
                    if (x + l > _width)
                        x = _width - l;

                    // shift y up or down to create space.
                    if (y + MINIH+3 < _height) y += MINIH+3; // 2 margin
                    else y -= 5; // 5 margin
                    
                    // paint our changes to real screen
                    updateScreen(scr);

                    // use system for mini text
                    PrintMini(x, iy - y, (const unsigned char*)s, MINI_OVER);
                    _poiZero = pos;
                    return true;
                }
            }

            int pl = _poiLast;
            if (p2->_fx > p1->_fx)
            {
                _poiLastUp = pos;
                _poiLast = 1;
                if (pl < 0 && _poiMinMax(_poiLastDown, pos + 1))
                {
                    _poiZero = pos;
                    return true;
                }
            }
            else if (p2->_fx < p1->_fx)
            {
                _poiLastDown = pos;                
                _poiLast = -1;
                if (pl > 0 &&  _poiMinMax(_poiLastUp, pos + 1))
                {
                    _poiZero = pos;
                    return true;
                }
            }
        }
        else
        {
            // if we go though an undefined point there is no min/max
            _poiLast = 0;
        }
    }
    return false;
}



static unsigned int Navigate(int& offset, int& zoom, bool wait)
{
    unsigned int k;

    offset = 0;
    zoom = 0;

    k = GetKeyNonblocking(wait ? 1 : 0, 1);

    switch (k)
    {
    case KEY_CTRL_LEFT:
        offset = -6;
        break;
    case KEY_CTRL_RIGHT:
        offset = 6;
        break;
    case KEY_CTRL_DOWN:
        zoom = 105;
        break;
    case KEY_CTRL_UP:
        zoom = 95;
        break;
    }
    return k;
}

void PlotGraph(Term* t, BCD& xmin, BCD& xmax)
{
    AC();

    int h = 64;
    int w = 128;
    int d = 1;
    Bitmap2D* scr;
    DC2D dc;
    Plot plot;
    int offset;

    CreateDC2D(&dc);
    scr =  CreateBitmap2D(w, h, d);
    if (!scr) return; // bail

    // use this to render
    SelectStaticBitmap2D(&dc, scr);

    if (plot.setTerm(t))
    {
        bool done = false;
        bool recal = true;
        unsigned long delayMax = 2000; // initial delay
        int zoom = 0;
        bool more;
        unsigned int k;

        while (!done)
        {
            if (recal)
            {
                if (!plot.calibrate(&dc, xmin, xmax, w, h))
                    break; // bail
                recal = false;
                more = true;
            }
        
            if (more)
            {
                // create initial timer
                StartTimer();

                // boost speed during calculation
                CPUSpeedFast();
                do
                {
                    more = (plot.update1() > 0);
                    unsigned int dt = ReadTimer();
                    if (dt > delayMax) break; // times up!
                    
                } while (more);

                // back to normal again!
                CPUSpeedNormal();
                StopTimer();
            }

            // smaller delay for increments
            delayMax = 50;

            // draw what we have so far
            plot.scalePoints();
            plot.plotPoints();
            plot.plotAxes();
            updateScreen(scr);

            bool poimode = false;
            plot.resetPoi();
            do
            {
                k = Navigate(offset, zoom, !more);
                if (poimode)
                {
                    // repaint
                    plot.plotPoints();
                    plot.plotAxes();
                    updateScreen(scr);
                }
                
                poimode = (k == KEY_CTRL_EXE && !more);

                if (poimode && plot.poi(scr))
                {
                    refreshScreen();
                }

            } while (poimode);

            if (k == KEY_CTRL_AC) 
                break;

            if (offset)
            {
                plot.recalibrateShift(offset);
                xmin = plot._x1;
                xmax = plot._x2;
                more = true;
            }
            else if (zoom)
            {
                xmin *= BCD(zoom)/100;
                xmax *= BCD(zoom)/100;
                recal = true;

                // when zooming, give the first time round 
                // a bit extra time.
                delayMax = 100;
            }
            else
            {
                // escape on any other key
                if (!more) break;
            }
        }

        // finish up and go back to normal
        AC();
    }
    DeleteBitmap2D(scr);
}
