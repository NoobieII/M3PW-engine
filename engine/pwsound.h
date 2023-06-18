//Lithio (The developer's pseudonym)
//June 11, 2022

//added features for filter on 17 Jun 23

#ifndef M3PW_PWSOUND_H
#define M3PW_PWSOUND_H

#include "vorbis/ivorbisfile.h"

typedef struct PWSound {
	OggVorbis_File vf;
	vorbis_info *vi;
	int is_decoded; //short sounds (< 100k) will be decoded 
	char *buf;
	int len; //number of samples in sound file
	int num_channels;
	int loop_start; //sample where to begin the loop
	int is_loop; //0 or 1
} PWSound;

//returns 0 if successful, -1 if not
int  pwsound_open(PWSound *s, const char *filename);
void pwsound_close(PWSound *s);

#endif

