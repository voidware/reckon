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

#include "bcdh.h"
#include "plot3d.h"

#ifdef WIN32
#include "oswin.h"
#else
extern "C"
{
#include "utils.h"
#include "common.h"
#include "fxlib.h"
}
#endif

// 3d includes
#include "l3defs.h"
#include "l3api.h"
#include "world.h"
#include "files.h"
#include "controller.h"
#include "matrix.h"
#include "view.h"
#include "matrix.h"

#define MODEL_SIZE 4000
#define MODEL_SIZE_MIN 500
#define MODEL_SIZE_MAX 8000

static unsigned int Navigate(int& dx, int& dy, int& dz, int& spin,
                             int& ox, int &oy, int& oz,
                             bool wait)
{
    unsigned int k;
    dx = 0;
    dy = 0;
    dz = 0;
    ox = 0;
    oy = 0;
    oz = 0;    

    k = GetKeyNonblocking(wait ? 1 : 0, 1);
    switch (k)
    {
    case KEY_CTRL_LEFT:
        dx -= 40;
        break;
    case KEY_CTRL_RIGHT:
        dx += 40;
        break;
    case KEY_CTRL_DOWN:
        dy -= 40;
        break;
    case KEY_CTRL_UP:
        dy += 40;
        break;
    case '+':
        dz -= 40;
        break;
    case '-':
        dz += 40;
        break;
    case '0':
        spin = 0;
        break;
    case '1':
        spin -= 1;
        break;
    case '2':
        spin = 0;
        break;
    case '3':
        spin += 1;
        break;
    case '7':
        ox += 1;
        break;
    case '8':
        oy += 1;
        break;
    case '9':
        oz += 1;
        break;
    case '4':
        ox -= 1;
        break;
    case '5':
        oy -= 1;
        break;
    case '6':
        oz -= 1;
        break;
    }

    // clamp spin
    if (spin > 20) spin = 20;
    else if (spin < -20) spin = -20;

    return k;
}

static const int screen_w = 128;
static const int screen_h = 64; 

// define screen size function for l3
int screen_height() { return screen_h; }
int screen_width() { return screen_w; }

void PlotGraph3D(Term* t,
                 const BCD& xmin, const BCD& xmax,
                 const BCD& ymin, const BCD& ymax,
                 const BCD& np)
{
    int h = screen_height();
    int w = screen_width();
    int d = 8;
    Bitmap2D* scr;
    DC2D dc;
    Plot3D plot;
    int offx, offy, offz;
    int ox, oy, oz;
    int spin = 0;

    int npts = itrunc(np);
    if (!npts) npts = 10;       // default to 10
    if (npts < 4) npts = 4;
    else if (npts > 20) npts = 20;
    
    AC();

    CreateDC2D(&dc);
    scr =  CreateBitmap2D(w, h, d);
    if (!scr) return; // bail

    // use this to render
    SelectStaticBitmap2D(&dc, scr);

    // init 3d tables
    extern void Init(Bitmap2D*);
    Init(scr);
    
    if (plot.setTerm(t, 2) &&
        plot.setRange(xmin, xmax, ymin, ymax, npts, npts))

    {
        current_view  = create_viewport(0, 0, w, h, MODELS_MAX);
        if (current_view)
        {
            p_model = add_model_to_world(createDummy());
            if (p_model >= 0)
            {
                set_position(p_model, 0, 0,-1000);
                set_rotating(p_model,0,0,0);
                view_model = p_model;
                align_viewport_with_model(current_view, view_model); 
                    
                if (plot.createModel())
                {
#if 0
                    {
                        // debugging
                        // apply max refinement now.
                        int c = 0;
                        while (plot.refineModel()) ++c;
                        int a = 1;
                    }
#endif

                    int m = add_model_to_world(plot.model());
                    if (m >= 0)
                    {
                        plot.modelH(m);
                        set_relative_position(view_model, m, 0, 0, 2*MODEL_SIZE);
                        int more = 1;
                        bool draw = true;
                        for (;;)
                        {
                            // TICK!
                            if (draw)
                            {
                                update_world();
                                update_viewport(current_view);
                                Update3DScreen();
                            }
                        
                            int k;
                            k = Navigate(offx, offy, offz, spin,
                                         ox, oy, oz, !more);
                            if (k == KEY_CTRL_AC) break;
                            if (k)
                            {
                                more = 1;
                                draw = true;
                                plot.move(offx, offy, offz);
                                plot.spin(spin>>2);
                                plot.orient(ox, oy, oz);
                                if (k == '0') plot.resetView();
                                else if (k == '(') plot.rescale(102,102,100);
                                else if (k == ')') plot.rescale(98,98,100);
                                if (k == '.')
                                {
                                    plot.applyRefinement();
                                }
                            }
                            else 
                            {
                                more = (spin>>2);

                                // when no key is pressed, try refining
                                if (plot.refineModel())
                                {
                                    // we refined. 
                                    // keep going but don't draw
                                    if (!more) draw = false;
                                    more = 1;

                                }
                                else
                                    draw = true;

                            }
                        }

                        // purge model right away so we delete
                        // memory in reverse order (hoping to help!)
                        plot.purge();
                    }
                }
                // destroy dummy
                destroy_model(get_model(p_model));
            }
            // finish up and go back to normal
            destroy_viewport(current_view);
        }
    }

    AC();
    DeleteBitmap2D(scr);
}

