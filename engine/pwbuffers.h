//Lithio (The developer's pseudonym)
//May 28, 2022

#ifndef M3PW_PWBUFFERS_H
#define M3PW_PWBUFFERS_H

#include <GL/glew.h>

typedef struct PWVBuffer     PWVBuffer;
typedef struct PWIndexBuffer PWIndexBuffer;
typedef struct PWVertexArray PWVertexArray;

//vertex buffer functions, copies to b and returns an identical buffer object
PWVBuffer pwvbuffer_init(PWVBuffer *b, GLfloat *data, GLsizei count, GLuint component_count);
void pwvbuffer_bind(PWVBuffer *b);
void pwvbuffer_unbind(PWVBuffer *b);
void pwvbuffer_close(PWVBuffer *b);

//index buffer functions - note that GLushort is used
PWIndexBuffer pwindexbuffer_init(PWIndexBuffer *b, GLushort *data, GLsizei count);
void pwindexbuffer_bind(PWIndexBuffer *b);
void pwindexbuffer_unbind(PWIndexBuffer *b);
void pwindexbuffer_close(PWIndexBuffer *b);

void pwvertexarray_init(PWVertexArray *v);
void pwvertexarray_close(PWVertexArray *v);
void pwvertexarray_add_buffer(PWVertexArray *v, PWVBuffer *b, GLuint index);
void pwvertexarray_bind(PWVertexArray *v);
void pwvertexarray_unbind(PWVertexArray *v);

//vertex buffer
typedef struct PWVBuffer{
	GLuint buffer_id;
	GLuint component_count;
} PWVBuffer;

//index buffer
typedef struct PWIndexBuffer{
	GLuint buffer_id;
	GLuint count;
} PWIndexBuffer;

typedef struct PWVertexArray{
	GLuint array_id;
	int buffers_len;
	int buffers_size;
	PWVBuffer *buffers;
} PWVertexArray;

#endif

