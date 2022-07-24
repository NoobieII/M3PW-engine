//May 28, 2022
//Lithio (The developer's pseudonym)

//Shader object for the PWEngine

#ifndef M3PW_PWSHADER_H
#define M3PW_PWSHADER_H

#include "pwmath.h"

typedef struct PWShader{
	GLuint program;
} PWShader;


GLuint pwshader_init(PWShader *s, const char *vert_path, const char *frag_path);

void pwshader_enable(PWShader *s);
void pwshader_disable(PWShader *s);
void pwshader_close(PWShader *s);

GLint pwshader_get_uniform_location(PWShader *s, const GLchar *name);
GLint pwshader_get_attrib_location(PWShader *s, const GLchar *name);

void pwshader_set_uniform_1f(PWShader *s, const GLchar *name, float value);
void pwshader_set_uniform_1fv(PWShader *s, const GLchar *name, float *value, int size);
void pwshader_set_uniform_1i(PWShader *s, const GLchar *name, int value);
void pwshader_set_uniform_1iv(PWShader *s, const GLchar *name, int *value, int size);
void pwshader_set_uniform_2f(PWShader *s, const GLchar *name, PWVec2 vec);
void pwshader_set_uniform_3f(PWShader *s, const GLchar *name, PWVec3 vec);
void pwshader_set_uniform_4f(PWShader *s, const GLchar *name, PWVec4 vec);
void pwshader_set_uniform_mat4(PWShader *s, const GLchar *name, PWMat4 matrix);

#endif


