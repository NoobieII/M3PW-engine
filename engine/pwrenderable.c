//Lithio (The developer's pseudonym)
//May 28, 2022

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "pwrenderable.h"

void pwrenderable_init_rect(PWRenderable *r, PWVec3 position, PWVec2 size, unsigned int color){
	r->index_count = 6;
	r->indices = (unsigned short*) malloc(sizeof(unsigned short) * 6);
	r->indices[0] = 0;
	r->indices[1] = 1;
	r->indices[2] = 2;
	r->indices[3] = 0;
	r->indices[4] = 2;
	r->indices[5] = 3;
	r->vertex_count = 4;
	r->p = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->p[0] = position;
	position.x += size.x;
	r->p[1] = position;
	position.y += size.y;
	r->p[2] = position;
	position.x -= size.x;
	r->p[3] = position;
	r->uv = (PWVec2*) malloc(sizeof(PWVec2) * r->vertex_count);
	r->color = (unsigned int*) malloc(sizeof(unsigned int) * r->vertex_count);
	r->color[0] = color;
	r->color[1] = color;
	r->color[2] = color;
	r->color[3] = color;
	r->texture = NULL;
	r->tid = 0.0;
	r->str = NULL;
	r->font = NULL;
	r->n = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->n[0] = PWM_vec3(0, 0, 1);
	r->n[1] = PWM_vec3(0, 0, 1);
	r->n[2] = PWM_vec3(0, 0, 1);
	r->n[3] = PWM_vec3(0, 0, 1);
}

void pwrenderable_init_texture(PWRenderable *r, PWVec3 position, PWVec2 size, unsigned int color, PWTexture *t){
	pwrenderable_init_rect(r, position, size, color);
	r->uv[0] = PWM_vec2(0, 1);
	r->uv[1] = PWM_vec2(1, 1);
	r->uv[2] = PWM_vec2(1, 0);
	r->uv[3] = PWM_vec2(0, 0);
	r->texture = t;
	r->tid = (float)(r->texture->texture_id);
}

void pwrenderable_init_str(PWRenderable *r, PWVec3 position, PWVec2 size, const char *str, const char *font, unsigned int color){
	pwrenderable_init_rect(r, position, size, color);
	if(!font){
		return;
	}
	r->font = (char*) malloc(strlen(font) + 1);
	strcpy(r->font, font);
	r->str = str;
}

void pwrenderable_close(PWRenderable *r){
	if(r->indices){
		free(r->indices);
	}
	if(r->p){
		free(r->p);
	}
	if(r->uv){
		free(r->uv);
	}
	if(r->color){
		free(r->color);
	}
	if(r->n){
		free(r->n);
	}
	if(r->font){
		free(r->font);
	}
}

inline void pwrenderable_set_position_size(PWRenderable *r, PWVec3 position, PWVec2 size){
	r->p[0] = position;
	position.x += size.x;
	r->p[1] = position;
	position.y += size.y;
	r->p[2] = position;
	position.x -= size.x;
	r->p[3] = position;
}

inline void pwrenderable_set_uv(PWRenderable *r, PWVec2 position, PWVec2 size){
	r->uv[3] = position;
	position.y += size.y;
	r->uv[0] = position;
	position.x += size.x;
	r->uv[1] = position;
	position.y -= size.y;
	r->uv[2] = position;
}

void pwrenderable_set_color(PWRenderable *r, unsigned int color){
	int i;
	for(i = 0; i < r->vertex_count; ++i){
		r->color[i] = color;
	}
}

void pwrenderable_set_texture(PWRenderable *r, PWTexture *t){
	r->texture = t;
	r->tid = (float)(r->texture->texture_id);
}

inline unsigned int PW_COLOR(float r, float g, float b, float a){
	int color;
	char *c = (char*)(&color);
	c[0] = r * 255.0f;
	c[1] = g * 255.0f;
	c[2] = b * 255.0f;
	c[3] = a * 255.0f;
	return color;
}

