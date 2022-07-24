//Lithio (the developer's pseudonym)
//July 06, 2022

//An editor for 3d objects!

#include <math.h>

#include "engine/pwengine.h"
#include "engine/pwlayer.h"
#include "engine/pwmath.h"
#include "engine/pwrenderable.h"

#include "cursor.h"
#include "textbox.h"

//Linux
//gcc -I/usr/include/SDL2 *.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -msse -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lGL -lGLU -lGLEW -lm -o editor1

//Windows
//gcc -IC:/msys64/mingw32/include/SDL2 *.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o editor1 -static

//state: 3d view: move camera angle, select textbox, select vertex, apply transforms
//state: menu with options: add polygon, save file, load file

enum state{
	STATE_3DVIEW,
	STATE_3D_VX,
	STATE_3D_VY,
	STATE_3D_VZ,
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
	STATE_ADD_N,
	STATE_SAVE_FILE,
	STATE_LOAD_FILE,
	STATE_LOAD_TEXTURE
};

typedef struct editor_interface{
	int state;
	int t_ms;
	
	//3d view state variables
	float rotx;
	float roty;
	float dist;
	PWMat4 proj;
	
	int no_vertices;     //nonzero when the renderable is empty
	int vertex_current;  //currently selected vertex
	unsigned int vertex_current_color; //color of currently selected vertex
	int polygon_size;
	int vertex_begin;    //index number of the first vertex in the polygon
	
	int vertex_copy;     //existing vertex whose value is copied
	unsigned int vertex_copy_color;
	
	int transform_all;
	int sound_playing;
	
	Cursor cursor_3dx;
	Cursor cursor_3dy;
	TextBox textbox_vx;
	TextBox textbox_vy;
	TextBox textbox_vz;
	TextBox textbox_rx;
	TextBox textbox_ry;
	TextBox textbox_rz;
	TextBox textbox_tx;
	TextBox textbox_ty;
	TextBox textbox_tz;
	TextBox textbox_sx;
	TextBox textbox_sy;
	TextBox textbox_sz;
	PWRenderable *sprites_3dview[20];
	char vertex_str[16];
	
	//menu with options
	Cursor cursor_menu;
	TextBox textbox_add_n;
	TextBox textbox_save_file;
	TextBox textbox_load_file;
	TextBox textbox_load_texture;
	PWRenderable *sprites_menu[9];
	
	Cursor *cursor_focus;
	TextBox *textbox_focus;
	PWRenderable *r;     //reference to the renderable object
	
	PWLayer layer_3dview;
	PWLayer layer_3dview_hud;
	PWLayer layer_menu;
} UI;

