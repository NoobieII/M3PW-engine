//NoobieII
//27 Dec 2022

#include <stdio.h>
#include <string.h>
#include "pwmodel.h"

#define INPUT_LEN 200

/*
Add renderable with indices only
  create a renderable with only the vertices of the part. Index array empty
  Add the renderable to the layer
*/


int pwmodel_load(PWModel *model, const char *filename, PWEngine *engine, PWLayer *layer){
	FILE *in;
	char str[INPUT_LEN];
	char dir[INPUT_LEN];
	char path[INPUT_LEN];
	PWTexture *texture;
	PWRenderable part_entire;
	PWRenderable part_indices;
	PWRenderable part_vertices;
	int start_index;
	int end_index;
	PWMat4 detransform;
	PWMat4 group_transform;
	int num_group_end;
	int i;
	int j = 0;
	int k = 0;
	int result;
	
	if(!engine){
		return -1;
	}
	
	in = fopen(filename, "r");
	if(!in){
		return -1;
	}
	
	//find directory
	strcpy(dir, filename);
	while(strlen(dir) > 0 && (dir[strlen(dir) - 1] != '/' && dir[strlen(dir) - 1] != '\\')){
		dir[strlen(dir) - 1] = '\0';
	}
	
	
	//expect: PWMODEL
	do{
		fgets(str, INPUT_LEN, in);
	} while(strlen(str) < 2);
	if(strncmp(str, "PWMODEL", 7) != 0){
		goto pwmodel_error_close;
	}
	
	//expect: <model_filename>
	do{
		fgets(str, INPUT_LEN, in);
	} while(strlen(str) < 2);
	str[strcspn(str, "\n")] = '\0';
	strcpy(path, dir);
	strcat(path, str);
	if(pwrenderable_load(&part_entire, path) != 0){
		goto pwmodel_error_close;
	}
	
	//expect: <sprite_filename>
	do{
		fgets(str, INPUT_LEN, in);
	} while(strlen(str) < 2);
	str[strcspn(str, "\n")] = '\0';
	strcpy(path, dir);
	strcat(path, str);
	texture = pwengine_get_texture(engine, path);
	
	//expect: num_parts <number>
	fgets(str, INPUT_LEN, in);
	if(sscanf(str, "%*s%d", &model->num_parts) != 1){
		goto pwmodel_error_close;
	}
	
	if(model->num_parts < 0 || model->num_parts > 100){
		goto pwmodel_error_close;
	}
	
	//add all the model's indices first
	pwrenderable_init_none(&part_indices);
	part_indices.index_count = part_entire.index_count;
	part_indices.indices = (unsigned short*) malloc(sizeof(unsigned short) * part_indices.index_count);
	memcpy(part_indices.indices, part_entire.indices, sizeof(unsigned short) * part_indices.index_count);
	model->model_indices = pwlayer_add(layer, &part_indices);
	
	model->transforms = (PWMat4**) malloc(sizeof(PWMat4*) * model->num_parts);
	model->parts_vertices = (PWRenderable**) malloc(sizeof(PWRenderable*) * model->num_parts);
	
	//read from file
	for(i = 0; i < model->num_parts; ++i){
		//vertices <start> <end> detransform <detransform> transform <transform> num_group_end <num_group_end>
		//filename transform num_group_end
		do{
			fgets(str, INPUT_LEN, in);
		} while(strlen(str) < 2);
		
		result = sscanf(str, 
			"%*s"
			"%d%d"                             //vertex indices
			"%*s"
			"%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f" //detransform
			"%*s"
			"%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f" //transform from last group
			"%*s"
			"%d",                              //num group end
			&start_index,
			&end_index,
			&detransform.elements[0],
			&detransform.elements[1],
			&detransform.elements[2],
			&detransform.elements[3],
			&detransform.elements[4],
			&detransform.elements[5],
			&detransform.elements[6],
			&detransform.elements[7],
			&detransform.elements[8],
			&detransform.elements[9],
			&detransform.elements[10],
			&detransform.elements[11],
			&detransform.elements[12],
			&detransform.elements[13],
			&detransform.elements[14],
			&detransform.elements[15],
			&group_transform.elements[0],
			&group_transform.elements[1],
			&group_transform.elements[2],
			&group_transform.elements[3],
			&group_transform.elements[4],
			&group_transform.elements[5],
			&group_transform.elements[6],
			&group_transform.elements[7],
			&group_transform.elements[8],
			&group_transform.elements[9],
			&group_transform.elements[10],
			&group_transform.elements[11],
			&group_transform.elements[12],
			&group_transform.elements[13],
			&group_transform.elements[14],
			&group_transform.elements[15],
			&num_group_end
			);
		
		
		if(result != 35){
			printf("pwmodel_load: invalid contents in file %s\n%s\n", filename, str);
		}
		
		//create groups
		pwlayer_add_group(layer, group_transform, 0);
		model->transforms[i] = pwlayer_add_group(layer, PWM_ini(1.0, NULL), 0);
		
		//create renderable containing only vertices
		pwrenderable_init_none(&part_vertices);
		part_vertices.vertex_count = end_index - start_index;
		
		part_vertices.p = (PWVec3*) malloc(sizeof(PWVec3) * part_vertices.vertex_count);
		part_vertices.uv = (PWVec2*) malloc(sizeof(PWVec2) * part_vertices.vertex_count);
		part_vertices.color = (unsigned int*) malloc(sizeof(unsigned int) * part_vertices.vertex_count);
		part_vertices.n = (PWVec3*) malloc(sizeof(PWVec3) * part_vertices.vertex_count);
		
		memcpy(part_vertices.p, &part_entire.p[start_index], sizeof(PWVec3) * part_vertices.vertex_count);
		memcpy(part_vertices.uv, &part_entire.uv[start_index], sizeof(PWVec2) * part_vertices.vertex_count);
		memcpy(part_vertices.color, &part_entire.color[start_index], sizeof(unsigned int) * part_vertices.vertex_count);
		memcpy(part_vertices.n, &part_entire.n[start_index], sizeof(PWVec3) * part_vertices.vertex_count);
		
		//detranform the vertices and normals
		for(j = 0; j < end_index - start_index; ++j){
			part_vertices.p[j] = PWM_mul_vec3(detransform, part_vertices.p[j]);
			part_vertices.n[j] = PWM_mul_vec3_notranslate(detransform, part_vertices.n[j]);
		}
		
		pwrenderable_set_texture(&part_vertices, texture);
		model->parts_vertices[i] = pwlayer_add(layer, &part_vertices);
		
		j = 0;
		while(j < num_group_end){
			pwlayer_end_group(layer);
			pwlayer_end_group(layer);
			++j;
			++k;
		}
	}
	
	//end any remaining group transforms
	if(k > model->num_parts){
		printf("pwmodel_load: too many group ends\n");
	}
	while(k < model->num_parts){
		pwlayer_end_group(layer);
		pwlayer_end_group(layer);
		++k;
	}
	
	pwrenderable_close(&part_entire);
	fclose(in);
	return 0;
	
pwmodel_error_deallocate:
	free(model->parts_vertices);
	free(model->transforms);
pwmodel_error_close:
	fclose(in);
	return -1;
}

void pwmodel_close(PWModel *model){
	free(model->parts_vertices);
	free(model->transforms);
}