void pwrenderable_init_prism(PWRenderable *r, PWVec3 position, PWVec2 size, int n){
	int i;
	int v = 0;
	PWVec3 *p;
	unsigned short *indices;
	
	if(n < 3 || n > 16){
		r->index_count = 0;
		r->indices = NULL;
		r->vertex_count = 0;
		r->p = NULL;
		r->uv = NULL;
		r->font = NULL;
		return;
	}
	
	//indices
	r->index_count = 3*(n-2 + n*2 + n-2);
	r->indices = (unsigned short*) malloc(sizeof(unsigned short) * r->index_count);
	indices = r->indices;
	
	//bottom face
	for(i = 0; i < n - 2; ++i){
		indices[i*3 + 0] = 0;
		indices[i*3 + 1] = i + 2;
		indices[i*3 + 2] = i + 1;
	}
	v += n;
	indices += 3*(n-2);
	
	//side faces
	for(i = 0; i < n; ++i){
		indices[i*6 + 0] = v;
		indices[i*6 + 1] = v + 1;
		indices[i*6 + 2] = v + 2;
		indices[i*6 + 3] = v;
		indices[i*6 + 4] = v + 2;
		indices[i*6 + 5] = v + 3;
		v += 4;
	}
	
	indices += 6*n;
	
	//top face
	for(i = 0; i < n - 2; ++i){
		indices[i*3 + 0] = v;
		indices[i*3 + 1] = v + i + 1;
		indices[i*3 + 2] = v + i + 2;
	}
	
	//vertices
	r->vertex_count = 2*n + 4*n;
	r->p = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	p = r->p;
	
	//bottom face
	for(i = 0; i < n; ++i){
		p[i].x = (float) cos(i * 2.0f * PI / n) * size.x + position.x;
		p[i].y = 0.0f + position.y;
		p[i].z = (float) sin(i * 2.0f * PI / n) * size.x + position.z;
	}
	
	//top face
	for(i = 0; i < n; ++i){
		p[5*n + i] = p[i];
		p[5*n + i].y += size.y;
	}
	
	//side faces
	for(i = 0; i < n; ++i){
		p[n + i*4 + 0] = p[i];
		p[n + i*4 + 1] = p[(i+1) % n];
		p[n + i*4 + 2] = p[5*n + (i+1) % n];
		p[n + i*4 + 3] = p[5*n + i];
	}
	
	
	
	//normals
	r->n = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	p = r->n;
	
	//bottom face
	for(i = 0; i < n; ++i){
		*(p++) = PWM_vec3(0, -1, 0);
	}
	
	//side faces
	for(i = 0; i < n; ++i){
		p[0] = PWM_vec3(cos((((float)i)+0.5) * 2.0f * PI / n), 0, sin((((float)i)+0.5) * 2.0f * PI / n));
		p[1] = p[0];
		p[2] = p[0];
		p[3] = p[0];
		p += 4;
	}
	
	//top face
	for(i = 0; i < n; ++i){
		*(p++) = PWM_vec3(0, 1, 0);
	}
	
	r->uv = (PWVec2*) malloc(sizeof(PWVec2) * r->vertex_count);
	
	r->color = (unsigned int*) malloc(sizeof(unsigned int) * r->vertex_count);
	for(i = 0; i < r->vertex_count; ++i){
		r->color[i] = 0xffffffff;
	}
	
	r->texture = NULL;
	r->tid = 0.0;
	r->str = NULL;
	r->font = NULL;
}

