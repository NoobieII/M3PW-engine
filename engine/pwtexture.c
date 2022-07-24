//Lithio (The developer's pseudonym)
//June 01, 2022

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "pwtexture.h"

unsigned char *load_image(const char *path, GLsizei *width, GLsizei *height);

void pwtexture_init(PWTexture *t, const char *path){
	unsigned char *pixels;
	
	if(!path){
		t->path = NULL;
		t->texture_id = 0;
		t->width = 0;
		t->height = 0;
		printf("pwtexture_init: No file path provided\n");
		return;
	}
	
	t->path = (char*) malloc(strlen(path) + 1);
	strcpy(t->path, path);
	
	pixels = load_image(path, &t->width, &t->height);
	if(!pixels){
		printf("pwtexture_init: Could not load image %s\n", path);
		return;
	}
	pwtexture_init_pixels(t, t->width, t->height, pixels);
	free(pixels);
}

void pwtexture_init_pixels(PWTexture *t, int width, int height, unsigned char *pixels){
	t->path = NULL;
	if(!pixels){
		//there was a problem loading the image
		if(t->path){
			free(t->path);
		}
		t->texture_id = 0;
		t->width = 0;
		t->height = 0;
		return;
	}
	
	glGenTextures(1, &t->texture_id);
	glBindTexture(GL_TEXTURE_2D, t->texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void pwtexture_close(PWTexture *t){
	if(t->path){
		free(t->path);
	}
	if(t->texture_id){
		glDeleteTextures(1, &t->texture_id);
	}
}

void pwtexture_bind(PWTexture *t){
	glBindTexture(GL_TEXTURE_2D, t->texture_id);
}

void pwtexture_unbind(PWTexture *t){
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned char *load_image(const char *path, GLsizei *width, GLsizei *height){
	SDL_Surface *new_surface = NULL;
	SDL_Surface* loaded_surface = NULL;
	unsigned char *pixels = NULL;
	int x;
	int len;
	
	loaded_surface = IMG_Load(path);
	
	if(loaded_surface == NULL){
		printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
		return NULL;
	}

	//set colour key to black
	SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, 0x00, 0x00, 0x00));
	
	new_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_ABGR8888, 0);
	
	
	if(new_surface == NULL){
		printf("Unable to convert surface. SDL Error: %s\n", SDL_GetError());
		goto load_image_free_new_surface;
	}
	
	if(SDL_LockSurface(new_surface) < 0){
		goto load_image_free_new_surface2;
	}
	
	len = new_surface->h * new_surface->pitch;
	pixels = (unsigned char*) malloc(len);
	for(x = 0; x < len; ++x){
		pixels[x] = ((unsigned char*)new_surface->pixels)[x];
	}
	if(width){
		*width = new_surface->w;
	}
	if(height){
		*height = new_surface->h;
	}	
	
load_image_free_new_surface2:
	SDL_FreeSurface(new_surface);
load_image_free_new_surface:
	SDL_FreeSurface(loaded_surface);
	
	return pixels;
}


