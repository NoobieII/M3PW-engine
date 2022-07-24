//Lithio (The developer's pseudonym)
//June 01, 2022

#ifndef M3PW_PWTEXTURE_H
#define M3PW_PWTEXTURE_H

#include <GL/glew.h>
#include <GL/gl.h>

typedef struct PWTexture {
	char *path;
	GLuint texture_id;
	GLsizei width;
	GLsizei height;
} PWTexture;

void pwtexture_init(PWTexture *t, const char *path);
void pwtexture_init_pixels(PWTexture *t, int width, int height, unsigned char *pixels);
void pwtexture_close(PWTexture *t);
void pwtexture_bind(PWTexture *t);
void pwtexture_unbind(PWTexture *t);

#endif

