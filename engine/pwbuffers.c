//Lithio (The developer's pseudonym)
//May 28, 2022

#include <stdio.h>
#include <stdlib.h>
#include "pwbuffers.h"

PWVBuffer pwvbuffer_init(PWVBuffer *b, GLfloat *data, GLsizei count, GLuint component_count){
	PWVBuffer bc;
	if(!b){
		b = &bc;
	}
	b->component_count = component_count;
	glGenBuffers(1, &b->buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, b->buffer_id);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	bc = *b;
	return bc;
}

void pwvbuffer_bind(PWVBuffer *b){
	glBindBuffer(GL_ARRAY_BUFFER, b->buffer_id);
}

void pwvbuffer_unbind(PWVBuffer *b){
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void pwvbuffer_close(PWVBuffer *b){
	glDeleteBuffers(1, &b->buffer_id);
}

PWIndexBuffer pwindexbuffer_init(PWIndexBuffer *b, GLushort *data, GLsizei count){
	PWIndexBuffer bc;
	if(!b){
		b = &bc;
	}
	b->count = count;
	glGenBuffers(1, &b->buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	bc = *b;
	return bc;
}

void pwindexbuffer_bind(PWIndexBuffer *b){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->buffer_id);
}

void pwindexbuffer_unbind(PWIndexBuffer *b){
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void pwindexbuffer_close(PWIndexBuffer *b){
	glDeleteBuffers(1, &b->buffer_id);
}

void pwvertexarray_init(PWVertexArray *v){
	glGenVertexArrays(1, &v->array_id);
	v->buffers_len = 0;
	v->buffers_size = 0;
	v->buffers = NULL;
}

void pwvertexarray_close(PWVertexArray *v){
	int i;
	for(i = 0; i < v->buffers_len; ++i){
		pwvbuffer_close(&v->buffers[i]);
	}
	if(v->buffers){
		free(v->buffers);
	}
	glDeleteVertexArrays(1, &v->array_id);
}

void pwvertexarray_add_buffer(PWVertexArray *v, PWVBuffer *b, GLuint index){
	PWVBuffer *tmp;
	
	pwvertexarray_bind(v);
	pwvbuffer_bind(b);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, b->component_count, GL_FLOAT, GL_FALSE, 0, 0);
	pwvbuffer_unbind(b);
	pwvertexarray_unbind(v);
	
	//add to buffer
	if(v->buffers_len == v->buffers_size){
		v->buffers_size += 16;
		tmp = (PWVBuffer*) realloc(v->buffers, v->buffers_size * sizeof(PWVBuffer));
		if(!tmp){
			printf("Could not allocate vertex array buffer memory\n");
		}
		else{
			v->buffers = tmp;
		}
	}
	v->buffers[v->buffers_len++] = *b;
}

void pwvertexarray_bind(PWVertexArray *v){
	glBindVertexArray(v->array_id);
}

void pwvertexarray_unbind(PWVertexArray *v){
	glBindVertexArray(0);
}

