//engine.c

#include <stdlib.h>
#include <string.h>
#include <time.h>

#if (defined(unix) || defined(__unix) || defined(__unix__))
#include "unistd.h"
#endif

#if defined(_WIN32)
#include "windows.h"
#endif

#include <SDL_image.h>
#include <SDL_ttf.h>

#include "pwengine.h"
#include "pwmath.h"
#include "pwobject.h"
#include "pwsound.h"
#include "pwsound_clip.h"

void pwengine_audio_callback(void* userdata, Uint8* stream, int len);

PWEngine *pwengine_init(int width, int height, int framerate, const char *name){
	PWEngine *e;
	SDL_AudioSpec desired, obtained;
	GLenum glerror = GL_NO_ERROR;
	PWMat4 ortho, vw, ml;
	int max_textures;
	int i;
	
	GLint texture_ids[] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29, 30, 31
	};
	
	e = (PWEngine*) malloc(sizeof(PWEngine));
	memset(e, 0, sizeof(PWEngine));
	
	e->width = width;
	e->height = height;
	
	//make there be no interpolation!
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE){
		printf("Error setting scaling to nearest pixel sampling\n");
		goto pwengine_init_cleanup;
	}
	if(SDL_SetHint(SDL_HINT_RENDER_OPENGL_SHADERS, "1") == SDL_FALSE){
		printf("Error setting opengl shaders\n");
		goto pwengine_init_cleanup;
	}
	
	//initialize SDL
	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0){
		printf("something went wrong with video init\n");
		goto pwengine_init_cleanup;
	}
	if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0){
		printf("something went wrong with audio init\n");
		goto pwengine_init_cleanup;
	}
	
	//initialize SDL sound
	//specify the desired format of the audio stream
	SDL_zero(desired);
	desired.freq = 44100;
	desired.format = AUDIO_S16; //same as S16LSB
	desired.channels = 2;
	desired.samples = 1024;
	desired.callback = pwengine_audio_callback;
	desired.userdata = e;
	
	e->audio_device_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	
	if(e->audio_device_id == 0){
		printf("device couldn't be opened\n");
		goto pwengine_init_cleanup;
	}
	
	//Initialize SDL2 image
	if(IMG_Init(IMG_INIT_JPG & IMG_INIT_PNG) != (IMG_INIT_JPG & IMG_INIT_PNG)){
		printf("Error in initializing SDL_IMG: %s\n", IMG_GetError());
		goto pwengine_init_cleanup;
	}
	
	//initialize SDL2 TTF
	if(TTF_Init() == -1){
		printf("Error in initializing SDL_TTF: %s\n", TTF_GetError());
		goto pwengine_init_cleanup;
	}
	
	//initialize SDL2Net
	if(SDLNet_Init() == -1){
		printf("Error in initializing SDLNet: \n%s", SDLNet_GetError());
		goto pwengine_init_cleanup;
	}
	
	//use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	
	
	
	
	//create a window and renderer
	e->window = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	if(!e->window){
		printf("could not create window: %s\n", SDL_GetError());
		goto pwengine_init_cleanup;
	}
	
	e->gl_context = SDL_GL_CreateContext(e->window);
	if(!e->gl_context){
		printf("OpenGL context couldn't be created: %s\n", SDL_GetError());
		goto pwengine_init_cleanup2;
	}
	
	//initialize opengl
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glerror = glGetError();
	if(glerror != GL_NO_ERROR){
		printf("Error initializing OpenGL: %s\n", gluErrorString(glerror));
	}
	//initialize Modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glerror = glGetError();
	if(glerror != GL_NO_ERROR){
		printf("Error initializing OpenGL: %s\n", gluErrorString(glerror));
	}
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	
	/*
	e->renderer = SDL_CreateRenderer(e->window, -1, SDL_RENDERER_ACCELERATED);
	if(!e->renderer){
		printf("Could not create renderer. %s\n", SDL_GetError());
		goto pwengine_init_cleanup2;
	}
	*/
	//initialize GLEW
	if(glewInit() != GLEW_OK){
		printf("Could not initialize GLEW\n");
		goto pwengine_init_cleanup2;
	}
	
	//initiailize shader
	ortho = PWM_orthographic(
		0.0,
		4.0,
		0.0,
		3.0,
		-1.0,
		1.0
	);
	vw = ml = PWM_ini(1.0, NULL);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_textures);
	if(max_textures == 16){
		pwshader_init(&e->shader, "basic3.vert", "basic16.frag");
	}
	else if(max_textures >= 32){
		pwshader_init(&e->shader, "basic3.vert", "basic32.frag");
	}
	pwshader_enable(&e->shader);
	pwshader_set_uniform_1iv(&e->shader, "textures", texture_ids, 32);
	pwshader_set_uniform_mat4(&e->shader, "pr_matrix", ortho);
	pwshader_set_uniform_mat4(&e->shader, "vw_matrix", vw);
	pwshader_set_uniform_mat4(&e->shader, "ml_matrix", ml);	
	
	pwshader_set_uniform_3f(&e->shader, "light_direction", PWM_vec3(0, 0, 1));
	pwshader_set_uniform_1f(&e->shader, "ambient_intensity", 0.5);
	
	//initialize layer
	e->layers_active[0] = 1;
	pwlayer_init(&e->layers[0], &e->shader, ortho);
	
	for(i = 1; i < 10; ++i){
		e->layers_active[i] = 0;
	}
	
	//initialize time
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	{	
		struct timespec tp;
		clock_gettime(CLOCK_MONOTONIC, &tp);
		
		e->last_update_time = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
	}
	#endif
	
	#if defined(_WIN32)
	e->last_update_time = clock() / (CLOCKS_PER_SEC / 1000);
	#endif
	
	e->framerate = framerate;
	e->last_sleep_time = e->last_update_time;
	e->next_sleep_time = e->last_update_time + (1000 / framerate);
	
	//tables for tracking the buttons pressed
	e->is_key_pressed = hashtable_init();
	e->is_key_lifted = hashtable_init();
	e->is_key_held = hashtable_init();
	
	e->sound_mutex = SDL_CreateMutex();
	if(!e->sound_mutex){
		printf("culdn't create sound mutex\n");
	}
	e->sound_id = 1;
	e->sounds_playing = hashtable_init();
	
	//initialize sound loader
	pwsoundloader_init(&e->sound_loader);
	
	//this must be called to unpause any audio device
	SDL_PauseAudioDevice(e->audio_device_id, 0);
	
	pwtextureloader_init(&e->texture_loader);
	
	//initialize objects
	e->object_id = 1;
	e->objects = hashtable_init();
	e->objects_id_name = hashtable_init();
	e->objects_name_id = hashtable_init();
	
	//networking
	e->net_mutex = SDL_CreateMutex();
	if(!e->net_mutex){
		printf("couldn't create net mutex\n");
	}
	e->net_thread = NULL;
	e->socket = NULL;
	e->is_connected = 0;
	e->recv_bytes = 0;
	e->read_bytes = 0;
	
	return e;
	
