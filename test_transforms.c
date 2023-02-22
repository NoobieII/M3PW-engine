//Windows
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 test_transforms.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o test_transforms -static

#include <stdio.h>
#include <stdlib.h>
#include "engine/pwengine.h"
#include "engine/pwlayer.h"
#include "engine/pwrenderable.h"

int main(int argc, char *argv[]){
	PWEngine *engine;
	PWRenderable *boxes[3];
	PWRenderable r;
	PWLayer l;
	PWLayer *layer;
	PWMat4 *transforms[3];
	PWBezier rotations[3];
	int i;
	
	engine = pwengine_init(640, 480, 30, "M3PW 3D Editor 2");
	//pwlayer_init(&l, pwengine_get_shader(engine), PWM_orthographic(0, 4, 0, 3, -1, 1));
	//layer = &l;
	
	layer = pwengine_get_layer(engine, 0);
	pwlayer_set_projection(layer, PWM_orthographic(0, 4, 0, 3, -1, 1));
	
	pwrenderable_init_rect(&r, PWM_vec3(0,0,0), PWM_vec2(0.5,1), 0xffffffff);
	transforms[0] = pwlayer_add_group(layer, PWM_ini(1, NULL), 0);
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0, 0, 0)), 0);
	boxes[0] = pwlayer_add(layer, &r);
	pwrenderable_init_rect(&r, PWM_vec3(0,0,0), PWM_vec2(0.5,1), 0xffffffff);
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0, 1, 0)), 0);
	transforms[1] = pwlayer_add_group(layer, PWM_ini(1, NULL), 0);
	boxes[1] = pwlayer_add(layer, &r);
	pwrenderable_init_rect(&r, PWM_vec3(0,0,0), PWM_vec2(0.5,1), 0xffffffff);
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0, 1, 0)), 0);
	transforms[2] = pwlayer_add_group(layer, PWM_ini(1, NULL), 0);
	boxes[2] = pwlayer_add(layer, &r);
	
	rotations[2] = pwbezier3(0, 45, -45, 0);
	rotations[1] = pwbezier3(0, -45, 45, 0);
	rotations[0] = pwbezier3(0, 45, -45, 0);
	
	rotations[0].t = 0.0;
	rotations[1].t = 0.4;
	rotations[2].t = 0.2;
	
	
	if(!engine){
		printf("PWEngine couldn't initialize.\n");
		return 0;
	}
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		pwengine_update(engine);
		//calculate the rotation matrices
		for(i = 0; i < 3; ++i){
			*(transforms[i]) = PWM_rotation(pwbezier_add(&rotations[i], 1.0/30), PWM_vec3(0, 0, 1));
		}
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwengine_free(engine);
	printf("Program closes gracefully.\n");
	
	return 0;
}