void ui_init(UI *ui, PWShader *s){
	PWRenderable rr;
	pwlayer_init(&ui->layer_3dview, s, PWM_ini(1, NULL));
	pwlayer_init(&ui->layer_3dview_hud, s, PWM_orthographic(0, 4, 0, 3, -1, 1));
	pwlayer_init(&ui->layer_menu, s, PWM_orthographic(0, 4, 0, 3, -1, 1));
	
	ui->state = STATE_MENU;
	ui->t_ms = 0;
	
	ui->rotx = 0;
	ui->roty = 0;
	ui->dist = 6.0;
	ui->no_vertices = 1;
	ui->vertex_current = 0;
	ui->vertex_current_color = 0;
	ui->polygon_size = 0;
	ui->vertex_begin = 0;
	ui->vertex_copy = 0;
	ui->vertex_copy_color = 0;
	ui->transform_all = 0;
	ui->sound_playing = -1;
	
	//create the arrows
	pwrenderable_init_box(&rr, PWM_vec3(1, 0.1, 0.1), 0xff0000ff, NULL); //x
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0.5, 0, 0)), 0);
	pwlayer_add(&ui->layer_3dview, &rr);
	pwlayer_end_group(&ui->layer_3dview);
	pwrenderable_init_box(&rr, PWM_vec3(0.1, 1, 0.1), 0xff00ff00, NULL); //y
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0, 0.5, 0)), 0);
	pwlayer_add(&ui->layer_3dview, &rr);
	pwlayer_end_group(&ui->layer_3dview);
	pwrenderable_init_box(&rr, PWM_vec3(0.1, 0.1, 1), 0xffff0000, NULL); //z
	pwlayer_add_group(&ui->layer_3dview, PWM_translation(PWM_vec3(0, 0, 0.5)), 0);
	pwlayer_add(&ui->layer_3dview, &rr);
	pwlayer_end_group(&ui->layer_3dview);
	
	
	cursor_init(&ui->cursor_3dx, 3, 1);
	cursor_init(&ui->cursor_3dy, 4, 1);
	textbox_init(&ui->textbox_vx, 16);
	textbox_replace(&ui->textbox_vx, "0");
	textbox_init(&ui->textbox_vy, 16);
	textbox_replace(&ui->textbox_vy, "0");
	textbox_init(&ui->textbox_vz, 16);
	textbox_replace(&ui->textbox_vz, "0");
	textbox_init(&ui->textbox_rx, 16);
	textbox_replace(&ui->textbox_rx, "0");
	textbox_init(&ui->textbox_ry, 16);
	textbox_replace(&ui->textbox_ry, "0");
	textbox_init(&ui->textbox_rz, 16);
	textbox_replace(&ui->textbox_rz, "0");
	textbox_init(&ui->textbox_tx, 16);
	textbox_replace(&ui->textbox_tx, "0");
	textbox_init(&ui->textbox_ty, 16);
	textbox_replace(&ui->textbox_ty, "0");
	textbox_init(&ui->textbox_tz, 16);
	textbox_replace(&ui->textbox_tz, "0");
	textbox_init(&ui->textbox_sx, 16);
	textbox_replace(&ui->textbox_sx, "1");
	textbox_init(&ui->textbox_sy, 16);
	textbox_replace(&ui->textbox_sy, "1");
	textbox_init(&ui->textbox_sz, 16);
	textbox_replace(&ui->textbox_sz, "1");
	
	//3dview hud
	pwrenderable_init_str(&rr, PWM_vec3(1, 0.4, 0), PWM_vec2(2, 0.1), "X", "FreeSans.ttf", 0xff8080ff);
	ui->sprites_3dview[0] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.5, 0.4, 0), PWM_vec2(2, 0.1), "Y", "FreeSans.ttf", 0xff80ff80);
	ui->sprites_3dview[1] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2, 0.4, 0), PWM_vec2(2, 0.1), "Z", "FreeSans.ttf", 0xffff8080);
	ui->sprites_3dview[2] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(0, 0.3, 0), PWM_vec2(2, 0.1), ui->vertex_str, "FreeSans.ttf", 0xffc0c0c0);
	ui->sprites_3dview[3] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(0, 0.2, 0), PWM_vec2(2, 0.1), "Rotation", "FreeSans.ttf", 0xffc0c0c0);
	ui->sprites_3dview[4] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(0, 0.1, 0), PWM_vec2(2, 0.1), "Translation", "FreeSans.ttf", 0xffc0c0c0);
	ui->sprites_3dview[5] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(0, 0, 0), PWM_vec2(2, 0.1), "Scale", "FreeSans.ttf", 0xffc0c0c0);
	ui->sprites_3dview[6] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	
	pwrenderable_init_str(&rr, PWM_vec3(1, 0.3, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_vx), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[7] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.5, 0.3, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_vy), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[8] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2, 0.3, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_vz), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[9] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1, 0.2, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_rx), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[10] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.5, 0.2, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_ry), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[11] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2, 0.2, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_rz), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[12] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1, 0.1, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_tx), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[13] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.5, 0.1, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_ty), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[14] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2, 0.1, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_tz), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[15] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1, 0, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_sx), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[16] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.5, 0, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_sy), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[17] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2, 0, 0), PWM_vec2(2, 0.1), textbox_get_str(&ui->textbox_sz), "FreeSans.ttf", 0xff808080);
	ui->sprites_3dview[18] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(0, 0.4, 0), PWM_vec2(2, 0.1), "TRANSFORM ALL", "FreeSans.ttf", 0x0000ff40);
	ui->sprites_3dview[19] = pwlayer_add(&ui->layer_3dview_hud, &rr);
	
	cursor_init(&ui->cursor_menu, 5, 1);
	textbox_init(&ui->textbox_add_n, 4);
	textbox_init(&ui->textbox_save_file, 32);
	textbox_init(&ui->textbox_load_file, 32);
	textbox_init(&ui->textbox_load_texture, 32);
	
	//menu sprites
	pwrenderable_init_str(&rr, PWM_vec3(1.0, 2, 0), PWM_vec2(2, 0.2), "Add n-sided", "FreeSans.ttf", 0xffff8080);
	ui->sprites_menu[0] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.0, 1.8, 0), PWM_vec2(2, 0.2), "Save file", "FreeSans.ttf", 0xffff8080);
	ui->sprites_menu[1] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.0, 1.6, 0), PWM_vec2(2, 0.2), "Load file", "FreeSans.ttf", 0xffff8080);
	ui->sprites_menu[2] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.0, 1.4, 0), PWM_vec2(2, 0.2), "Load texture", "FreeSans.ttf", 0xffff8080);
	ui->sprites_menu[3] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(1.0, 1.2, 0), PWM_vec2(2, 0.2), "Copy shape", "FreeSans.ttf", 0xffff8080);
	ui->sprites_menu[4] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2.0, 2, 0), PWM_vec2(2, 0.2), textbox_get_str(&ui->textbox_add_n), "FreeSans.ttf", 0xffffffff);
	ui->sprites_menu[5] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2.0, 1.8, 0), PWM_vec2(2, 0.2), textbox_get_str(&ui->textbox_save_file), "FreeSans.ttf", 0xffffffff);
	ui->sprites_menu[6] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2.0, 1.6, 0), PWM_vec2(2, 0.2), textbox_get_str(&ui->textbox_load_file), "FreeSans.ttf", 0xffffffff);
	ui->sprites_menu[7] = pwlayer_add(&ui->layer_menu, &rr);
	pwrenderable_init_str(&rr, PWM_vec3(2.0, 1.4, 0), PWM_vec2(2, 0.2), textbox_get_str(&ui->textbox_load_texture), "FreeSans.ttf", 0xffffffff);
	ui->sprites_menu[8] = pwlayer_add(&ui->layer_menu, &rr);
	
	ui->cursor_focus = &ui->cursor_menu;
	ui->textbox_focus = NULL;
	
	pwrenderable_init_none(&rr);
	ui->r = pwlayer_add(&ui->layer_3dview, &rr);
}