pwengine_init_cleanup2:
	SDL_DestroyWindow(e->window);
pwengine_init_cleanup:
	free(e);
	return NULL;
}

void pwengine_free(PWEngine *e){
	int i;
	void *ptr;
	int result;
	
	if(!e){
		return;
	}
	
	if(e->net_thread){
		SDL_WaitThread(e->net_thread, &result);
		printf("Net thread returns %d (0 == OK)\n", result);
	}
	
	//close layer(s)
	for(i = 0; i < 10; ++i){
		if(e->layers_active[i]){
			pwlayer_close(&e->layers[i]);
		}
	}
	
	if(e->window){
		SDL_DestroyWindow(e->window);
	}
	pwshader_close(&e->shader);
	
	//clear input data
	ptr = hashtable_begin(e->is_key_pressed);
	while(ptr){
		free(ptr);
		ptr = hashtable_next(e->is_key_pressed);
	}
	hashtable_clear(e->is_key_pressed);
	ptr = hashtable_begin(e->is_key_lifted);
	while(ptr){
		free(ptr);
		ptr = hashtable_next(e->is_key_lifted);
	}
	hashtable_clear(e->is_key_lifted);
	ptr = hashtable_begin(e->is_key_held);
	while(ptr){
		free(ptr);
		ptr = hashtable_next(e->is_key_held);
	}
	hashtable_clear(e->is_key_held);
	
	//delete sounds playing
	SDL_LockMutex(e->sound_mutex);
	//TODO delete the sounds... and do it correctly
	ptr = hashtable_begin(e->sounds_playing);
	while(ptr){
		free(ptr);
		ptr = hashtable_next(e->sounds_playing);
	}
	hashtable_clear(e->sounds_playing);
	SDL_UnlockMutex(e->sound_mutex);
	
	SDL_DestroyMutex(e->sound_mutex);
	pwsoundloader_close(&e->sound_loader);
	pwtextureloader_close(&e->texture_loader);
	
	//destroy objects
	ptr = hashtable_begin(e->objects);
	while(ptr){
		pwobject_close((PWObject*)ptr);
		ptr = hashtable_next(ptr);
	}
	hashtable_clear(e->objects);
	
	//clear object name/id tables
	hashtable_clear(e->objects_id_name);
	hashtable_clear(e->objects_name_id);
	
	SDL_DestroyMutex(e->net_mutex);
	if(e->is_connected){
		SDLNet_TCP_Close(e->socket);
	}
	
	free(e);
	IMG_Quit();
	TTF_Quit();
	if(e->net_thread) SDLNet_Quit();
	SDL_Quit();
}

