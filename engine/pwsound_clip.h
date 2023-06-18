//Lithio (The developer's pseudonym)
//June 11, 2022

//17 Jun 23 filter modes added

#ifndef PWSOUND_CLIP_H
#define PWSOUND_CLIP_H

typedef struct PWSoundClip{
	int pos; //raw position, given by vf->offset
	
	int filter_mode; //0 for normal, 1 for attenuated, 2 for fade in, 4 for fade out, 8 for filter
	short *last_sample; //used for filter
	float filter; //valued between 0 and 1
	int fade_length; //number of samples for fade out/in
	int fade_time; //number of samples passed
	float sound_level; //level of final sound between 0 and 1
	
	PWSound *sound;
} PWSoundClip;

#endif


