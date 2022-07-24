//Lithio (The developer's pseudonym)
//May 31, 2022

#include <stdlib.h>
#include "pwlayer.h"

void pwlayer_init(PWLayer *layer, PWShader *s, PWMat4 projection_matrix){
	pwrenderer_init(&layer->renderer, s);
	layer->rsize = 0;
	layer->rlen = 0;
	layer->renderables = NULL;
	layer->shader = *s;
	layer->projection_matrix = projection_matrix;
	
	layer->group_size = 0;
	layer->group_len = 0;
	layer->group_indices = NULL;
	layer->group_is_end = NULL;
	layer->transformations_size = 0;
	layer->transformations_len = 0;
	layer->transformations_overwrite = NULL;
	layer->group_transformations = NULL;
	
	pwshader_enable(s);
	pwshader_set_uniform_mat4(s, "pr_matrix", projection_matrix); 
}

void pwlayer_close(PWLayer *layer){
	int i;
	
	pwrenderer_close(&layer->renderer);
	if(layer->renderables){
		for(i = 0; i < layer->rlen; ++i){
			pwrenderable_close(layer->renderables[i]);
			free(layer->renderables[i]);
		}
		free(layer->renderables);
		layer->renderables = NULL;
	}
	if(layer->group_indices){
		free(layer->group_indices);
	}
	if(layer->group_is_end){
		free(layer->group_is_end);
	}
	if(layer->group_transformations){
		for(i = 0; i < layer->transformations_len; ++i){
			free(layer->group_transformations[i]);
		}
		free(layer->group_transformations);
	}
	if(layer->transformations_overwrite){
		free(layer->transformations_overwrite);
	}
}


PWRenderable *pwlayer_add(PWLayer *layer, PWRenderable *renderable){
	PWRenderable **tmp;
	if(layer->rlen >= layer->rsize){
		//reallocate
		layer->rsize += 16;
		tmp = (PWRenderable**) realloc(layer->renderables, layer->rsize * sizeof(PWRenderable*));
		if(tmp){
			layer->renderables = tmp;
		}
		else {
			return NULL;
		}
	}
	layer->renderables[layer->rlen] = (PWRenderable*) malloc(sizeof(PWRenderable));
	*layer->renderables[layer->rlen] = *renderable;
	return layer->renderables[layer->rlen++];
}

PWMat4 *pwlayer_add_group(PWLayer *layer, PWMat4 transform, int overwrite){
	void *tmp;
	//reallocate if necessary
	if(layer->group_len >= layer->group_size){
		layer->group_size += 16;
		tmp = realloc(layer->group_indices, layer->group_size * sizeof(int));
		if(tmp){
			layer->group_indices = (int*) tmp;
		}
		else {
			return NULL;
		}
		tmp = realloc(layer->group_is_end, layer->group_size * sizeof(int));
		if(tmp){
			layer->group_is_end = (int*) tmp;
		}
		else {
			return NULL;
		}
	}
	if(layer->transformations_len >= layer->transformations_size){
		layer->transformations_size += 16;
		tmp = realloc(layer->group_transformations, layer->transformations_size * sizeof(PWMat4*));
		if(tmp){
			layer->group_transformations = (PWMat4**) tmp;
		}
		else {
			return NULL;
		}
		
		tmp = realloc(layer->transformations_overwrite, layer->transformations_size * sizeof(int));
		if(tmp){
			layer->transformations_overwrite = (int*) tmp;
		}
		else {
			return NULL;
		}
	}
	//the actual function
	layer->group_indices[layer->group_len] = layer->rlen;
	layer->group_is_end[layer->group_len] = 0;
	++layer->group_len;
	layer->group_transformations[layer->transformations_len] = (PWMat4*) malloc(sizeof(PWMat4));
	*layer->group_transformations[layer->transformations_len] = transform;
	layer->transformations_overwrite[layer->transformations_len] = overwrite;
	return layer->group_transformations[layer->transformations_len++];
}

void pwlayer_end_group(PWLayer *layer){
	void *tmp;
	if(layer->group_len >= layer->group_size){
		layer->group_size += 16;
		tmp = realloc(layer->group_indices, layer->group_size * sizeof(int));
		if(tmp){
			layer->group_indices = (int*) tmp;
		}
		else {
			return;
		}
		tmp = realloc(layer->group_is_end, layer->group_size * sizeof(int));
		if(tmp){
			layer->group_is_end = (int*) tmp;
		}
		else {
			return;
		}
	}
	layer->group_indices[layer->group_len] = layer->rlen;
	layer->group_is_end[layer->group_len] = 1;
	++layer->group_len;
}

void pwlayer_render(PWLayer *layer){
	int i;
	int group_len = 0;
	int transformations_len = 0;
	int pushes = 0;
	int pops = 0;
	
	pwshader_enable(&layer->shader);
	
	//set projection matrix
	pwshader_set_uniform_mat4(&layer->shader, "pr_matrix", layer->projection_matrix);
	
	pwrenderer_begin(&layer->renderer);
	for(i = 0; i < layer->rlen; ++i){
		//push/pop a transform matrix at this element
		while(group_len < layer->group_len && i == layer->group_indices[group_len]){
			if(layer->group_is_end[group_len++]){
				pwrenderer_pop(&layer->renderer);
				++pops;
			}
			else if(transformations_len < layer->transformations_len){
				pwrenderer_push(&layer->renderer, *layer->group_transformations[transformations_len], layer->transformations_overwrite[transformations_len]);
				++transformations_len;
				++pushes;
			}
		}
		if(layer->renderables[i]->str){
			pwrenderer_submit_str(&layer->renderer, layer->renderables[i]);
		}
		else {
			pwrenderer_submit(&layer->renderer, layer->renderables[i]);
		}
	}
	//make sure that transformations aren't accumulating
	while(pushes > pops++){
		pwrenderer_pop(&layer->renderer);
	}
	pwrenderer_end(&layer->renderer);
	pwrenderer_flush(&layer->renderer);
}

void pwlayer_set_projection(PWLayer *layer, PWMat4 projection_matrix){
	layer->projection_matrix = projection_matrix;
}
