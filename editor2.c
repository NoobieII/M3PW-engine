//Lithio (The developer's pseudonym)
//July 22, 2022

//Linux
//gcc -I/usr/include/SDL2 cursor.c editor2.c textbox.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -msse -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lGL -lGLU -lGLEW -lm -o editor2

//Windows
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 cursor.c editor2.c textbox.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o editor2 -static

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine/pwengine.h"
#include "engine/pwlayer.h"
#include "engine/pwrenderable.h"
#include "cursor.h"
#include "textbox.h"

//adding basic 3d shapes together

//3dview textboxes (12)
//position x
//position y
//position z
//rotation x
//rotation y
//rotation z
//translation x
//translation y
//translation z
//scale x
//scale y
//scale z

//menu textboxes
//add shape
//copy shape
//save file
//load file
//load texture
//scale UV


typedef struct UI {
	int state;
	int t_ms;
	
	//3d view state variables
	float rotx;
	float roty;
	float dist;
	PWVec3 position;
	int transform_all;
	
	//3dview HUD
	Cursor cursor_3dx;
	Cursor cursor_3dy;
	TextBox textbox_3dview[12];
	PWRenderable *sprite_3dview[12]; //references to the transformations
	PWRenderable *sprite_transform_all;
	
	//3d models
	PWRenderable *model; //all added shapes
	PWRenderable *current_shape; //all shapes already added
	
	//menu when the application starts
	Cursor cursor_menu;
	TextBox textbox_menu[6];
	PWRenderable *sprite_menu[6]; //references to the cursor options
	
	Cursor *cursor_focus;   //pressing the up/down buttons controls this cursor
	TextBox *textbox_focus; //text input controls this textbox
	
	//layers for actually rendering the stuff
	PWLayer layer_3dview;
	PWLayer layer_3dview_hud;
	PWLayer layer_menu;
} UI;

enum state{
	STATE_3DVIEW,
	STATE_3D_PX,
	STATE_3D_PY,
	STATE_3D_PZ,
	STATE_3D_RX,
	STATE_3D_RY,
	STATE_3D_RZ,
	STATE_3D_TX,
	STATE_3D_TY,
	STATE_3D_TZ,
	STATE_3D_SX,
	STATE_3D_SY,
	STATE_3D_SZ,
	STATE_MENU,
	STATE_ADD_SHAPE,
	STATE_COPY_SHAPE,
	STATE_SAVE_FILE,
	STATE_LOAD_FILE,
	STATE_LOAD_TEXTURE,
	STATE_EDIT_UV
};

void ui_init(UI *ui, PWShader *s);
void ui_update(UI *ui, PWEngine *e);
void ui_close(UI *ui);

