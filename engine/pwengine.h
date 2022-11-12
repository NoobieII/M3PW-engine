//engine.h

//12 Nov 2022
//For later consideration:
//For every time update() is called, after the objects' update function is called, do not transform all vertices of a renderable. Transform the group instead.
//
//Layer: group transform, renderable
//Object: referencing to group transform.
//
//Moving a player towards an enemy object.
//in player update(), recalculate transform so that the player is closer to the enemy
//in render
//
//if the group is nested (as for 3d things with multiple moving parts), how are things handled?
//How do we get the transform WRT the origin?
//solution 1: Make the layer hold a new array of transforms, working at the side of the group transforms. When render() is called, the new array of transforms will hold a value WRT the origin.
//solution 2: Make something else handle the transforms. Can one thing handle the world transform? A reference to the group transforms of parent objects will do, but how will they be retrieved?
//Solution 1 seems the most feasible. 
//get_world_transform(

#ifndef M3PW_ENGINE_H
#define M3PW_ENGINE_H

#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_net.h>
#include "pwlayer.h"
#include "pwrenderable.h"
#include "pwsound_loader.h"
#include "pwshader.h"
#include "pwtexture_loader.h"
#include "utilities/hashtable.h"

typedef struct PWObject PWObject;
typedef struct Packet Packet;

typedef struct PWEngine{
	int id;
	int width;
	int height;
	int deltatime;
	int framerate;
	int ticks;
	int last_update_time;
	int last_sleep_time;
	int next_sleep_time;
	
	//input
	int is_quit;
	int mouse_x;
	int mouse_y;
	int scroll_x;
	int scroll_y;
	int is_left_button_pressed;
	int is_right_button_pressed;
	int is_middle_button_pressed;
	int is_left_button_lifted;
	int is_right_button_lifted;
	int is_middle_button_lifted;
	HashTable *is_key_pressed;
	HashTable *is_key_lifted;
	HashTable *is_key_held;
	char text_input[32];
	
	//graphics related
	SDL_Window *window;
	SDL_GLContext *gl_context;
	PWShader shader;
	
	int layers_active[10];
	PWLayer layers[10];
	//SDL_Renderer *renderer;
	
	//audio
	SDL_AudioDeviceID audio_device_id;
	SDL_mutex *sound_mutex;
	int sound_id;
	HashTable *sounds_playing;
	PWSoundLoader sound_loader;
	
	//textures
	PWTextureLoader texture_loader;
	
	//objects
	int object_id;
	HashTable *objects;
	HashTable *objects_id_name;
	HashTable *objects_name_id;
	
	//networking!
	SDL_mutex *net_mutex;
	SDL_Thread *net_thread;
	TCPsocket socket;
	int is_connected;
	int recv_bytes;
	int read_bytes;
	char recv[1024];
} PWEngine;

PWEngine *pwengine_init(int width, int height, int framerate, const char *name);
void      pwengine_free(PWEngine *e);

void      pwengine_update(PWEngine *e);
int       pwengine_deltatime(PWEngine *e);
	
//number of game ticks since the start of the program
int       pwengine_ticks(PWEngine *e);
void      pwengine_poll_events(PWEngine *e);
void      pwengine_quit(PWEngine *e);
int       pwengine_is_quit(PWEngine *e);
void      pwengine_render(PWEngine *e);
void      pwengine_sleep_until_next_frame(PWEngine *e);

int       pwengine_is_key_pressed(PWEngine *e, const char *keyname);
int       pwengine_is_key_lifted(PWEngine *e, const char *keyname);
int       pwengine_is_key_held(PWEngine *e, const char *keyname);

//returns any text typed by the keyboard. If there is none, NULL is returned
const char *pwengine_text_input(PWEngine *e);

int       pwengine_play_sound(PWEngine *e, const char *filename);
void      pwengine_stop_sound(PWEngine *e, int sound_id);
void      pwengine_clear_sounds(PWEngine *e);

void      pwengine_add_object(PWEngine *e, PWObject *o);

//loads a PWRenderable object
//Usage:
//PWRenderable r;
//pwengine_load_texture(e, &r, filename);
//...use the renderable...
//pwrenderable_close(&r);
void      pwengine_load_texture(PWEngine *e, PWRenderable *r, const char *filename);
PWTexture *pwengine_get_texture(PWEngine *e, const char *filename);

//get shader and set perspective used for layer 0 (default layer)
PWShader *pwengine_get_shader(PWEngine *e);
void      pwengine_set_perspective(PWEngine *e, PWMat4 m);

//set layer between layer 0 and 9, shader should only be used for initializing layer
void      pwengine_set_layer(PWEngine *e, int num, PWMat4 pr_matrix, PWShader *s);
PWLayer  *pwengine_get_layer(PWEngine *e, int num);

//void      pwengine_set_orthographic

//returns 0 if successful, -1 if not
int       pwengine_connect(PWEngine *e, const char *host, int port);
void      pwengine_disconnect(PWEngine *e);
int       pwengine_read_8(PWEngine *e);
int       pwengine_read_16(PWEngine *e);
int       pwengine_read_32(PWEngine *e);
float     pwengine_read_f(PWEngine *e);
char     *pwengine_read_str(PWEngine *e);
void      pwengine_read_bytes(PWEngine *e, void *ptr, int n);
void      pwengine_send(PWEngine *e, Packet *p);


#endif