void ui_revert_current_color(UI *ui){
	PWVec3 position;
	PWVec2 uv;
	PWVec3 normal;
	
	//undo the color
	pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, &uv, NULL, &normal);
	pwrenderable_edit_vertex(ui->r, ui->vertex_current, position, uv, ui->vertex_current_color, normal);
}

//use the text in the textbox - returns 0 if successful, -1 if not
int ui_save_file(UI *ui){
	PWVec3 position;
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal, v0, v1, v2;
	FILE *out;
	int i;
	
	ui_revert_current_color(ui);
	if(pwrenderable_save(ui->r, textbox_get_str(&ui->textbox_save_file), 1) == -1){
		return -1;
	};
	
	/*
	//don't actually count the vertices, but check the shapes
	int vertex = 0;
	int index = 0;
	int polygon_size = 2;
	
	out = fopen(textbox_get_str(&ui->textbox_save_file), "wt");
	if(!out){
		return -1;
	}
	
	ui_revert_current_color(ui);
	
	while(index < ui->r->index_count){
		//it's part of the shape
		if(ui->r->indices[index] == vertex){
			index += 3;
			polygon_size++;
		}
		else{
			//save the shape to file
			//number of sides
			fprintf(out, "%d\n", polygon_size);
			
			//TODO: calculate normal
			pwrenderable_get_vertex(ui->r, vertex, &v0, NULL, NULL, NULL);
			pwrenderable_get_vertex(ui->r, vertex + 1, &v1, NULL, NULL, NULL);
			pwrenderable_get_vertex(ui->r, vertex + 2, &v2, NULL, NULL, NULL);
			normal = PWM_normalize3(PWM_cross3(PWM_sub3(v1, v0), PWM_sub3(v2, v0)));
			
			for(i = 0; i < polygon_size; ++i){
				pwrenderable_get_vertex(ui->r, vertex + i, &position, &uv, &color, NULL);
				fprintf(out, "position %f %f %f uv %f %f color %x normal %f %f %f\n", position.x, position.y, position.z, uv.x, uv.y, color, normal.x, normal.y, normal.z);
			}
			fprintf(out, "\n");
			
			vertex += polygon_size;
			polygon_size = 2;
		}
	}
	//save the last shape to file
	fprintf(out, "%d\n", polygon_size);
	for(i = 0; i < polygon_size; ++i){
		pwrenderable_get_vertex(ui->r, vertex + i, &position, &uv, &color, &normal);
		
		//TODO: calculate normal
		fprintf(out, "position %f %f %f uv %f %f color %x normal %f %f %f\n", position.x, position.y, position.z, uv.x, uv.y, color, normal.x, normal.y, normal.z);
	}
	fprintf(out, "\n");
	
	fclose(out);
	*/
	return 0;
}

//rotate the UV values of the current polygon
void ui_rotate_uv(UI *ui){
	int i;
	PWVec3 position;
	PWVec2 uv;
	PWVec2 uv_tmp;
	unsigned int color;
	PWVec3 normal;
	
	//store the UV of the first vertex in the polygon
	pwrenderable_get_vertex(ui->r, ui->vertex_begin, NULL, &uv_tmp, NULL, NULL);
	for(i = 0; i < ui->polygon_size - 1; ++i){
		pwrenderable_get_vertex(ui->r, ui->vertex_begin + i, &position, NULL, &color, &normal);
		pwrenderable_get_vertex(ui->r, ui->vertex_begin + i + 1, NULL, &uv, NULL, NULL);
		pwrenderable_edit_vertex(ui->r, ui->vertex_begin + i, position, uv, color, normal);
	}
	//the last vertex gets the first vertex UV
	pwrenderable_get_vertex(ui->r, ui->vertex_begin + i, &position, NULL, &color, &normal);
	pwrenderable_edit_vertex(ui->r, ui->vertex_begin + i, position, uv_tmp, color, normal);
}

