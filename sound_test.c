//NoobieII
//17 Jun 23

//Sound test program

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 sound_test.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o sound_test -static

#include "engine/pwengine.h"

int main(int argc, char *argv[]){
	PWEngine *engine;
	int short_sound = 0;
	int long_sound = 0;
	int filter_freq = 44100;
	
	engine = pwengine_init(640, 480, 30, "Sound Test");
	
	
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		
		//your code goes here before update()
		
		if(pwengine_is_key_pressed(engine, "Q") && long_sound == 0){
			long_sound = pwengine_play_sound(engine, "SHOP.ogg");
			pwengine_lp_filter_sound(engine, long_sound, filter_freq);
			pwengine_fade_in_sound(engine, long_sound, 2.0f);
		}
		if(long_sound != 0){
			if(pwengine_is_key_pressed(engine, "W")){
				pwengine_stop_sound(engine, long_sound);
				long_sound = 0;
			}
			if(pwengine_is_key_pressed(engine, "Up")){
				filter_freq += 1000;
				pwengine_lp_filter_sound(engine, long_sound, filter_freq);
			}
			if(pwengine_is_key_pressed(engine, "Down")){
				filter_freq -= 1000;
				pwengine_lp_filter_sound(engine, long_sound, filter_freq);
			}
			if(pwengine_is_key_pressed(engine, "A")){
				pwengine_fade_in_sound(engine, long_sound, 1.0f);
			}
			if(pwengine_is_key_pressed(engine, "S")){
				pwengine_fade_out_sound(engine, long_sound, 1.0f);
				long_sound = 0;
			}
		}
		
		printf("Sound playing at %d Hz          \r", filter_freq);
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwengine_free(engine);
	
	printf("Program closes gracefully.\n");
	return 0;
}