bool Plot3D::createModel()
{
    bool res = false;
    int n_vertices;
    int n_planes;
    Point3*   vp;
    int i, j;

    n_vertices  = (_nx+1)*(_ny+1); 
    n_planes    = _nx*_ny;
    
    // for now set the capacity to the exact size
    int maxp = 256; // 15^2
    int dv = 0;
    if (n_planes < maxp)
        dv = (maxp - n_planes)/3;

    int vertCap = n_vertices + dv;
    int planeCap = n_planes + (dv*3);

    _model = create_model(n_vertices,
                          1, // subgroups
                          n_planes,
                          vertCap, planeCap);

    if (_model)
    {
        vp = _model->vertices;
        Plane* pp = _model->planes;
        Point3* vbase = vp;

        _model->num_subgroups = 1;        
        _model->subgroups->plane = pp;
        _model->subgroups->num_planes = n_planes;
        _model->subgroups->point = 0;

        // accelerate CPU whilst evaluating mesh
        CPUSpeedFast();
        
        BCDh x = _xmin;
        BCD z;
        int pc = 0;
        for (i = 0; i <= _nx; ++i)
        {
            BCDh y = _ymin;
            for (j = 0; j <= _ny; ++j)
            {
                vp->vx = x;
                vp->vy = y;

                if (_eval(x.asBCD(), y.asBCD(), z))
                {
                    vp->vz = z;
                    
                    if (!_zminmaxValid)
                    {
                        _zmin = vp->vz;
                        _zmax = _zmin;
                        _zminmaxValid = true;
                    }
                    else
                    {
                        if (vp->vz < _zmin) _zmin = vp->vz;
                        else if (vp->vz > _zmax) _zmax = vp->vz;
                    }
                }
                else
                {
                    // use clip flag to mark as invalid during creation
                    vp->clip = 1;
                }

                y += _dy;

                // now create the planes
                if (i && j)
                {
                    pp->num_verts = 4;
                    pp->color = TRANSPARENT_COLOUR;
                    pp->type = WIRE_POLYGON;
                    pp->area = VISIBLE;
                    pp->p1 = vbase + (pc-_ny-2);  // 0,4,5,1
                    pp->p2 = vbase + (pc-_ny-1);
                    pp->p3 = vbase + pc;
                    pp->p4 = vbase + (pc-1);
                    ++pp;
                }

                ++vp;
                ++pc;
            }
            x += _dx;
        }


        // now fix all the undefined points 
        vp = _model->vertices;
        for (i = 0; i <= _nx; ++i)
        {
            for (j = 0; j <= _ny; ++j)
            {
                if (vp->clip)
                {
                    // undefined point. take the average of the neighbours
                    int c = 0;
                    BCDh av = 0;

                    // collect valid neighbours
                    if (j && !vp[-1].clip) { av += vp[-1].vz; ++c; }
                    if (j < _ny-1 && !vp[1].clip) { av += vp[1].vz; ++c; }
                    if (i && !vp[-_nx].clip) { av += vp[-_nx].vz; ++c; }
                    if (i < _nx-1 && !vp[_nx].clip) { av += vp[_nx].vz; ++c; }
                    
                    if (c)
                        av /= c;
                    else av = _ymin;
                    
                    // fill in average value
                    vp->vz = av;
                }
                ++vp;
            }
        }

        // find the scales and offset
        _xcent = (_xmax + _xmin)/2;
        _ycent = (_ymax + _ymin)/2;
        _zcent = (_zmax + _zmin)/2;

        BCDh sz(MODEL_SIZE);
        _xSize = sz;
        _ySize = sz;
        
        // create a 6/8 aspect ratio for the height.
        _zSize = sz*BCDh(3)/BCDh(4);
        _xScale = _xSize/(_xmax - _xmin);
        _yScale = _ySize/(_ymax - _ymin);
        _zScale = (_zmax != _zmin) ? _zSize/(_zmax - _zmin) : BCDh(0);
        
        // now create the model coordinates
        vp = _model->vertices;

        int radius = 0;
        for (i = 0; i < n_vertices; ++i)
        {
            int x, y, z, t;
            vp->clip = 0; // reset, now all valid

            x = itrunc((vp->vx - _xcent)*_xScale);
            y = itrunc((vp->vy - _ycent)*_yScale);
            z = itrunc((vp->vz - _zcent)*_zScale);

            vp->x = x;
            vp->y = z; // z into screen
            vp->z = y;

            if ((t = isqrt(x*x+y*y+z*z)) > radius)
                radius = t;
            ++vp;
        }
        
        _model->radius = radius;
        //init_normals(_model);
        //shade_planes(_model);

        // so we can find the edges
        calcVertexRefs(_model);

        // back to normal now we have the points
        CPUSpeedNormal();

        res = true;
    }
    return res;
}