int ui_load_file(UI *ui){
	PWVec3 position;
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal;
	FILE *in;
	int i;
	
	//don't actually count the vertices, but check the shapes
	int vertex = 0;
	int polygon_size = 0;
	
	in = fopen(textbox_get_str(&ui->textbox_load_file), "rt");
	if(!in){
		return -1;
	}
	
	pwrenderable_close(ui->r);
	pwrenderable_init_none(ui->r);
	
	while(1){
		fscanf(in, "%d", &polygon_size);
		if(feof(in)){
			break;
		}
		pwrenderable_add(ui->r, polygon_size);
		
		//read the vertices
		for(i = 0; i < polygon_size; ++i){
			fscanf(in, "%*s %f%f%f %*s %f%f %*s %x %*s %f%f%f", &position.x, &position.y, &position.z, &uv.x, &uv.y, &color, &normal.x, &normal.y, &normal.z);
			pwrenderable_edit_vertex(ui->r, vertex + i, position, uv, color, normal);
		}
		vertex += i;
	}
	
	//initialize state variables
	ui->rotx = 0;
	ui->roty = 0;
	ui->dist = 6.0;
	ui->no_vertices = vertex == 0;
	if(vertex == 0){
		ui->vertex_current = 0;
		ui->vertex_current_color = 0;
		ui->polygon_size = 0;
		ui->vertex_begin = 0;
		ui->vertex_copy = 0;
		ui->vertex_copy_color = 0;
		ui->transform_all = 0;
	}
	else{
		ui->vertex_current = vertex - 1;  //currently selected vertex
		ui->vertex_current_color = color; //color of currently selected vertex
		ui->polygon_size = polygon_size;
		ui->vertex_begin = vertex - polygon_size;    //index number of the first vertex in the polygon
		
		if(ui->vertex_begin > 0){
			ui->vertex_copy = 0;     //existing vertex whose value is copied
			pwrenderable_get_vertex(ui->r, ui->vertex_copy, NULL, NULL, &ui->vertex_copy_color, NULL);
		}
		else{
			ui->vertex_copy = 0;
			ui->vertex_copy_color = 0;
		}
	}
	return 0;
}
	

