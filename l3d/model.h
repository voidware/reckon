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

#ifndef MODEL_H
#define MODEL_H

#include "primitiv.h"

extern int TotalTrianglesDrawn;

#define HIDDEN 0
#define VISIBLE 1

struct Group
{
    unsigned short     point;
    unsigned short     num_planes;
    Plane*             plane;  
};

struct Motion
{
    Vector3*            velocity;
    Matrix3*            orientat; 
    int                 force; 
    int                 max_force;
    unsigned short      model;
    unsigned short      damper;
    unsigned short      mass;
    unsigned short      move_flags;
};

 /* movement flags */
#define MOVE_DAMPING     1
#define MOVE_COLLISIONS  2


// draw flags
#define ROTATING          1
#define CLIPPED_BY_WINDOW 2

struct Model
{
    Vector3             position;
    Vector3             velocity;
    Point3              relpos;
    int                 screen_x;
    int                 screen_y;
    int                 draw_flags;
    int                 clip_code;
    int                 radius;           /* radius of sphere enclosing model */
    int                 screen_size;
    Vector3*            light;
    Matrix3             orientat;
    Matrix3             offset;
    unsigned short      num_sources; 
    unsigned short      orientat_age;
    unsigned short      model_number;
    unsigned short      num_vertices;  
    unsigned short      num_planes;
    unsigned short      num_subgroups;
    unsigned short      vertexCapacity;
    unsigned short      planeCapacity;
    Point3*             vertices;
    Plane*              planes;
    Group*              subgroups;
    Motion*             motion;		/* NULL => not motion */
    Plane**             planes_order;
};

Model* create_model(int n_points,
                    int n_subgroups,
                    int n_planes,
                    int vertexCapacity,
                    int planeCapacity);
void destroy_model(Model*);
void move_model(Model*);
void refine_radius(Model*);
void shade_planes(Model*);
void shade_vertices(Model*);
Model* createDummy();
void calcVertexRefs(Model*);

#endif
