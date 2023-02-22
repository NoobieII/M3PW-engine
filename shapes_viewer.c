//24 Dec 2022
//NoobieII

//Viewer for a model

//Windows
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 shapes_viewer.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o shapes_viewer -static

#include <math.h>

#include "engine/pwanimation.h"
#include "engine/pwengine.h"
#include "engine/pwlayer.h"
#include "engine/pwmath.h"
#include "engine/pwmodel.h"
#include "engine/pwperspective.h"
#include "engine/pwrenderable.h"

#include "engine/utilities/fsize.h"

void reload(PWLayer *layer, PWEngine *engine, PWRenderable **shape, char **vertices_str){
	PWRenderable r;
	int filesize;
	char str[10];
	int i;
	int j;
	int num_repeats;
	
	pwlayer_init(layer, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	
	//red green blue axes
	pwrenderable_init_box(&r, PWM_vec3(1, 0.1, 0.1), 0xff0000ff, NULL); //x
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0.5, 0, 0)), 0);
	pwlayer_add(layer, &r);
	pwlayer_end_group(layer);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 1, 0.1), 0xff00ff00, NULL); //y
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0, 0.5, 0)), 0);
	pwlayer_add(layer, &r);
	pwlayer_end_group(layer);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 0.1, 1), 0xffff0000, NULL); //z
	pwlayer_add_group(layer, PWM_translation(PWM_vec3(0, 0, 0.5)), 0);
	pwlayer_add(layer, &r);
	pwlayer_end_group(layer);
	
	filesize = fsize("shape.txt");
	if(filesize > 0){
		//initialize model
		pwrenderable_load(&r, "shape.txt");
		pwrenderable_set_texture(&r, pwengine_get_texture(engine, "texture.png"));
		*shape = pwlayer_add(layer, &r);
		
		//billboard makes the numbers always point toward the viewer
		*vertices_str = (char*) realloc(*vertices_str, (*shape)->vertex_count * 5);
		for(i = 0; i < (*shape)->vertex_count; ++i){
			sprintf(&(*vertices_str)[5*i], "%d", i);
			
			num_repeats = 0;
			for(j = 0; j < i; ++j){
				//would be nice if we could just compare PWVec3 types normally!
				if(((*shape)->p[j].x == (*shape)->p[i].x) && ((*shape)->p[j].y == (*shape)->p[i].y) && ((*shape)->p[j].z == (*shape)->p[i].z)){
					num_repeats++;
				}
			}
			
			pwrenderable_init_str(&r, PWM_add3((*shape)->p[i], PWM_vec3(0, -0.2*num_repeats, 0)), PWM_vec2(1.0f, 0.2f), &(*vertices_str)[5*i], "LiberationMono-Regular.ttf", 0xffffff00);
			pwlayer_add(layer, &r);
		}
		pwlayer_end_group(layer);
	}
}

//to be called after reload()
void init_aabbs(PWRenderable *r, PWAabb **aabb){
	int i;
	PWVec3 v;
	
	if(r == NULL) return;
	
	*aabb = realloc(*aabb, sizeof(PWAabb) * r->vertex_count);
	v = PWM_vec3(.1, .1, .1);
	for(i = 0; i < r->vertex_count; ++i){
		(*aabb)[i] = PWM_aabb(PWM_add3(r->p[i], v), PWM_sub3(r->p[i], v));
	}
}

int main(int argc, char *argv[]){
	PWEngine *engine;
	PWLayer layer;
	PWRenderable r;
	PWRenderable *shape;
	PWMat4 proj;
	char *vertices_str = NULL;
	PWVec3 v1, v2;
	PWRay ray;
	PWAabb *vertices_aabb = NULL;
	
	//3d view state variables
	PWPersp persp;
	
	
	//for reloading the shape
	int filesize;
	int i = 0;
	
	int selected;
	PWVec3 near;
	PWVec3 hit;
	
	int j;
	int k;
	
	engine = pwengine_init(640, 480, 30, "Shapes Viewer");
	if(!engine){
		printf("PWEngine couldn't initialize.\n");
		return 0;
	}
	
	persp = pwpersp();
	persp.pov = PWM_vec3(0, 0, persp.dist);
	
	filesize = fsize("shape.txt");
	reload(&layer, engine, &shape, &vertices_str);
	init_aabbs(shape, &vertices_aabb);
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		
		if(pwengine_is_key_held(engine, "W")){
			persp.rot.x += 2;
		}
		if(pwengine_is_key_held(engine, "S")){
			persp.rot.x -= 2;
		}
		if(pwengine_is_key_held(engine, "A")){
			persp.rot.y -= 2;
		}
		if(pwengine_is_key_held(engine, "D")){
			persp.rot.y += 2;
		}
		if(pwengine_is_key_held(engine, "Q")){
			persp.dist *= 1.01;
			persp.pov = PWM_mul3(persp.pov, 1.01);
		}
		if(pwengine_is_key_held(engine, "E")){
			persp.dist *= 0.99;
			persp.pov = PWM_mul3(persp.pov, 0.99);
		}
		if(pwengine_is_key_pressed(engine, "Tab")){
			//toggle transparency of the model when Tab is pressed
			shape->c[3] ^= 0xff;
		}
		if(pwengine_is_left_button_lifted(engine)){
			//calculate the ray position
			//calculate ray coming from mouse
			ray = pwpersp_ray(&persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
			
			//check if any of the vertices collide
			near = PWM_add3(ray.origin, PWM_mul3(ray.dir, persp.dist));
			selected = 0;
			for(j = 0; j < shape->vertex_count; ++j){
				//v1 is location of where ray hit
				if(PWM_ray_x_aabb(&ray, &vertices_aabb[j], &hit)){
					if(PWM_norm3(PWM_sub3(hit, v1)) < PWM_norm3(PWM_sub3(near, v1))){
						near = hit;
					}
				}
			}
			for(j = 0; j < shape->vertex_count; ++j){
				if(PWM_ray_x_aabb(&ray, &vertices_aabb[j], &hit)){
					if(PWM_norm3(PWM_sub3(hit, near)) < 0.01f){
						shape->color[j] = 0xff0000ff;
						
						if(selected > 0) printf(", %d", j);
						else printf("Vertex %d", j);
						
						selected++;
					}
					else shape->color[j] = 0xffffffff;
				}
				else shape->color[j] = 0xffffffff;
			}
			if(selected == 0) printf("No vertex");
			printf(" is selected.\n");
		}
		i++;
		if(i >= 60){
			if(filesize != fsize("shape.txt")){
				pwlayer_close(&layer);
				reload(&layer, engine, &shape, &vertices_str);
				init_aabbs(shape, &vertices_aabb);
				filesize = fsize("shape.txt");
			}
			i = 0;
		}
		
		//calculate projection matrix
		pwlayer_set_projection(&layer, pwpersp_mat4(&persp));
		
		
		pwlayer_render(&layer);
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwlayer_close(&layer);
	pwengine_free(engine);
	printf("Program closes gracefully.\n");
	
	return 0;
}