void ui_init(UI *ui, PWShader *s){
	int i;
	PWRenderable r;
	
	ui->state = STATE_MENU;
	ui->t_ms = 0;
	ui->rotx = 0;
	ui->roty = 0;
	ui->position = PWM_vec3(0,0,0);
	ui->dist = 6.0;
	
	//the sprites below will depend on the layers being initialized
	pwlayer_init(&ui->layer_3dview, s, PWM_ini(1, NULL));
	pwlayer_init(&ui->layer_3dview_hud, s, PWM_orthographic(0, 4, 0, 3, -1, 1));
	pwlayer_init(&ui->layer_menu, s, PWM_orthographic(0, 4, 0, 3, -1, 1));
	
	cursor_init(&ui->cursor_3dx, 3, 1);
	cursor_init(&ui->cursor_3dy, 4, 1);
	//initialize all textboxes as having 0, except for the scaling
	for(i = 0; i < 9; ++i){
		textbox_init(&ui->textbox_3dview[i], 16);
		textbox_replace(&ui->textbox_3dview[i], "0");
	}
	for(i = 9; i < 12; ++i){
		textbox_init(&ui->textbox_3dview[i], 16);
		textbox_replace(&ui->textbox_3dview[i], "1");
	}
	
	//x y z colour text
	pwrenderable_init_str(&r, PWM_vec3(1, 0.4, 0), PWM_vec2(2, 0.1), "X", "FreeSans.ttf", 0xff8080ff);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.5, 0.4, 0), PWM_vec2(2, 0.1), "Y", "FreeSans.ttf", 0xff80ff80);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	pwrenderable_init_str(&r, PWM_vec3(2, 0.4, 0), PWM_vec2(2, 0.1), "Z", "FreeSans.ttf", 0xffff8080);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	
	
	pwrenderable_init_str(&r, PWM_vec3(0, 0.3, 0), PWM_vec2(2, 0.1), "Position", "FreeSans.ttf", 0xffc0c0c0);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	pwrenderable_init_str(&r, PWM_vec3(0, 0.2, 0), PWM_vec2(2, 0.1), "Rotation", "FreeSans.ttf", 0xffc0c0c0);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	pwrenderable_init_str(&r, PWM_vec3(0, 0.1, 0), PWM_vec2(2, 0.1), "Translation", "FreeSans.ttf", 0xffc0c0c0);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	pwrenderable_init_str(&r, PWM_vec3(0, 0, 0), PWM_vec2(2, 0.1), "Scale", "FreeSans.ttf", 0xffc0c0c0);
	pwlayer_add(&ui->layer_3dview_hud, &r);
	
	for(i = 0; i < 4; ++i){
		pwrenderable_init_str(&r, PWM_vec3(1.0, 0.3 - 0.1 * i, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_3dview[i*3 + 0]), "FreeSans.ttf", 0xff808080);
		ui->sprite_3dview[i*3 + 0] = pwlayer_add(&ui->layer_3dview_hud, &r);
		pwrenderable_init_str(&r, PWM_vec3(1.5, 0.3 - 0.1 * i, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_3dview[i*3 + 1]), "FreeSans.ttf", 0xff808080);
		ui->sprite_3dview[i*3 + 1] = pwlayer_add(&ui->layer_3dview_hud, &r);
		pwrenderable_init_str(&r, PWM_vec3(2.0, 0.3 - 0.1 * i, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_3dview[i*3 + 2]), "FreeSans.ttf", 0xff808080);
		ui->sprite_3dview[i*3 + 2] = pwlayer_add(&ui->layer_3dview_hud, &r);
	}
	
	pwrenderable_init_str(&r, PWM_vec3(0, 0.4, 0), PWM_vec2(2, 0.1), "TRANSFORM ALL", "FreeSans.ttf", 0x0000ff40);
	ui->sprite_transform_all = pwlayer_add(&ui->layer_3dview_hud, &r);
	
	//red green blue axes
	pwrenderable_init_box(&r, PWM_vec3(1, 0.1, 0.1), 0xff0000ff, NULL); //x
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0.5, 0, 0)), 0);
	pwlayer_add(&ui->layer_3dview, &r);
	pwlayer_end_group(&ui->layer_3dview);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 1, 0.1), 0xff00ff00, NULL); //y
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0, 0.5, 0)), 0);
	pwlayer_add(&ui->layer_3dview, &r);
	pwlayer_end_group(&ui->layer_3dview);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 0.1, 1), 0xffff0000, NULL); //z
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0, 0, 0.5)), 0);
	pwlayer_add(&ui->layer_3dview, &r);
	pwlayer_end_group(&ui->layer_3dview);
	
	//model
	pwrenderable_init_none(&r);
	ui->model = pwlayer_add(&ui->layer_3dview, &r);
	pwrenderable_init_none(&r);
	ui->current_shape = pwlayer_add(&ui->layer_3dview, &r);
	
	//menu
	cursor_init(&ui->cursor_menu, 6, 1);
	for(i = 0; i < 6; ++i){
		textbox_init(&ui->textbox_menu[i], 32);
	}
	pwrenderable_init_str(&r, PWM_vec3(1.0, 2.0, 0), PWM_vec2(2, 0.2), "Add Shape", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[0] = pwlayer_add(&ui->layer_menu, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.0, 1.8, 0), PWM_vec2(2, 0.2), "Copy Shape", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[1] = pwlayer_add(&ui->layer_menu, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.0, 1.6, 0), PWM_vec2(2, 0.2), "Save File", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[2] = pwlayer_add(&ui->layer_menu, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.0, 1.4, 0), PWM_vec2(2, 0.2), "Load File", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[3] = pwlayer_add(&ui->layer_menu, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.0, 1.2, 0), PWM_vec2(2, 0.2), "Load Texture", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[4] = pwlayer_add(&ui->layer_menu, &r);
	pwrenderable_init_str(&r, PWM_vec3(1.0, 1.0, 0), PWM_vec2(2, 0.2), "Edit UV", "FreeSans.ttf", 0xff808080);
	ui->sprite_menu[5] = pwlayer_add(&ui->layer_menu, &r);
	
	//textboxes are on the right of the options
	for(i = 0; i < 6; ++i){
		pwrenderable_init_str(&r, PWM_vec3(2.0, 2.0 - 0.2*i, 0), PWM_vec2(2, 0.2), textbox_get_str(&ui->textbox_menu[i]), "FreeSans.ttf", 0xffffffff);
		pwlayer_add(&ui->layer_menu, &r);
	}
	
	ui->cursor_focus = &ui->cursor_menu;
	ui->textbox_focus = NULL;
}