void pwrenderable_init_cube(PWRenderable *r, float width, unsigned int color, PWTexture *texture){
	int i;
	float l = width / 2.0f;
	
	r->index_count = 36;
	r->indices = (unsigned short*) malloc(sizeof(unsigned short) * r->index_count);
	for(i = 0; i < 6; ++i){
		r->indices[i*6 + 0] = i*4 + 0;
		r->indices[i*6 + 1] = i*4 + 1;
		r->indices[i*6 + 2] = i*4 + 2;
		r->indices[i*6 + 3] = i*4 + 0;
		r->indices[i*6 + 4] = i*4 + 2;
		r->indices[i*6 + 5] = i*4 + 3;
	}
	
	r->vertex_count = 24;
	r->p = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->p[0] = PWM_vec3(-l, -l,  l);
	r->p[1] = PWM_vec3( l, -l,  l);
	r->p[2] = PWM_vec3( l,  l,  l);
	r->p[3] = PWM_vec3(-l,  l,  l);
	r->p[4] = PWM_vec3(-l, -l, -l);
	r->p[5] = PWM_vec3( l, -l, -l);
	r->p[6] = PWM_vec3( l, -l,  l);
	r->p[7] = PWM_vec3(-l, -l,  l);
	r->p[8] = PWM_vec3(-l,  l,  l);
	r->p[9] = PWM_vec3( l,  l,  l);
	r->p[10] = PWM_vec3( l,  l, -l);
	r->p[11] = PWM_vec3(-l,  l, -l);
	r->p[12] = PWM_vec3( l, -l,  l);
	r->p[13] = PWM_vec3( l, -l, -l);
	r->p[14] = PWM_vec3( l,  l, -l);
	r->p[15] = PWM_vec3( l,  l,  l);
	r->p[16] = PWM_vec3(-l, -l, -l);
	r->p[17] = PWM_vec3(-l, -l,  l);
	r->p[18] = PWM_vec3(-l,  l,  l);
	r->p[19] = PWM_vec3(-l,  l, -l);
	r->p[20] = PWM_vec3( l, -l, -l);
	r->p[21] = PWM_vec3(-l, -l, -l);
	r->p[22] = PWM_vec3(-l,  l, -l);
	r->p[23] = PWM_vec3( l,  l, -l);
	
	/*
	for(i = 0; i < 24; ++i){
		printf("vertex %d = (%f, %f, %f)\n", i, r->p[i].x, r->p[i].y, r->p[i].z);
	}
	*/
	
	r->uv = (PWVec2*) malloc(sizeof(PWVec3) * r->vertex_count);
	
	r->color = (unsigned int*) malloc(sizeof(unsigned int) * r->vertex_count);
	for(i = 0; i < r->vertex_count; ++i){
		r->color[i] = color;
	}
	
	r->tid = 0.0;
	r->texture = NULL;
	r->font = NULL;
	r->str = NULL;
	
	r->n = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->n[0] = PWM_vec3(0, 0, 1);
	r->n[4] = PWM_mul_vec3(PWM_rotation(90, PWM_vec3(1, 0, 0)), r->n[0]);
	r->n[8] = PWM_mul_vec3(PWM_rotation(-90, PWM_vec3(1, 0, 0)), r->n[0]);
	r->n[12] = PWM_mul_vec3(PWM_rotation(90, PWM_vec3(0, 1, 0)), r->n[0]);
	r->n[16] = PWM_mul_vec3(PWM_rotation(-90, PWM_vec3(0, 1, 0)), r->n[0]);
	r->n[20] = PWM_mul_vec3(PWM_rotation(180, PWM_vec3(0, 1, 0)), r->n[0]);
	
	for(i = 0; i < 6; ++i){
		r->n[i*4 + 1] = r->n[i*4];
		r->n[i*4 + 2] = r->n[i*4];
		r->n[i*4 + 3] = r->n[i*4];
	}
}

