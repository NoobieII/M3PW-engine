//Lithio (The developer's pseudonym)
//June 11, 2022

#ifndef M3PW_PWSOUND_LOADER_H
#define M3PW_PWSOUND_LOADER_H

#include "pwsound.h"
#include "utilities/hashtable.h"

typedef struct PWSoundLoader {
	HashTable *sounds;
} PWSoundLoader;

void pwsoundloader_init(PWSoundLoader *s);
//returns a reference, don't deallocate it
PWSound *pwsoundloader_load(PWSoundLoader *s, const char *filename);
void pwsoundloader_close(PWSoundLoader *s);

#endif

