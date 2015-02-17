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

#ifndef __plot3d_h__
#define __plot3d_h__

#include "types.h"
#include "2d.h"
#include "eeval.h"

#include "model.h"


// external interface
extern void PlotGraph3D(Term* t,
                        const BCD& xmin, const BCD& xmax,
                        const BCD& ymin, const BCD& ymax,
                        const BCD& npts);
                        

struct Plot3D: public ExprEvaluator
{
    Plot3D()
    {
        _init();
    }

    ~Plot3D() { purge(); }

    // Accessors
    Model*      model() { return _model; }

    // Modifiers
    void        modelH(int h) { _modelH = h; }

    bool setRange(const BCD& x1, const BCD& x2,
                  const BCD& y1, const BCD& y2,
                  int nx, int ny)
    {
        if (x2 >= x1)
        {
            _xmin = x1; _xmax = x2;
        }
        else
        {
            _xmin = x2; _xmax = x1;
        }

        if (y2 >= y1)
        {
            _ymin = y1; _ymax = y2;
        }
        else
        {
            _ymin = y2; _ymax = y1;
        }

        _nx = nx;
        _ny = ny;
        
        bool res = _nx && _ny && _xmax != _xmin && _ymax != _ymin;
        if (res)
        {
            _dx = (_xmax - _xmin)/_nx;
            _dy = (_ymax - _ymin)/_ny;
        }
        return res;
    }

    void purge() 
    {
        if (_model)
        {
            destroy_model(_model);
            _model = 0;
        }
        _zminmaxValid = false;
    }

    bool createModel();

    BCDh scalex(const BCD& x) { return (x - _xmin)*_xScale; }
    BCDh scaley(const BCD& y) { return (y - _ymin)*_yScale; }
    BCDh scalez(const BCD& z) { return (z - _zmin)*_zScale; }

    void move(int dx, int dy, int dz);
    void spin(int spin);
    void orient(int x, int y, int z);
    void resetView();
    void rescale(int x, int y, int z);
    bool refineModel();
    bool applyRefinement();

private:

    void _init()
    {
        _model = 0;
        _modelH = -1;
        _dc = 0;
        _zminmaxValid = false;
        _refined = false;
    }

    Model*              _model;
    int                 _modelH;
    DC2D*               _dc;

    BCDh                _xmin;
    BCDh                _xmax;
    int                 _nx;
    BCDh                _dx;
    BCDh                _xcent;
    BCDh                _xScale;
    BCDh                _xSize;

    BCDh                _ymin;
    BCDh                _ymax;
    int                 _ny;
    BCDh                _dy;
    BCDh                _ycent;
    BCDh                _yScale;
    BCDh                _ySize;

    bool                _zminmaxValid;
    BCDh                _zmin;
    BCDh                _zmax;
    BCDh                _zcent;
    BCDh                _zScale;
    BCDh                _zSize;
    bool                _refined;
};

#endif // __plot3d_h__