void pwrenderable_init_box(PWRenderable *r, PWVec3 width, unsigned int color, PWTexture *texture){
	int i;
	float x = width.x / 2;
	float y = width.y / 2;
	float z = width.z / 2;
	
	r->index_count = 36;
	r->indices = (unsigned short*) malloc(sizeof(unsigned short) * r->index_count);
	for(i = 0; i < 6; ++i){
		r->indices[i*6 + 0] = i*4 + 0;
		r->indices[i*6 + 1] = i*4 + 1;
		r->indices[i*6 + 2] = i*4 + 2;
		r->indices[i*6 + 3] = i*4 + 0;
		r->indices[i*6 + 4] = i*4 + 2;
		r->indices[i*6 + 5] = i*4 + 3;
	}
	
	r->vertex_count = 24;
	r->p = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->p[0] = PWM_vec3(-x, -y,  z);
	r->p[1] = PWM_vec3( x, -y,  z);
	r->p[2] = PWM_vec3( x,  y,  z);
	r->p[3] = PWM_vec3(-x,  y,  z);
	r->p[4] = PWM_vec3(-x, -y, -z);
	r->p[5] = PWM_vec3( x, -y, -z);
	r->p[6] = PWM_vec3( x, -y,  z);
	r->p[7] = PWM_vec3(-x, -y,  z);
	r->p[8] = PWM_vec3(-x,  y,  z);
	r->p[9] = PWM_vec3( x,  y,  z);
	r->p[10] = PWM_vec3( x,  y, -z);
	r->p[11] = PWM_vec3(-x,  y, -z);
	r->p[12] = PWM_vec3( x, -y,  z);
	r->p[13] = PWM_vec3( x, -y, -z);
	r->p[14] = PWM_vec3( x,  y, -z);
	r->p[15] = PWM_vec3( x,  y,  z);
	r->p[16] = PWM_vec3(-x, -y, -z);
	r->p[17] = PWM_vec3(-x, -y,  z);
	r->p[18] = PWM_vec3(-x,  y,  z);
	r->p[19] = PWM_vec3(-x,  y, -z);
	r->p[20] = PWM_vec3( x, -y, -z);
	r->p[21] = PWM_vec3(-x, -y, -z);
	r->p[22] = PWM_vec3(-x,  y, -z);
	r->p[23] = PWM_vec3( x,  y, -z);
	
	r->uv = (PWVec2*) malloc(sizeof(PWVec3) * r->vertex_count);
	
	r->color = (unsigned int*) malloc(sizeof(unsigned int) * r->vertex_count);
	for(i = 0; i < r->vertex_count; ++i){
		r->color[i] = color;
	}
	
	r->tid = 0.0;
	r->texture = NULL;
	r->font = NULL;
	r->str = NULL;
	
	r->n = (PWVec3*) malloc(sizeof(PWVec3) * r->vertex_count);
	r->n[0] = PWM_vec3(0, 0, 1);
	r->n[4] = PWM_mul_vec3(PWM_rotation(90, PWM_vec3(1, 0, 0)), r->n[0]);
	r->n[8] = PWM_mul_vec3(PWM_rotation(-90, PWM_vec3(1, 0, 0)), r->n[0]);
	r->n[12] = PWM_mul_vec3(PWM_rotation(90, PWM_vec3(0, 1, 0)), r->n[0]);
	r->n[16] = PWM_mul_vec3(PWM_rotation(-90, PWM_vec3(0, 1, 0)), r->n[0]);
	r->n[20] = PWM_mul_vec3(PWM_rotation(180, PWM_vec3(0, 1, 0)), r->n[0]);
	
	for(i = 0; i < 6; ++i){
		r->n[i*4 + 1] = r->n[i*4];
		r->n[i*4 + 2] = r->n[i*4];
		r->n[i*4 + 3] = r->n[i*4];
	}
}

//initialize empty renderable
void pwrenderable_init_none(PWRenderable *r){
	r->index_count = 0;
	r->indices = NULL;
	r->vertex_count = 0;
	r->p = NULL; //vertex points
	r->uv = NULL; //texture coordinates
	r->color = NULL;
	r->tid = 0;
	r->texture = NULL; //reference to a texture
	r->font = NULL; 
	r->str = NULL;    //reference to a string
	r->n = NULL; //normals
}