void pwengine_update(PWEngine *e){
	void *ptr;
	PWObject *object;
	PWObject *tmp;
	
	e->deltatime = e->next_sleep_time - e->last_sleep_time;
	e->last_update_time = e->last_sleep_time;
	
	//game object related things
	object = (PWObject*) hashtable_begin(e->objects);
	while(object){
		if(pwobject_is_destroy(object)){
			//object must point to the next before removing the item
			tmp = object;
			object = (PWObject*) hashtable_next(e->objects);
			
			//remove the name and id's in the lookup tables
			hashtable_remove_at_int(e->objects, tmp->id);
			hashtable_remove_at_int(e->objects_id_name, tmp->id);
			hashtable_remove_at_str(e->objects_name_id, tmp->name);
			pwobject_close(tmp);
		}
		else{
			if(pwobject_is_active(object)){
				pwobject_update(object);
			}
			object = (PWObject*) hashtable_next(e->objects);
		}
	}
	e->mouse_delta_x = 0;
	e->mouse_delta_y = 0;
	
	//clear all events
	e->is_left_button_pressed = 0;
	e->is_right_button_pressed = 0;
	e->is_middle_button_pressed = 0;
	
	e->is_left_button_lifted = 0;
	e->is_right_button_lifted = 0;
	e->is_middle_button_lifted = 0;		
	
	ptr = hashtable_begin(e->is_key_pressed);
	while(ptr){
		*(int*)(ptr) = 0;
		ptr = hashtable_next(e->is_key_pressed);
	}
	ptr = hashtable_begin(e->is_key_lifted);
	while(ptr){
		*(int*)(ptr) = 0;
		ptr = hashtable_next(e->is_key_lifted);
	}
	e->scroll_x = 0;
	e->scroll_y = 0;
	
	e->text_input[0] =  '\0';
}

int       pwengine_deltatime(PWEngine *e){
	return e->deltatime;
}
	
int       pwengine_ticks(PWEngine *e){
	return e->ticks;
}

