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
#include "world.h"
#include "l3api.h"

World *theWorld;

void create_world()
{    
    
    // fixed amount for the world
    static char worldMem[sizeof(World) + sizeof(ModelBox)*MODELS_MAX];
    theWorld = (World*)worldMem;
    memset(worldMem, 0, sizeof(worldMem));
    
    theWorld->all_models         = (ModelBox*) (theWorld + 1);
    theWorld->model_stak_top     = theWorld->all_models;
    theWorld->num_models         = 0;
    theWorld->max_num_models     = MODELS_MAX;
}

void update_world()
{
    ModelBox* ms = theWorld->all_models;

    /* move all models */
    for (int n = 0; n < theWorld->top_model; n++) 
    {
        if (ms[n].ismodel == FULL)
            if (ms[n].movement == MOVING) move_model(ms[n].model);
    }
}

void delete_model(int num)
{
    ModelBox *ms = theWorld->all_models;
  
    if (num == view_model)  // dont delete the view model!
        return;

    if (ms[num].ismodel == EMPTY)
        return;
    else
    {
        if (theWorld->next_free_slot > num)
            theWorld->next_free_slot = num;
        ms[num].ismodel = EMPTY;
        if((num + 1) == theWorld->top_model) theWorld->top_model--; 
                         /* if we delete the last model shrink stack */
        theWorld->num_models--;
    {
        Model *m = ms[num].model;
        if (m->motion) MemoryFree(m->motion);
        if (m)        MemoryFree(m); 
    }
    }
}

int add_model_to_world(Model *m)
{
    ModelBox *ms = theWorld->all_models;
    int n;

    if(m == NULL)
        return -1; 

    for (n = theWorld->next_free_slot; n < theWorld->top_model; n++) 
    {
        /* is there an empty slot */
        if (ms[n].ismodel == EMPTY)
        {
            ms[n].ismodel =  FULL;
            ms[n].model = m;                     /* drop in model pointer */
            theWorld->num_models++;
            m->model_number = n;
            theWorld->next_free_slot = n;
            return n;
        }
    }

    /* if no empty slots */
    /* put model at top */                   

    if (n >= theWorld->max_num_models) 
    {
        //printf("World is full\n");
        //abort(); /* poo */
        return -1;
    }

    ms[n].ismodel =  FULL;
    ms[n].model = m; 
    theWorld->num_models = n+1;
    theWorld->top_model  = n+1;
    m->model_number = n;
    theWorld->next_free_slot = n;
    return n;
}

void set_movement(int num)
{
    theWorld->all_models[num].movement = MOVING;
}

Model* get_model(int num)
{
    if (num >= 0) {
        ModelBox* box = theWorld->all_models + num;
        if (box->ismodel == FULL) return box->model;
    }
    return 0;
}

Motion* get_motion(int num)
{
    ModelBox* box = theWorld->all_models + num;
    return box->ismodel == FULL ? box->model->motion : NULL;
}
                                    
void get_models(Viewport *view)
{
    int     n;
    Model  **store = view->all_models;
    ModelBox *ms = theWorld->all_models;
    Model    *sh;

    view->num_models = 0;        

    for (n = 0; n < theWorld->top_model; n++) 
    {
        if (ms[n].ismodel == FULL) 
        {
            sh = ms[n].model;
            *store++ = sh;
            view->num_models++;
        }
    }
}
