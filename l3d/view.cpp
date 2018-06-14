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

#include <string.h>
#include "primitiv.h"
#include "2d.h"
#include "misc.h"
#include "model.h"
#include "view.h"
#include "matrix.h"
#include "world.h"
#include "render.h"
#include "os.h"
#include "matrix.h"

void set_view_scale(Viewport *view)
{
    view->scaler._11 = (1 << ACCURACY) * view->scale_x_num >> 8;
    view->scaler._22 = (1 << ACCURACY) * -view->scale_y_num >> 8;
}

Viewport* create_viewport(int x, int y, int w, int h, int max_num_models)
{
    Viewport *view;
    int       view_mem_size;
                    
    view_mem_size = ( sizeof(Viewport) + 
                      ( sizeof(Model*)    * (2 * max_num_models)) +
                      ( sizeof(ModelView) *      max_num_models));
                

    if (w & 1) return NULL;
    if (h & 1) return NULL;

    view = (Viewport*) Memory(view_mem_size);      
    memset(view, 0, view_mem_size);    
    view->current = GetScreenBitmap2D();

    init_renderer(view->current);
                    
    view->window_x = x;
    view->window_y = y;
    view->window_w = w;
    view->window_h = h;

    /* set up views scaling */
    if (screen_height() < 300)
    {
        view->scale_x_num = 64;
        view->scale_y_num = 64;
    }
    else
    {
        view->scale_x_num = 256;
        view->scale_y_num = 256;
    }

    create_matrix(&view->scaler, 0 ,0 ,0);
    set_view_scale(view);
                         
    view->velocity.z = 0; 
    view->num_viewable_models = 0;
    
    SET_FLAG(view->flags, SOLID_FILLED);

    view->all_visible_models = (Model**) (view + 1);
    view->all_models         = (Model**) (view->all_visible_models + max_num_models);
                     
    view->orientp = &view->orientat;
    create_inverse(&view->orientat, &view->inverse);   
    return view;
}

void destroy_viewport(Viewport* view)
{
    MemoryFree(view);
}

