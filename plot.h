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

#ifndef __plot_h__
#define __plot_h__

#include "types.h"
#include "2d.h"
#include "eeval.h"

// external interface
extern void PlotGraph(Term* t, BCD& xmin, BCD& xmax);

// convensions
// _gx < 0 => undefined.
// _gy < 0 => not yet eval
struct PlotPoint
{
    BCD         _x;
    BCD         _fx;
    Ord         _gx;
    Ord         _gy;
};

struct Plot: public ExprEvaluator
{
    Plot()
    {
        _points = 0;
        resetPoi();
    }

    ~Plot() { _purge(); }

    bool calibrate(DC2D* dc, BCD& x1, BCD& x2, int w, int h);
    int  update1();
    void eval1(int pt);
    void scalePoints();
    void plotPoints();
    void plotAxes();
    void recalibrateShift(int);
    bool poi(Bitmap2D*);

    void resetPoi()
    {
        _poiZero = -1;
        _poiLastUp = 0;
        _poiLastDown = 0;
        _poiLast = 0;
    }

    int _scaleY(const BCD& y) const
    {
        BCD v = (y - _scaleymin)*_scaley;
        int yy;
        if (v <= 0) yy = 0;
        else if (v >= _height) yy = _height-1;
        else yy = ifloor(v);
        return yy;
    }

    int _scaleX(const BCD& x) const
    {
        BCD v = (x - _x1)*_scalex;
        int xx;
        if (v <= 0) xx = 0;
        else if (v >= _width) xx = _width-1;
        else xx = itrunc(v + BCD(1)/2);
        return xx;
    }

    void _purge()
    {
        delete [] _points; _points = 0; 
    }

    void _updateScale(int pt);
    bool _poiMinMax(int p1, int p2);


    BCD                 _x1;            // user start x
    BCD                 _x2;            // user end x

    // user canvas not including margin
    int                 _width;
    int                 _height;

    int                 _margin;        // extra points either side
    int                 _nPoints;       // sampling including margins.

    BCD                 _deltax;        // width of each sample.
    BCD                 _x0;            // margin left x
    BCD                 _x3;            // margin right x

    bool                _minBegun;      // track ymax and ymin
    BCD                 _ymin;
    BCD                 _ymax;

    BCD                 _scalex;
    BCD                 _scaley;
    BCD                 _scaleymin;
    
    // axes
    int                 _zx;            // pos of y axis
    int                 _zxmin;         // smallest integer on xaxis
    int                 _zxmax;         // largest integer on xaxis

    int                 _zy;
    int                 _zymin;
    int                 _zymax;

    DC2D*               _dc;
    PlotPoint*          _points;
    
    int                 _poiZero;       // left of a zero crossing or -1
    BCD                 _poiRoot;
    int                 _poiLastUp;
    int                 _poiLastDown;
    int                 _poiLast;  // 1=>up, -1=>down, 0=>none

};

#endif // __plot_h__