void ui_update(UI *ui, PWEngine *e){
	int i;
	int result;
	const char *input;
	char str[16];
	float f[12];
	int d[4];
	PWMat4 proj;
	PWRenderable r;
	PWTexture *texture;
	
	//handling input
	switch(ui->state){
	case STATE_3DVIEW:
		if(pwengine_is_key_pressed(e, "Left")){
			cursor_dec(&ui->cursor_3dx);
		}
		if(pwengine_is_key_pressed(e, "Right")){
			cursor_inc(&ui->cursor_3dx);
		}
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_MENU;
			ui->cursor_focus = &ui->cursor_menu;
		}
		if(pwengine_is_key_held(e, "W")){
			ui->rotx += 2;
			if(ui->rotx > 90){
				ui->rotx = 90;
			}
		}
		if(pwengine_is_key_held(e, "S")){
			ui->rotx -= 2;
			if(ui->rotx < -90){
				ui->rotx = -90;
			}
		}
		if(pwengine_is_key_held(e, "A")){
			ui->roty -= 2;
		}
		if(pwengine_is_key_held(e, "D")){
			ui->roty += 2;
		}
		if(pwengine_is_key_held(e, "Q")){
			ui->dist *= 1.01;
			if(ui->dist > 50.0){
				ui->dist = 50.0;
			}
		}
		if(pwengine_is_key_held(e, "E")){
			ui->dist *= 0.99;
			if(ui->dist < 5.0){
				ui->dist = 5.0;
			}
		}
		if(pwengine_is_key_pressed(e, "T")){
			ui->transform_all ^= 1;
			pwrenderable_set_color(ui->sprite_transform_all, (ui->transform_all ? 0xff00ff40 : 0x0000ff40) );
		}
		if(pwengine_is_key_pressed(e, "Return")){
			ui->state = STATE_3D_PX + cursor_pos(&ui->cursor_3dy) * 3 + cursor_pos(&ui->cursor_3dx);
			ui->cursor_focus = NULL;
			ui->textbox_focus = &ui->textbox_3dview[cursor_pos(&ui->cursor_3dy) * 3 + cursor_pos(&ui->cursor_3dx)];
		}
		break;
	case STATE_3D_PX:
	case STATE_3D_PY:
	case STATE_3D_PZ:
	case STATE_3D_RX:
	case STATE_3D_RY:
	case STATE_3D_RZ:
	case STATE_3D_TX:
	case STATE_3D_TY:
	case STATE_3D_TZ:
	case STATE_3D_SX:
	case STATE_3D_SY:
	case STATE_3D_SZ:
		if(pwengine_is_key_pressed(e, "Escape")){
			//revert all textboxes
			sprintf(str, "%f", ui->position.x);
			textbox_replace(&ui->textbox_3dview[0], str);
			sprintf(str, "%f", ui->position.y);
			textbox_replace(&ui->textbox_3dview[1], str);
			sprintf(str, "%f", ui->position.z);
			textbox_replace(&ui->textbox_3dview[2], str);
			
			for(i = 3; i < 9; ++i){
				textbox_replace(&ui->textbox_3dview[i], "0");
			}
			for(i = 9; i < 12; ++i){
				textbox_replace(&ui->textbox_3dview[i], "1");
			}
			
			ui->state = STATE_3DVIEW;
			ui->cursor_focus = &ui->cursor_3dy;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			//read from the textboxes
			result = 0;
			for(i = 0; i < 12; ++i){
				result += sscanf(textbox_get_str(&ui->textbox_3dview[i]), "%f", &f[i]);
				printf("%f\n", f[i]);
			}
			if(result == 12){
				proj = PWM_ini(1.0f, NULL);
				
				//check which textbox was changed
				ui->position = PWM_vec3(f[0], f[1], f[2]);
				
				if(f[3] != 0.0f){
					proj = PWM_rotation(f[3], PWM_vec3(1, 0, 0));
				}
				else if(f[4] != 0.0f){
					proj = PWM_rotation(f[4], PWM_vec3(0, 1, 0));
				}
				else if(f[5] != 0.0f){
					proj = PWM_rotation(f[5], PWM_vec3(0, 0, 1));
				}
				else if(f[6] != 0.0f || f[7] != 0.0f || f[8] != 0.0f){
					proj = PWM_translation(PWM_vec3(f[6], f[7], f[8]));
				}
				else if(f[9] != 1.0f || f[10] != 1.0f || f[11] != 1.0f){
					proj = PWM_scale(PWM_vec3(f[9], f[10], f[11]));
				}
				
				if(ui->transform_all){
					pwrenderable_transform(ui->model, proj);
				}
				pwrenderable_transform(ui->current_shape, proj);
			}
			
			//revert textboxes
			sprintf(str, "%f", ui->position.x);
			textbox_replace(&ui->textbox_3dview[0], str);
			sprintf(str, "%f", ui->position.y);
			textbox_replace(&ui->textbox_3dview[1], str);
			sprintf(str, "%f", ui->position.z);
			textbox_replace(&ui->textbox_3dview[2], str);
			
			for(i = 3; i < 9; ++i){
				textbox_replace(&ui->textbox_3dview[i], "0");
			}
			for(i = 9; i < 12; ++i){
				textbox_replace(&ui->textbox_3dview[i], "1");
			}
			
			ui->state = STATE_3DVIEW;
			ui->cursor_focus = &ui->cursor_3dy;
			ui->textbox_focus = NULL;
		}
		break;
	case STATE_MENU:
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_3DVIEW;
			ui->cursor_focus = &ui->cursor_3dy;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			ui->state = STATE_ADD_SHAPE + cursor_pos(&ui->cursor_menu);
			ui->cursor_focus = NULL;
			ui->textbox_focus = &ui->textbox_menu[cursor_pos(&ui->cursor_menu)];
		}
		break;
	case STATE_ADD_SHAPE:
	case STATE_COPY_SHAPE:
	case STATE_SAVE_FILE:
	case STATE_LOAD_FILE:
	case STATE_LOAD_TEXTURE:
	case STATE_EDIT_UV:
		if(pwengine_is_key_pressed(e, "Escape")){
			textbox_clear(ui->textbox_focus);
			ui->state = STATE_MENU;
			ui->cursor_focus = &ui->cursor_menu;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			switch(ui->state){
			case STATE_ADD_SHAPE:
				result = pwrenderable_load(&r, textbox_get_str(ui->textbox_focus));
				
				if(result == 0){
					pwrenderable_add_r(ui->model, ui->current_shape);
					pwrenderable_close(ui->current_shape);
					pwrenderable_init_none(ui->current_shape);
					pwrenderable_add_r(ui->current_shape, &r);
					pwrenderable_transform(ui->current_shape, PWM_translation(ui->position));
					pwrenderable_close(&r);
				}
				break;
			case STATE_COPY_SHAPE:
				result = 0;
				pwrenderable_add_r(ui->model, ui->current_shape);
				break;
			case STATE_SAVE_FILE:
				pwrenderable_init_none(&r);
				pwrenderable_add_r(&r, ui->model);
				pwrenderable_add_r(&r, ui->current_shape);
				result = pwrenderable_save2(&r, textbox_get_str(ui->textbox_focus));
				pwrenderable_close(&r);
				break;
			case STATE_LOAD_FILE:
				//TODO
				pwrenderable_reset(ui->model);
				pwrenderable_reset(ui->current_shape);
				pwrenderable_load(ui->model, textbox_get_str(ui->textbox_focus));
				result = 0;
				break;
			case STATE_LOAD_TEXTURE:
				//load texture from file
				texture = pwengine_get_texture(e, textbox_get_str(ui->textbox_focus));
				if(texture){
					result = 0;
					pwrenderable_set_texture(ui->model, texture);
					pwrenderable_set_texture(ui->current_shape, texture);
				}
				else{
					result = -1;
				}
				break;
			case STATE_EDIT_UV:
				result = sscanf(textbox_get_str(ui->textbox_focus), "%d %d %d %d", &d[0], &d[1], &d[2], &d[3]);
				if(result == 4){
					result = pwrenderable_transform_uv(ui->current_shape, d[0], d[1], d[2], d[3]);
				}
				break;
			}
			if(result == 0){
				textbox_clear(ui->textbox_focus);
				ui->state = STATE_3DVIEW;
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
			}
		}
		break;
	default:
		break;
	}
	
	if(ui->cursor_focus){
		if(pwengine_is_key_pressed(e, "Up")){
			cursor_dec(ui->cursor_focus);
		}
		if(pwengine_is_key_pressed(e, "Down")){
			cursor_inc(ui->cursor_focus);
		}
	}
	if(ui->textbox_focus){
		if(pwengine_is_key_pressed(e, "Backspace")){
			textbox_bks(ui->textbox_focus);
		}
		if(pwengine_text_input(e)){
			input = pwengine_text_input(e);
			for(i = 0; input[i]; ++i){
				textbox_add_char(ui->textbox_focus, input[i]);
			}
		}
	}
	
	//render
	switch(ui->state){
	case STATE_3DVIEW:
	case STATE_3D_PX:
	case STATE_3D_PY:
	case STATE_3D_PZ:
	case STATE_3D_RX:
	case STATE_3D_RY:
	case STATE_3D_RZ:
	case STATE_3D_TX:
	case STATE_3D_TY:
	case STATE_3D_TZ:
	case STATE_3D_SX:
	case STATE_3D_SY:
	case STATE_3D_SZ:
		//make the selected number lighter
		pwrenderable_set_color(ui->sprite_3dview[cursor_pos(&ui->cursor_3dy) * 3 + cursor_pos(&ui->cursor_3dx)], 0xffffffff);
		
		//calculate projection matrix
		proj = PWM_rotation(ui->roty, PWM_vec3(0, 1, 0));
		proj = PWM_mul(PWM_rotation(ui->rotx, PWM_vec3(1, 0, 0)), proj);
		proj = PWM_mul(PWM_translation(PWM_vec3(0, 0, -ui->dist)), proj);
		proj = PWM_mul(PWM_perspective(60, 4/3.0, 2, 2*ui->dist-2), proj);
		pwlayer_set_projection(&ui->layer_3dview, proj);
		
		pwlayer_render(&ui->layer_3dview);
		pwlayer_render(&ui->layer_3dview_hud);
		
		//make the selected number grey again
		pwrenderable_set_color(ui->sprite_3dview[cursor_pos(&ui->cursor_3dy) * 3 + cursor_pos(&ui->cursor_3dx)], 0xff808080);
		break;
	case STATE_MENU:
	case STATE_ADD_SHAPE:
	case STATE_COPY_SHAPE:
	case STATE_SAVE_FILE:
	case STATE_LOAD_FILE:
	case STATE_LOAD_TEXTURE:
	case STATE_EDIT_UV:
		//make the selected option lighter
		pwrenderable_set_color(ui->sprite_menu[cursor_pos(&ui->cursor_menu)], 0xffffffff);
		pwlayer_render(&ui->layer_menu);
		//make the selected option grey again
		pwrenderable_set_color(ui->sprite_menu[cursor_pos(&ui->cursor_menu)], 0xff808080);
		break;
	default:
		break;
	}
}

void ui_close(UI *ui){
	int i;
	for(i = 0; i < 12; ++i){
		textbox_close(&ui->textbox_3dview[i]);
	}
	for(i = 0; i < 6; ++i){
		textbox_close(&ui->textbox_menu[i]);
	}
	pwlayer_close(&ui->layer_3dview);
	pwlayer_close(&ui->layer_3dview_hud);
	pwlayer_close(&ui->layer_menu);
}

int main(int argc, char *argv[]){
	PWEngine *engine;
	UI *ui;
	
	engine = pwengine_init(640, 480, 30, "M3PW 3D Editor 2");
	if(!engine){
		printf("PWEngine couldn't initialize.\n");
		return 0;
	}
	ui = (UI*) malloc(sizeof(UI));
	ui_init(ui, pwengine_get_shader(engine));
	
	printf("Welcome to Ryan's 3D editor #2!\n");
	
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		ui_update(ui, engine);
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwengine_free(engine);
	ui_close(ui);
	free(ui);
	
	printf("Program closes gracefully.\n");
	
	return 0;
}