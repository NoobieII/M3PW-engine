//Lithio (The developer's pseudonym)
//May 31, 2022

//12 Nov 22
//For readability, I explain the struct. The layer renders renderable objects.
//Renderable objects may have a group transform attached to it
//
//This is an example of a struct with five renderable objects, with two level
//of groups.
//Origin            xx xx
//Transformation 1        xx    xx
//Transformation 2           xx
//
//The variables will be set as:
//
//rsize = 16
//rlen = 5
//renderables xx xx xx xx xx
//
//group_size = 16
//group_len = 3
//
//group_indices 2 3 4
//group_is_end  0 0 1
//
//transformations_size = 16
//transformations_len = 2
//transformations_overwrite 0 0
//group_transformations xx xx
//
//note that sizes are always a multiple of 16

#ifndef M3PW_PWLAYER_H
#define M3PW_PWLAYER_H

#include "pwmath.h"
#include "pwrenderer.h"
#include "pwrenderable.h"
#include "pwshader.h"

typedef struct PWLayer{
	PWRenderer renderer;
	int rsize;
	int rlen;
	PWRenderable **renderables; //deep copies of the renderables sent
	int group_size;
	int group_len;
	int *group_indices; //the transform is performed starting with this index
	int *group_is_end;  //if this is set, the transform is removed instead
	int transformations_size;
	int transformations_len;
	int *transformations_overwrite;
	PWMat4 **group_transformations;
	//PWMat4 **world_transformations
	PWShader shader;		//shader is a copy from outside
	PWMat4 projection_matrix;
} PWLayer;

void pwlayer_init(PWLayer *layer, PWShader *s, PWMat4 projection_matrix);

//whenever possible, pwlayer_reset should be used if the layer is reused
void pwlayer_reset(PWLayer *layer);
void pwlayer_close(PWLayer *layer);

//add a renderable object, PWLayer creates a shallow copy
//a reference to that copy is returned. PWLayer will close the renderable.
//Usage:
//1. Create renderable
//2. Call pwlayer_add
//...
//3. Call pwlayer_close
PWRenderable *pwlayer_add(PWLayer *layer, PWRenderable *renderable);

//all subsequent renderables added will be applied this transform
PWMat4 *pwlayer_add_group(PWLayer *layer, PWMat4 transform, int overwrite);

//all subsequent renderables will not be applied the last transform
void pwlayer_end_group(PWLayer *layer);

//apply transformations and submit all renderables to the renderer
void pwlayer_render(PWLayer *layer);

void pwlayer_set_projection(PWLayer *layer, PWMat4 projection_matrix);

#endif

