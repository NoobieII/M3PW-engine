//Lithio (The developer's pseudonym)
//June 20, 2022

#ifndef M3PW_PWTEXTURE_LOADER_H
#define M3PW_PWTEXTURE_LOADER_H

#include "pwtexture.h"
#include "utilities/hashtable.h"

//Texture loader, functionality should be about the same as sound loader

typedef struct PWTextureLoader {
	HashTable *textures;
} PWTextureLoader;

void pwtextureloader_init(PWTextureLoader *t);
//a reference is returned, don't deallocate it
PWTexture *pwtextureloader_load(PWTextureLoader *t, const char *filename);
void pwtextureloader_close(PWTextureLoader *t);


#endif