void      pwengine_poll_events(PWEngine *e){
	SDL_Event event;
	const char *keyname;
	
	while(SDL_PollEvent(&event)){
		//for now do nothing
		switch(event.type){
		case SDL_MOUSEMOTION:
			e->mouse_delta_x = event.motion.x - e->mouse_x;
			e->mouse_delta_y = event.motion.y - e->mouse_y;
			e->mouse_x = event.motion.x;
			e->mouse_y = event.motion.y;
			//printf("mouse at (%d, %d)\n", e->mouse_x, e->mouse_y);
			//printf("mouse at %.2f %.2f\n", pwengine_mouse_x(e), pwengine_mouse_y(e));
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch(event.button.button){
			case SDL_BUTTON_LEFT:
				e->is_left_button_pressed = 1;
				e->is_left_button_held = 1;
				//printf("mouse left button pressed\n");
				break;
			case SDL_BUTTON_RIGHT:
				e->is_right_button_pressed = 1;
				e->is_right_button_held = 1;
				//printf("mouse right button pressed\n");
				break;
			case SDL_BUTTON_MIDDLE:
				e->is_middle_button_pressed = 1;
				e->is_middle_button_held = 1;
				//printf("mouse middle button pressed\n");
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch(event.button.button){
			case SDL_BUTTON_LEFT:
				e->is_left_button_lifted = 1;
				e->is_left_button_held = 0;
				//printf("mouse left button lifted\n");
				break;
			case SDL_BUTTON_RIGHT:
				e->is_right_button_lifted = 1;
				e->is_right_button_held = 0;
				//printf("mouse right button lifted\n");
				break;
			case SDL_BUTTON_MIDDLE:
				e->is_middle_button_lifted = 1;
				e->is_middle_button_held = 0;
				//printf("mouse middle button lifted\n");
				break;
			}
			break;
		case SDL_MOUSEWHEEL:
			e->scroll_x = event.wheel.x;
			e->scroll_y = event.wheel.y;
			
			//printf("mouse scroll event (%d, %d)\n", e->scroll_x, e->scroll_y);
			break;
		case SDL_KEYDOWN:
			keyname = SDL_GetKeyName(event.key.keysym.sym);
			
			if(!hashtable_at_str(e->is_key_pressed, keyname)){
				hashtable_set_at_str(e->is_key_pressed, keyname, malloc(sizeof(int)));
			}
			*(int*)(hashtable_at_str(e->is_key_pressed, keyname)) = 1;
			
			if(!hashtable_at_str(e->is_key_held, keyname)){
				hashtable_set_at_str(e->is_key_held, keyname, malloc(sizeof(int)));
			}
			*(int*)(hashtable_at_str(e->is_key_held, keyname)) = 1;
			
			//fprintf(stderr, "key %s scancode %s was pressed\n",
				//SDL_GetKeyName(event.key.keysym.sym),
				//SDL_GetScancodeName(event.key.keysym.scancode));
			break;
		case SDL_KEYUP:
			keyname = SDL_GetKeyName(event.key.keysym.sym);

			if(!hashtable_at_str(e->is_key_lifted, keyname)){
				hashtable_set_at_str(e->is_key_lifted, keyname, malloc(sizeof(int)));
			}
			*(int*)(hashtable_at_str(e->is_key_lifted, keyname)) = 1;			
			
			if(!hashtable_at_str(e->is_key_held, keyname)){
				hashtable_set_at_str(e->is_key_held, keyname, malloc(sizeof(int)));
			}
			*(int*)(hashtable_at_str(e->is_key_held, keyname)) = 0;
			
			//printf("key %s scancode %s was lifted\n",
				//keyname,
				//SDL_GetScancodeName(event.key.keysym.scancode));
			break;
		case SDL_TEXTINPUT:
			strcat(e->text_input, event.text.text);
			//printf("received textinput event, text_input_ = %s\n", e->text_input);
			break;
		case SDL_TEXTEDITING:
			//printf("received textediting event\n");
			break;
		case SDL_WINDOWEVENT:
			//printf("received window event\n");
			if(event.window.event == SDL_WINDOWEVENT_RESIZED){
				e->width = event.window.data1;
				e->height = event.window.data2;
				glViewport(0, 0, e->width, e->height);
			}
			break;
		case SDL_QUIT:
			e->is_quit = 1;
			break;
		}
	}
}

void      pwengine_quit(PWEngine *e){
	e->is_quit = 1;
}

int       pwengine_is_quit(PWEngine *e){
	return e->is_quit;
}

void      pwengine_render(PWEngine *e){
	int i;
	//draw the screen
	//SDL_RenderPresent(e->renderer);
	
	for(i = 0; i < 10; ++i){
		if(e->layers_active[i]){
			pwlayer_render(&e->layers[i]);
		}
	}
	
	//check for openGL issues
	GLenum error = glGetError();
	if(error != GL_NO_ERROR){
		printf("OpenGL error: %d\n", (int) error);
	}
	
	//draw the screen
	SDL_GL_SwapWindow(e->window);
	
	//clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//clear screen
	//SDL_RenderClear(e->renderer);	
	
}

void      pwengine_sleep_until_next_frame(PWEngine *e){
	int current_time;
	
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	
	current_time = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
	#endif
	
	#if defined(_WIN32)
	current_time = clock() / (CLOCKS_PER_SEC / 1000);
	#endif
	
	if(current_time < e->next_sleep_time){
		//linux specific
		#if (defined(unix) || defined(__unix) || defined(__unix__))
		usleep((e->next_sleep_time - current_time) * 1000);
		#endif

		//windows specific
		#if defined(_WIN32)
		Sleep(e->next_sleep_time - current_time);
		#endif
	}
	
	e->last_sleep_time = e->next_sleep_time;
	e->next_sleep_time = e->last_sleep_time + (1000 / e->framerate);
	
	e->ticks++;
}

int pwengine_is_key_pressed(PWEngine *e, const char *keyname){
	if(!hashtable_at_str(e->is_key_pressed, keyname)){
		return 0;
	}
	return *(int*)(hashtable_at_str(e->is_key_pressed, keyname));
}

int pwengine_is_key_lifted(PWEngine *e, const char *keyname){
	if(!hashtable_at_str(e->is_key_lifted, keyname)){
		return 0;
	}
	return *(int*)(hashtable_at_str(e->is_key_lifted, keyname));
}

int pwengine_is_key_held(PWEngine *e, const char *keyname){
	if(!hashtable_at_str(e->is_key_held, keyname)){
		return 0;
	}
	return *(int*)(hashtable_at_str(e->is_key_held, keyname));
}

int pwengine_is_left_button_pressed(PWEngine *e){
	return e->is_left_button_pressed;
}

int pwengine_is_right_button_pressed(PWEngine *e){
	return e->is_right_button_pressed;
}

int pwengine_is_middle_button_pressed(PWEngine *e){
	return e->is_middle_button_pressed;
}

int pwengine_is_left_button_lifted(PWEngine *e){
	return e->is_left_button_lifted;
}

int pwengine_is_right_button_lifted(PWEngine *e){
	return e->is_right_button_lifted;
}

int pwengine_is_middle_button_lifted(PWEngine *e){
	return e->is_middle_button_lifted;
}

int pwengine_is_left_button_held(PWEngine *e){
	return e->is_left_button_held;
}

int pwengine_is_right_button_held(PWEngine *e){
	return e->is_right_button_held;
}

int pwengine_is_middle_button_held(PWEngine *e){
	return e->is_middle_button_held;
}

float pwengine_mouse_x(PWEngine *e){
	return 2.0f * e->mouse_x / e->width - 1;
}

float pwengine_mouse_y(PWEngine *e){
	return -2.0f * e->mouse_y / e->height + 1;
}

float pwengine_mouse_delta_x(PWEngine *e){
	return 2.0f * e->mouse_delta_x / e->width;
}

float pwengine_mouse_delta_y(PWEngine *e){
	return -2.0f * e->mouse_delta_y / e->height;
}

const char *pwengine_text_input(PWEngine *e){
	if(e->text_input[0]){
		return e->text_input;
	}
	return NULL;
}

void pwengine_audio_callback(void* userdata, Uint8* stream, int len){
	int i;
	int j;
	int k;	
	int *sample;
	int result = 0;
	int bitstream;
	
	PWEngine *e = (PWEngine*) userdata;
	int id = 0;
	int delete_node;
	PWSoundClip *sound_clip = (PWSoundClip*) hashtable_begin(e->sounds_playing);
	PWSound *sound;
	
	short *decoded_sample;
	short *decoded_end;
	short *buffer_sample;
	short *buffer = (short*) malloc(len);
	int *stream_unclipped = (int*) malloc(len * 4);
	
	//set stream to zero
	memset(stream, 0, len);
	memset(stream_unclipped, 0, len * 2);
	
	if(SDL_LockMutex(e->sound_mutex) != 0){
		printf("error in locking sound mutex\n");
		return;
	}
	
	while(sound_clip){
		sound = sound_clip->sound;
		delete_node = 0;
		
		sample = stream_unclipped;
		
		if(sound->is_decoded){
			//audio already decoded
			if(sound->num_channels == 1){
				result = len / 2 / sizeof(short);
			}
			if(sound->num_channels == 2){
				result = len / sizeof(short);
			}
			
			i = 0; //number PCM samples read
			decoded_sample = ((short*)(sound->buf) + sound_clip->pos * sound->num_channels);
			decoded_end = ((short*)(sound->buf) + sound->len * sound->num_channels);
			do{
				if(sound->num_channels == 1){
					*(sample++) += *(decoded_sample);
					*(sample++) += *(decoded_sample++);
				}
				if(sound->num_channels == 2){
					*(sample++) += *(decoded_sample++);
					*(sample++) += *(decoded_sample++);
				}
				++i;
				if(decoded_sample >= decoded_end){
					if(sound->is_loop){
						decoded_sample = ((short*)(sound->buf)) + (sound->loop_start * sound->num_channels);
					}
					else{
						delete_node = 1;
						break;
					}
				}
			} while(i * sizeof(short) * 2 / sound->num_channels < len);
		}
		else{
			//undecoded audio
			
			//seek to position for this sound clip
			if(ov_pcm_seek(&sound->vf, sound_clip->pos) != 0){
				delete_node = 1;
			}
			else {
				i = 0; //number of bytes decoded
				j = 0; //number of times 0 bytes decoded per function call
				
				do{
					buffer_sample = buffer;
					result = ov_read(&sound->vf, buffer, len*sound->num_channels/2 - i, &bitstream);
					result /= sizeof(short);
					
					//copy the sound and add to the stream
					if(sound->num_channels == 1){
						//copy half the number of samples and distribute
						//accross two channels
						for(k = 0; k < result; ++k){
							*(sample++) += *(buffer_sample);
							*(sample++) += *(buffer_sample++);
						}
					}
					if(sound->num_channels == 2){
						for(k = 0; k < result; ++k){
							*(sample++) += *(buffer_sample++);
						}
					}
					result *= sizeof(short);
					i += result;
					
					if(result == 0){
						++j;
					}
					
					if(j == 10){
						if(sound->is_loop){
							if(ov_pcm_seek(&sound->vf, sound->loop_start) != 0){
								delete_node = 1;
								break;
							}
						}
						else{
							delete_node = 1;
							break;
						}
					}
				} while (i * 2 / sound->num_channels < len);
			}
		}
		
		if(delete_node){
			free(sound_clip);
			
			id = hashtable_key_i(e->sounds_playing);
			sound_clip = (PWSoundClip*) hashtable_next(e->sounds_playing);
			hashtable_remove_at_int(e->sounds_playing, id);
		}
		else{
			//save PCM position
			sound_clip->pos += len / (2 * sizeof(short));
			if(sound_clip->pos >= sound->len){
				sound_clip->pos = sound->loop_start + sound_clip->pos - sound->len;
			}
			sound_clip = (PWSoundClip*) hashtable_next(e->sounds_playing);
		}
	}
	
	
		
	//do necessary clipping
	for(i = 0; i < len / sizeof(short); ++i){
		if(stream_unclipped[i] < -32768){
			((short*)(stream))[i] = -32768;
		}
		else if(stream_unclipped[i] > 32767){
			((short*)(stream))[i] = 32767;
		}
		else{
			((short*)(stream))[i] = stream_unclipped[i];
		}
	}
	
	SDL_UnlockMutex(e->sound_mutex);
	
	free(buffer);
	free(stream_unclipped);
}

int pwengine_play_sound(PWEngine *e, const char *filename){
	PWSoundClip *clip;
	PWSound *sound = pwsoundloader_load(&e->sound_loader, filename);
	
	if(!sound){
		return 0;
	}
	
	clip = (PWSoundClip*) malloc(sizeof(PWSoundClip));
	
	SDL_LockMutex(e->sound_mutex);
	
	clip->sound = sound;
	clip->pos = 0;
	hashtable_set_at_int(e->sounds_playing, e->sound_id, clip);
	
	SDL_UnlockMutex(e->sound_mutex);
	
	return e->sound_id++;
}

void pwengine_stop_sound(PWEngine *e, int sound_id){
	void *sound_clip;
	
	SDL_LockMutex(e->sound_mutex);
	sound_clip = hashtable_at_int(e->sounds_playing, sound_id);
	if(sound_clip){
		free(sound_clip);
	}
	hashtable_remove_at_int(e->sounds_playing, sound_id);
	SDL_UnlockMutex(e->sound_mutex);
}
	
void pwengine_clear_sounds(PWEngine *e){
	void *sound_clip;
	int id;
	
	SDL_LockMutex(e->sound_mutex);
	sound_clip = hashtable_begin(e->sounds_playing);
	while(sound_clip){
		id = hashtable_key_i(e->sounds_playing);
		free(sound_clip);
		sound_clip = hashtable_next(e->sounds_playing);
		hashtable_remove_at_int(e->sounds_playing, id);
	}
	SDL_UnlockMutex(e->sound_mutex);
	
	e->sound_id = 1;
}

void pwengine_load_texture(PWEngine *e, PWRenderable *r, const char *filename){
	PWTexture *t = pwtextureloader_load(&e->texture_loader, filename);
	pwrenderable_init_texture(r, PWM_vec3(0,0,0), PWM_vec2(1,1), 0xFFFFFFFF, t);
}

PWTexture *pwengine_get_texture(PWEngine *e, const char *filename){
	return pwtextureloader_load(&e->texture_loader, filename);
}

PWShader *pwengine_get_shader(PWEngine *e){
	return &e->shader;
}

void pwengine_set_perspective(PWEngine *e, PWMat4 m){
	pwshader_set_uniform_mat4(&e->shader, "pr_matrix", m);
}

void pwengine_set_layer(PWEngine *e, int num, PWMat4 pr_matrix, PWShader *s){
	if(num < 0 || num > 10){
		return;
	}
	if(e->layers_active[num]){
		pwlayer_set_projection(&e->layers[num], pr_matrix);
	}
	else {
		if(!s){
			printf("pwengine_set_layer: Shader for layer %d is null.\n", num);
		}
		pwlayer_init(&e->layers[num], s, pr_matrix);
		e->layers_active[num] = 1;
	}
}
	
PWLayer *pwengine_get_layer(PWEngine *e, int num){
	if(num < 0 || num > 10 || !(e->layers_active[num])){
		return NULL;
	}
	return &e->layers[num];
}

void pwengine_add_object(PWEngine *e, PWObject *o){
	o->id = e->object_id++;
	hashtable_set_at_int(e->objects, o->id, o);
	//add to id->name
	hashtable_set_at_int(e->objects_id_name, o->id, o->name);
	//add to name->id... if the same name exists, it will be overwritten!
	hashtable_set_at_str(e->objects_name_id, o->name, &o->id);
}

static int socket_recv_thread(void *ptr);
int       pwengine_connect(PWEngine *e, const char *host, int port){
	IPaddress ipaddress;
	
	if(SDLNet_ResolveHost(&ipaddress, host, (Uint16)port) == -1){
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return -1;
	}
	e->socket = SDLNet_TCP_Open(&ipaddress);
	if(!e->socket){
		printf("SDLNet_TPC_Open: %s\n", SDLNet_GetError());
		return -1;
	}
	e->is_connected = 1;
	e->recv_bytes = 0;
	e->read_bytes = 0;
	
	//create a thread
	e->net_thread = SDL_CreateThread(socket_recv_thread, "PWNetThread", (void*)e);
	if(!e->net_thread){
		printf("pwengine_connect: failed to create thread: %s\n", SDL_GetError());
		return -1;
	}
	
	return 0;
}

void      pwengine_disconnect(PWEngine *e){
	SDLNet_TCP_Close(e->socket);
	e->is_connected = 0;
}

int       pwengine_read_8(PWEngine *e){
	int ret = 0;
	
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + 1 > e->recv_bytes){
		printf("pwengine_read_8: reading outside of array bound\n");
	}
	else{
		ret = (int) e->recv[e->read_bytes++];
	}
	SDL_UnlockMutex(e->net_mutex);
	return ret;
}

