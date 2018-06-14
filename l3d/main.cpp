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

#include "l3defs.h"
#include "os.h"
#include "l3api.h"
#include "world.h"
#include "files.h"
#include "controller.h"
#include "matrix.h"
#include "view.h"

#ifdef _WIN32
#define SCREEN_W        1024
#define SCREEN_H        768
#else
#define SCREEN_W        128
#define SCREEN_H        64
#endif

int screen_width()
{
    return SCREEN_W;
}

int screen_height()
{
    return SCREEN_H;
}

int L3Start(const char* modelName)
{                   
    int w, h, x, y;
    int m;

    extern void Init(Bitmap2D*);

    x = 0;
    y = 0;
    w = SCREEN_W;
    h = SCREEN_H;
    Init(CreateBitmap2D(w, h, 8));

    current_view  = create_viewport(0, 0, w, h, MODELS_MAX);

    p_model = add_model_to_world(createDummy());
    set_position(p_model, 200, 900, 000);               
    set_rotating(p_model,0,0,0);
    view_model = p_model;
    {
        Model *v = get_model(view_model);
        add_controller(view_model);
        if (v) v->motion->damper = 1;
        set_mass(view_model, 80);
    }

    align_viewport_with_model(current_view, view_model); 

    Model* mp = load_model(modelName);
    if (mp) m = add_model_to_world(mp);
    if (m >= 0)
    {
        int col = 1;
        set_model_shading(m, SHADING_SIMPLE);
        //set_model_shading(m, SHADING_GOURAUD);
        
#if 0
        // force wire!
        col = TRANSPARENT_COLOUR;
#endif
        colour_model_h(m, col);
            
        set_relative_position(view_model, m, 10, 0, 800);
        set_rotating(m, 1,1,0);
    }
    return m >= 0;
}

void L3Tick()
{

    //static unsigned long last_time = 0;
    static int frame = 0;  
    static int frame_count = 0;

    update_world();
    update_viewport(current_view);

    frame++;
    frame_count++;

#if 0
    if (frame == 100) {

        int a = 0;
        static int dummy = 0;
        static int TotalTriangles = 0;
             
        TotalTriangles += TotalTrianglesDrawn;

        frame = Time();
        a     = frame - last_time;

        printf("%d:%ld tks=> %d frames/s, %d tri/s\r",
               dummy++, a, 10000 / a, 
               TotalTrianglesDrawn*100/a); 
        fflush(stdout);
        frame = 0;
          
        TotalTrianglesDrawn = 0;
        last_time = Time();
    }
#endif
}

#define FORCE 511

void L3HandleInput(int key)
{
    Model *m = get_model(view_model);
    int offset;
    int the = 0;
    int phi = 0;
    int rho = 0;
              
    if (!m) return;
                        
    if (key == 'q') offset = 30;
    else if (key == 'w') offset = 4;
    else offset = 10;

    if      (key == '7') rho -= offset;
    else if (key == '9') rho += offset;

    if      (key == '8') the -= offset;
    else if (key == '2') the += offset;
        
    if      (key == '6') phi -= offset;
    else if (key == '4') phi += offset;
    if      (key == '.') phi -= offset;
    else if (key == ',') phi += offset;
    if      (key == 'z') the -= offset;
    else if (key == 'x') the += offset;

    if (phi > 50) phi = 50;
    else if (phi < -50) phi = -50;
    if (the > 50) the = 50;
    else if (the < -50) the = -50;
    if (rho > 50) rho = 50;
    else if (rho < -50) rho = -50;

    set_force(view_model,0,0);
    if (key == 't')      set_force(view_model, FORCE, 1);
    else if (key == 'r') set_force(view_model, -FORCE/2, 1);
    else if (key == 's') set_velocity(view_model, 0, 0, 0); // stop!

#if 0
    if (Mouse_Attached) {
        true_mouse_xy(&a, &b);
        phi = -(a - (screen_width()/2))/12;
        the = (b - (screen_height()/2))/6;
        c = mouse_buttons();
        if (CHECK_FLAG(c, LEFT_BUTTON)) rho = -20;
        if (CHECK_FLAG(c, RIGHT_BUTTON)) rho = 20;
	}
#endif

    if (phi >  40) phi = 40;
    if (phi < -40) phi = 40;
    if (the >  40) the = 40;
    if (the < -40) the = 40;

    set_model_offset(m, the, phi, rho);
}