void ui_handle_input(UI *ui, PWEngine *e){
	PWMat4 proj;
	PWVec3 position, rotation, translation, scale;
	PWMat4 rotation_matrix, translation_matrix, scale_matrix;
	char str[32];
	//set the position
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal;
	const char *input;
	int i;
	PWTexture *texture;
	
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
			pwrenderable_set_color(ui->sprites_3dview[19], (ui->transform_all ? 0xff00ff40 : 0x0000ff40) );
		}
		if(pwengine_is_key_pressed(e, "P")){
			if(ui->sound_playing == -1){
				ui->sound_playing = pwengine_play_sound(e, "SHOP.ogg");
			}
			else{
				pwengine_stop_sound(e, ui->sound_playing);
				ui->sound_playing = -1;
			}
		}
		
		if(!ui->no_vertices){
			if(pwengine_is_key_pressed(e, "Tab")){
				if(ui->vertex_begin > 1){
					ui->vertex_copy++;
					if(ui->vertex_copy >= ui->vertex_begin){
						ui->vertex_copy = 0;
					}
					
					//copy the position to the current vertex
					pwrenderable_get_vertex(ui->r, ui->vertex_copy, &position, NULL, &ui->vertex_copy_color, NULL);
					pwrenderable_get_vertex(ui->r, ui->vertex_current, NULL, &uv, NULL, &normal);
					pwrenderable_edit_vertex(ui->r, ui->vertex_current, position, uv, ui->vertex_current_color, normal);
					
					//copy the position to the textbox
					sprintf(str, "%.6f", position.x);
					textbox_replace(&ui->textbox_vx, str);
					sprintf(str, "%.6f", position.y);
					textbox_replace(&ui->textbox_vy, str);
					sprintf(str, "%.6f", position.z);
					textbox_replace(&ui->textbox_vz, str);
				}
			}
			if(pwengine_is_key_pressed(e, "Left Shift") || pwengine_is_key_pressed(e, "Right Shift")){
				if(ui->vertex_begin > 1){
					ui->vertex_copy--;
					if(ui->vertex_copy < 0){
						ui->vertex_copy = ui->vertex_begin - 1;
					}
					
					//copy the position to the current vertex
					pwrenderable_get_vertex(ui->r, ui->vertex_copy, &position, NULL, &ui->vertex_copy_color, NULL);
					pwrenderable_get_vertex(ui->r, ui->vertex_current, NULL, &uv, NULL, &normal);
					pwrenderable_edit_vertex(ui->r, ui->vertex_current, position, uv, ui->vertex_current_color, normal);
					
					//copy the position to the textbox
					sprintf(str, "%.6f", position.x);
					textbox_replace(&ui->textbox_vx, str);
					sprintf(str, "%.6f", position.y);
					textbox_replace(&ui->textbox_vy, str);
					sprintf(str, "%.6f", position.z);
					textbox_replace(&ui->textbox_vz, str);
				}
			}
			if(pwengine_is_key_pressed(e, "PageUp")){
				ui_revert_current_color(ui);
				
				//increment selected vertex of the polygon
				ui->vertex_current++;
				if(ui->vertex_current >= ui->vertex_begin + ui->polygon_size){
					ui->vertex_current = ui->vertex_begin;
				}
				
				//copy the position to the textbox
				pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, &ui->vertex_current_color, NULL);
				
				//make position textbox position of vertex
				sprintf(str, "%.6f", position.x);
				textbox_replace(&ui->textbox_vx, str);
				sprintf(str, "%.6f", position.y);
				textbox_replace(&ui->textbox_vy, str);
				sprintf(str, "%.6f", position.z);
				textbox_replace(&ui->textbox_vz, str);
			}
			if(pwengine_is_key_pressed(e, "PageDown")){
				ui_revert_current_color(ui);
				
				ui->vertex_current--;
				if(ui->vertex_current < ui->vertex_begin){
					ui->vertex_current = ui->vertex_begin + ui->polygon_size - 1;
				}
				
				//copy the position to the textbox
				pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, &ui->vertex_current_color, NULL);
				
				//make position textbox position of vertex
				sprintf(str, "%.6f", position.x);
				textbox_replace(&ui->textbox_vx, str);
				sprintf(str, "%.6f", position.y);
				textbox_replace(&ui->textbox_vy, str);
				sprintf(str, "%.6f", position.z);
				textbox_replace(&ui->textbox_vz, str);
			}
			if(pwengine_is_key_pressed(e, "R")){
				ui_rotate_uv(ui);
			}
			if(pwengine_is_key_pressed(e, "Return")){
				switch(3 * cursor_pos(&ui->cursor_3dy) + cursor_pos(&ui->cursor_3dx)){
				case 0:
					ui->textbox_focus = &ui->textbox_vx;
					ui->state = STATE_3D_VX;
					break;
				case 1:
					ui->textbox_focus = &ui->textbox_vy;
					ui->state = STATE_3D_VY;
					break;
				case 2:
					ui->textbox_focus = &ui->textbox_vz;
					ui->state = STATE_3D_VZ;
					break;
				case 3:
					ui->textbox_focus = &ui->textbox_rx;
					ui->state = STATE_3D_RX;
					break;
				case 4:
					ui->textbox_focus = &ui->textbox_ry;
					ui->state = STATE_3D_RY;
					break;
				case 5:
					ui->textbox_focus = &ui->textbox_rz;
					ui->state = STATE_3D_RZ;
					break;
				case 6:
					ui->textbox_focus = &ui->textbox_tx;
					ui->state = STATE_3D_TX;
					break;
				case 7:
					ui->textbox_focus = &ui->textbox_ty;
					ui->state = STATE_3D_TY;
					break;
				case 8:
					ui->textbox_focus = &ui->textbox_tz;
					ui->state = STATE_3D_TZ;
					break;
				case 9:
					ui->textbox_focus = &ui->textbox_sx;
					ui->state = STATE_3D_SX;
					break;
				case 10:
					ui->textbox_focus = &ui->textbox_sy;
					ui->state = STATE_3D_SY;
					break;
				case 11:
					ui->textbox_focus = &ui->textbox_sz;
					ui->state = STATE_3D_SZ;
					break;
				}
				
				ui->cursor_focus = NULL;
			}
		}
		
		//make the selected option white, the rest 0xff808080
		for(i = 0; i < 12; ++i){
			pwrenderable_set_color(ui->sprites_3dview[7+i], 0xff808080);
		}
		pwrenderable_set_color(ui->sprites_3dview[7 + 3 * cursor_pos(&ui->cursor_3dy) + cursor_pos(&ui->cursor_3dx)], 0xffffffff);
		break;
	case STATE_3D_VX:
	case STATE_3D_VY:
	case STATE_3D_VZ:
		if(pwengine_is_key_pressed(e, "Escape")){
			//revert the textbox to the vertex position
			pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, NULL, NULL);
			
			//make position textbox position of vertex
			sprintf(str, "%.6f", position.x);
			textbox_replace(&ui->textbox_vx, str);
			sprintf(str, "%.6f", position.y);
			textbox_replace(&ui->textbox_vy, str);
			sprintf(str, "%.6f", position.z);
			textbox_replace(&ui->textbox_vz, str);
			
			ui->cursor_focus = &ui->cursor_3dy;
			ui->textbox_focus = NULL;
			ui->state = STATE_3DVIEW;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			i = 0;
			i += sscanf(textbox_get_str(&ui->textbox_vx), "%f", &position.x);
			i += sscanf(textbox_get_str(&ui->textbox_vy), "%f", &position.y);
			i += sscanf(textbox_get_str(&ui->textbox_vz), "%f", &position.z);
			
			if(i == 3){
				//modify the vertex position
				pwrenderable_get_vertex(ui->r, ui->vertex_current, NULL, &uv, NULL, &normal);
				pwrenderable_edit_vertex(ui->r, ui->vertex_current, position, uv, ui->vertex_current_color, normal);
				
				//make position textbox position of vertex
				sprintf(str, "%.6f", position.x);
				textbox_replace(&ui->textbox_vx, str);
				sprintf(str, "%.6f", position.y);
				textbox_replace(&ui->textbox_vy, str);
				sprintf(str, "%.6f", position.z);
				textbox_replace(&ui->textbox_vz, str);
				
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
				ui->state = STATE_3DVIEW;
			}
			else{
				//do nothing
			}
		}
		break;
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
			ui->state = STATE_3DVIEW;
			
			//make all textboxes 0, except scale
			textbox_replace(&ui->textbox_rx, "0");
			textbox_replace(&ui->textbox_ry, "0");
			textbox_replace(&ui->textbox_rz, "0");
			textbox_replace(&ui->textbox_tx, "0");
			textbox_replace(&ui->textbox_ty, "0");
			textbox_replace(&ui->textbox_tz, "0");
			textbox_replace(&ui->textbox_sx, "1");
			textbox_replace(&ui->textbox_sy, "1");
			textbox_replace(&ui->textbox_sz, "1");
			
			ui->cursor_focus = &ui->cursor_3dy;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			
			//read number from all textboxes
			i = 0;
			i += sscanf(textbox_get_str(&ui->textbox_rx), "%f", &rotation.x);
			i += sscanf(textbox_get_str(&ui->textbox_ry), "%f", &rotation.y);
			i += sscanf(textbox_get_str(&ui->textbox_rz), "%f", &rotation.z);
			i += sscanf(textbox_get_str(&ui->textbox_tx), "%f", &translation.x);
			i += sscanf(textbox_get_str(&ui->textbox_ty), "%f", &translation.y);
			i += sscanf(textbox_get_str(&ui->textbox_tz), "%f", &translation.z);
			i += sscanf(textbox_get_str(&ui->textbox_sx), "%f", &scale.x);
			i += sscanf(textbox_get_str(&ui->textbox_sy), "%f", &scale.y);
			i += sscanf(textbox_get_str(&ui->textbox_sz), "%f", &scale.z);
			
			//if all the inputs are read correctly
			if(i == 9){
				rotation_matrix = PWM_rotation(rotation.x, PWM_vec3(1, 0, 0));
				rotation_matrix = PWM_mul(PWM_rotation(rotation.y, PWM_vec3(0, 1, 0)), rotation_matrix);
				rotation_matrix = PWM_mul(PWM_rotation(rotation.z, PWM_vec3(0, 0, 1)), rotation_matrix);
				
				translation_matrix = PWM_translation(translation);
				
				scale_matrix = PWM_scale(scale);
				
				position = PWM_mul_vec3(rotation_matrix, position);
				position = PWM_mul_vec3(translation_matrix, position);
				position = PWM_mul_vec3(scale_matrix, position);
				
				//TODO: round to the nearest 1/64th
				
				
				//set the position for all vertices or just current shape
				ui_revert_current_color(ui);
				if(ui->transform_all){
					for(i = 0; i < ui->vertex_begin + ui->polygon_size; ++i){
						pwrenderable_get_vertex(ui->r, i, &position, &uv, &color, &normal);
						
						position = PWM_mul_vec3(rotation_matrix, position);
						position = PWM_mul_vec3(translation_matrix, position);
						position = PWM_mul_vec3(scale_matrix, position);
						
						pwrenderable_edit_vertex(ui->r, i, position, uv, color, normal);
					}
				}
				else{
					for(i = 0; i < ui->polygon_size; ++i){
						pwrenderable_get_vertex(ui->r, ui->vertex_begin + i, &position, &uv, &color, &normal);
						
						position = PWM_mul_vec3(rotation_matrix, position);
						position = PWM_mul_vec3(translation_matrix, position);
						position = PWM_mul_vec3(scale_matrix, position);
						
						pwrenderable_edit_vertex(ui->r, ui->vertex_begin + i, position, uv, color, normal);
					}
				}
				
				pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, NULL, NULL);
				
				//make position textbox position of vertex
				sprintf(str, "%.6f", position.x);
				textbox_replace(&ui->textbox_vx, str);
				sprintf(str, "%.6f", position.y);
				textbox_replace(&ui->textbox_vy, str);
				sprintf(str, "%.6f", position.z);
				textbox_replace(&ui->textbox_vz, str);
				
				//make all textboxes 0, except scale
				textbox_replace(&ui->textbox_rx, "0");
				textbox_replace(&ui->textbox_ry, "0");
				textbox_replace(&ui->textbox_rz, "0");
				textbox_replace(&ui->textbox_tx, "0");
				textbox_replace(&ui->textbox_ty, "0");
				textbox_replace(&ui->textbox_tz, "0");
				textbox_replace(&ui->textbox_sx, "1");
				textbox_replace(&ui->textbox_sy, "1");
				textbox_replace(&ui->textbox_sz, "1");
				
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
				ui->state = STATE_3DVIEW;
			}
			else{
				//do nothing
			}
		}
		
		
		break;
	case STATE_MENU:
		if(pwengine_is_key_pressed(e, "Return")){
			switch(cursor_pos(&ui->cursor_menu)){
			case 0:
				ui->state = STATE_ADD_N;
				ui->cursor_focus = NULL;
				ui->textbox_focus = &ui->textbox_add_n;
				break;
			case 1:
				ui->state = STATE_SAVE_FILE;
				ui->cursor_focus = NULL;
				ui->textbox_focus = &ui->textbox_save_file;
				break;
			case 2:
				ui->state = STATE_LOAD_FILE;
				ui->cursor_focus = NULL;
				ui->textbox_focus = &ui->textbox_load_file;
				break;
			case 3:
				//load texture
				ui->state = STATE_LOAD_TEXTURE;
				ui->cursor_focus = NULL;
				ui->textbox_focus = &ui->textbox_load_texture;
				break;
			case 4:
				//copy shape
				if(!ui->no_vertices){
					ui_revert_current_color(ui);
					
					pwrenderable_add(ui->r, ui->polygon_size);
					
					//set the state variables
					ui->vertex_begin += ui->polygon_size;
					pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, &uv, NULL, &normal);
					ui->vertex_current = ui->vertex_begin;
					pwrenderable_get_vertex(ui->r, ui->vertex_current, NULL, NULL, &ui->vertex_current_color, NULL);
					ui->vertex_copy = ui->vertex_begin - 1;
					pwrenderable_get_vertex(ui->r, ui->vertex_copy, NULL, NULL, &ui->vertex_copy_color, NULL);
					
					//copy the vertices from the previous polygon
					for(i = 0; i < ui->polygon_size; ++i){
						pwrenderable_get_vertex(ui->r, ui->vertex_begin - ui->polygon_size + i, &position, &uv, &color, &normal);
						pwrenderable_edit_vertex(ui->r, ui->vertex_begin + i, position, uv, color, normal);
					}
					
					//make position textbox position of vertex
					pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, NULL, NULL);
					sprintf(str, "%.6f", position.x);
					textbox_replace(&ui->textbox_vx, str);
					sprintf(str, "%.6f", position.y);
					textbox_replace(&ui->textbox_vy, str);
					sprintf(str, "%.6f", position.z);
					textbox_replace(&ui->textbox_vz, str);
				}
				
				ui->state = STATE_3DVIEW;
				ui->cursor_focus = &ui->cursor_3dy;
				break;
			default:
				break;
			}
		}
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_3DVIEW;
			ui->cursor_focus = &ui->cursor_3dy;
		}
		
		//make the selected option white, the rest 0xffff8080
		for(i = 0; i < 5; ++i){
			pwrenderable_set_color(ui->sprites_menu[i], 0xff808080);
		}
		pwrenderable_set_color(ui->sprites_menu[cursor_pos(&ui->cursor_menu)], 0xffffffff);
		
		break;
	case STATE_ADD_N:
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_MENU;
			textbox_clear(&ui->textbox_add_n);
			ui->cursor_focus = &ui->cursor_menu;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			ui->state = STATE_MENU;
			if(sscanf(textbox_get_str(&ui->textbox_add_n), "%d", &i) == 1){
				if(i < 3 || i > 8){
					//do nothing
				}
				else{
					//create the polygon
					pwrenderable_add(ui->r, i);
					if(!ui->no_vertices){
						ui_revert_current_color(ui);
					}
					
					ui->no_vertices = 0;
					ui->vertex_begin += ui->polygon_size;
					pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, &uv, NULL, &normal);
					ui->vertex_current = ui->vertex_begin;
					pwrenderable_get_vertex(ui->r, ui->vertex_current, NULL, NULL, &ui->vertex_current_color, NULL);
					ui->vertex_copy = ui->vertex_begin - 1;
					pwrenderable_get_vertex(ui->r, ui->vertex_copy, NULL, NULL, &ui->vertex_copy_color, NULL);
					ui->polygon_size = i;
					
					//make position textbox position of vertex
					pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, NULL, NULL, NULL);
					sprintf(str, "%.6f", position.x);
					textbox_replace(&ui->textbox_vx, str);
					sprintf(str, "%.6f", position.y);
					textbox_replace(&ui->textbox_vy, str);
					sprintf(str, "%.6f", position.z);
					textbox_replace(&ui->textbox_vz, str);
					
					ui->state = STATE_3DVIEW;
					ui->cursor_focus = &ui->cursor_3dy;
				}
			}
			else{
				ui->cursor_focus = &ui->cursor_menu;
			}
			textbox_clear(&ui->textbox_add_n);
			ui->textbox_focus = NULL;
		}
		break;
	case STATE_SAVE_FILE:
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_MENU;
			textbox_clear(&ui->textbox_save_file);
			ui->cursor_focus = &ui->cursor_menu;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			if(ui_save_file(ui) == 0){
				textbox_clear(&ui->textbox_save_file);
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
				ui->state = STATE_3DVIEW;
			}
		}
		break;
	case STATE_LOAD_FILE:
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_MENU;
			textbox_clear(&ui->textbox_load_file);
			ui->cursor_focus = &ui->cursor_menu;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			if(ui_load_file(ui) == 0){
				textbox_clear(&ui->textbox_load_file);
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
				ui->state = STATE_3DVIEW;
			}
		}
		break;
	case STATE_LOAD_TEXTURE:
		if(pwengine_is_key_pressed(e, "Escape")){
			ui->state = STATE_MENU;
			textbox_clear(&ui->textbox_load_texture);
			ui->cursor_focus = &ui->cursor_menu;
			ui->textbox_focus = NULL;
		}
		if(pwengine_is_key_pressed(e, "Return")){
			//TODO load a texture from the engine
			texture = pwengine_get_texture(e, textbox_get_str(&ui->textbox_load_texture));
			if(texture){
				pwrenderable_set_texture(ui->r, texture);
				textbox_clear(&ui->textbox_load_texture);
				ui->cursor_focus = &ui->cursor_3dy;
				ui->textbox_focus = NULL;
				ui->state = STATE_3DVIEW;
			}
		}
		break;
	default:
		break;
	}
	
	//projection matrix
	switch(ui->state){
	case STATE_3DVIEW:
	case STATE_3D_VX:
	case STATE_3D_VY:
	case STATE_3D_VZ:
	case STATE_3D_RX:
	case STATE_3D_RY:
	case STATE_3D_RZ:
	case STATE_3D_TX:
	case STATE_3D_TY:
	case STATE_3D_TZ:
	case STATE_3D_SX:
	case STATE_3D_SY:
	case STATE_3D_SZ:
		proj = PWM_rotation(ui->roty, PWM_vec3(0, 1, 0));
		proj = PWM_mul(PWM_rotation(ui->rotx, PWM_vec3(1, 0, 0)), proj);
		proj = PWM_mul(PWM_translation(PWM_vec3(0, 0, -ui->dist)), proj);
		proj = PWM_mul(PWM_perspective(60, 4/3.0, 2, 2*ui->dist-2), proj);
		pwlayer_set_projection(&ui->layer_3dview, proj);
		break;
	default:
		break;
	}
	
	ui->t_ms += pwengine_deltatime(e);
	
	//rendering
	switch(ui->state){
	case STATE_3DVIEW:
	case STATE_3D_VX:
	case STATE_3D_VY:
	case STATE_3D_VZ:
	case STATE_3D_RX:
	case STATE_3D_RY:
	case STATE_3D_RZ:
	case STATE_3D_TX:
	case STATE_3D_TY:
	case STATE_3D_TZ:
	case STATE_3D_SX:
	case STATE_3D_SY:
	case STATE_3D_SZ:
		//make the selected vertex have a flashing color
		pwrenderable_get_vertex(ui->r, ui->vertex_current, &position, &uv, NULL, &normal);
		color = (unsigned int)((1.1 + 0.9*sin(ui->t_ms / 250.0)) * 0.5 * ((float)(ui->vertex_current_color & 0xff000000)));
		color = color | (ui->vertex_current_color & 0x00ffffff);
		pwrenderable_edit_vertex(ui->r, ui->vertex_current, position, uv, color, normal);
		
		sprintf(ui->vertex_str, "Vertex %d / %d", ui->vertex_current - ui->vertex_begin, ui->polygon_size);
		pwlayer_render(&ui->layer_3dview);
		pwlayer_render(&ui->layer_3dview_hud);
		break;
	case STATE_MENU:
	case STATE_ADD_N:
	case STATE_SAVE_FILE:
	case STATE_LOAD_FILE:
	case STATE_LOAD_TEXTURE:
		pwlayer_render(&ui->layer_menu);
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
}

