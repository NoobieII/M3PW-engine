//Lithio (The developer's pseudonym)
//June 20, 2022

#include "pwtexture_loader.h"

void pwtextureloader_init(PWTextureLoader *t){
	t->textures = hashtable_init();
}


PWTexture *pwtextureloader_load(PWTextureLoader *t, const char *filename){
	PWTexture *texture = hashtable_at_str(t->textures, filename);
	if(!texture){
		texture = (PWTexture*) malloc(sizeof(PWTexture));
		pwtexture_init(texture, filename);
		hashtable_set_at_str(t->textures, filename, texture);
	}
	return texture;
}

void pwtextureloader_close(PWTextureLoader *t){
	PWTexture *texture = (PWTexture*) hashtable_begin(t->textures);
	while(texture){
		pwtexture_close(texture);
		free(texture);
		texture = (PWTexture*) hashtable_next(t->textures);
	}
	hashtable_clear(t->textures);
}
