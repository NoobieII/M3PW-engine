//May 28, 2022
//Lithio

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "utilities/fsize.h"
#include "pwshader.h"

GLuint pwshader_init(PWShader *s, const char *vert_path, const char *frag_path){
	char *vert_src;
	char *frag_src;
	
	GLuint program;
	GLuint vertex;
	GLuint fragment;
	
	GLint result;
	GLint length;
	char *error;
	
	program = glCreateProgram();
	vertex = glCreateShader(GL_VERTEX_SHADER);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	
	vert_src = read_from_file(vert_path);
	if(!vert_src){
		printf("pwshader_init: error reading file %s\n", vert_path);
		return program;
	}
	frag_src = read_from_file(frag_path);
	if(!frag_src){
		printf("pwshader_init: error reading file %s\n", frag_path);
		return program;
	}
	
	
	glShaderSource(vertex, 1, (const char *const*)&vert_src, NULL);
	glCompileShader(vertex);
	free(vert_src);
	
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE){
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);
		error = (char*) malloc(length);
		glGetShaderInfoLog(vertex, length, &length, error);
		printf("Failed to compile vertex shader: %s\n", error);
		free(error);
		glDeleteShader(vertex);
		return 0;
	}
	
	glShaderSource(fragment, 1, (const char *const*)&frag_src, NULL);
	glCompileShader(fragment);
	free(frag_src);
	
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE){
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);
		error = (char*) malloc(length);
		glGetShaderInfoLog(fragment, length, &length, error);
		printf("Failed to compile fragment shader: %s\n", error);
		free(error);
		glDeleteShader(fragment);
		return 0;
	}
	
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	
	glLinkProgram(program);
	glValidateProgram(program);
	
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	
	s->program = program;
	return program;
}

inline void pwshader_enable(PWShader *s){
	glUseProgram(s->program);
}

inline void pwshader_disable(PWShader *s){
	glUseProgram(0);
}

inline void pwshader_close(PWShader *s){
	glDeleteProgram(s->program);
}

inline GLint pwshader_get_uniform_location(PWShader *s, const GLchar *name){
	return glGetUniformLocation(s->program, name);
}

GLint pwshader_get_attrib_location(PWShader *s, const GLchar *name){
	return glGetAttribLocation(s->program, name);
}

inline void pwshader_set_uniform_1f(PWShader *s, const GLchar *name, float value){
	glUniform1f(pwshader_get_uniform_location(s, name), value);
}

void pwshader_set_uniform_1fv(PWShader *s, const GLchar *name, float *value, int size){
	glUniform1fv(pwshader_get_uniform_location(s, name), size, value);
}

inline void pwshader_set_uniform_1i(PWShader *s, const GLchar *name, int value){
	glUniform1i(pwshader_get_uniform_location(s, name), value);
}

void pwshader_set_uniform_1iv(PWShader *s, const GLchar *name, int *value, int size){
	glUniform1iv(pwshader_get_uniform_location(s, name), size, value);
}

inline void pwshader_set_uniform_2f(PWShader *s, const GLchar *name, PWVec2 vec){
	glUniform2f(pwshader_get_uniform_location(s, name), vec.x, vec.y);
}

inline void pwshader_set_uniform_3f(PWShader *s, const GLchar *name, PWVec3 vec){
	glUniform3f(pwshader_get_uniform_location(s, name), vec.x, vec.y, vec.z);
}

inline void pwshader_set_uniform_4f(PWShader *s, const GLchar *name, PWVec4 vec){
	glUniform4f(pwshader_get_uniform_location(s, name), vec.x, vec.y, vec.z, vec.w);
}

inline void pwshader_set_uniform_mat4(PWShader *s, const GLchar *name, PWMat4 matrix){
	glUniformMatrix4fv(pwshader_get_uniform_location(s, name), 1, GL_FALSE, matrix.elements);
}

