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
#include "model.h"
#include "matrix.h"
#include "misc.h"
#include "view.h"
#include "render.h"
#include "os.h"
#include "editor.h"

/* Private Declarations */
static void correct_vertices(Model* m, Viewport *view);

#define MAX_NUM_SOURCES 2
#define LIGHT_X 130
#define LIGHT_Y +45
#define LIGHT_Z -77

#define UPDATE_MIN_MAX(a, b, c)         \
{                       \
    if ((c) > (b)) (b) = (c);           \
    else if ((c) < (a)) (a) = (c);      \
}

int Ambient_Light = 16;
Vector3  Light_Source = {LIGHT_X, LIGHT_Y, LIGHT_Z};

typedef int (*CompareFn)(const void*, const void*);

static int compare_planes_z(const Plane** s1, const Plane** s2)
{

    // NB: can get away with just this 
    //return (*s2)->p1->tz - (*s1)->p1->tz;

    int t,u,v;
    const Plane* p;

    p = *s1;
    u = p->p1->tz;
    t = p->p2->tz;
    if (t < u) u = t;
    t = p->p3->tz;
    if (t < u) u = t;
    if (p->num_verts == 4)
    {
        t = p->p4->tz;
        if (t < u) u = t;
    }

    p = *s2;
    v = p->p1->tz;
    t = p->p2->tz;
    if (t < v) v = t;
    t = p->p3->tz;
    if (t < v) v = t;
    if (p->num_verts == 4)
    {
        t = p->p4->tz;
        if (t < v) v = t;
    }
    return v - u;
}

static void sort_planes(Model* m, CompareFn comp_forward)
{
    /* Create planes to sort into `planes_order' then
     * sort them.
     */

    qsort2(m->planes_order,
          m->num_planes,
          sizeof(Plane*),
          comp_forward);
}

Model* create_model(int n_vertices, 
                    int n_subgroups,
                    int n_planes,
                    int vertexCapacity,
                    int planeCapacity)
{
    /*  allocate memory for a new model  */
    Model* model;
    int model_mem_size;
    int i; 

    // ASSUME capacities >= numbers

    model_mem_size = (sizeof(Model) +
                      vertexCapacity * sizeof(Point3) +
                      planeCapacity * sizeof(Plane) +
                      n_subgroups * sizeof(Group) +
                      (planeCapacity) * sizeof(Plane*) +
                      MAX_NUM_SOURCES * sizeof(Vector3));

    model = (Model*) Memory(model_mem_size);
    if (!model) return 0;

    memset(model, 0, model_mem_size);    
    
    // set up all the pointers in the model
    model->vertices = (Point3*)(model + 1);
    model->planes =
        (Plane*)(model->vertices + vertexCapacity);
    
    model->subgroups =
        (Group*)(model->planes + planeCapacity);

    model->light = (Vector3*)(model->subgroups+n_subgroups);
    model->planes_order = (Plane**)(model->light + MAX_NUM_SOURCES);

    // build the default plane order
    Plane* p = model->planes;
    Plane** pp = model->planes_order;
    for (i = 0; i < planeCapacity; ++i) *pp++ = p++;

    model->subgroups->plane = model->planes;
    model->num_vertices = n_vertices;
    model->vertexCapacity = vertexCapacity;
    model->num_planes = n_planes;
    model->planeCapacity = planeCapacity;
    model->num_subgroups = n_subgroups;    

    model->num_sources = 1;
    model->light->x = LIGHT_X;
    model->light->y = LIGHT_Y;
    model->light->z = LIGHT_Z;
    normalise8(model->light);

    model->orientat_age = 1024; // ticks before renormalise matrix
    init_matrices(model);
    return model;
}
             
#define SHADE_SHIFT 6
#define SHADE_MAX 255

void shade_planes(Model *m)
{
    Plane *pln  = m->planes;
    int x,y,z;
    Vector3 light;

    int mt = pln->type & SHADING_MASK;
    if (mt != SHADING_SIMPLE) return;
    int n = m->num_planes;
    int shaded_types = TRIANGLE | POLYGON;

    inverse_rotate_vector(&m->orientat, m->light, &light);

    x = light.x;
    y = light.y;
    z = light.z;

    for (; n; n--)
    {
        if (pln->area > 0 && CHECK_FLAG(pln->type, shaded_types))
        {
            if (pln->color != TRANSPARENT_COLOUR)
            {
                long shade = x*pln->normx + y*pln->normy + z*pln->normz;
                shade = shade >> SHADE_SHIFT;
                if (shade < 0 ) shade = 0;

                shade += Ambient_Light;
                shade += GET_COLOUR(pln);
                if (shade >= SHADE_MAX) shade = SHADE_MAX;
                pln->pixel = (unsigned char)shade;
            }
            else
                pln->pixel = 0;
        }
        pln++;
    }
}

