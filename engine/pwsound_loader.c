//Lithio (The developer's pseudonym)
//June 11, 2022

#include "pwsound.h"
#include "pwsound_loader.h"

void pwsoundloader_init(PWSoundLoader *s){
	s->sounds = hashtable_init();
}

//returns a reference, don't deallocate it
PWSound *pwsoundloader_load(PWSoundLoader *s, const char *filename){
	PWSound *sound = (PWSound*) hashtable_at_str(s->sounds, filename);
	if(!sound){
		sound = (PWSound*) malloc(sizeof(PWSound));
		if(pwsound_open(sound, filename) == -1){
			return NULL;
		}
		hashtable_set_at_str(s->sounds, filename, sound);
	}
	return sound;
}

void pwsoundloader_close(PWSoundLoader *s){
	PWSound *sound;
	sound = (PWSound*) hashtable_begin(s->sounds);
	while(sound){
		pwsound_close(sound);
		free(sound);
		sound = (PWSound*) hashtable_next(s->sounds);
	}
	hashtable_clear(s->sounds);
}
