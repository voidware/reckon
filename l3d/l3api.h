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

#ifndef __l3api_h__
#define __l3api_h__

#include "view.h"
#include "editor.h"

extern int		view_model;
extern int		p_model;			
extern Viewport*	current_view;

int set_position(int, int,int,int);
int set_velocity(int, int,int,int);
int set_delta_position(int num, int dx, int dy, int dz);

int set_force(int, int, int);
int set_max_force(int num, int t);

int set_rotating(int, int,int,int);
int set_accurate_rotating(int num, int t,int a,int i);
int stop_rotation(int);

int set_orientation(int, int,int,int);
int set_delta_orientation(int num, int t, int a, int i);

int copy_position(int, int);
int copy_velocity(int, int);
int copy_orientation(int, int);

int set_velocity_in_frame(int, int, int,int,int);
int set_position_in_frame(int, int, int,int,int);
int set_relative_velocity(int, int, int,int,int);
int set_relative_position(int, int, int,int,int);
int set_collision(int);
int set_view_orientation(Viewport*, int,int,int);
int set_view_position(Viewport*, int,int,int);

int  scale_model(int num, int scal, int div);

int current_view_model(int* Model);
int explode_model_h(int);
int colour_model_h(int, int);
int position_of_model(int, int*, int*, int*);
int set_view_model(int);
int set_model_shading(int, unsigned int type);
void set_mass(int num, int mass);

#endif 

