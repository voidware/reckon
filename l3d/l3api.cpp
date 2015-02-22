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

#include "l3api.h"
#include "matrix.h"
#include "view.h"
#include "world.h"
#include "files.h"
#include "controller.h"
#include "os.h"

#define ANGLE_C (TABLE_SIZE / 360)
#define ANGLE(_a) ((TABLE_SIZE * (_a)) / 360)

int view_model;
int p_model;
Viewport* current_view;

int set_position(int num, int x, int y, int z)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        sp->position.x = x; 
        sp->position.y = y; 
        sp->position.z = z;
    }
    return sp == NULL;
}

int set_delta_position(int num, int dx, int dy, int dz)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        sp->position.x += dx;
        sp->position.y += dy;
        sp->position.z += dz;
    }
    return sp == NULL;
}

int set_velocity(int num, int x, int y, int z)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        set_movement(num);
        sp->velocity.x = x << 8; 
        sp->velocity.y = y << 8; 
        sp->velocity.z = z << 8;
    }
    return sp == NULL;
}

void set_mass(int num, int mass)
{
    Motion *cr = get_motion(num);
    if (cr) 
        cr->mass = mass;
}

int set_force(int num, int t, int on)
{
    Model *sp = get_model(num);
    if (sp) 
    {
    	Motion *cr;
        add_controller(num);
    	cr = sp->motion;
	if (cr)
        {
	    t = t << 4;
	    if (t > cr->max_force) t = cr->max_force;
	    if (t < (-cr->max_force/2)) t = (-cr->max_force/2);
            cr->force = t;
	}
    }
    return sp == NULL;
}

int set_max_force(int num, int t)
{
    Model *sp = get_model(num);
    if (sp)
    {
    	Motion *cr;
        add_controller(num);
    	cr = sp->motion;
	if (cr)
        {
            cr->force = t << 4;
            cr->max_force = t << 4;
	}
    }
    return sp == NULL;
}

int set_rotating(int num, int t,int a,int i)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        set_movement(num);
        create_matrix(&sp->offset, ANGLE(t), ANGLE(a), ANGLE(i));
        SET_FLAG(sp->draw_flags, ROTATING);
    }
    return sp == NULL;
}

int set_accurate_rotating(int num, int t,int a,int i)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        set_movement(num);
        create_matrix(&sp->offset, t, a, i);
        SET_FLAG(sp->draw_flags, ROTATING);
    }
    return sp == NULL;
}


int stop_rotation(int num)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        create_matrix(&sp->offset, 0, 0, 0);
        CLEAR_FLAG(sp->draw_flags, ROTATING);
    }
    return sp == NULL;
}
 
int set_orientation(int num, int t, int a, int i)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        create_matrix (&sp->orientat, ANGLE(t), ANGLE(a), ANGLE(i));
        shade_planes(sp);
    }
    return sp == NULL;
}

int set_delta_orientation(int num, int t, int a, int i)
{
    Model *sp = get_model(num);
    if (sp) 
    {
        Matrix3 m, r;
        create_matrix (&m, ANGLE(t), ANGLE(a), ANGLE(i));
        matrix_product(&m, &sp->orientat, &r);
        sp->orientat = r;
        shade_planes(sp);
    }
    return sp == NULL;
}

int copy_position(int num, int num1)
{
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) 
    {
        s2->position.x = s1->position.x; 
        s2->position.y = s1->position.y; 
        s2->position.z = s1->position.z;
    }
    return s1 == NULL || s2 == NULL;
}

int copy_velocity(int num, int num1)
{
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) 
    {
        set_movement(num1);

        s2->velocity.x = s1->velocity.x; 
        s2->velocity.y = s1->velocity.y; 
        s2->velocity.z = s1->velocity.z;
    }
    return s1 == NULL || s2 == NULL;
}

int copy_orientation(int num, int num1)
{
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) s2->orientat = s1->orientat;
    return s1 == NULL || s2 == NULL;
}