#ifdef SUPPORT_GOURAUD
void shade_vertices(Model *m)
{
    int n = m->num_vertices;
    Point3 *pln  = m->vertices;
    Vector3 light;

    inverse_rotate_vector(&m->orientat, m->light, &light);

    for (; n; n--) 
    {
        int shade = DOT(light, pln->norm) >> SHADE_SHIFT;
        if (shade < 0 ) shade = 0;
        shade += Ambient_Light;
        if (shade >= SHADE_MAX) shade = SHADE_MAX;
        pln->ti = shade;
        pln++;
    }
}
#endif // SUPPORT_GOURAUD

void check_visibility(Model *m)
{                                
    int n = m->num_planes;
    Plane *pp  = m->planes;
    int x, y;
 
    for (; n; n--) 
    {
        Point3 p2 = *pp->p2;

        Point3 *p = pp->p3;
        x = p->tx - p2.tx;
        y = p->ty - p2.ty;
        p = pp->p1;
        int a = x*(p->ty-p2.ty) - y*(p->tx - p2.tx);
        int h;

        if (a >= 0) h = VISIBLE;
        else
        {
            h = HIDDEN;
            if ((pp->type & SHADING_MASK) == SHADING_WIRE)
            {
                // wire polygons are always visible
                h = VISIBLE;
                flip_plane(pp);
                pp->pixel = !pp->pixel;
            }
        }
        pp++->area = h;
    }
}

static void correct_vertices(Model* m, Viewport *view)
{
    Point3* pt;
    Point3* end;

    pt = m->vertices;
    end = pt + m->num_vertices;
    while (pt < end)
    {
        pt->tx >>= 3;
        pt->ty >>= 3;
        pt++;
    }                          
}


/*  create transformed vertices for a given viewport */
static void perspective_transform_clipped(Model *model, Viewport *view)
{
    int  n = model->num_vertices;
    Point3* vert = model->vertices;
    int xp,yp,zp;

    int x_add = view->window_w << 2;
    int y_add = view->window_h << 2;
    int x,y,z;    
    int clip;

    x = model->relpos.tx;                 
    y = model->relpos.ty;
    z = model->relpos.tz;

    for (; n; n--) 
    {
        int  d;
        zp = z + vert->tz;
        if (zp > Z_CLIP_DEPTH) 
        {
            d = zp + PERSPECTIVE_2;
            xp = ((vert->tx + x) << 12) / d;
            yp = ((vert->ty + y) << 12) / d;

            xp += x_add;
            yp += y_add;
    
            clip = 0;
            if( yp <  0 )  SET_FLAG(clip, CLIP_TOP);
            if( yp >= (y_add << 1)) SET_FLAG(clip, CLIP_BOTTOM);
            if( xp >= (x_add << 1)) SET_FLAG(clip, CLIP_RIGHT);
            if( xp <  0 )  SET_FLAG(clip, CLIP_LEFT);
    
            vert->tx = xp;
            vert->ty = yp;
            vert->tz = zp;   
            vert->clip = clip;
        } else 
            vert->clip = CLIP_Z;
        
        vert++;
    }
}

static void perspective_transform(Model *model, Viewport *view)
{
    int  n = model->num_vertices;
    Point3* vert = model->vertices;
    int xp,yp,zp;

    int x_add = view->window_w << 2;
    int y_add = view->window_h << 2;
    int x,y,z;   

    x = model->relpos.tx;                 
    y = model->relpos.ty;
    z = model->relpos.tz;
    
    for (; n; n--)
    {                            
        zp = z + vert->tz;

        xp = (( (vert->tx + x) << 12 )/(zp + PERSPECTIVE_2));
        yp = (( (vert->ty + y) << 12 )/(zp + PERSPECTIVE_2));
 
        vert->tx = xp + x_add;
        vert->ty = yp + y_add;
        vert->tz = zp;
        vert->clip = 0;  
        vert++;
    }
}

void draw_model(Model* model, Viewport *view)
{              
    /* Plot the given `model' within this `view'. Instantiate `extent'
     * to a bounding box around this object.
     */

    Matrix3      screen_rot;
                    
    if (view->model == model) return;

    matrix_product(&view->cat_mat, &model->orientat, &screen_rot);
    rotate_vertices(&screen_rot, model->vertices, model->num_vertices);

    if (model->clip_code > 0) 
        perspective_transform_clipped(model, view);
    else perspective_transform(model, view);
    
    check_visibility(model);

    sort_planes(model, (CompareFn)compare_planes_z);
    correct_vertices(model, view);
    
    if (CHECK_FLAG(model->draw_flags, ROTATING))
    {
        int mt = model->planes->type & SHADING_MASK;

#ifdef SUPPORT_GOURAUD
        if (mt == SHADING_GOURAUD) shade_vertices(model); 
#endif
        shade_planes(model);
    }

    if (model->clip_code > 0) 
        render_clipped_model_planes(model);
    else 
        render_model_planes(model);
}