int       pwengine_read_16(PWEngine *e){
	int ret = 0;
	
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + 2 > e->recv_bytes){
		printf("pwengine_read_16: reading outside of array bound\n");
	}
	else{
		ret &= (int) e->recv[e->read_bytes++];
		ret &= ((int)e->recv[e->read_bytes++]) << 8;
	}
	SDL_UnlockMutex(e->net_mutex);
	return ret;
}

int       pwengine_read_32(PWEngine *e){
	int ret = 0;
	
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + 4 > e->recv_bytes){
		printf("pwengine_read_32: reading outside of array bound\n");
	}
	else{
		ret &= (int) e->recv[e->read_bytes++];
		ret &= ((int)e->recv[e->read_bytes++]) << 8;
		ret &= ((int)e->recv[e->read_bytes++]) << 16;
		ret &= ((int)e->recv[e->read_bytes++]) << 24;
	}
	SDL_UnlockMutex(e->net_mutex);
	return ret;
}

float     pwengine_read_f(PWEngine *e){
	float ret = 0.0f;
	
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + 4 > e->recv_bytes){
		printf("pwengine_read_f: reading outside of array bound\n");
	}
	else{
		ret = *(float*)(e->recv + e->read_bytes);
		e->read_bytes += 4;
	}
	SDL_UnlockMutex(e->net_mutex);
	return ret;
}
	