//add an n-sided shape
void pwrenderable_add(PWRenderable *r, int n){
	int i;
	
	//shape must have 3 to 8 sides/corners
	if(n < 3 || n > 8){
		return;
	}
	
	r->indices = (unsigned short*) realloc(r->indices, sizeof(unsigned short) * (r->index_count + (n - 2) * 3));
	for(i = 0; i < n - 2; ++i){
		r->indices[r->index_count + i*3 + 0] = r->vertex_count;
		r->indices[r->index_count + i*3 + 1] = r->vertex_count + i + 1;
		r->indices[r->index_count + i*3 + 2] = r->vertex_count + i + 2;
	}
	r->index_count += (n - 2) * 3;
	
	r->p = (PWVec3*) realloc(r->p, sizeof(PWVec3) * (r->vertex_count + n));
	for(i = 0; i < n; ++i){
		r->p[r->vertex_count + i] = PWM_vec3(cos(i * 2 * PI / n), sin(i * 2 * PI / n), 0);
	}
	r->uv = (PWVec2*) realloc(r->uv, sizeof(PWVec2) * (r->vertex_count + n));
	switch(n){
	case 3:
		r->uv[r->vertex_count + 0] = PWM_vec2(0, 1);
		r->uv[r->vertex_count + 1] = PWM_vec2(1, 1);
		r->uv[r->vertex_count + 2] = PWM_vec2(0, 0.5);
		break;
	case 4:
		r->uv[r->vertex_count + 0] = PWM_vec2(0, 1);
		r->uv[r->vertex_count + 1] = PWM_vec2(1, 1);
		r->uv[r->vertex_count + 2] = PWM_vec2(1, 0);
		r->uv[r->vertex_count + 3] = PWM_vec2(0, 0);
		break;
	case 6:
		r->uv[r->vertex_count + 0] = PWM_vec2(0.25, 1);
		r->uv[r->vertex_count + 1] = PWM_vec2(0.75, 1);
		r->uv[r->vertex_count + 2] = PWM_vec2(1, 0.5);
		r->uv[r->vertex_count + 3] = PWM_vec2(0.75, 0);
		r->uv[r->vertex_count + 4] = PWM_vec2(0.25, 0);
		r->uv[r->vertex_count + 5] = PWM_vec2(0, 0.5);
		break;
	case 8:
		r->uv[r->vertex_count + 0] = PWM_vec2(0.28125, 1);
		r->uv[r->vertex_count + 1] = PWM_vec2(0.71875, 1);
		r->uv[r->vertex_count + 2] = PWM_vec2(1, 0.71875);
		r->uv[r->vertex_count + 3] = PWM_vec2(1, 0.28125);
		r->uv[r->vertex_count + 4] = PWM_vec2(0.71875, 0);
		r->uv[r->vertex_count + 5] = PWM_vec2(0.28125, 0);
		r->uv[r->vertex_count + 6] = PWM_vec2(0, 0.28125);
		r->uv[r->vertex_count + 7] = PWM_vec2(0, 0.71875);
		break;
	default:
		for(i = 0; i < n; ++i){
			r->uv[r->vertex_count + i] = PWM_vec2(0,0);
		}
		break;
	}
	r->color = (unsigned int *) realloc(r->color, sizeof(unsigned int) * (r->vertex_count + n));
	for(i = 0; i < n; ++i){
		r->color[r->vertex_count + i] = 0xffffffff;
	}
	r->n = (PWVec3*) realloc(r->n, sizeof(PWVec3) * (r->vertex_count + n));
	for(i = 0; i < n; ++i){
		r->n[r->vertex_count + i] = PWM_vec3(0,0,1);
	}
	r->vertex_count += n;
}
	

void pwrenderable_get_vertex(PWRenderable *r, int n, PWVec3 *position, PWVec2 *uv, unsigned int *color, PWVec3 *normal){
	if(n < 0 || n >= r->vertex_count){
		return;
	}
	if(position){
		*position = r->p[n];
	}
	if(uv){
		*uv = r->uv[n];
	}
	if(color){
		*color = r->color[n];
	}
	if(normal){
		*normal = r->n[n];
	}
}
		
