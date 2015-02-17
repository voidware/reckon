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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitiv.h"
#include "model.h"
#include "misc.h"
#include "matrix.h"
#include "files.h"
#include "world.h"
#include "l3api.h"
#include "os.h"


#define MODEL_VERSION 0x02000000L
#define MAX_VERTICES 6
#define SAVExx

#ifdef SAVE
void putByte(char num, FILE *fp)
{        
    putc(num, fp);
}

void
putShort(short num, FILE *fp)
{        
    putc(num & 0xff, fp);
    putc(num >> 8, fp);
}

void
putLong(long num, FILE* fp)
{        
    putShort((short)(num & 0xffff), fp);
    putShort((short)(num >> 16), fp);
}
#endif // SAVE

char getByte(FILE* fp)
{ 
    return getc(fp); 
}

short getShort(FILE* fp)
{ 
    short tmp = getc(fp);
    tmp |= getc(fp) << 8; 
    return tmp;
}

long getLong(FILE* fp)
{ 
    long tmp = getShort(fp);
    tmp |=  getShort(fp) << 16;
    return tmp;
}

Model* load_object(const char *name)
{
    Model *load_model;
    FILE *fp;  
    int scale = 0;

    if (!(fp = fopen(name, "rb"))) 
    {
        print("file not found");
        return NULL;
    }
     
    if (fp) 
    {
        int num,radius = 0;
        int n_vertices    ;
        int n_planes      ;
        int n_subgroups   ;
        Point3 *vert_p;
        Plane   *plan_p;
        Group   *sub_p;

        long id = getLong (fp);

        if (id != MODEL_VERSION) 
        {
            print("Incompatible model version");
            return 0;
        }

        n_vertices  = getShort(fp);
        n_planes    = getShort(fp);
        n_subgroups = getShort(fp);
         
        load_model = create_model(n_vertices, n_subgroups, n_planes, n_vertices, n_planes);
        if (load_model)
        {
            load_model->radius= getShort(fp); 

            vert_p = load_model->vertices;
            
            for(; n_vertices; n_vertices--) 
            {
                int x,y,z,tmp;
                x = getShort(fp)<<scale;
                y = getShort(fp)<<scale;
                z = getShort(fp)<<scale;
                vert_p->x = x;
                vert_p->y = y;
                vert_p->z = z;
                if ((tmp = isqrt(x*x+y*y+z*z)) > radius)
                    radius = tmp;
                vert_p++;
            }

            load_model->radius = radius;

            plan_p = load_model->planes;
            sub_p  = load_model->subgroups;
            load_model->num_subgroups = n_subgroups;                          

            for(; n_subgroups; n_subgroups--) 
            {
                Point3 **op;
                int i;
                int subnum;
                Point3 *base = load_model->vertices;

                sub_p->plane      = plan_p;
                subnum            = getByte(fp);
                sub_p->num_planes = getShort(fp);
                sub_p->point      = getShort(fp);                         
                num               = sub_p->num_planes;

                for(; num; num--) 
                {
                    plan_p->num_verts = getByte(fp);

                    if (plan_p->num_verts > MAX_VERTICES)
                        plan_p->num_verts = MAX_VERTICES;

                    op = &(plan_p->p1);
                    for (i = 0; i < plan_p->num_verts; i++) 
                    {
                        (*op)   = base + getShort(fp);
                        if (*op < base) *op = base;
                        op++;
                    }	   

                    plan_p->color = getByte(fp);
                    if (i == 3) 
                        plan_p->type = WIRE_TRIANGLE;
                    else
                        plan_p->type = WIRE_POLYGON;

                    plan_p->area = VISIBLE;
                    plan_p++;
                }
                sub_p++;
            }
            load_model->scale  = scale;
            init_normals(load_model);
            shade_planes(load_model);
            calcVertexRefs(load_model);
#ifdef SUPPORT_GOURAUD
            shade_vertices(load_model);
#endif
        }
        fclose(fp);
    }
    return load_model;
}

#ifdef SAVE
void save_object(Model* save_model, char* name)
{
    FILE *fp = fopen(name, "wb");
    Point3 *vert_p = save_model->_vertices;
    int n_verts      = save_model->num_vertices;    
    int n_subs       = save_model->num_subgroups;
    Plane *plan_p   = save_model->planes;
    Group *sub_p    = save_model->subgroups;

    if (fp) {  
        int n_plans;

        putLong( MODEL_VERSION, fp);   

        printf("saving - %s ",name);
        putShort(save_model->num_vertices , fp);
        putShort(save_model->num_planes   , fp );
        putShort(save_model->num_subgroups, fp);
        putShort(save_model->radius, fp);                             

        for(; n_verts; n_verts--) {
            putShort(vert_p->x, fp);
            putShort(vert_p->y, fp);
            putShort(vert_p->z, fp);
            vert_p++;
        }

        for(; n_subs; n_subs--) {
	    Point3 **op;
	    int i;
            Point3 *base = save_model->_vertices;
            putByte(1/*sub_p->num*/, fp);  
            putShort(sub_p->num_planes, fp);
            putShort(sub_p->point, fp);
            n_plans = sub_p->num_planes;
            plan_p  = sub_p->plane;     
        
            for(; n_plans; n_plans--) {
                putByte(plan_p->num_verts, fp);
		op = &(plan_p->p1);
	        for (i = 0; i < plan_p->num_verts; i++) {
                    putShort((*op) - base, fp);
	    	    op++;
		}
                putByte(plan_p->color, fp);
                plan_p++;
            }
            sub_p++;
        }
        printf("ok\n");
        fclose(fp);
    }
}
#endif // SAVE
         
Model* load_model(const char *name)
{
    Model* m;
    m = load_object(name);
    if (m) refine_radius(m);
    return m;
}
