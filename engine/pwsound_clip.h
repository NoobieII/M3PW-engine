//Lithio (The developer's pseudonym)
//June 11, 2022

#ifndef PWSOUND_CLIP_H
#define PWSOUND_CLIP_H

typedef struct PWSoundClip{
	int pos; //raw position, given by vf->offset
	PWSound *sound;
} PWSoundClip;

#endif