int main(int argc, char *argv[]){
	PWEngine *engine;
	UI ui;
	
	engine = pwengine_init(640, 480, 30, "M3PW 3D Editor 1");
	if(!engine){
		printf("PWEngine couldn't initialize.\n");
		return 0;
	}
	
	printf("Welcome to Ryan's 3D editing program.\n");
	printf("The first screen is the menu. Press Escape to toggle between the menu and the\n");
	printf("editor.\n");
	printf("In the menu, press Enter to select one options and type your answer.\n");
	printf("\n");
	printf("In the editor, use the WASD buttons to change the angle of view.\n");
	printf("Press the arrow keys to select the number to modify\n");
	printf("Press PageUp and PageDown to change the vertex selection.\n");
	printf("Press Tab and Shift to change the vertex position to an existing vertex\n");
	printf("position.\n");
	printf("Press Q and E to zoom in and out.\n");
	printf("Press R to rotate the UV's\n");
	printf("Press P to play the sound file.\n");
	printf("\n");
	printf("The front face is counter clockwise.\n");
	
	
	//initialize interface
	ui_init(&ui, pwengine_get_shader(engine));
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		ui_handle_input(&ui, engine);
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	pwengine_free(engine);
	
	printf("Program closes gracefully\n");
	
	return 0;
}
