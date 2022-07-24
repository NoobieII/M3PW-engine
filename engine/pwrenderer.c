//Lithio (The developer's pseudonym)
//May 28, 2022

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "pwrenderer.h"

void pwrenderer_init(PWRenderer *renderer, PWShader *s){
	GLushort *indices;
	GLushort i;
	GLuint position_location;
	GLuint uv_location;
	GLuint tid_location;
	GLuint color_location;
	GLuint normal_location;
	
	TTF_Font *font;
	int font_skip;
	float loaded_size;
	int glyph_map_size = 1024;
	SDL_Surface *glyph;
	SDL_Surface *glyph_map;
	int advance;
	SDL_Color color = {255, 255, 255};
	char cc[2] = {0, 0};
	SDL_Rect rect;
	
	//init stuff
	renderer->index_count = 0;
	
	pwshader_enable(s);
	glGenVertexArrays(1, &renderer->vao);
	glGenBuffers(1, &renderer->vbo);
	
	glBindVertexArray(renderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferData(GL_ARRAY_BUFFER, PWRENDERER_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
	
	position_location = pwshader_get_attrib_location(s, "position");
	uv_location = pwshader_get_attrib_location(s, "uv");
	tid_location = pwshader_get_attrib_location(s, "tid");
	color_location = pwshader_get_attrib_location(s, "colour");
	normal_location = pwshader_get_attrib_location(s, "normal");
	glEnableVertexAttribArray(position_location);
	glEnableVertexAttribArray(uv_location);
	glEnableVertexAttribArray(tid_location);
	glEnableVertexAttribArray(color_location);
	glEnableVertexAttribArray(normal_location);
	
	glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, PWRENDERER_VERTEX_SIZE, (const GLvoid*) 0);
	glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, PWRENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(PWVertexData, uv)));
	glVertexAttribPointer(tid_location, 1, GL_FLOAT, GL_FALSE, PWRENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(PWVertexData, tid)));
	glVertexAttribPointer(color_location, 4, GL_UNSIGNED_BYTE, GL_TRUE, PWRENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(PWVertexData, color)));
	glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, PWRENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(PWVertexData, normal)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	//index buffer
	glGenBuffers(1, &renderer->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, PWRENDERER_MAX_INDICES * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glBindVertexArray(0);
	
	//transformation stack - always have an identity matrix at the start
	renderer->stack_len = 0;
	renderer->stack_size = 16;
	renderer->transformation_stack = (PWMat4*) malloc(sizeof(PWMat4) * 16);
	renderer->transformation_stack[renderer->stack_len++] = PWM_ini(1.0, NULL);
	
	renderer->texture_slots_len = 0;
	
	pwtextloader_init(&renderer->text_loader);
}

void pwrenderer_push(PWRenderer *renderer, PWMat4 matrix, int overwrite){
	PWMat4 *tmp;
	
	//check allocation
	if(renderer->stack_len >= renderer->stack_size){
		renderer->stack_size += 16;
		tmp = realloc(renderer->transformation_stack, renderer->stack_size * sizeof(PWMat4));
		if(!tmp){
			return;
		}
		else {
			renderer->transformation_stack = tmp;
		}
	}
	
	if(!overwrite){
		PWM_mul_ref(&renderer->transformation_stack[renderer->stack_len],
		            &renderer->transformation_stack[renderer->stack_len - 1],
		            &matrix);
		++renderer->stack_len;
	}
	else {
		renderer->transformation_stack[renderer->stack_len++] = matrix;
	}
}

void pwrenderer_pop(PWRenderer *renderer){
	if(renderer->stack_len == 1){
		return;
	}
	--renderer->stack_len;
}