char     *pwengine_read_str(PWEngine *e){
	char *ret = NULL;
	int len = pwengine_read_16(e);
	if(len == 0){
		return ret;
	}
	
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + len > e->recv_bytes){
		printf("pwengine_read_str: reading outside of array bound\n");
	}
	else{
		ret = e->recv + e->read_bytes;
		e->read_bytes += len;
	}
	SDL_UnlockMutex(e->net_mutex);
	return ret;
}

void      pwengine_read_bytes(PWEngine *e, void *ptr, int n){
	SDL_LockMutex(e->net_mutex);
	if(e->read_bytes + n > e->recv_bytes){
		printf("pwengine_read_str: reading outside of array bound\n");
	}
	else{
		memcpy(ptr, e->recv + e->read_bytes, n);
		e->read_bytes += n;
	}
	SDL_UnlockMutex(e->net_mutex);
}

//void      pwengine_send(PWEngine *e, PWPacket *p);

static int socket_recv_thread(void *arg){
	PWEngine *e = (PWEngine*) arg;
	char *ptr;
	int result;
	int body_size;
	int read_bytes;
	
	while(1){
		//if there are lots of bytes in the buffer, wait for them to be read
		if(e->recv_bytes > 512){
			do{
				SDL_LockMutex(e->net_mutex);
				read_bytes = e->read_bytes;
				SDL_UnlockMutex(e->net_mutex);
			} while(read_bytes < e->recv_bytes);
			
			//make the buffer point to the array beginning
			SDL_LockMutex(e->net_mutex);
			e->recv_bytes = 0;
			e->read_bytes = 0;
			SDL_UnlockMutex(e->net_mutex);
		}	
			
		ptr = e->recv + e->recv_bytes;
		result = SDLNet_TCP_Recv(e->socket, ptr, 4);
		if(result != 4){
			break;
		}
		body_size = (int) *(unsigned short*)(ptr + 2);
		
		//recv body
		ptr += 4;
		result = SDLNet_TCP_Recv(e->socket, ptr, body_size);
		if(result != body_size){
			break;
		}
		
		
		SDL_LockMutex(e->net_mutex);
		e->recv_bytes += 4 + body_size;
		SDL_UnlockMutex(e->net_mutex);
	}
	
	pwengine_disconnect(e);
}

