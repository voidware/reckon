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

#ifndef VIEW_H
#define VIEW_H
 
#include "model.h"

struct ModelView
{
    Vector3      position;
    Model*       model;
};

enum {WIRE_FRAME, SOLID_FILLED, NUM_RENDER_TYPES};

enum {ON_MODEL, ON_INPUT, NUM_MOVEMENT_TYPES};

#define SOLID_FILLED      1
#define ACTIVE            2
#define CROSSHAIR         4
#define PLANETS           16
#define V_ROTATED         32

#define MAX_MODEL_CLIP_DIST 20000
#define MAX_MODEL_VIS_DIST 100000

struct Viewport
{
    Bitmap2D* current;
    Vector3   position;
    Vector3   velocity;
    Vector3*  positionp;
    Matrix3*  orientp;
    Matrix3   orientat;
    Matrix3   inverse;
    Matrix3   cat_mat;
    Matrix3   scaler;
    int       scale_x_num; 
    int       scale_x_den; 
    int       scale_y_num; 
    int       scale_y_den;
    int       window_x;
    int       window_y;
    int       window_w;
    int       window_h;
    Model     *model;         
    unsigned short flags;
    unsigned short num_models;
    unsigned short num_visible_models;
    unsigned short num_viewable_models;
    unsigned short num_relative_models;
    unsigned short movement_type;
    Model    **all_visible_models;
    Model    **all_models;
    ModelView *viewable_models;
};

Viewport *create_viewport(int, int, int, int, int max_num_models);
void      update_viewport(Viewport*);
void      plot_viewport(void);
void      draw_model(Model*, Viewport*);
void      move_viewport(Viewport*, int);
void      align_viewport_with_model(Viewport*, int);
void      align_viewport_with_input(Viewport*);
void      set_view_scale(Viewport *view);
void    destroy_viewport(Viewport* view);

#endif
