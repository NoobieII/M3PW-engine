//Lithio (The developer's pseudonym)
//May 28, 2022

#ifndef M3PW_PWRENDERER2D
#define M3PW_PWRENDERER2D

#include <stddef.h> //for offsetof()
#include "pwrenderable.h"
#include "pwshader.h"
#include "pwtext_loader.h"
#include "utilities/hashtable.h"

/*
#define PWRENDERER2D_MAX_SPRITES  10000
#define PWRENDERER2D_VERTEX_SIZE  sizeof(PWVertexData)
#define PWRENDERER2D_SPRITE_SIZE  (PWRENDERER2D_VERTEX_SIZE * 4)
#define PWRENDERER2D_BUFFER_SIZE  (PWRENDERER2D_SPRITE_SIZE * PWRENDERER2D_MAX_SPRITES)
#define PWRENDERER2D_INDICES_SIZE (PWRENDERER2D_MAX_SPRITES * 6)
*/

#define PWRENDERER_MAX_INDICES 60000
#define PWRENDERER_VERTEX_SIZE sizeof(PWVertexData)
#define PWRENDERER_BUFFER_SIZE (PWRENDERER_MAX_INDICES * PWRENDERER_VERTEX_SIZE)

#define PWRENDERER_MAX_TEXTURE_SLOTS 32

#define SHADER_VERTEX_INDEX 0
#define SHADER_UV_INDEX     1
#define SHADER_TID_INDEX    2
#define SHADER_COLOR_INDEX  3


typedef struct PWRenderer{
	GLuint vao;
	//PWIndexBuffer ibo;
	GLuint ibo;
	GLuint vbo;
	GLsizei index_count;
	GLsizei vertex_count;
	GLushort *index_data;
	PWVertexData *vertex_data;
	
	int stack_len;
	int stack_size;
	PWMat4 *transformation_stack;
	
	int texture_slots_len;
	GLuint texture_slots[PWRENDERER_MAX_TEXTURE_SLOTS];
	
	PWTextLoader text_loader;
} PWRenderer;


void pwrenderer_init(PWRenderer *renderer, PWShader *s);
void pwrenderer_close(PWRenderer *renderer);

//send 0 to not overwrite, any other value to overwrite
void pwrenderer_push(PWRenderer *renderer, PWMat4 matrix, int overwrite);
void pwrenderer_pop(PWRenderer *renderer);
void pwrenderer_submit(PWRenderer *renderer, PWRenderable *renderable);
void pwrenderer_submit_str(PWRenderer *renderer, PWRenderable *renderable);
void pwrenderer_flush(PWRenderer *renderer);


void pwrenderer_begin(PWRenderer *renderer);
void pwrenderer_end(PWRenderer *renderer);


#endif