void pwrenderer_submit(PWRenderer *renderer, PWRenderable *renderable){
	float ts;
	float tid;
	int found;
	int i;
	GLushort *index_data;
	PWVertexData *vertex_data;
	int max_textures;
	
	if(renderable->vertex_count == 0){
		return;
	}
	
	//skip if the renderable is transparent
	if(renderable->c[3] == 0x00){
		return;
	}
	
	if(renderer->index_count + renderable->index_count >= PWRENDERER_MAX_INDICES){
		pwrenderer_end(renderer);
		pwrenderer_flush(renderer);
		pwrenderer_begin(renderer);
	}
	
	tid = renderable->tid;
	ts = 0.0;
	
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_textures);
	if(max_textures > 32){
		max_textures = 32;
	}
	
	//if texture id is nonzero, then a texture is to be displayed
	if(tid > 0){
		found = 0;
		for(i = 0; i < renderer->texture_slots_len; ++i){
			if(renderer->texture_slots[i] == tid){
				ts = (float)i + 1;
				found = 1;
				break;
			}
		}
		//flush the renderer if there are already 32 textures allocated
		if(!found){
			if(renderer->texture_slots_len >= max_textures/*PWRENDERER2D_MAX_TEXTURE_SLOTS*/){
				pwrenderer_end(renderer);
				pwrenderer_flush(renderer);
				pwrenderer_begin(renderer);
			}
			renderer->texture_slots[renderer->texture_slots_len++] = tid;
			ts = (float)(renderer->texture_slots_len);
		}
	}
	
	
	index_data = renderer->index_data;
	vertex_data = renderer->vertex_data;
	
	//change the position based on transformation stack matrix
	//matrix multiplication is done by reference for less data copying
	for(i = 0; i < renderable->vertex_count; ++i){
		PWM_mul_vec3_ref(&vertex_data->position, &renderer->transformation_stack[renderer->stack_len - 1], &renderable->p[i]);
		//printf("vertex %d = (%f, %f, %f)\n", i, vertex_data->position.x, vertex_data->position.y, vertex_data->position.z);
		vertex_data->uv = renderable->uv[i];
		vertex_data->tid = ts;
		vertex_data->color = renderable->color[i];
		vertex_data->normal = renderable->n[i];
		//PWM_mul_vec3_notranslate_ref(&vertex_data->normal, &renderer->transformation_stack[renderer->stack_len - 1], &renderable->n[i]);
		vertex_data++;
	}
	renderer->vertex_data = vertex_data;
	
	for(i = 0; i < renderable->index_count; ++i){
		*(index_data++) = renderable->indices[i] + renderer->vertex_count;
	}
	renderer->index_data = index_data;
	
	//square requires 6 vertex indices
	renderer->vertex_count += renderable->vertex_count;
	renderer->index_count += renderable->index_count;
}

void pwrenderer_submit_str(PWRenderer *renderer, PWRenderable *renderable){
	GLuint font_tid;
	PWRenderable glyph;
	PWVec3 position = renderable->p[0];
	PWTexture *font_texture;
	float *text_advance;		//text advance for each glyph
	PWVec2 size;
	PWVec2 glyph_size;          //texture coordinate and size
	PWVec2 glyph_pos;
	const char *str = renderable->str;
	int i = 0;
	float left;
	float right;
	float top;
	float bottom;
	
	if(renderable->c[3] == 0x00){
		return;
	}
	
	pwtextloader_load(&renderer->text_loader, renderable->font, &font_texture, &text_advance);
	if(!font_texture){
		printf("pwrenderer_submit_str: couldn't load font %s\n", renderable->font);
		return;
	}
	
	//now that a texture is loaded
	font_tid = font_texture->texture_id;
	
	//break each string character glyph into a renderable object and submit it
	size.y = renderable->p[2].y - renderable->p[0].y;
	glyph_size.y = 1.0/16;
	while(str[i]){
		size.x = 16.0 * size.y * text_advance[str[i]];
		if(position.x + size.x >= renderable->p[2].x){
			position.x = renderable->p[0].x;
			position.y -= size.y;
		}
		pwrenderable_init_texture(&glyph, position, size, renderable->color[0], font_texture);
		//printf("renderable->color = %x\n", renderable->color[0]);
		glyph_pos.x = (str[i] & 0x0f) / 16.0f;
		glyph_pos.y = (str[i] & ~0x0f) / 256.0f;
		glyph_size.x = text_advance[str[i]];
		pwrenderable_set_uv(&glyph, glyph_pos, glyph_size);
		
		pwrenderer_submit(renderer, &glyph);
		pwrenderable_close(&glyph);
		
		//update position
		position.x += size.x;
		++i;
	}
}

void pwrenderer_begin(PWRenderer *renderer){
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	renderer->vertex_data = (PWVertexData*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	renderer->index_data = (GLushort*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}
	

void pwrenderer_flush(PWRenderer *renderer){
	int i;
	
	for(i = 0; i < renderer->texture_slots_len; ++i){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, renderer->texture_slots[i]);
	}
	glBindVertexArray(renderer->vao);
	//pwindexbuffer_bind(&renderer->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	
	//enable transparency, depth buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//enable face culling
	glEnable(GL_CULL_FACE);
	
	glDrawElements(GL_TRIANGLES, renderer->index_count, GL_UNSIGNED_SHORT, NULL);
	
	glDisable(GL_CULL_FACE);
	
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	
	//pwindexbuffer_unbind(&renderer->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	renderer->vertex_count = 0;
	renderer->index_count = 0;
	renderer->index_data = NULL;
	renderer->vertex_data = NULL;
	renderer->texture_slots_len = 0;
}

void pwrenderer_end(PWRenderer *renderer){
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void pwrenderer_close(PWRenderer *renderer){
	float *advance = NULL;
	PWTexture *font_map = NULL;
	
	//pwindexbuffer_close(&renderer->ibo);
	glDeleteBuffers(1, &renderer->ibo);
	glDeleteBuffers(1, &renderer->vbo);
	glDeleteBuffers(1, &renderer->vao);
	if(renderer->transformation_stack){
		free(renderer->transformation_stack);
	}
	
	pwtextloader_close(&renderer->text_loader);
}


