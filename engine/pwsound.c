//Lithio (The developer's pseudonym)
//June 11, 2022

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwsound.h"

int pwsound_open(PWSound *s, const char *filename){
	FILE *soundfile;
	char **comments;
	vorbis_info *vi;
	int i;
	int result;
	int bitstream;
	
	s->buf = NULL;
	s->is_decoded = 0;
	s->len = 0;
	s->is_loop = 0;
	
	soundfile = fopen(filename, "rb");
	if(!soundfile){
		printf("pwsound_open: Failed to open %s\n", filename);
		return -1;
	}
	
	if(ov_open(soundfile, &s->vf, NULL, 0) < 0){
		printf("pwsound_open: Could not initialize OGG stream for %s\n", filename);
		fclose(soundfile);
		return -1;
	}
	
	comments = ov_comment(&s->vf, -1)->user_comments;
	vi = ov_info(&s->vf, -1);
	s->vi = vi;
	
	//check if the music loops, using "loop_start" comment
	while(*comments){
		if(strncmp(*comments, "loop_start=", 11) == 0){
			sscanf((*comments) + 11, "%d", &s->loop_start);
			s->is_loop = 1;
		}
		++comments;
	}
	
	s->len = ov_pcm_total(&s->vf, -1);
	s->num_channels = vi->channels;
	
	if(vi->rate != 44100){
		printf("pwsound_open: Warning: sample rate of %s is not 44100 Hz!\n", filename);
	}
	if(ov_seekable(&s->vf) == 0){
		printf("pwsound_open: Warning: %s is not seekable.\n", filename);
	}
	
	//decode the sound if it is small
	if(s->len * s->num_channels < 500000){
		s->buf = (char*) malloc(s->len * s->num_channels * sizeof(short));
		
		if(s->buf){
			s->is_decoded = 1;
			result = s->len * s->num_channels * sizeof(short);
			
			i = 0; //num bytes decoded
			do{
				i += ov_read(&s->vf, s->buf + i, s->len * s->num_channels * sizeof(short) - i, &bitstream);
			} while(i < result);
			
			ov_clear(&s->vf);
		}
	}
	
	return 0;
}

void pwsound_close(PWSound *s){
	if(s->len > 0){
		if(s->buf){
			free(s->buf);
		}
		else{
			ov_clear(&s->vf);
		}
	}
}