int set_velocity_in_frame(int num, int num1, int x, int y, int z)
{                        
    Point3 vel;
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) 
    {
        set_movement(num1);

        vel.x = x << 8;
        vel.y = y << 8;
        vel.z = z << 8;
             
        rotate_vertex(&s1->orientat, &vel);

        s2->velocity.x = s1->velocity.x + vel.tx; 
        s2->velocity.y = s1->velocity.y + vel.ty; 
        s2->velocity.z = s1->velocity.z + vel.tz;
     }
     return s1 == NULL || s2 == NULL;
}

int set_relative_velocity(int num, int num1, int x, int y, int z)
{                        
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) 
    {
        set_movement(num1);
        s2->velocity.x = s1->velocity.x + (x << 8); 
        s2->velocity.y = s1->velocity.y + (y << 8); 
        s2->velocity.z = s1->velocity.z + (z << 8);
    }
    return s1 == NULL || s2 == NULL;
}

int set_relative_position(int num, int num1, int x, int y, int z)
{                        
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) 
    {
        s2->position.x = s1->position.x + x; 
        s2->position.y = s1->position.y + y; 
        s2->position.z = s1->position.z + z;
    }
    return s1 == NULL || s2 == NULL;
}

int set_position_in_frame(int num, int num1, int x, int y, int z)
{                        
    Point3 pos;
    Model *s1 = get_model(num);
    Model *s2 = get_model(num1);

    if (s1 && s2) {
        pos.x = x;
        pos.y = y;
        pos.z = z;
             
        rotate_vertex(&s1->orientat, &pos);

        s2->position.x = s1->position.x + pos.tx; 
        s2->position.y = s1->position.y + pos.ty; 
        s2->position.z = s1->position.z + pos.tz;
    }
    return s1 == NULL || s2 == NULL;
}

int scale_model(int num, int scal, int div)
{

    Model     *s1   = get_model(num);
    Point3 *vert;
    int       n;
    int       radius = 0;

    if (s1)
    {
        vert = s1->vertices;
        n    = s1->num_vertices;
        for (;n;n--)
        {
            vert->x = (vert->x * scal) / div;
            vert->y = (vert->y * scal) / div;
            vert->z = (vert->z * scal) / div;

            int rad = isqrt(vert->x * vert->x + 
                        vert->y * vert->y +
                        vert->z * vert->z);

            if (rad > radius) radius = rad;
            vert++;
        }
        s1->radius = radius;
    }
    return s1 == NULL;
}

int set_collision(int num)
{
    Model *sp = get_model(num);
    if (sp)
    {
       add_controller(num);
       SET_FLAG (sp->motion->move_flags,MOVE_COLLISIONS);
    }
    return sp == NULL;
}

int set_view_position(Viewport* view, int x, int y, int z)
{
    view->position.x = x; 
    view->position.y = y; 
    view->position.z = z;
    return 0;
}

int set_view_orientation(Viewport *view, int t, int a, int i)
{
    create_matrix(view->orientp, ANGLE(t), ANGLE(a), ANGLE(i));
    return 0;
}

int current_view_model(int* model)
{
    if (model) *model = view_model;
    return model == NULL;
}

int set_view_model(int m)
{
    Model* sh = get_model(m);
    if (sh) {
       view_model = m;
       p_model = m;
       align_viewport_with_model(current_view, view_model); 
    }
    return sh == NULL;
}

int colour_model_h(int m, int col)
{
    Model* sh = get_model(m);
    if (sh) set_model_colour(sh, col);
    return sh != 0;
}

int position_of_model(int m, int* xp, int* yp, int* zp)
{
    Model* sh = get_model(m);
    if (sh) 
    {
        *xp = sh->position.x;
	*yp = sh->position.y;
	*zp = sh->position.z;
    }
    return sh == NULL;
}


int set_model_shading(int m, unsigned int type)
{
    Model* sh = get_model(m);
    if (sh)
    {
        if (type < (1<<SHADING_BITS))
            set_shading_type(sh, type);
    }
    return sh != 0;
}
