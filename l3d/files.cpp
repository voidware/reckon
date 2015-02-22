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

// comment in to include "save" code
//#define SAVE

#ifdef SAVE
static void putByte(char num, FILE *fp)
{        
    putc(num, fp);
}

static void putShort(short num, FILE *fp)
{        
    putc(num & 0xff, fp);
    putc(num >> 8, fp);
}

static void putLong(long num, FILE* fp)
{        
    putShort((short)(num & 0xffff), fp);
    putShort((short)(num >> 16), fp);
}
#endif // SAVE

static char getByte(FILE* fp)
{ 
    return getc(fp); 
}

static short getShort(FILE* fp)
{ 
    short tmp = getc(fp);
    tmp |= getc(fp) << 8; 
    return tmp;
}

static long getLong(FILE* fp)
{ 
    long tmp = getShort(fp);
    tmp |=  getShort(fp) << 16;
    return tmp;
}

Model* load_object(const char *name)
{
    Model* m;
    FILE* fp;  

    if (!(fp = fopen(name, "rb"))) 
    {
        print("file not found");
        return NULL;
    }
     
    if (fp) 
    {
        int num;
        int n_vertices;
        int n_planes;
        int n_subgroups;
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
         
        m = create_model(n_vertices, n_subgroups, n_planes, n_vertices, n_planes);
        if (m)
        {
            m->radius= getShort(fp); 

            vert_p = m->vertices;
            
            for(; n_vertices; n_vertices--) 
            {
                vert_p->x = getShort(fp);
                vert_p->y = getShort(fp);
                vert_p->z = getShort(fp);
                vert_p++;
            }

            plan_p = m->planes;
            sub_p  = m->subgroups;
            m->num_subgroups = n_subgroups;                          

            for(; n_subgroups; n_subgroups--) 
            {
                Point3 **op;
                int i;
                int subnum;
                Point3 *base = m->vertices;

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

                    op = &plan_p->p1;
                    for (i = 0; i < plan_p->num_verts; i++) 
                    {
                        (*op)  = base + getShort(fp);
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
            init_normals(m);
            shade_planes(m);
            calcVertexRefs(m);

#ifdef SUPPORT_GOURAUD
            shade_vertices(m);
#endif
            
        }
        fclose(fp);
    }
    return m;
}

#ifdef SAVE
void save_object(Model* m, char* name)
{
    FILE *fp = fopen(name, "wb");
    Point3 *vert_p = m->vertices;
    int n_verts    = m->num_vertices;    
    int n_subs     = m->num_subgroups;
    Plane *plan_p  = m->planes;
    Group *sub_p   = m->subgroups;

    if (fp)
    {  
        putLong( MODEL_VERSION, fp);   

        printf("saving - %s ",name);
        putShort(m->num_vertices, fp);
        putShort(m->num_planes, fp );
        putShort(m->num_subgroups, fp);
        putShort(m->radius, fp);                             

        for(; n_verts; n_verts--)
        {
            putShort(vert_p->x, fp);
            putShort(vert_p->y, fp);
            putShort(vert_p->z, fp);
            vert_p++;
        }

        for(; n_subs; n_subs--)
        {
            Point3 **op;
            int i;
            Point3 *base = m->vertices;
            putByte(1/*sub_p->num*/, fp);  
            putShort(sub_p->num_planes, fp);
            putShort(sub_p->point, fp);
            int np = sub_p->num_planes;
            plan_p  = sub_p->plane;     
        
            for(; np; np--)
            {
                putByte(plan_p->num_verts, fp);
                op = &plan_p->p1;
                for (i = 0; i < plan_p->num_verts; i++)
                {
                    putShort((*op) - base, fp);
                    op++;
                }
                putByte(plan_p->color, fp);
                plan_p++;
            }
            sub_p++;
        }
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
