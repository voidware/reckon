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
#include "model.h"
#include "view.h"
#include "world.h"
#include "matrix.h"
#include "controller.h"

int add_controller(int num)
{
    int ret = 0;
    Model *m = get_model(num);
    if (m)
    {
        Motion* motion = (Motion*) Memory(sizeof(Motion));
        if (motion)
        {
            memset(motion, 0, sizeof(Motion));   
            motion->orientat = &m->orientat;
            motion->velocity = &m->velocity;
            motion->model = m->model_number;
            motion->mass = 3; /* unit mass */
            motion->damper = 32;
            motion->max_force = 10 << 4;
            SET_FLAG(motion->manoeuvre_flags, VELOCITY_DAMPERS);
            m->motion = motion;
            ret = 1;
        }
    }
    return ret;
}

/* moves a model in space , does not care about the viewport */
/* should only come here for a moving model */

void move_motion(Motion *motion)
{
    if (motion->force != 0) 
    {
        Point3 force;

        force.x = 0;                         
        force.y = 0;   /* original models were orientated along the y axis */ 
        force.z = motion->force;
        rotate_vertex(motion->orientat, &force);

        motion->velocity->x += (force.tx << 4) / motion->mass;
        motion->velocity->y += (force.ty << 4) / motion->mass;
        motion->velocity->z += (force.tz << 4) / motion->mass;
    }
                                                  
    if (CHECK_FLAG(motion->manoeuvre_flags, VELOCITY_DAMPERS)) 
    {
        int a = motion->velocity->x;
        int b = motion->velocity->y;
        int c = motion->velocity->z;

       /*   	Fdam = k V 
       	*   	where  k is the damping factor.
	*   	the force can not exceed the max damping force of
	*   	the model.
	*/
	/* not true at the moment */
	
     	a = (a / (motion->damper * motion->mass));
     	b = (b / (motion->damper * motion->mass));
     	c = (c / (motion->damper * motion->mass));

        motion->velocity->x -= a;
        motion->velocity->y -= b;
        motion->velocity->z -= c;
    }                 
}

void move_model(Model *m)
{
    if (m->motion)
	move_motion(m->motion);

    if (CHECK_FLAG(m->draw_flags, ROTATING)) 
    {
        if (m->orientat_age == 0)
        {
            m->orientat_age = 1024;
            normalise_matrix(&m->orientat);
        }
        m->orientat_age--;

        /* move model by it */
        matrix_product(&m->orientat, &m->offset, &m->orientat);
    }

    m->position.x += m->velocity.x >> 8;
    m->position.y += m->velocity.y >> 8;
    m->position.z += m->velocity.z >> 8;
}