//edit vertex n... bounds checking is done in the function
void pwrenderable_edit_vertex(PWRenderable *r, int n, PWVec3 position, PWVec2 uv, unsigned int color, PWVec3 normal){
	if(n < 0 || n >= r->vertex_count){
		return;
	}
	r->p[n] = position;
	r->uv[n] = uv;
	r->color[n] = color;
	r->n[n] = normal;
}

int pwrenderable_save(PWRenderable *r, const char *filename, int recalculate_normals){
	PWVec3 position;
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal, v0, v1, v2;
	FILE *out;
	int i;
	
	//don't actually count the vertices, but check the shapes
	int vertex = 0;
	int index = 0;
	int polygon_size = 2;
	
	out = fopen(filename, "wt");
	if(!out){
		return -1;
	}
	
	while(index < r->index_count){
		//it's part of the shape
		if(r->indices[index] == vertex){
			index += 3;
			polygon_size++;
		}
		else{
			//save the shape to file
			//number of sides
			fprintf(out, "%d\n", polygon_size);
			
			if(recalculate_normals){
				v0 = r->p[vertex];
				v1 = r->p[vertex + 1];
				v2 = r->p[vertex + 2];
				normal = PWM_normalize3(PWM_cross3(PWM_sub3(v1, v0), PWM_sub3(v2, v0)));
			}
			
			for(i = 0; i < polygon_size; ++i){
				position = r->p[vertex + i];
				uv = r->uv[vertex + i];
				color = r->color[vertex + i];
				if(!recalculate_normals){
					normal = r->n[vertex + i];
				}
				fprintf(out, "position %f %f %f uv %f %f color %x normal %f %f %f\n", position.x, position.y, position.z, uv.x, uv.y, color, normal.x, normal.y, normal.z);
			}
			fprintf(out, "\n");
			
			vertex += polygon_size;
			polygon_size = 2;
		}
	}
	//save the last shape to file
	fprintf(out, "%d\n", polygon_size);
	
	//calculate normal
	v0 = r->p[vertex];
	v1 = r->p[vertex + 1];
	v2 = r->p[vertex + 2];
	normal = PWM_normalize3(PWM_cross3(PWM_sub3(v1, v0), PWM_sub3(v2, v0)));
	for(i = 0; i < polygon_size; ++i){
		position = r->p[vertex + i];
		uv = r->uv[vertex + i];
		color = r->color[vertex + i];
		fprintf(out, "position %f %f %f uv %f %f color %x normal %f %f %f\n", position.x, position.y, position.z, uv.x, uv.y, color, normal.x, normal.y, normal.z);
	}
	fprintf(out, "\n");
	
	fclose(out);
	return 0;
}

int pwrenderable_load(PWRenderable *r, const char *filename){
	PWVec3 position;
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal;
	FILE *in;
	int i;
	
	//don't actually count the vertices, but check the shapes
	int vertex = 0;
	int polygon_size = 0;
	
	in = fopen(filename, "rt");
	if(!in){
		return -1;
	}
	
	pwrenderable_close(r);
	pwrenderable_init_none(r);
	
	while(1){
		fscanf(in, "%d", &polygon_size);
		if(feof(in)){
			break;
		}
		pwrenderable_add(r, polygon_size);
		
		//read the vertices
		for(i = 0; i < polygon_size; ++i){
			fscanf(in, "%*s %f%f%f %*s %f%f %*s %x %*s %f%f%f", &position.x, &position.y, &position.z, &uv.x, &uv.y, &color, &normal.x, &normal.y, &normal.z);
			r->p[vertex + i] = position;
			r->uv[vertex + i] = uv;
			r->color[vertex + i] = color;
			r->n[vertex + i] = normal;
		}
		vertex += i;
	}
	
	fclose(in);
	return 0;
}