void destroy_model(Model* m)
{
    MemoryFree(m); 
}

Model* createDummy()
{
    Point3 *vert_p;
    Plane   *plan_p;
    Group   *sub_p;
    Model *m = create_model(1, 1, 1, 1, 1);
    m->radius = 1;
    vert_p = m->vertices;
    vert_p->x = 0;
    vert_p->y = 0;
    vert_p->z = 0;
    plan_p = m->planes;
    sub_p  = m->subgroups;
    m->num_subgroups = 1;
    {
        Point3 **op;
        int i;
        Point3 *base = vert_p;

        sub_p->plane      = plan_p;
        sub_p->num_planes = 1;
        sub_p->point      = 0;

        plan_p->num_verts = 3;
        op = &(plan_p->p1);
        for (i = 0; i < plan_p->num_verts; i++) {
            (*op) = base;
            op++;
        }	   

        plan_p->color = 0;
        plan_p->type = TRIANGLE;
        //plan_p->area = VISIBLE;
    }
    return m;
}

void calcVertexRefs(Model* m)
{
    // calculate the refcount of each vertex
    int np = m->num_planes;
    int nv = m->num_vertices;
    int i;
    Plane* pp = m->planes;
    Point3* p = m->vertices;

    // initialise counters to zero
    for (i = 0; i < nv; ++i) p++->refs = 0;
    
    for (i = 0; i < np; ++i, ++pp)
    {
        int j;
        Point3** vp = &(pp->p1);
        for (j = 0; j < pp->num_verts; ++j) ++(*vp++)->refs;
    }
}

static void plane_normal(Point3* a, Point3* b, Point3* c, Vector3* v)
{
    NORMVEC(a, b, c, *v);
    normalise8(v);
}

void init_vertex_normals(Model *m)
{
#ifdef SUPPORT_GOURAUD
    Point3 *vert;

    // require higher precision whilst calculating
    int sz = m->num_vertices * sizeof(Vector3);
    Vector3* vNormals = (Vector3*)xmalloc(sz);
    memset(vNormals, 0, sz);
    
    sz = m->num_vertices * sizeof(int);
    int* vNormalsCount = (int*)xmalloc(sz);
    memset(vNormalsCount, 0, sz);
    
    Plane *pln = m->planes;
    int i,p;

    for (i = 0; i < m->num_planes; i++, pln++)
    {
        Point3 **op = &pln->p1;
        for (p = 0; p < pln->num_verts; p++, op++)
        {
            int vi = *op - m->vertices;
            if (vi < 0 || vi >= m->num_vertices)
                return; // bad data. bail

            vNormals[vi].x += pln->normx;
            vNormals[vi].y += pln->normy;
            vNormals[vi].z += pln->normz;
            vNormalsCount[vi]++;
        }
    }

    vert = m->vertices;
    for (i = 0; i < m->num_vertices; i++, vert++)
    {
        int n = vNormalsCount[i];
        if (n)
        {
            vert->norm.x = vNormals[i].x/n;
            vert->norm.y = vNormals[i].y/n;
            vert->norm.z = vNormals[i].z/n;
        }
    }
    
    xfree(vNormalsCount);
    xfree(vNormals);

#endif // SUPPORT_GOURAUD
}

void init_normals(Model *model)
{  
    /* calculate plane normals */
    int n = model->num_planes;
    Plane* pp = model->planes;
    for (; n; n--) 
    {
        Vector3 v;
        plane_normal(pp->p1, pp->p2, pp->p3, &v);
        
        pp->normx = v.x;
        pp->normy = v.y;
        pp->normz = v.z;
        ++pp;
    }
    init_vertex_normals(model);
}

void refine_radius(Model* m)
{
    /* Refine the value of the radius.
     * Find the min and max extents of each dimension.
     * This defines a bounding box. The distance from the centre
     * of this box to a corner is the radius.
     */

     int            n;
     int            min_x, max_x;
     int            min_y, max_y;
     int            min_z, max_z;

     n = m->num_vertices;
     if (n-- > 0) 
     {
         Point3*  p = m->vertices;
         int        dx, dy, dz;

         min_x = max_x = p->x;
         min_y = max_y = p->y;
         min_z = max_z = p->z;

         while (n-- > 0) 
         {
             ++p;
             if (p->x < min_x) min_x = p->x;
             else if (p->x > max_x) max_x = p->x;

             if (p->y < min_y) min_y = p->y;
             else if (p->y > max_y) max_y = p->y;

             if (p->z < min_z) min_z = p->z;
             else if (p->z > max_z) max_z = p->z;
         }

         dx = (max_x - min_x);
         dy = (max_y - min_y);
         dz = (max_z - min_z);
         m->radius = isqrt(dx * dx + dy * dy + dz * dz) / 2; 
     }
}

