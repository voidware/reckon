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

#ifndef MATRIX_H
#define MATRIX_H

#include "primitiv.h"
#include "misc.h"
#include "model.h"


void    create_inverse(Matrix3 *a, Matrix3 *b);
/*	    creates in inverse of a in b, provided a is orthogonal. 
 */
void    copy_matrix(Matrix3 *a, Matrix3 *b);
/*	    creates a copy of a in b.
 */
void    create_matrix(Matrix3 *a, int, int, int); 

void    matrix_product(Matrix3 *, Matrix3*, Matrix3*);
void    orient_model(Model *, int, int, int);
void    rotate_vertices(Matrix3 *, Point3*, int);
void    rotate_vertex(Matrix3 *, Point3*);
void    inverse_rotate_vertex(Matrix3 *, Point3*);
int     v_rot_v(Matrix3 *, Point3*, int);
void    fixed_rotate_vertex(Matrix3 *, Point3*);
void    rotate_vector(Matrix3 *, Vector3*, Vector3*);
void    inverse_rotate_vector(Matrix3 *, Vector3*, Vector3*);
void    init_normals(Model *);
void    init_matrices(Model *);
void    normalise8(Vector3 *);
void    cross_product(Vector3 *v1, Vector3 *v2, Vector3 *res);
void    normalise_matrix(Matrix3 *);
void    set_model_offset(Model*, int t, int a, int i);
void    plane_normal(Point3* p1, Point3* p2, Point3* d3, Vector3* v);

#define CROSS_PRODUCT(_v1, _v2, _v3)                    \
{                                                       \
    (_v3).x = (_v1).y * (_v2).z - (_v1).z * (_v2).y;    \
    (_v3).y = (_v1).z * (_v2).x - (_v1).x * (_v2).z;    \
    (_v3).z = (_v1).x * (_v2).y - (_v1).y * (_v2).x;    \
}

#define NORMVEC(_p1, _p2, _p3, _v)              \
{                                               \
    Point3 *p1 = (_p1);                       \
    Point3 *p2;                               \
    Vector3 v1, v2;                              \
    int x1 = p1->x;                             \
    int y1 = p1->y;                             \
    int z1 = p1->z;                             \
    p2 = (_p2);                                 \
    v1.x = p2->x - x1;                          \
    v1.y = p2->y - y1;                          \
    v1.z = p2->z - z1;                          \
    p2 = (_p3);                                 \
    v2.x = p2->x - x1;                          \
    v2.y = p2->y - y1;                          \
    v2.z = p2->z - z1;                          \
    CROSS_PRODUCT(v1, v2, _v);                  \
}


#endif

