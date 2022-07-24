//Lithio (The developer's pseudonym)
//June 10, 2022

//The text loader is separate from the renderer to make things a bit organized

#ifndef M3PW_PWTEXT_LOADER_H
#define M3PW_PWTEXT_LOADER_H

#include "pwtexture.h"
#include "utilities/hashtable.h"

typedef struct PWTextLoader{
	HashTable *glyph_maps;
	HashTable *glyph_advances;
} PWTextLoader;

void pwtextloader_init(PWTextLoader *t);
//returns references through texture and advances, don't deallocate
void pwtextloader_load(PWTextLoader *t, const char *filename, PWTexture **texture, float **advances);
void pwtextloader_close(PWTextLoader *t);

#endif

