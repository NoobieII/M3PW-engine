//Lithio (The developer's pseudonym)
//May 28, 2022

#ifndef M3PW_PWRENDERABLE2D
#define M3PW_PWRENDERABLE2D

#include "pwbuffers.h"
#include "pwmath.h"
#include "pwtexture.h"

typedef struct PWVertexData{
	PWVec3 position;
	PWVec2 uv;
	float tid;
	unsigned int color;
	PWVec3 normal;
} PWVertexData;

typedef struct PWRenderable{
	int index_count;
	unsigned short *indices;
	int vertex_count;
	PWVec3 *p; //vertex points
	PWVec2 *uv; //texture coordinates
	union {
		//unsigned int color;
		//unsigned char c[4];
		unsigned int *color;
		unsigned char *c;
	};
	float tid;
	PWTexture *texture; //reference to a texture
	char *font; 
	const char *str;    //reference to a string
	PWVec3 *n; //normals
} PWRenderable;

//initialize different kinds of 2d rectangular renderable objects
//note that color is in ABGR... so 0xffff00ff would be violet
void pwrenderable_init_rect(PWRenderable *r, PWVec3 position, PWVec2 size, unsigned int color);
void pwrenderable_init_texture(PWRenderable *r, PWVec3 position, PWVec2 size, unsigned int color, PWTexture *t);
void pwrenderable_init_str(PWRenderable *r, PWVec3 position, PWVec2 size, const char *str, const char *font, unsigned int color);
void pwrenderable_close(PWRenderable *r);

//for rect renderables
void pwrenderable_set_position_size(PWRenderable *r, PWVec3 position, PWVec2 size);
void pwrenderable_set_uv(PWRenderable *r, PWVec2 position, PWVec2 size);

//set color of all vertices
void pwrenderable_set_color(PWRenderable *r, unsigned int color);

void pwrenderable_set_texture(PWRenderable *r, PWTexture *t);

//to be implemented later (we will also be renaming everything without 2D)
void pwrenderable_init_cube(PWRenderable *r, float width, unsigned int color, PWTexture *texture);
void pwrenderable_init_box(PWRenderable *r, PWVec3 width, unsigned int color, PWTexture *texture);
//size.x is the diameter, size.y is the height, n is the number of sides
//prism doesnt render properly
void pwrenderable_init_prism(PWRenderable *r, PWVec3 position, PWVec2 size, int n);
void pwrenderable_init_pyramid(PWRenderable *r, PWVec3 position, PWVec2 size, int n);

//initialize empty renderable
void pwrenderable_init_none(PWRenderable *r);

//add an n-sided shape (normal facing the z direction)
void pwrenderable_add(PWRenderable *r, int n);

//add everything contained in the renderable src
void pwrenderable_add_r(PWRenderable *r, const PWRenderable *src);

void pwrenderable_get_vertex(PWRenderable *r, int n, PWVec3 *position, PWVec2 *uv, unsigned int *color, PWVec3 *normal);
//edit vertex n... bounds checking is done in the function
void pwrenderable_edit_vertex(PWRenderable *r, int n, PWVec3 position, PWVec2 uv, unsigned int color, PWVec3 normal);

//remove all vertices, but don't close the renderable
void pwrenderable_reset(PWRenderable *r);

//save the renderable to a file, returns 0 if successful, -1 if not
//if recalculating normals (flat faces), send nonzero value
int pwrenderable_save(PWRenderable *r, const char *filename, int recalculate_normals);

//load a renderable to a file, returns 0 if successful, -1 if not
int pwrenderable_load(PWRenderable *r, const char *filename);

unsigned int PW_COLOR(float r, float g, float b, float a);

#endif

