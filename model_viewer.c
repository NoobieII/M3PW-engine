//17 Dec 2022
//NoobieII

//Viewer for a model

//Windows
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 model_viewer.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o model_viewer -static

#include "engine/pwanimation.h"
#include "engine/pwengine.h"
#include "engine/pwlayer.h"
#include "engine/pwmath.h"
#include "engine/pwmodel.h"
#include "engine/pwrenderable.h"

int main(int argc, char *argv[]){
	PWEngine *engine;
	PWLayer layer;
	PWRenderable r;
	PWMat4 proj;
	PWModel model;
	int model_loaded = 0;
	PWAnimation animation;
	int animation_loaded = 0;
	
	//3d view state variables
	float rotx;
	float roty;
	float dist;
	
	
	engine = pwengine_init(640, 480, 60, "Model Viewer");
	if(!engine){
		printf("PWEngine couldn't initialize.\n");
		return 0;
	}
	
	rotx = 0;
	roty = 0;
	dist = 6.0;
	
	pwlayer_init(&layer, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	if(argc == 2){
		if(pwmodel_load(&model, argv[1], engine, &layer) != 0){
			printf("error loading model\n");
		}
		else{
			model_loaded = 1;
		}
	}
	if(argc == 3){
		if(pwmodel_load(&model, argv[1], engine, &layer) != 0){
			printf("error loading model\n");
		}
		else{
			model_loaded = 1;
		}
		if(pwanimation_load(&animation, argv[2], &model) != 0){
			printf("error loading animation\n");
		}
		else{
			animation_loaded = 1;
		}
	}
	
	
	//red green blue axes
	pwrenderable_init_box(&r, PWM_vec3(1, 0.1, 0.1), 0xff0000ff, NULL); //x
	pwlayer_add_group(&layer, PWM_translation(PWM_vec3(0.5, 0, 0)), 0);
	pwlayer_add(&layer, &r);
	pwlayer_end_group(&layer);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 1, 0.1), 0xff00ff00, NULL); //y
	pwlayer_add_group(&layer, PWM_translation(PWM_vec3(0, 0.5, 0)), 0);
	pwlayer_add(&layer, &r);
	pwlayer_end_group(&layer);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 0.1, 1), 0xffff0000, NULL); //z
	pwlayer_add_group(&layer, PWM_translation(PWM_vec3(0, 0, 0.5)), 0);
	pwlayer_add(&layer, &r);
	pwlayer_end_group(&layer);
	
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		
		if(pwengine_is_key_held(engine, "W")){
			rotx += 2;
			if(rotx > 90){
				rotx = 90;
			}
		}
		if(pwengine_is_key_held(engine, "S")){
			rotx -= 2;
			if(rotx < -90){
				rotx = -90;
			}
		}
		if(pwengine_is_key_held(engine, "A")){
			roty -= 2;
		}
		if(pwengine_is_key_held(engine, "D")){
			roty += 2;
		}
		if(pwengine_is_key_held(engine, "Q")){
			dist *= 1.01;
			if(dist > 50.0){
				dist = 50.0;
			}
		}
		if(pwengine_is_key_held(engine, "E")){
			dist *= 0.99;
			if(dist < 5.0){
				dist = 5.0;
			}
		}
		
		//calculate projection matrix
		proj = PWM_rotation(roty, PWM_vec3(0, 1, 0));
		proj = PWM_mul(PWM_rotation(rotx, PWM_vec3(1, 0, 0)), proj);
		proj = PWM_mul(PWM_translation(PWM_vec3(0, 0, -dist)), proj);
		proj = PWM_mul(PWM_perspective(60, 4/3.0, 2, 2*dist-2), proj);
		pwlayer_set_projection(&layer, proj);
		pwlayer_render(&layer);
		
		
		if(animation_loaded){
			pwanimation_update(&animation, pwengine_deltatime(engine));
		}
		
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwlayer_close(&layer);
	if(model_loaded){
		pwmodel_close(&model);
	}
	if(animation_loaded){
		pwanimation_close(&animation);
	}
	printf("Closing PWEngine... \n");
	pwengine_free(engine);
	printf("Program closes gracefully.\n");
	
	return 0;
}