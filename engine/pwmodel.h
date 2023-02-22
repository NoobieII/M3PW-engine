//NoobieII
//17 Dec 2022

#ifndef M3PW_MODEL_H
#define M3PW_MODEL_H

#include "pwengine.h"
#include "pwlayer.h"
#include "pwmath.h"
#include "pwrenderable.h"

//How this works,

typedef struct PWModel{
	int num_parts;
	PWRenderable *model_indices;   //Reference returned by layer. 
	PWRenderable **parts_vertices; //References returned by layer.
	PWMat4 **transforms;           //Transforms may be modified by outsiders.
	PWEngine *engine;
}PWModel;

//return 0 if successful, -1 if not
int pwmodel_load(PWModel *model, const char *filename, PWEngine *engine, PWLayer *layer);
void pwmodel_close(PWModel *model);

//the following functions are for editing a model. Not for use in a game
void pwmodel_init(PWModel *model, PWEngine *engine);
void pwmodel_add_shape(PWModel *model, const char *filename);
void pwmodel_set_transform(PWModel *model, PWMat4 transform);
void pwmodel_save(PWModel *model, PWEngine *engine);

#endif