void Plot3D::rescale(int x, int y, int z)
{
    int i;
    Point3* vp = _model->vertices;

    BCDh h(100);
    BCDh sx = _xSize*BCDh(x)/h;
    BCDh sy = _ySize*BCDh(y)/h;
    BCDh sz = _zSize*BCDh(z)/h;

    BCDh mx(MODEL_SIZE_MAX);
    BCDh mi(MODEL_SIZE_MIN);
    if (sx < mi || sx > mx || sy < mi || sy > mx || sz < mi || sz > mx)
        return; // scale out of range

    _xSize = sx;
    _ySize = sy;
    _zSize = sz;

    _xScale = _xSize/(_xmax - _xmin);
    _yScale = _ySize/(_ymax - _ymin);
    _zScale = (_zmax != _zmin) ? _zSize/(_zmax - _zmin) : BCDh(0);

    for (i = 0; i < _model->num_vertices; ++i)
    {       
        vp->x = itrunc((vp->vx - _xcent)*_xScale);
        vp->z = itrunc((vp->vy - _ycent)*_yScale);
        vp->y = itrunc((vp->vz - _zcent)*_zScale);
        ++vp;        
    }
}

void Plot3D::resetView()
{
    set_accurate_rotating(_modelH, 0, 0, 0);
    set_orientation(_modelH, 0,0,0);
    set_relative_position(view_model, _modelH, 0, 0, 2*MODEL_SIZE);
}

void Plot3D::move(int dx, int dy, int dz)
{
    set_delta_position(_modelH, dx, dy, dz);
}

void Plot3D::spin(int spin)
{
    set_rotating(_modelH, 0, spin, 0);
}

void Plot3D::orient(int x, int y, int z)
{
    set_delta_orientation(_modelH, x, y, z);
}