static int modelInView(Viewport *view, Model *m)
{   
    long        scale;
    int         scal;
    int         radius = m->radius;
    int         clip_posL;
    int         clip_posR;
    int         clip_posT;
    int         clip_posB;
    int         half_height = view->window_h >> 1;
    int         half_width  = view->window_w >> 1;
    int         s1,s2;
    long        x,y,p;

    p     = m->relpos.tz + PERSPECTIVE_2;
    if (p == 0) p = 1;
    scale = PERSPECTIVE_1 / p;

    x     = m->relpos.tx * scale;
    y     = m->relpos.ty * scale;

    scal = radius * scale;
    m->screen_size = scal>>12;

    if (m->screen_size < DOT_SIZE)
    {

        int screen_x = (( m->relpos.tx * PERSP_1 ) / p) >> 1;
        int screen_y = (( m->relpos.ty * PERSP_1 ) / p) >> 1;

        if (screen_x < -half_width)
            return FALSE;
        if (screen_x >  half_width)
            return FALSE;
        if (screen_y < -half_height)
            return FALSE;
        if (screen_y >  half_height)
            return FALSE;
        m->screen_x = screen_x + half_width;
        m->screen_y = screen_y + half_height;
        return TRUE;
    }

    s1 = (scal * view->scale_x_num) >> 8;

    clip_posL = (int)((x - s1) >> 12);

    if (clip_posL < half_width)
    {
        /* model is inside right of window */
        clip_posR = (int)((x + s1) >> 12);                            
        if (clip_posR > -half_width)
        {
            s2 = (scal * view->scale_y_num) >> 8;
            clip_posT = (int)((y + s2) >> 12);
            if (clip_posT > -half_height)
            {
                clip_posB = (int)((y - s2) >> 12);
                if (clip_posB < half_height)
                {
                    /*            
                     * model is now definitly on screen
                     * the bounding box calculated here is not strictly correct,
                     * but it works. It will not fully contain a model if the viewing
                     * angle is large as it does not fully account for perspective.
                     */
                    p     = m->relpos.tz + PERSPECTIVE_2 - m->radius;
                    if (p == 0) p = 1;
                    scale = PERSPECTIVE_1 / p;
                    scal = radius * scale;

                    s1 = (scal * view->scale_x_num) >> 8;
                    s2 = (scal * view->scale_y_num) >> 8;

                    clip_posL = (int)((x - s1) >> 12);
                    clip_posR = (int)((x + s1) >> 12);
                    clip_posT = (int)((y + s2) >> 12);
                    clip_posB = (int)((y - s2) >> 12);

                    if (clip_posR >= half_width)
                    {
                        SET_FLAG(m->clip_code, CLIPPED_BY_WINDOW);
                        clip_posR = half_width-1;
                    }
                    if (clip_posL < -half_width)
                    {
                        SET_FLAG(m->clip_code, CLIPPED_BY_WINDOW);
                        clip_posL = -half_width;
                    }
                    if (clip_posB <= -half_height)
                    {
                        SET_FLAG(m->clip_code, CLIPPED_BY_WINDOW);
                        clip_posB = -half_height+1;
                    }
                    if (clip_posT > half_height)
                    {
                        SET_FLAG(m->clip_code, CLIPPED_BY_WINDOW);
                        clip_posT = half_height;
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

extern "C"
{
static int compare_models_z(const void* a, const void* b)
{
    const Model** m1 = (const Model**)a;
    const Model** m2 = (const Model**)b;
    return (*m2)->relpos.tz + (*m2)->radius 
        - (*m1)->relpos.tz - (*m1)->radius;
}
}

void update_viewport(Viewport *view)
{                          
    int                 n;
    Model                **m;
    Model                **vis_models = view->all_visible_models;

    set_view_scale(view);
    create_inverse(view->orientp, &view->inverse);               
    matrix_product(&view->scaler, &view->inverse, &view->cat_mat);

    get_models(view);
    view->num_visible_models = 0;

    for (n = 0; n < view->num_models; n++)
    {
        Model *m = view->all_models[n];
        int z,r;

        // a vector in world space joining view to model  
        VSUB(m->relpos, m->position, *view->positionp);

        // rotate vector to account for view rotation 
        r = m->radius;
        if (v_rot_v(&view->cat_mat, &m->relpos, r)) 
        {
            m->clip_code = 0;
            z = m->relpos.tz;
            if (z > 2*r) 
            {
                if (modelInView(view, m)) 
                {
                    view->num_visible_models++;
                    *vis_models++ = m;
                }

            }
            else
            {
                // model straddles the z clip plane, modelInView can't
                // deal with this special case. At the moment if this occurs
                // allow code to attempt to display it but set clipping so
                // if it is half visible it is clipped correctly.
                if (m->relpos.tx < r &&
                    m->relpos.tx > -r &&
                    m->relpos.ty < r &&
                    m->relpos.ty > -r)
                {
                    SET_FLAG(m->clip_code, CLIP_Z);
                    m->screen_size = 100;/* in do_p_and_t) */
                    view->num_visible_models++;
                    *vis_models++ = m;
                }
            }
        }
    }

    m = view->all_visible_models;

    // sort model order
    qsort2(m,
          view->num_visible_models,
          sizeof(*m),
          compare_models_z);
       
    SET_FLAG(view->flags, V_ROTATED);

    /* Plot all models in view */
    for (n = 0; n < view->num_visible_models; n++, m++)
    {
        if (*m) 
        {
            if ((*m)->screen_size < DOT_SIZE) 
            {
                int sx = (*m)->screen_x;
                int sy = (*m)->screen_y;
                plot (view->current, 0x18, sx,sy);
            } 
            else
                draw_model(*m, view);
        }
    }

    init_renderer(view->current);
}
                     
void align_viewport_with_model(Viewport *view, int num)
{
    Model *m = get_model(num);
    if (m) {
        view->model = m;                               
        view->movement_type = (int)ON_MODEL;
        view->orientp = &m->orientat;
        view->positionp     = &m->position;
    }
}

void align_viewport_with_input(Viewport *view)
{                                       
    view->movement_type = (int)ON_INPUT;
    view->orientp = &view->orientat;
    view->positionp     = &view->position;
}                        

#define MAX_SPEED 200

void move_viewport(Viewport *view, int step)
{
    static int speed = 0;

    Point3 force;
                    
    speed += step;

    if (speed < -MAX_SPEED) speed = -MAX_SPEED;
    else if (speed > MAX_SPEED) speed = MAX_SPEED;

    force.x = 0;
    force.y = 0; 
    force.z = speed;
    /*
      printf("\nspeed - %d   \n", speed);
    */
    rotate_vertex(view->orientp, &force);
                                                      
    view->positionp->x += force.tx;
    view->positionp->y += force.ty;
    view->positionp->z += force.tz;
}