bool Plot3D::applyRefinement()
{
    // return true if applied refinement, false otherwise
    bool res = false;
    if (_refined)
    {
        Point3* p1 = _model->vertices + _model->num_vertices;
        Point3* p2 = _model->vertices + _model->vertexCapacity;

        Plane* s1 = _model->planes + _model->num_planes;
        Plane* s2 = _model->planes + _model->planeCapacity;

        int vc = p2 - p1;
        int pc = s2 - s1;
        
        if (pc >= 3 && vc) // have capacity left
        {
            if (p1->y) // number of refinements left
            {
                // move this to next free slot for next time
                if (vc > 1) p1[1].y = p1->y - 1;
                
                Point3* p = p1;
                Plane* pp = _model->planes + p->z;

                // recalculate x & y
                int j;
                BCDh ax = 0;
                BCDh ay = 0;
                Point3** pt = &pp->p1;
                for (j = 0; j < 4; ++j)
                {
                    ax += (*pt)->vx;
                    ay += (*pt)->vy;
                    ++pt;
                }
                
                p->vx = ax/4;
                p->vy = ay/4;
                // vz already correct

                // we have a new point
                ++_model->num_vertices;

                // calc model coordinates
                p->x = itrunc((p->vx - _xcent)*_xScale);
                p->z = itrunc((p->vy - _ycent)*_yScale);
                p->y = itrunc((p->vz - _zcent)*_zScale);

                // makes sure we're facing the right way whilst we refine
                if (pp->pixel) flip_plane(pp);
                
                // now add three triangles
                pt = &pp->p1;
                Plane* snew = _model->planes + _model->num_planes;
                for (j = 0; j < 3; ++j)
                {
                    snew->num_verts = 3;
                    snew->color = TRANSPARENT_COLOUR;
                    snew->type = WIRE_TRIANGLE;
                    snew->area = VISIBLE;
                    snew->p1 = pt[0];
                    snew->p2 = pt[1];
                    snew->p3 = p;
                    ++pt;
                    if (pp->pixel)
                    {
                        flip_plane(snew);
                        snew->pixel = 1;
                    }
                    ++snew;
                }

                _model->num_planes += 3;

                // recycle the original quad as the final triangle
                pp->num_verts = 3;
                pp->type = WIRE_TRIANGLE;
                pp->p2 = p;
                pp->p3 = pp->p4;
                if (pp->pixel) flip_plane(pp);
                res = true;
            }
        }
    }
    return res;
}


bool Plot3D::refineModel()
{
    // perform one refinement step. return false if no more.
    bool res = false;

    // if we're fully refined, don't bother
    if (_refined) return res;

    Point3* p1 = _model->vertices + _model->num_vertices;
    Point3* p2 = _model->vertices + _model->vertexCapacity;

    Plane* s1 = _model->planes + _model->num_planes;
    Plane* s2 = _model->planes + _model->planeCapacity;

    int vc = p2 - p1;
    int pc = s2 - s1;

    /* we store information in the as-yet unused vertices
     * vx = not used
     * vy = relative error
     * vz = function value at centre
     * x = next face to consider (in p1 only)
     * y = first free spare vertex (in p1 only)
     * z = face index for this vertex info.
     */
    
    // we're going to split one quad into 4 triangles
    if (pc >= 3 && vc)
    {
        // work through the faces. 
        int lastf = p1->x;
        BCDh eMin = BCDh::epsilon(2);

        while (lastf < _model->num_planes)
        {
            Plane* pp = _model->planes + lastf;

            p1->x = ++lastf;
            
            if (pp->num_verts == 4)
            {
                BCDh e;

                // eval the mid point
                int j;
                BCDh ax = 0;
                BCDh ay = 0;
                BCDh az = 0;
                Point3** pt = &pp->p1;
                for (j = 0; j < 4; ++j)
                {
                    ax += (*pt)->vx;
                    ay += (*pt)->vy;
                    az += (*pt)->vz;
                    ++pt;
                }
                
                ax /= 4;
                ay /= 4;
                az /= 4;
                
                // eval the function at the mid point
                BCD z;
                if (_eval(ax.asBCD(), ay.asBCD(), z))
                {
                    e = az - z;
                    if (_zmax != _zmin)   // normalise
                        e/=(_zmax - _zmin);
                    
                    if (e.isNeg()) e.negate();
                    if (e > eMin)
                    {
                        // store in spare verts
                        // ordered biggest to smallest error
                        int n = p1->y;
                        Point3* p = p1;
                        while (n && e <= p->vy) { n--; ++p; }
                        
                        // p points to insertion point
                        if (p < p2)
                        {
                            n = p1->y;
                            if (n < vc)
                                ++n;

                            Point3* q2 = p1 + n - 1;
                            Point3* q1 = q2-1;
                            while (q2 != p)
                                *q2-- = *q1--;
                            
                            p1->y = n; // update next free index
                            p1->x = lastf; // ensure we have next face
                            
                            p->vz = z;
                            p->vy = e;
                            p->z = lastf-1; // face index
                        }
                    }
                }
                break;
            }
        }

        if (lastf == _model->num_planes)
        {
            _refined = true;
        }
        else res = true; // more to do
    }
    return res;
}
