//improved shapes editor
//NoobieII
//15 Jan 2023

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc -IC:/msys64/mingw32/include/SDL2 shapes3.c engine/*.c engine/utilities/*.c engine/vorbis/*.c -m32 -msse -lmingw32 -lSDL2main -lSDL2.dll -lSDL2_image.dll -lSDL2_ttf.dll -lSDL2_net.dll -lws2_32 -lopengl32 -lglu32 -lglew32 -lglew32s -o shapes3 -static

//CONTROLS
//R - Rotation
//S - Scale
//G - Translation
//A - Add new shape
//E - Extrude from existing shape
//B - Select by shape
//M - Mirror selection
//C - Copy selection
//Arrows - Change perspective
//Keypad 1, 3, 7 - change perspective
//Middle mouse button - change perspective
//Middle mouse button + CTRL - zoom in/out
//Shift - snap to nearest .125 or 15 degrees

//Save is automatic

//Display:

#include "engine/pwengine.h"
#include "engine/pwmath.h"
#include "engine/pwlayer.h"
#include "engine/pwperspective.h"
#include "engine/pwrenderable.h"

#include <math.h>

typedef struct Extrude{
	int num_points; //num cross section points
	int degree; //num lengthwise points
} Extrude;

#define SHAPE_EXTRUDE 0
#define SHAPE_COPY 1

typedef struct Shape Shape;

typedef struct Shape{
	int type;
	union{
		Extrude extrude;
	};
	
	int vertex_count;
	int index_count;
	PWVertexData *vertices;
	unsigned short *indices;
	
	int start_index;        //updated when shape_to_vertices() is called
	int end_index;          //updated when shape_to_vertices() is called
	
	PWVec2 uv_top_left;
	PWVec2 uv_bottom_right;
	
	Shape *next;
	Shape *child;
} Shape;

//these convert functions get sent the entire vertex/index array.

//return 1 if ray collides with the vertices
//return the first shape found to collide, NULL if none
int mesh_x_ray(PWVertexData *vertices, unsigned short *indices, int index_count, PWRay ray);
Shape *shape_x_ray(Shape *list, PWRay ray);

//set triangle indices to 0 from the vertices that are selected
//update selected_vertices so that other vertices with the same position are selected
//update selected_indices so that triangles with all indices from selected_vertices are selected
int nullify_indices(PWVertexData **selected_vertices, int vertex_count, unsigned short **indices, int index_count);
int get_vertices_with_same_position(PWVertexData **selected_vertices, PWVertexData *vertices, int vertex_count);
int get_indices_with_vertices(unsigned short **selected_indices, unsigned short *indices, int index_count, PWVertexData **selected_vertices, int vertex_count);

//get average position, equal weight to all unique positions
PWVec3 get_average_position(PWVec3 *position, int n);
PWVec3 get_average_position_selection(PWVertexData **selected_vertices, int vertex_count);

//if p[] are all equal to shape's front/end face vertices, return 1, else 0
int is_extrude_start(PWVec3 *p, int n, Shape *shape);
int is_extrude_end(PWVec3 *p, int n, Shape *shape);

//if p[] are all equal to shape's front/end face vertices, return the corresponding shape, else NULL
Shape *get_extrude_start(PWVec3 *p, int n, Shape *shape);
Shape *get_extrude_end(PWVec3 *p, int n, Shape *shape);

//returns a shape the majority of the selected vertices are from a shape
Shape *get_shape_from_selection(PWVertexData **selected_vertices, int vertex_count, Shape *list);

Shape *add_shape(Shape *list, Shape *shape);

//returns the first found shape where all of base_pos matches shape vertex
//positions. returns NULL if not found
Shape *get_parent_shape(Shape *list, PWVec3 *base_pos, int n);

//Don't call these two functions, will be handled by a wrapper function
//convert shape list to a single vertex/index array (all values at address must be 0 or NULL by at initial caller)
//convert vertex/index array to renderable object - r must have been initialized before
int shape_to_vertices(Shape *shape, PWVertexData **vertices, int *vertex_count, unsigned short **indices, int *index_count);
int vertices_to_renderable(PWRenderable *r, PWVertexData *vertices, int vertex_count, unsigned short *indices, int index_count);

//Called when vertices are transformed. Use initial value *total=0
int vertices_to_shape(PWVertexData *vertices, int *v_total, unsigned short *indices, int *i_total, Shape *shape);

int shape_to_vertices(Shape *shape, PWVertexData **vertices, int *vertex_count, unsigned short **indices, int *index_count){
	Shape *current_shape;
	int i;
	
	if(!shape || !vertices || !vertex_count || !indices || !index_count){
		printf("Error converting shape.\n");
		return -1;
	}
	
	current_shape = shape;
	do{
		//copy the shape's data to the caller's data
		*vertices = realloc(*vertices, sizeof(PWVertexData) * ((*vertex_count) + current_shape->vertex_count));
		*indices = realloc(*indices, sizeof(unsigned short) * ((*index_count) + current_shape->index_count));
		memcpy(&(*vertices)[*vertex_count], current_shape->vertices, sizeof(PWVertexData) * current_shape->vertex_count);
		memcpy(&(*indices)[*index_count], current_shape->indices, sizeof(unsigned short) * current_shape->index_count);
		
		//account for uv position
		for(i = 0; i < current_shape->vertex_count; ++i){
			(*vertices)[*vertex_count + i].uv.x *= (current_shape->uv_bottom_right.x - current_shape->uv_top_left.x);
			(*vertices)[*vertex_count + i].uv.x += current_shape->uv_top_left.x;
			(*vertices)[*vertex_count + i].uv.y *= (current_shape->uv_bottom_right.y - current_shape->uv_top_left.y);
			(*vertices)[*vertex_count + i].uv.y += current_shape->uv_top_left.y;
		}
		
		//add vertex count to index values
		for(i = 0; i < current_shape->index_count; ++i){
			(*indices)[*index_count + i] += *vertex_count;
		}
		
		//TODO remove this line after debugging
		//printf("shape vertex count: %d total: %d\n", current_shape->vertex_count, *vertex_count);
		//for(i = 0; i < current_shape->vertex_count; ++i) printf("position = %f %f %f color = %x\n", current_shape->vertices[i].position.x, current_shape->vertices[i].position.y, current_shape->vertices[i].position.z, current_shape->vertices[i].color);
		current_shape->start_index = *vertex_count;
		current_shape->end_index = *vertex_count + current_shape->vertex_count;
		
		//printf("Shape indices (%d total %d)\n", current_shape->index_count, *index_count);
		//for(i = 0; i < current_shape->index_count; i += 3){
		//	printf("%d %d %d\n", current_shape->indices[i], current_shape->indices[i + 1], current_shape->indices[i //+ 2]);
		//}
		
		*vertex_count += current_shape->vertex_count;
		*index_count += current_shape->index_count;
		
		if(current_shape->child){
			shape_to_vertices(current_shape->child, vertices, vertex_count, indices, index_count);
		}
		current_shape = current_shape->next;
	} while(current_shape);
	return 0;
}

int vertices_to_renderable(PWRenderable *r, PWVertexData *vertices, int vertex_count, unsigned short *indices, int index_count){
	int i;
	
	//assume r was initialized properly
	pwrenderable_reset(r);
	
	//initialize content
	r->index_count = index_count;
	r->indices = malloc(sizeof(unsigned short) * index_count);
	memcpy(r->indices, indices, sizeof(unsigned short) * index_count);
	r->vertex_count = vertex_count;
	r->p = malloc(sizeof(PWVec3) * vertex_count);
	r->uv = malloc(sizeof(PWVec2) * vertex_count);
	r->n = malloc(sizeof(PWVec3) * vertex_count);
	r->color = malloc(sizeof(unsigned int) * vertex_count);
	
	for(i = 0; i < vertex_count; ++i){
		r->p[i] = vertices[i].position;
		r->uv[i] = vertices[i].uv;
		r->n[i] = vertices[i].normal;
		r->color[i] = vertices[i].color;
	}
	
	r->tid = 0.0f;
	
	//save to file
	pwrenderable_save2(r, "shape.txt");
	
	return 0;
}

int vertices_to_shape(PWVertexData *vertices, int *v_total, unsigned short *indices, int *i_total, Shape *shape){
	int a, b, i;
	if(!shape) return 0;
	if(!v_total){a = 0; v_total = &a;}
	if(!i_total){b = 0; i_total = &b;}
	
	for(i = 0; i < shape->vertex_count; ++i){
		shape->vertices[i].position = vertices[*v_total + i].position;
		shape->vertices[i].normal = vertices[*v_total + i].normal;
	}
	for(i = 0; i < shape->index_count; ++i){
		shape->indices[i] = indices[*i_total + i] - *v_total;
	}
	
	*v_total += shape->vertex_count;
	*i_total += shape->index_count;
	
	vertices_to_shape(vertices, v_total, indices, i_total, shape->child);
	vertices_to_shape(vertices, v_total, indices, i_total, shape->next);
	
	return 0;
}

int mesh_x_ray(PWVertexData *vertices, unsigned short *indices, int index_count, PWRay ray){
	//float t = 10000.0f;
	int i;
	
	for(i = 0; i < index_count; i += 3){
		if(PWM_ray_x_triangle(&ray, vertices[indices[i]].position, vertices[indices[i+1]].position, vertices[indices[i+2]].position, NULL)){
			return 1;
		}
	}
	
	return 0;
}

Shape *shape_x_ray(Shape *list, PWRay ray){
	Shape *shape;
	
	if(!list) return NULL;
	if(mesh_x_ray(list->vertices, list->indices, list->index_count, ray)) return list;
	
	shape = shape_x_ray(list->child, ray);
	if(shape) return shape;
	return shape_x_ray(list->next, ray);
}

int nullify_indices(PWVertexData **selected_vertices, int vertex_count, unsigned short **indices, int index_count){
	int i;
	int j;
	int n;
	
	//mark the indices to be removed with 0's
	for(i = 0; i < index_count; i += 3){
		n = 0;
		for(j = 0; j < vertex_count; ++j){
			if(selected_vertices[j] && (*indices)[i] == j){
				++n;
				break;
			}
		}
		for(j = 0; j < vertex_count; ++j){
			if(selected_vertices[j] && (*indices)[i + 1] == j){
				++n;
				break;
			}
		}
		for(j = 0; j < vertex_count; ++j){
			if(selected_vertices[j] && (*indices)[i + 2] == j){
				++n;
				break;
			}
		}
		if(n == 3){
			(*indices)[i + 0] = 0; 
			(*indices)[i + 1] = 0; 
			(*indices)[i + 2] = 0; 
		}
	}
	
	return 0;
}

int get_vertices_with_same_position(PWVertexData **selected_vertices, PWVertexData *vertices, int vertex_count){
	int i;
	int k;
	
	for(i = 0; i < vertex_count; ++i){
		if(selected_vertices[i]){
			for(k = 0; k < vertex_count; ++k){
				if(vertices[k].position.x == vertices[i].position.x
					&& vertices[k].position.x == vertices[i].position.x
					&& vertices[k].position.x == vertices[i].position.x){
					
					selected_vertices[k] = &vertices[k];
				}
			}
		}
	}
	
	return 0;
}

int get_indices_with_vertices(unsigned short **selected_indices, unsigned short *indices, int index_count, PWVertexData **selected_vertices, int vertex_count){
	int i;
	int j;
	int n;
	
	for(i = 0; i < index_count; i += 3){
		n = 0;
		for(j = 0; j < vertex_count; ++j){
			if(!selected_vertices[j]) continue;
			if(indices[i + 0] == j) ++n;
			if(indices[i + 1] == j) ++n;
			if(indices[i + 2] == j) ++n;
		}
		
		if(n == 3){
			selected_indices[i + 0] = &indices[i + 0];
			selected_indices[i + 1] = &indices[i + 1];
			selected_indices[i + 2] = &indices[i + 2];
		}
	}
	
	return 0;
}

//get average position, equal weight to all unique positions
PWVec3 get_average_position(PWVec3 *position, int n){
	PWVec3 polygon_center;
	int i, j, k;
	
	//polygon center is the average of the vertices selected
	polygon_center = PWM_vec3(0, 0, 0);
	k = 0;
	for(i = 0; i < n; ++i){
		//check if previous vertices have the same position
		for(j = 0; j < i; ++j){
			if(position[j].x == position[i].x
				&& position[j].y == position[i].y
				&& position[j].z == position[i].z){
				
				break;
			}
		}
		if(j == i){
			polygon_center = PWM_add3(position[i], polygon_center);
			++k;
		}
	}
	polygon_center = PWM_mul3(polygon_center, 1.0f / k);
	
	return polygon_center;
}

PWVec3 get_average_position_selection(PWVertexData **selected_vertices, int vertex_count){
	PWVec3 polygon_center;
	int i, j, k;
	
	polygon_center = PWM_vec3(0, 0, 0);
	k = 0;
	for(i = 0; i < vertex_count; ++i){
		if(!selected_vertices[i]) continue;
		for(j = 0; j < i; ++j){
			if(!selected_vertices[j]) continue;
			if(selected_vertices[j]->position.x == selected_vertices[i]->position.x
				&& selected_vertices[j]->position.y == selected_vertices[i]->position.y
				&& selected_vertices[j]->position.z == selected_vertices[i]->position.z){
				
				break;
			}
		}
		if(j == i){
			polygon_center = PWM_add3(selected_vertices[i]->position, polygon_center);
			++k;
		}
	}
	polygon_center = PWM_mul3(polygon_center, 1.0f / k);
	
	return polygon_center;
}

int is_extrude_start(PWVec3 *p, int n, Shape *shape){
	int i, j;
	int pass = 0;
	char *matches;
	
	if(!shape) return 0;
	if(shape->type != SHAPE_EXTRUDE) return 0;
	if(shape->extrude.num_points != n) return 0;
	
	matches = malloc(n);
	memset(matches, 0, n);
	
	for(i = 0; i < shape->extrude.num_points; ++i){
		for(j = 0; j < n; ++j){
			if(shape->vertices[i].position.x == p[j].x
				&& shape->vertices[i].position.y == p[j].y
				&& shape->vertices[i].position.z == p[j].z){
			
				matches[i] = 1;
			}
		}
	}
	for(i = 0; i < n; ++i) pass += matches[i];
	free(matches);
	
	if(pass == n) return 1;
	i = is_extrude_start(p, n, shape->child);
	if(i) return i;
	return is_extrude_start(p, n, shape->next);
}

int is_extrude_end(PWVec3 *p, int n, Shape *shape){
	int i, j;
	int pass = 0;
	char *matches;
	
	if(!shape) return 0;
	if(shape->type != SHAPE_EXTRUDE) return 0;
	if(shape->extrude.num_points != n) return 0;
	
	matches = malloc(n);
	memset(matches, 0, n);
	
	for(i = 0; i < shape->extrude.num_points; ++i){
		for(j = 0; j < n; ++j){
			if(shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.x == p[j].x
				&& shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.y == p[j].y
				&& shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.z == p[j].z){
			
				matches[i] = 1;
			}
		}
	}
	for(i = 0; i < n; ++i) pass += matches[i];
	free(matches);
	
	if(pass == n) return 1;
	i = is_extrude_end(p, n, shape->child);
	if(i) return i;
	return is_extrude_end(p, n, shape->next);
}

Shape *get_extrude_start(PWVec3 *p, int n, Shape *shape){
	Shape *child_shape;
	int i, j;
	int pass = 0;
	char *matches;
	
	if(!shape) return NULL;
	if(shape->type != SHAPE_EXTRUDE) return NULL;
	if(shape->extrude.num_points != n) return NULL;
	
	matches = malloc(n);
	memset(matches, 0, n);
	
	for(i = 0; i < shape->extrude.num_points; ++i){
		for(j = 0; j < n; ++j){
			if(shape->vertices[i].position.x == p[j].x
				&& shape->vertices[i].position.y == p[j].y
				&& shape->vertices[i].position.z == p[j].z){
			
				matches[i] = 1;
			}
		}
	}
	for(i = 0; i < n; ++i) pass += matches[i];
	free(matches);
	
	if(pass == n) return shape;
	child_shape = get_extrude_start(p, n, shape->child);
	if(child_shape) child_shape;
	return get_extrude_start(p, n, shape->next);
}

Shape *get_extrude_end(PWVec3 *p, int n, Shape *shape){
	Shape *child_shape;
	int i, j;
	int pass = 0;
	char *matches;
	
	if(!shape) return NULL;
	if(shape->type != SHAPE_EXTRUDE) return NULL;
	if(shape->extrude.num_points != n) return NULL;
	
	matches = malloc(n);
	memset(matches, 0, n);
	
	for(i = 0; i < shape->extrude.num_points; ++i){
		for(j = 0; j < n; ++j){
			if(shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.x == p[j].x
				&& shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.y == p[j].y
				&& shape->vertices[i + shape->vertex_count - shape->extrude.num_points].position.z == p[j].z){
			
				matches[i] = 1;
			}
		}
	}
	for(i = 0; i < n; ++i) pass += matches[i];
	free(matches);
	
	if(pass == n) return shape;
	child_shape = get_extrude_end(p, n, shape->child);
	if(child_shape) child_shape;
	return get_extrude_end(p, n, shape->next);
}

Shape *add_shape(Shape *list, Shape *shape){
	Shape *ptr;
	if(!list) return shape;
	ptr = list;
	while(ptr->next) ptr = ptr->next;
	ptr->next = shape;
	return list;
}

Shape *get_parent_shape(Shape *list, PWVec3 *base_pos, int n){
	int i, j, num_matches;
	Shape *shape;
	
	if(!list) return NULL;
	
	num_matches = 0;
	for(i = 0; i < n; ++i){
		for(j = 0; j < list->vertex_count; ++j){
			if(list->vertices[j].position.x == base_pos[i].x && list->vertices[j].position.y == base_pos[i].y && list->vertices[j].position.z == base_pos[i].z){
				num_matches++;
				break;
			}
		}
	}
	
	if(num_matches == n) return list;
	
	shape = get_parent_shape(list->child, base_pos, n);
	if(shape) return shape;
	
	return get_parent_shape(list->next, base_pos, n);
}

#define MODE_SELECT 0
#define MODE_CROSS_SECTION 1
#define MODE_EXTRUDE_SELECTION 2
#define MODE_EXTRUDE 4
#define MODE_ROTATION 8
#define MODE_SCALE 16
#define MODE_TRANSLATION 32
#define MODE_UV_EDIT 64

#define FIXED_NONE 0
#define FIXED_X 1
#define FIXED_Y 2
#define FIXED_Z 4

typedef struct UI{
	PWLayer layer_axes;
	PWLayer layer_shape;
	PWLayer layer_vertex_boxes;
	PWLayer layer_new_vertices;
	PWRenderable *model;
	PWRenderable *new_vertices[256];
	PWRenderable **vertex_boxes;
	PWPersp persp;
	PWVec3 persp_rotation;
	
	Shape *shape;
	int state;
	int fixed_axis;
	int vertex_count;
	PWVertexData *vertices;
	int index_count;
	unsigned short *indices;
	
	//select mode - 0 for vertices, 1 for entire shapes
	PWVertexData **selected_vertices;
	unsigned short **selected_indices;
	int select_shape_mode; 
	
	//cross section mode
	int num_cross_section_points;
	PWVec3 cross_section_points[16];
	
	//extrude selection mode
	int num_extrude_points;
	unsigned short extrude_indices[16];
	
	//extrude mode
	int extrude_facing_out;
	PWVec3 extrude_direction;
	float extrude_length;
	
	//either rotation, scale, translation mode
	PWVec2 xform_original_mouse_position;
	
	//rotation mode
	PWVec3 rotation_axis;
	float rotation;
	
	//scale mode
	PWVec3 scale;
	
	//translation mode
	PWVec3 translation;
	
	//UV edit mode
	Shape *uv_edit_shape;
	PWLayer layer_uv_edit;
	PWVec3 uv_top_left;
	PWVec3 uv_bottom_right;
	PWRenderable *r_top_left;
	PWRenderable *r_bottom_right;
	PWRenderable *r_uv_edit_texture;
	int uv_edit_state;
} UI;

int ui_init(UI *ui, PWEngine *engine);
int ui_close(UI *ui);

int handle_persp(PWEngine *engine, UI *ui);
int handle_select(PWEngine *engine, UI *ui);
int handle_cross_section(PWEngine *engine, UI *ui);
int handle_extrude_selection(PWEngine *engine, UI *ui);
int handle_extrude(PWEngine *engine, UI *ui);
int handle_rotation(PWEngine *engine, UI *ui);
int handle_scale(PWEngine *engine, UI *ui);
int handle_translation(PWEngine *engine, UI *ui);
int handle_uv_edit(PWEngine *engine, UI *ui);

//to be called after a shape has been added to the list.
//updates index/vertex array
//updates renderables
//updates selection arrays
int ui_update_add_shape(UI *ui, PWEngine *engine);

//transform/remove vertices/indices in shape and vertex array
//ui->selected_vertices selected
//ui->selected_indices selected
int transform_indices(UI *ui, PWMat4 transform);
int remove_indices(UI *ui);

//convention: selected_vertices will be an array of length vertices. Vertices
//that are selected will not be NULL.

int main(int argc, char *argv[]){
	PWEngine *engine;
	UI ui;
	
	//initialize engine
	engine = pwengine_init(640, 480, 30, "Shapes Editor");
	
	//initialize the ui and graphics
	ui_init(&ui, engine);
	
	while(!pwengine_is_quit(engine)){
		pwengine_poll_events(engine);
		
		//handle events that change the perspective
		handle_persp(engine, &ui);
		
		switch(ui.state){
		case MODE_SELECT:
			handle_select(engine, &ui);
			break;
		case MODE_CROSS_SECTION:
			handle_cross_section(engine, &ui);
			break;
		case MODE_EXTRUDE_SELECTION:
			handle_extrude_selection(engine, &ui);
			break;
		case MODE_EXTRUDE:
			handle_extrude(engine, &ui);
			break;
		case MODE_ROTATION:
			handle_rotation(engine, &ui);
			break;
		case MODE_SCALE:
			handle_scale(engine, &ui);
			break;
		case MODE_TRANSLATION:
			handle_translation(engine, &ui);
			break;
		case MODE_UV_EDIT:
			handle_uv_edit(engine, &ui);
			break;
		}
		
		
		pwengine_update(engine);
		pwengine_render(engine);
		pwengine_sleep_until_next_frame(engine);
	}
	
	ui_close(&ui);
	pwengine_free(engine);
	
	printf("Program closes gracefully.\n");
	return 0;
}

int ui_init(UI *ui, PWEngine *engine){
	int i;
	PWRenderable r;
	
	pwlayer_init(&ui->layer_axes, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	pwlayer_init(&ui->layer_shape, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	pwlayer_init(&ui->layer_vertex_boxes, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	pwlayer_init(&ui->layer_new_vertices, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	
	//axes
	pwrenderable_init_box(&r, PWM_vec3(1, 0.1, 0.1), 0xff0000ff, NULL); //x
	pwrenderable_transform(&r, PWM_translation(PWM_vec3(0.5, 0, 0)));
	pwlayer_add(&ui->layer_axes, &r);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 1, 0.1), 0xff00ff00, NULL); //y
	pwrenderable_transform(&r, PWM_translation(PWM_vec3(0, 0.5, 0)));
	pwlayer_add(&ui->layer_axes, &r);
	pwrenderable_init_box(&r, PWM_vec3(0.1, 0.1, 1), 0xffff0000, NULL); //z
	pwrenderable_transform(&r, PWM_translation(PWM_vec3(0, 0, 0.5)));
	pwlayer_add(&ui->layer_axes, &r);
	
	//shape (nothing for now)
	pwrenderable_init_none(&r);
	ui->model = pwlayer_add(&ui->layer_shape, &r);
	
	//cross-section vertices
	for(i = 0; i < 256; ++i){
		pwrenderable_init_none(&r);
		ui->new_vertices[i] = pwlayer_add(&ui->layer_new_vertices, &r);
	}
	ui->vertex_boxes = NULL;
	
	ui->persp = pwpersp();
	ui->persp.pov = PWM_vec3(0, 0, ui->persp.dist);
	pwpersp_mat4(&ui->persp);
	ui->persp_rotation = PWM_vec3(0, 0, 0);
	
	ui->shape = NULL;
	ui->state = MODE_SELECT;
	ui->fixed_axis = FIXED_NONE;
	ui->vertex_count = 0;
	ui->vertices = NULL;
	ui->index_count = 0;
	ui->indices = NULL;
	
	ui->selected_vertices = NULL;
	ui->selected_indices = NULL;
	ui->select_shape_mode = 0;
	
	ui->num_cross_section_points = 0;
	
	ui->num_extrude_points = 0;
	
	pwlayer_init(&ui->layer_uv_edit, pwengine_get_shader(engine), PWM_ini(1.0, NULL));
	ui->uv_top_left = PWM_vec3(-1.0f, 1.0f, 0);
	ui->uv_bottom_right = PWM_vec3(1.0f, -1.0f, 0);
	pwrenderable_init_none(&r);
	ui->r_top_left = pwlayer_add(&ui->layer_uv_edit, &r);
	ui->r_bottom_right = pwlayer_add(&ui->layer_uv_edit, &r);
	ui->r_uv_edit_texture = pwlayer_add(&ui->layer_uv_edit, &r);
	pwrenderable_init_texture(ui->r_uv_edit_texture, PWM_vec3(-1.0, -1.0, 0), PWM_vec2(2, 2), 0xffffffff, pwengine_get_texture(engine, "texture.png"));
	ui->uv_edit_state = 0;
	
	return 0;
}
	
int ui_close(UI *ui){
	pwlayer_close(&ui->layer_axes);
	pwlayer_close(&ui->layer_shape);
	pwlayer_close(&ui->layer_vertex_boxes);
	return 0;
}

int handle_persp(PWEngine *engine, UI *ui){
	PWVec3 rot;
	
	PWVec3 translation;
	
	pwpersp_mat4(&ui->persp);
	
	rot = PWM_vec3(100*pwengine_mouse_delta_y(engine), -100*pwengine_mouse_delta_x(engine), 0);
	
	
	//TODO: use arrow buttons alternatively
	if(pwengine_is_middle_button_held(engine)){
		if(pwengine_is_key_held(engine, "Left Shift") || pwengine_is_key_held(engine, "Right Shift")){
			//find distance from front plane
			translation = PWM_vec3(-10 * pwengine_mouse_delta_x(engine), -10 * pwengine_mouse_delta_y(engine), 0); //reaction to motion of mouse
			pwpersp_add_global_translation(&ui->persp,  translation);
		}
		else if(pwengine_is_key_held(engine, "Left Ctrl") || pwengine_is_key_held(engine, "Right Ctrl")){
			ui->persp.dist *= 1.00 - pwengine_mouse_delta_x(engine); //right motion magnifies
			ui->persp.pov = PWM_vec3(0, 0, ui->persp.dist);
		}
		else{
			pwpersp_add_global_rotation(&ui->persp, rot);
		}
	}
	
	/*
	if(pwengine_is_key_held(engine, "Q")){
		ui->persp.dist *= 1.01;
		ui->persp.pov = PWM_vec3(0, 0, ui->persp.dist);
	}
	if(pwengine_is_key_held(engine, "E")){
		ui->persp.dist *= 0.99;
		ui->persp.pov = PWM_vec3(0, 0, ui->persp.dist);
	}
	*/
	
	if(pwengine_is_key_held(engine, "Left")){
		pwpersp_add_global_rotation(&ui->persp, PWM_vec3(0, -2, 0));
	}
	if(pwengine_is_key_held(engine, "Right")){
		pwpersp_add_global_rotation(&ui->persp, PWM_vec3(0, 2, 0));
	}
	if(pwengine_is_key_held(engine, "Up")){
		pwpersp_add_global_rotation(&ui->persp, PWM_vec3(2, 0, 0));
	}
	if(pwengine_is_key_held(engine, "Down")){
		pwpersp_add_global_rotation(&ui->persp, PWM_vec3(-2, 0, 0));
	}
	
	//TODO: toggle fixed_axis with x/y/z
	if(pwengine_is_key_pressed(engine, "X")){
		ui->fixed_axis ^= FIXED_X;
	}
	if(pwengine_is_key_pressed(engine, "Y")){
		ui->fixed_axis ^= FIXED_Y;
	}
	if(pwengine_is_key_pressed(engine, "Z")){
		ui->fixed_axis ^= FIXED_Z;
	}
	
	if(pwengine_is_key_pressed(engine, "Keypad 1")){
		ui->persp.rot = PWM_vec3(90, 0, 0);
		ui->persp.txn = PWM_vec3(0, 0, 0);
	}
	if(pwengine_is_key_pressed(engine, "Keypad 3")){
		ui->persp.rot = PWM_vec3(90, 0, 90);
		ui->persp.txn = PWM_vec3(0, 0, 0);
	}
	if(pwengine_is_key_pressed(engine, "Keypad 7")){
		ui->persp.rot = PWM_vec3(0, 0, 0);
		ui->persp.txn = PWM_vec3(0, 0, 0);
	}
	
	if(ui->fixed_axis){
		printf("FIXED:");
		if(ui->fixed_axis & FIXED_X) printf("X");
		if(ui->fixed_axis & FIXED_Y) printf("Y");
		if(ui->fixed_axis & FIXED_Z) printf("Z");
		printf(" ");
	}
	
	pwlayer_set_projection(&ui->layer_axes, pwpersp_mat4(&ui->persp));
	pwlayer_set_projection(&ui->layer_shape, pwpersp_mat4(&ui->persp));
	pwlayer_set_projection(&ui->layer_vertex_boxes, pwpersp_mat4(&ui->persp));
	pwlayer_set_projection(&ui->layer_new_vertices, pwpersp_mat4(&ui->persp));
	
	return 0;
}

int handle_select(PWEngine *engine, UI *ui){
	PWRay ray;
	PWVec3 hit;
	PWVec3 near;
	PWAabb *v_box;
	Shape *shape;
	int i, j;
	int n, m;
	short tmp;
	
	//if clicked, check for collisions with any vertices
	if(pwengine_is_left_button_pressed(engine)){
		if(ui->select_shape_mode){
			ray = pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
			shape = shape_x_ray(ui->shape, ray);
			
			
			if(!pwengine_is_key_held(engine, "Left Shift")){
				for(i = 0; i < ui->vertex_count; ++i){
					ui->selected_vertices[i] = NULL;
					pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
				}
			}
			if(shape){
				for(i = shape->start_index; i < shape->end_index; ++i){
					ui->selected_vertices[i] = &ui->vertices[i];
					pwrenderable_set_color(ui->vertex_boxes[i], 0xff0080ff);
				}
			}
		}
		else{
			v_box = malloc(sizeof(PWAabb) * ui->vertex_count);
			for(i = 0; i < ui->vertex_count; ++i){
				v_box[i] = PWM_aabb(PWM_add3(ui->vertices[i].position, PWM_vec3(.1, .1, .1)), PWM_sub3(ui->vertices[i].position, PWM_vec3(.1, .1, .1)));
			}
			
			ray = pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
			
			//find which vertex on the ray is nearest to the viewer
			near = ui->persp.p_far;
			n = 0;
			for(i = 0; i < ui->vertex_count; ++i){
				if(PWM_ray_x_aabb(&ray, &v_box[i], &hit)){
					if(PWM_norm3(PWM_sub3(hit, ui->persp.p_near)) < PWM_norm3(PWM_sub3(near, ui->persp.p_near))){
						near = hit;
					}
				}
			}
			for(i = 0; i < ui->vertex_count; ++i){
				if(PWM_ray_x_aabb(&ray, &v_box[i], &hit)){
					if(PWM_norm3(PWM_sub3(hit, near)) < 0.01f){
						ui->selected_vertices[i] = &ui->vertices[i];
						pwrenderable_set_color(ui->vertex_boxes[i], 0xff0080ff);
					}//TODO check if this small block is necessary
					else if(!pwengine_is_key_held(engine, "Left Shift")){
						ui->selected_vertices[i] = NULL;
						pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
					}
				}
				else if(!pwengine_is_key_held(engine, "Left Shift")){
					ui->selected_vertices[i] = NULL;
					pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
				}
			}
			
			free(v_box);
		}
	}
	//if Escape is pressed, unselect all
	if(pwengine_is_key_pressed(engine, "Escape")){
		for(i = 0; i < ui->vertex_count; ++i){
			ui->selected_vertices[i] = NULL;
			pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
		}
	}
	
	//if delete is pressed, delete vertices
	if(pwengine_is_key_pressed(engine, "Delete")){
		//TODO
	}
	
	if(pwengine_is_key_pressed(engine, "R") && ui->vertex_count > 0){
		//if nothing is selected, select everything
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]) break;
		}
		if(i == ui->vertex_count){
			for(i = 0; i < ui->vertex_count; ++i){
				ui->selected_vertices[i] = &ui->vertices[i];
			}
		}
		ui->xform_original_mouse_position = PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine));
		ui->state = MODE_ROTATION;
	}
	
	if(pwengine_is_key_pressed(engine, "S") && ui->vertex_count > 0){
		//if nothing is selected, select everything
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]) break;
		}
		if(i == ui->vertex_count){
			for(i = 0; i < ui->vertex_count; ++i){
				ui->selected_vertices[i] = &ui->vertices[i];
			}
		}
		ui->xform_original_mouse_position = PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine));
		ui->state = MODE_SCALE;
	}
	
	if(pwengine_is_key_pressed(engine, "G") && ui->vertex_count > 0){
		//if nothing is selected, select everything
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]) break;
		}
		if(i == ui->vertex_count){
			for(i = 0; i < ui->vertex_count; ++i){
				ui->selected_vertices[i] = &ui->vertices[i];
			}
		}
		ui->xform_original_mouse_position = PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine));
		ui->state = MODE_TRANSLATION;
	}
	
	if(pwengine_is_key_pressed(engine, "A")){
		//unselect everything
		for(i = 0; i < ui->vertex_count; ++i){
			ui->selected_vertices[i] = NULL;
			pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
		}
		
		ui->num_cross_section_points = 0;
		
		ui->state = MODE_CROSS_SECTION;
	}
	
	if(pwengine_is_key_pressed(engine, "E") && ui->vertex_count > 0){
		//unselect everything
		for(i = 0; i < ui->vertex_count; ++i){
			ui->selected_vertices[i] = NULL;
			pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
		}
		ui->state = MODE_EXTRUDE_SELECTION;
	}
	
	if(pwengine_is_key_pressed(engine, "B")){
		ui->select_shape_mode ^= 1;
	}
	
	if(pwengine_is_key_pressed(engine, "M")){
		//TODO mirror selection
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]){
				if(ui->fixed_axis & FIXED_X) ui->selected_vertices[i]->position.x *= -1.0f;
				if(ui->fixed_axis & FIXED_Y) ui->selected_vertices[i]->position.y *= -1.0f;
				if(ui->fixed_axis & FIXED_Z) ui->selected_vertices[i]->position.z *= -1.0f;
			}
		}
		//reverse indices for affected vertices
		if(ui->fixed_axis == (FIXED_X | FIXED_Y | FIXED_Z) || ui->fixed_axis == FIXED_X || ui->fixed_axis == FIXED_Y || ui->fixed_axis == FIXED_Z){
			for(i = 0; i < ui->index_count; i += 3){
				if(ui->selected_vertices[ui->indices[i]] && ui->selected_vertices[ui->indices[i+1]] && ui->selected_vertices[ui->indices[i+2]]){
					tmp = ui->indices[i+1];
					ui->indices[i+1] = ui->indices[i+2];
					ui->indices[i+2] = tmp;
				}
			}
		}
		//modify the shapes and recreate the renderable 
		vertices_to_shape(ui->vertices, NULL, ui->indices, NULL, ui->shape);
		ui_update_add_shape(ui, engine);
	}
	if(pwengine_is_key_pressed(engine, "C")){
		n = 0;
		for(i = 0; i < ui->vertex_count; ++i) if(ui->selected_vertices[i]) ++n;
		
		if(n > 0){
			//TODO
			//copy whatever is selected (put it in its own single shape)
			shape = malloc(sizeof(Shape));
			shape->type = SHAPE_COPY;
			shape->start_index = ui->vertex_count;
			shape->end_index = ui->vertex_count + n;
			shape->vertex_count = n;
			shape->vertices = malloc(sizeof(PWVertexData) * n);
			n = 0;
			for(i = 0; i < ui->index_count; i += 3){
				if(ui->selected_vertices[ui->indices[i]] && ui->selected_vertices[ui->indices[i+1]] && ui->selected_vertices[ui->indices[i+2]]){
					n += 3;
				}
			}
			shape->index_count = n;
			shape->indices = malloc(sizeof(unsigned short) * n);
			n = 0;
			m = 0;
			for(i = 0; i < ui->vertex_count; ++i){
				if(ui->selected_vertices[i]){
					shape->vertices[n] = ui->vertices[i];/*
					//make instances where index[j] == i , index[m] = n 
					for(j = 0; j < ui->index_count; j += 3){
						if(ui->selected_vertices[ui->indices[j]] && ui->selected_vertices[ui->indices[j+1]] && ui->selected_vertices[ui->indices[j+2]]){
							if(ui->indices[j] == i) shape->indices[m++] = n;
							if(ui->indices[j+1] == i) shape->indices[m++] = n;
							if(ui->indices[j+2] == i) shape->indices[m++] = n;
						}
					}*/
					n++;
				}
			}
			n = 0;
			for(i = 0; i < ui->index_count; i += 3){
				if(ui->selected_vertices[ui->indices[i]] && ui->selected_vertices[ui->indices[i+1]] && ui->selected_vertices[ui->indices[i+2]]){
					//which vertex does indices[i] refer to?
					shape->indices[n++] = ui->indices[i];
					shape->indices[n++] = ui->indices[i+1];
					shape->indices[n++] = ui->indices[i+2];
				}
			}
			for(i = 0; i < shape->index_count; ++i){
				//TODO translate index
				//shape->indices[i] will correspond to ui vertices. find index n
				//such that shape->vertices[n] == ui->vertices[shape->indices[i]]
				for(j = 0; j < shape->vertex_count; ++j){
					if(memcmp(&shape->vertices[j], &ui->vertices[shape->indices[i]], sizeof(PWVertexData)) == 0) break;
				}
				shape->indices[i] = j;
			}
			printf("shape->index_count = %d\n", shape->index_count);
			shape->uv_top_left = PWM_vec2(0, 0);
			shape->uv_bottom_right = PWM_vec2(1, 1);
			shape->next = NULL;
			shape->child = NULL;
			
			ui->shape = add_shape(ui->shape, shape);
			ui_update_add_shape(ui, engine);
			
			//change selection to the copied shape
			for(i = 0; i < ui->vertex_count - shape->vertex_count; ++i){
				ui->selected_vertices[i] = NULL;
				pwrenderable_set_color(ui->vertex_boxes[i], 0x00000000);
			}
			for(i = ui->vertex_count - shape->vertex_count; i < ui->vertex_count; ++i){
				ui->selected_vertices[i] = &ui->vertices[i];
				pwrenderable_set_color(ui->vertex_boxes[i], 0xff0080ff);
			}
		}
	}
	
	if(ui->select_shape_mode) printf("[] ");
	printf("Vertices selected: ");
	for(i = 0; i < ui->vertex_count; ++i) if(ui->selected_vertices[i]) printf("%d ", i);
	printf("\r");
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_vertex_boxes);
	
	return 0;
}

int handle_cross_section(PWEngine *engine, UI *ui){
	PWVec3 position;
	PWPlane plane;
	PWRay ray;
	int i;
	
	pwpersp_mat4(&ui->persp);
	pwpersp_ray(&ui->persp, PWM_vec2(0, 0));
	position = PWM_mul3(PWM_add3(ui->persp.p_near, ui->persp.p_far), 0.5);
	
	//show where the current box is
	pwrenderable_reset(ui->new_vertices[ui->num_cross_section_points]);
	pwrenderable_init_box(ui->new_vertices[ui->num_cross_section_points], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
	
	//place cube at intersection of mouse position and plane at origin
	ray = pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
	position = PWM_mul3(PWM_add3(ui->persp.p_near, ui->persp.p_far), 0.5);
	
	//optional, snap on nearest 1/8
	if(pwengine_is_key_held(engine, "Left Ctrl")){
		position.x += (position.x > 0 ? (0.125f / 2) : (-0.125f / 2));
		position.x *= 8;
		position.x = (float)(((int)(position.x)) / 8.0f);
		position.y += (position.y > 0 ? (0.125f / 2) : (-0.125f / 2));
		position.y *= 8;
		position.y = (float)(((int)(position.y)) / 8.0f);
		position.z += (position.z > 0 ? (0.125f / 2) : (-0.125f / 2));
		position.z *= 8;
		position.z = (float)(((int)(position.z)) / 8.0f);
	}
	
	//position of renderable
	pwrenderable_transform(ui->new_vertices[ui->num_cross_section_points], PWM_translation(position));
	
	if(pwengine_is_left_button_pressed(engine)){
		ui->cross_section_points[ui->num_cross_section_points] = position;
		ui->num_cross_section_points++;
	}
	
	if(pwengine_is_key_pressed(engine, "Return")){
		//find extrude direction. Does not matter whether forward or backward facing
		ui->num_extrude_points = 0;
		
		ui->extrude_facing_out = 1;
		ui->extrude_direction = PWM_vec3(0,0,0);
		for(i = 0; i < ui->num_cross_section_points - 2; ++i){
			ui->extrude_direction = PWM_add3(PWM_cross3(PWM_sub3(ui->cross_section_points[i + 2], ui->cross_section_points[i + 1]), PWM_sub3(ui->cross_section_points[i + 1], ui->cross_section_points[i])), ui->extrude_direction);
		}
		ui->extrude_direction = PWM_normalize3(ui->extrude_direction);
		ui->extrude_length = 0;
		
		ui->state = MODE_EXTRUDE;
	}
	
	if(pwengine_is_key_pressed(engine, "Escape")){
		//clear the renderables
		for(i = 0; i < ui->num_cross_section_points + 1; ++i){
			pwrenderable_reset(ui->new_vertices[i]);
		}
		ui->state = MODE_SELECT;
	}
	
	printf("Position %f %f %f ", position.x, position.y, position.z);
	printf("Num points: %d\r", ui->num_cross_section_points);
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_new_vertices);
	
	return 0;
}

int handle_extrude_selection(PWEngine *engine, UI *ui){
	//TODO
	
	//if mouse is clicked, queue vertex index if there are collisions with vertices
	PWRay ray;
	PWVec3 hit;
	PWVec3 near;
	PWAabb *v_box;
	int i;
	int n;
	
	//if clicked, check for collisions with any vertices
	if(pwengine_is_left_button_pressed(engine)){
		v_box = malloc(sizeof(PWAabb) * ui->vertex_count);
		for(i = 0; i < ui->vertex_count; ++i){
			v_box[i] = PWM_aabb(PWM_add3(ui->vertices[i].position, PWM_vec3(.1, .1, .1)), PWM_sub3(ui->vertices[i].position, PWM_vec3(.1, .1, .1)));
		}
		
		pwpersp_mat4(&ui->persp);
		ray = pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
		
		//find which vertex on the ray is nearest to the viewer
		near = ui->persp.p_far;
		n = 0;
		for(i = 0; i < ui->vertex_count; ++i){
			if(PWM_ray_x_aabb(&ray, &v_box[i], &hit)){
				if(PWM_norm3(PWM_sub3(hit, ui->persp.p_near)) < PWM_norm3(PWM_sub3(near, ui->persp.p_near))){
					near = hit;
				}
			}
		}
		for(i = 0; i < ui->vertex_count; ++i){
			if(PWM_ray_x_aabb(&ray, &v_box[i], &hit)){
				if(PWM_norm3(PWM_sub3(hit, near)) < 0.01f){
					//create the vertex box
					pwrenderable_reset(ui->new_vertices[ui->num_cross_section_points]);
					pwrenderable_init_box(ui->new_vertices[ui->num_cross_section_points], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
					pwrenderable_transform(ui->new_vertices[ui->num_cross_section_points], PWM_translation(ui->vertices[i].position));
					
					//set position
					ui->cross_section_points[ui->num_cross_section_points] = ui->vertices[i].position;
					ui->num_cross_section_points++;
					
					//set index
					ui->extrude_indices[ui->num_extrude_points] = i;
					ui->num_extrude_points++;
					
					//ensure no duplicate points are selected
					break;
				}
			}
		}
		
		free(v_box);
	}
	
	
	
	//if enter is pressed, go to extrude mode
	if(pwengine_is_key_pressed(engine, "Return")){
		ui->extrude_direction = PWM_vec3(0,0,0);
		ui->extrude_facing_out = 1;
		for(i = 0; i < ui->num_cross_section_points - 2; ++i){
			ui->extrude_direction = PWM_add3(PWM_cross3(PWM_sub3(ui->cross_section_points[i + 2], ui->cross_section_points[i + 1]), PWM_sub3(ui->cross_section_points[i + 1], ui->cross_section_points[i])), ui->extrude_direction);
		}
		ui->extrude_direction = PWM_normalize3(ui->extrude_direction);
		
		ui->state = MODE_EXTRUDE;
	}
	
	if(pwengine_is_key_pressed(engine, "Escape")){
		//clean up variables
		for(i = 0; i < ui->num_cross_section_points; ++i) pwrenderable_reset(ui->new_vertices[i]);
		
		ui->num_cross_section_points = 0;
		ui->num_extrude_points = 0;
		
		ui->state = MODE_SELECT;
	}
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_new_vertices);
	printf("Num points: %d\r", ui->num_cross_section_points);
	return 0;
}

int handle_extrude(PWEngine *engine, UI *ui){
	PWVec3 extrude_origin;
	Shape *shape, *parent_shape;
	int i, j;
	float f;
	PWVec3 normal;
	unsigned short tmp;
	
	//find position on line
	extrude_origin = get_average_position(ui->cross_section_points, ui->num_cross_section_points);
	
	pwpersp_mat4(&ui->persp);
	pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
	
	ui->extrude_length = PWM_dot3(ui->extrude_direction, PWM_sub3(PWM_mul3(PWM_add3(ui->persp.p_near, ui->persp.p_far), 0.5), extrude_origin));
	
	//optional, snap on nearest 1/8
	if(pwengine_is_key_held(engine, "Left Ctrl")){
		ui->extrude_length += (ui->extrude_length > 0 ? (0.125f / 2) : (-0.125f / 2));
		ui->extrude_length *= 8;
		ui->extrude_length = (float)(((int)(ui->extrude_length)) / 8.0f);
	}
	
	printf("extrude_length %f", ui->extrude_length);
	if(ui->extrude_facing_out) printf(" facing out");
	else printf(" facing in");
	printf("\r");
	
	//show dots where the extrude will go
	for(i = 0; i < ui->num_cross_section_points; ++i){
		pwrenderable_reset(ui->new_vertices[i]);
		pwrenderable_init_box(ui->new_vertices[i], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
		pwrenderable_transform(ui->new_vertices[i], PWM_translation(ui->cross_section_points[i]));
		
		pwrenderable_reset(ui->new_vertices[i + ui->num_cross_section_points]);
		pwrenderable_init_box(ui->new_vertices[i + ui->num_cross_section_points], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
		pwrenderable_transform(ui->new_vertices[i + ui->num_cross_section_points], PWM_translation(PWM_add3(ui->cross_section_points[i], PWM_mul3(ui->extrude_direction, ui->extrude_length))));
	}
	
	
	
	
	//upon left-clicking, place the extrude and create/reevaluate the shapes
	if(pwengine_is_left_button_pressed(engine)){
		//if the cross_section_points are all equal to an extrude end or beginning, create only half the points
		if(is_extrude_start(ui->cross_section_points, ui->num_cross_section_points, ui->shape)){
			shape = get_extrude_start(ui->cross_section_points, ui->num_cross_section_points, ui->shape);
			
			//update the shapes
			shape->extrude.degree += 1;
			
			//reallocate memory
			shape->vertices = realloc(shape->vertices, sizeof(PWVertexData) * (shape->vertex_count + shape->extrude.num_points));
			shape->indices = realloc(shape->indices, sizeof(unsigned short) * (shape->index_count + 3 * 2 * (shape->extrude.num_points - 1)));
			
			//make room on the front of the arrays and update index values
			for(i = 0; i < shape->vertex_count; ++i){
				shape->vertices[shape->vertex_count + shape->extrude.num_points - i - 1] = shape->vertices[shape->vertex_count - i - 1];
			}
			
			
			//insert the points on the start of the vertices
			for(i = 0; i < shape->extrude.num_points; ++i){
				shape->vertices[i].position = PWM_add3(shape->vertices[i + shape->extrude.num_points].position, PWM_mul3(ui->extrude_direction, ui->extrude_length));
				shape->vertices[i].normal = PWM_normalize3(PWM_sub3(shape->vertices[i + shape->extrude.num_points].position, extrude_origin));
				shape->vertices[i].tid = 0.0;
				shape->vertices[i].color = 0xffffffff;
			}
			
			//update the UV's
			for(i = 0; i < shape->extrude.degree; ++i){
				for(j = 0; j < shape->extrude.num_points; ++j){
					shape->vertices[i * shape->extrude.num_points + j].uv = PWM_vec2(j / (shape->extrude.num_points - 1.0), i / (shape->extrude.degree - 1.0));
				}
			}
			
			//update indices
			for(i = 0; i < shape->extrude.num_points - 1; ++i){
				shape->indices[shape->index_count + i * 6 + 0] = shape->vertex_count - shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 1] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 2] = shape->vertex_count - shape->extrude.num_points + i + 1;
				
				shape->indices[shape->index_count + i * 6 + 3] = shape->vertex_count - shape->extrude.num_points + i + 1;
				shape->indices[shape->index_count + i * 6 + 4] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 5] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i + 1;
			}
			
			//check if vertex is facing out
			f = 0;
			for(i = shape->index_count; i < shape->index_count + 6 * (shape->extrude.num_points - 1); i += 3){
				normal = PWM_cross3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, shape->vertices[shape->indices[i]].position), PWM_sub3(shape->vertices[shape->indices[i+2]].position, shape->vertices[shape->indices[i+1]].position));
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+2]].position, extrude_origin), normal);
			}
			if(ui->extrude_facing_out == 0) f *= -1.0f;
			if(f < 0){
				//reverse the indices of the shape
				for(i = shape->index_count; i < shape->index_count + 6 * (shape->extrude.num_points - 1); i += 3){
					tmp = shape->indices[i + 1];
					shape->indices[i + 1] = shape->indices[i + 2];
					shape->indices[i + 2] = tmp;
				}
			}
			
			
			shape->vertex_count += shape->extrude.num_points;
			shape->index_count += 3 * 2 * (shape->extrude.num_points - 1);
			
			ui->state = MODE_SELECT;
		}
		else if(is_extrude_end(ui->cross_section_points, ui->num_cross_section_points, ui->shape)){
			shape = get_extrude_end(ui->cross_section_points, ui->num_cross_section_points, ui->shape);
			
			
			//update the shapes
			shape->extrude.degree += 1;
			
			//reallocate memory
			shape->vertices = realloc(shape->vertices, sizeof(PWVertexData) * (shape->vertex_count + shape->extrude.num_points));
			shape->indices = realloc(shape->indices, sizeof(unsigned short) * (shape->index_count + 3 * 2 * (shape->extrude.num_points - 1)));
			
			
			//insert the points on the start of the vertices
			for(i = shape->vertex_count; i < shape->vertex_count + shape->extrude.num_points; ++i){
				shape->vertices[i].position = PWM_add3(shape->vertices[i - shape->extrude.num_points].position, PWM_mul3(ui->extrude_direction, ui->extrude_length));
				shape->vertices[i].normal = PWM_normalize3(PWM_sub3(shape->vertices[i - shape->extrude.num_points].position, extrude_origin));
				shape->vertices[i].tid = 0.0;
				shape->vertices[i].color = 0xffffffff;
			}
			
			//update the UV's
			for(i = 0; i < shape->extrude.degree; ++i){
				for(j = 0; j < shape->extrude.num_points; ++j){
					shape->vertices[i * shape->extrude.num_points + j].uv = PWM_vec2(j / (shape->extrude.num_points - 1.0), i / (shape->extrude.degree - 1.0));
				}
			}
			
			//update indices
			for(i = 0; i < shape->extrude.num_points - 1; ++i){
				shape->indices[shape->index_count + i * 6 + 0] = shape->vertex_count - shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 1] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 2] = shape->vertex_count - shape->extrude.num_points + i + 1;
				
				shape->indices[shape->index_count + i * 6 + 3] = shape->vertex_count - shape->extrude.num_points + i + 1;
				shape->indices[shape->index_count + i * 6 + 4] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i;
				shape->indices[shape->index_count + i * 6 + 5] = shape->vertex_count - shape->extrude.num_points + shape->extrude.num_points + i + 1;
			}
			
			//check if vertex is facing out
			f = 0;
			for(i = shape->index_count; i < shape->index_count + 6 * (shape->extrude.num_points - 1); i += 3){
				normal = PWM_cross3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, shape->vertices[shape->indices[i]].position), PWM_sub3(shape->vertices[shape->indices[i+2]].position, shape->vertices[shape->indices[i+1]].position));
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+2]].position, extrude_origin), normal);
			}
			if(ui->extrude_facing_out == 0) f *= -1.0f;
			if(f < 0){
				//reverse the indices of the shape
				for(i = shape->index_count; i < shape->index_count + 6 * (shape->extrude.num_points - 1); i += 3){
					tmp = shape->indices[i + 1];
					shape->indices[i + 1] = shape->indices[i + 2];
					shape->indices[i + 2] = tmp;
				}
			}
			
			
			shape->vertex_count += shape->extrude.num_points;
			shape->index_count += 3 * 2 * (shape->extrude.num_points - 1);
			
			ui->state = MODE_SELECT;
		}
		else{
			//create a new shape
			shape = malloc(sizeof(Shape));
			
			shape->type = SHAPE_EXTRUDE;
			shape->extrude.num_points = ui->num_cross_section_points;
			shape->extrude.degree = 2;
			shape->vertex_count = shape->extrude.num_points * shape->extrude.degree;
			shape->index_count = 3 * 2 * (shape->extrude.num_points - 1) * (shape->extrude.degree - 1);
			//shape->start_index = ui->vertex_count;
			//shape->end_index = ui->vertex_count + shape->vertex_count;
			shape->uv_top_left = PWM_vec2(0, 0);
			shape->uv_bottom_right = PWM_vec2(1, 1);
			shape->next = NULL;
			shape->child = NULL;
			
			//calculate vertex and index
			shape->vertices = malloc(sizeof(PWVertexData) * shape->vertex_count);
			for(i = 0; i < ui->num_cross_section_points; ++i){
				shape->vertices[i].position = ui->cross_section_points[i];
				shape->vertices[ui->num_cross_section_points + i].position = PWM_add3(ui->cross_section_points[i], PWM_mul3(ui->extrude_direction, ui->extrude_length));
				shape->vertices[i].normal = PWM_normalize3(PWM_sub3(shape->vertices[i].position, extrude_origin));
				shape->vertices[ui->num_cross_section_points + i].normal = shape->vertices[i].normal;
				shape->vertices[i].uv = PWM_vec2(i / (shape->extrude.num_points - 1.0), 0);
				shape->vertices[ui->num_cross_section_points + i].uv = PWM_vec2(i / (shape->extrude.num_points - 1.0), 1);
				shape->vertices[i].tid = 0.0;
				shape->vertices[ui->num_cross_section_points + i].tid = 0.0;
				shape->vertices[i].color = 0xffffffff;
				shape->vertices[ui->num_cross_section_points + i].color = 0xffffffff;
			}
			
			shape->indices = malloc(sizeof(unsigned short) * shape->index_count);
			for(i = 0; i < shape->extrude.num_points - 1; ++i){
				shape->indices[i * 6 + 0] = i;
				shape->indices[i * 6 + 1] = shape->extrude.num_points + i;
				shape->indices[i * 6 + 2] = i + 1;
				
				shape->indices[i * 6 + 3] = i + 1;
				shape->indices[i * 6 + 4] = shape->extrude.num_points + i;
				shape->indices[i * 6 + 5] = shape->extrude.num_points + i + 1;
			}
			
			//check if vertex is facing out
			f = 0;
			for(i = 0; i < shape->index_count; i += 3){
				normal = PWM_cross3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, shape->vertices[shape->indices[i]].position), PWM_sub3(shape->vertices[shape->indices[i+2]].position, shape->vertices[shape->indices[i+1]].position));
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+1]].position, extrude_origin), normal);
				f += PWM_dot3(PWM_sub3(shape->vertices[shape->indices[i+2]].position, extrude_origin), normal);
			}
			if(ui->extrude_facing_out == 0) f *= -1.0f;
			if(f < 0){
				//reverse the indices of the shape
				for(i = 0; i < shape->index_count; i += 3){
					tmp = shape->indices[i + 1];
					shape->indices[i + 1] = shape->indices[i + 2];
					shape->indices[i + 2] = tmp;
				}
			}
			
			//check if all vertex positions match the positions from a shape
			//add shape to ui shape list
			//parent_shape = get_parent_shape(ui->shape, ui->cross_section_points, ui->num_cross_section_points);
			//if(parent_shape){
			//	parent_shape->child = add_shape(ui->shape, shape);
			//	printf("Creating child shape\n");
			//}
			//else 
			ui->shape = add_shape(ui->shape, shape);
			
			//select UV
			ui->uv_edit_shape = shape;
			ui->state = MODE_UV_EDIT;
		} //else (create new shape)
		
		//repopulate vertex array
		ui_update_add_shape(ui, engine);
		
		//clean up variables
		for(i = 0; i < ui->num_cross_section_points * 2; ++i){
			pwrenderable_reset(ui->new_vertices[i]);
		}
		ui->num_cross_section_points = 0;
		ui->num_extrude_points = 0;
	}
	
	//if Shift is pressed
	if(pwengine_is_key_pressed(engine, "Left Shift")){
		ui->extrude_facing_out ^= 1;
	}
	
	//back to select mode if ESC is pressed
	if(pwengine_is_key_pressed(engine, "Escape")){
		//clean up variables
		for(i = 0; i < ui->num_cross_section_points * 2; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->num_cross_section_points = 0;
		ui->num_extrude_points = 0;
		ui->state = MODE_SELECT;
	}
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_new_vertices);
	
	return 0;
}
	
int handle_rotation(PWEngine *engine, UI *ui){
	//check arctangent of mouse position wrt original position
	float rotation;
	float x, y;
	int num_vertices = 0;
	int i;
	PWVec3 v3;
	PWVec3 origin; //origin for rotation
	PWVec3 axis; //axis for rotation
	
	
	x = pwengine_mouse_x(engine) - ui->xform_original_mouse_position.x;
	y = ui->xform_original_mouse_position.y - pwengine_mouse_y(engine);
	if(x == 0 || y == 0){
		rotation = 0;
	}
	else{
		rotation = atan2(0.75 * x, y);
		rotation *= 180 / PI;
	}
	
	//optional, snap to nearest 15
	if(pwengine_is_key_pressed(engine, "Left Ctrl")){
		rotation += (rotation > 0 ? (7.5f) : (-7.5f));
		rotation *= 1.0f / 15.0f;
		rotation = (float)(((int)(rotation)) * 15.0f);
	}
	
	origin = get_average_position_selection(ui->selected_vertices, ui->vertex_count);
	
	pwpersp_ray(&ui->persp, PWM_vec2(0, 0));
	axis = PWM_sub3(ui->persp.p_near, ui->persp.p_far);
	if(ui->fixed_axis == FIXED_X) axis = PWM_mul3(PWM_vec3(1, 0, 0), PWM_dot3(axis, PWM_vec3(1, 0, 0)));
	if(ui->fixed_axis == FIXED_Y) axis = PWM_mul3(PWM_vec3(0, 1, 0), PWM_dot3(axis, PWM_vec3(0, 1, 0)));
	if(ui->fixed_axis == FIXED_Z) axis = PWM_mul3(PWM_vec3(0, 0, 1), PWM_dot3(axis, PWM_vec3(0, 0, 1)));
	axis = PWM_normalize3(axis);
	
	//show at most 256 vertices
	for(i = 0; i < ui->vertex_count; ++i){
		if(num_vertices == 256) break;
		if(ui->selected_vertices[i]){
			//create the renderable
			v3 = PWM_sub3(ui->selected_vertices[i]->position, origin);
			v3 = PWM_mul_vec3(PWM_rotation(rotation, axis), v3);
			v3 = PWM_add3(v3, origin);
			
			pwrenderable_reset(ui->new_vertices[i]);
			pwrenderable_init_box(ui->new_vertices[i], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
			pwrenderable_transform(ui->new_vertices[i], PWM_translation(v3));
			
			num_vertices++;
		}
	}
	
	if(pwengine_is_left_button_pressed(engine)){
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]){
				v3 = PWM_sub3(ui->selected_vertices[i]->position, origin);
				v3 = PWM_mul_vec3(PWM_rotation(rotation, axis), v3);
				ui->selected_vertices[i]->position = PWM_add3(v3, origin);
				
				//ui->selected_vertices[i]->normal = PWM_mul_vec3(PWM_rotation(rotation, axis), ui->selected_vertices[i]->normal);
			}
		}
		
		//modify the shapes and recreate the renderable 
		vertices_to_shape(ui->vertices, NULL, ui->indices, NULL, ui->shape);
		ui_update_add_shape(ui, engine);
		
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	if(pwengine_is_key_pressed(engine, "Escape")){
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_new_vertices);
	
	printf("Rotation: %.2f degrees\r", rotation);
	
	return 0;
}

int handle_scale(PWEngine *engine, UI *ui){
	float scale;
	PWVec3 v3, origin, v_scale;
	int i, num_vertices = 0;
	
	//scale is distance from original mouse position to current mouse position
	scale = 5 * PWM_norm2(PWM_sub2(ui->xform_original_mouse_position, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine))));
	
	//optional, snap scale to nearest 0.125
	if(pwengine_is_key_pressed(engine, "Left Ctrl")){
		scale += (scale > 0 ? (.0625f) : (-.0625f));
		scale *= 8.0f;
		scale = (float)(((int)(scale)) / 8.0f);
	}
	
	v_scale = PWM_vec3(scale, scale, scale);
	if(ui->fixed_axis & FIXED_X) v_scale.x = 1;
	if(ui->fixed_axis & FIXED_Y) v_scale.y = 1;
	if(ui->fixed_axis & FIXED_Z) v_scale.z = 1;
	
	origin = get_average_position_selection(ui->selected_vertices, ui->vertex_count);
	
	//show up to 256 points
	for(i = 0; i < ui->vertex_count; ++i){
		if(num_vertices == 256) break;
		if(ui->selected_vertices[i]){
			
			//create the renderable
			v3 = PWM_sub3(ui->selected_vertices[i]->position, origin);
			v3 = PWM_mul_vec3(PWM_scale(v_scale), v3);
			v3 = PWM_add3(v3, origin);
			
			pwrenderable_reset(ui->new_vertices[i]);
			pwrenderable_init_box(ui->new_vertices[i], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
			pwrenderable_transform(ui->new_vertices[i], PWM_translation(v3));
			
			num_vertices++;
		}
	}
	
	if(pwengine_is_left_button_pressed(engine)){
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]){
				v3 = PWM_sub3(ui->selected_vertices[i]->position, origin);
				v3 = PWM_mul_vec3(PWM_scale(v_scale), v3);
				ui->selected_vertices[i]->position = PWM_add3(v3, origin);
			}
		}
		
		//modify the shapes and recreate the renderable 
		vertices_to_shape(ui->vertices, NULL, ui->indices, NULL, ui->shape);
		ui_update_add_shape(ui, engine);
		
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	if(pwengine_is_key_pressed(engine, "Escape")){
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_new_vertices);
	
	printf("Scale: %f\r", scale);
	
	return 0;
}

int handle_translation(PWEngine *engine, UI *ui){
	PWVec3 v3, p1, p2, translation;
	int i, num_vertices;
	
	pwpersp_ray(&ui->persp, ui->xform_original_mouse_position);
	p1 = PWM_mul3(PWM_add3(ui->persp.p_far, ui->persp.p_near), 0.5f);
	pwpersp_ray(&ui->persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
	p2 = PWM_mul3(PWM_add3(ui->persp.p_far, ui->persp.p_near), 0.5f);
	translation = PWM_sub3(p2, p1);
	
	//optional, snap on nearest 1/8
	if(pwengine_is_key_pressed(engine, "Left Ctrl")){
		translation.x += (translation.x > 0 ? (0.125f / 2) : (-0.125f / 2));
		translation.x *= 8;
		translation.x = (float)(((int)(translation.x)) / 8.0f);
		translation.y += (translation.y > 0 ? (0.125f / 2) : (-0.125f / 2));
		translation.y *= 8;
		translation.y = (float)(((int)(translation.y)) / 8.0f);
		translation.z += (translation.z > 0 ? (0.125f / 2) : (-0.125f / 2));
		translation.z *= 8;
		translation.z = (float)(((int)(translation.z)) / 8.0f);
	}
	
	if(ui->fixed_axis & FIXED_X) translation.x = 0;
	if(ui->fixed_axis & FIXED_Y) translation.y = 0;
	if(ui->fixed_axis & FIXED_Z) translation.z = 0;
	
	//show up to 256 points
	for(i = 0; i < ui->vertex_count; ++i){
		if(num_vertices == 256) break;
		if(ui->selected_vertices[i]){
			//create the renderable
			v3 = PWM_add3(ui->selected_vertices[i]->position, translation);
			
			pwrenderable_reset(ui->new_vertices[i]);
			pwrenderable_init_box(ui->new_vertices[i], PWM_vec3(.1, .1, .1), 0xff00ffff, NULL);
			pwrenderable_transform(ui->new_vertices[i], PWM_translation(v3));
			
			num_vertices++;
		}
	}
	
	if(pwengine_is_left_button_pressed(engine)){
		for(i = 0; i < ui->vertex_count; ++i){
			if(ui->selected_vertices[i]){
				v3 = PWM_add3(ui->selected_vertices[i]->position, translation);
				ui->selected_vertices[i]->position = v3;
			}
		}
		//modify the shapes and recreate the renderable 
		vertices_to_shape(ui->vertices, NULL, ui->indices, NULL, ui->shape);
		ui_update_add_shape(ui, engine);
		
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	if(pwengine_is_key_pressed(engine, "Escape")){
		for(i = 0; i < 256; ++i) pwrenderable_reset(ui->new_vertices[i]);
		ui->state = MODE_SELECT;
	}
	
	pwlayer_render(&ui->layer_axes);
	pwlayer_render(&ui->layer_shape);
	pwlayer_render(&ui->layer_new_vertices);
	
	printf("Translation: %f %f %f \r", translation.x, translation.y, translation.z);
	
	return 0;
}

int ui_update_add_shape(UI *ui, PWEngine *engine){
	int i;
	PWRenderable r;
	
	if(ui->vertices) free(ui->vertices); ui->vertices = NULL;
	ui->vertex_count = 0;
	if(ui->indices) free(ui->indices); ui->indices = NULL;
	ui->index_count = 0;printf("a");
	shape_to_vertices(ui->shape, &ui->vertices, &ui->vertex_count, &ui->indices, &ui->index_count);
	vertices_to_renderable(ui->model, ui->vertices, ui->vertex_count, ui->indices, ui->index_count);
	pwrenderable_set_texture(ui->model, pwengine_get_texture(engine, "texture.png"));
	
	ui->selected_vertices = realloc(ui->selected_vertices, sizeof(PWVertexData*) * ui->vertex_count);
	ui->selected_indices = realloc(ui->selected_indices, sizeof(unsigned short*) * ui->index_count);
	memset(ui->selected_vertices, 0, sizeof(PWVertexData*) * ui->vertex_count);
	memset(ui->selected_indices, 0, sizeof(unsigned short*) * ui->index_count);
	
	pwlayer_reset(&ui->layer_vertex_boxes);
	ui->vertex_boxes = realloc(ui->vertex_boxes, sizeof(PWRenderable*) * ui->vertex_count);
	
	for(i = 0; i < ui->vertex_count; ++i){
		pwrenderable_init_box(&r, PWM_vec3(.1, .1, .1), 0x00000000, NULL);
		pwrenderable_transform(&r, PWM_translation(ui->vertices[i].position));
		ui->vertex_boxes[i] = pwlayer_add(&ui->layer_vertex_boxes, &r);
	}
	//the layer's projection matrix must also be updated since it is reinitialized
	pwlayer_set_projection(&ui->layer_vertex_boxes, pwpersp_mat4(&ui->persp));
	
	return 0;
}

int handle_uv_edit(PWEngine *engine, UI *ui){
	PWAabb aabb_top_left;
	PWAabb aabb_bottom_right;
	PWVec3 v3;
	PWPersp persp;
	PWRay ray;
	
	v3 = PWM_vec3(.1, .1, .1);
	
	//calculate the position of the box where it would go
	persp = pwpersp();
	persp.pov.z = persp.dist;
	ray = pwpersp_ray(&persp, PWM_vec2(pwengine_mouse_x(engine), pwengine_mouse_y(engine)));
	
	if(pwengine_is_left_button_pressed(engine)){
		//check if mouse collides with the aabb's
		aabb_top_left = PWM_aabb(PWM_add3(ui->uv_top_left, v3), PWM_sub3(ui->uv_top_left, v3));
		aabb_bottom_right = PWM_aabb(PWM_add3(ui->uv_bottom_right, v3), PWM_sub3(ui->uv_bottom_right, v3));
		
		if(PWM_ray_x_aabb(&ray, &aabb_top_left, &v3)){
			ui->uv_edit_state = 1;
		}
		if(PWM_ray_x_aabb(&ray, &aabb_bottom_right, &v3)){
			ui->uv_edit_state = 2;
		}
	}
	if(pwengine_is_left_button_lifted(engine)){
		ui->uv_edit_state = 0;
	}
	
	if(pwengine_is_left_button_held(engine)){
		v3 = PWM_mul3(PWM_add3(persp.p_near, persp.p_far), 0.5);
		v3.z = 0;
		
		v3.x += (v3.x > 0 ? (1.0f / 32) : (-1.0f / 32));
		v3.x *= 16;
		v3.x = (float)(((int)(v3.x)) / 16.0f);
		v3.y += (v3.y > 0 ? (1.0f / 32) : (-1.0f / 32));
		v3.y *= 16;
		v3.y = (float)(((int)(v3.y)) / 16.0f);
		
		//TODO restrict movement of corners to out of bounds area
		
		if(ui->uv_edit_state == 1){
			if(v3.x < -1.0f) v3.x = -1.0f;
			if(v3.x > 1.0f) v3.x = 1.0f;
			if(v3.y < -1.0f) v3.y = -1.0f;
			if(v3.y > 1.0f) v3.y = 1.0f;
			if(v3.x > ui->uv_bottom_right.x) v3.x = ui->uv_bottom_right.x;
			if(v3.y < ui->uv_bottom_right.y) v3.y = ui->uv_bottom_right.y;
			
			ui->uv_top_left = v3;
		}
		else if(ui->uv_edit_state == 2){
			if(v3.x < -1.0f) v3.x = -1.0f;
			if(v3.x > 1.0f) v3.x = 1.0f;
			if(v3.y < -1.0f) v3.y = -1.0f;
			if(v3.y > 1.0f) v3.y = 1.0f;
			if(v3.x < ui->uv_top_left.x) v3.x = ui->uv_top_left.x;
			if(v3.y > ui->uv_top_left.y) v3.y = ui->uv_top_left.y;
			
			ui->uv_bottom_right = v3;
		}
	}
	
	//render the boxes
	pwrenderable_reset(ui->r_top_left);
	pwrenderable_init_box(ui->r_top_left, PWM_vec3(.1, .1, .1), 0xffff0000, NULL);
	pwrenderable_transform(ui->r_top_left, PWM_translation(ui->uv_top_left));
	pwrenderable_reset(ui->r_bottom_right);
	pwrenderable_init_box(ui->r_bottom_right, PWM_vec3(.1, .1, .1), 0xffff0000, NULL);
	pwrenderable_transform(ui->r_bottom_right, PWM_translation(ui->uv_bottom_right));
	
	printf("UV TOP LEFT: %f %f, BOTTOM RIGHT: %f %f\r", ui->uv_top_left.x, ui->uv_top_left.y, ui->uv_bottom_right.x, ui->uv_bottom_right.y);
	
	if(pwengine_is_key_pressed(engine, "Return")){
		//set UV's for the shape TODO
		ui->uv_edit_shape->uv_top_left.x = (ui->uv_top_left.x + 1.0f) / 2.0f;
		ui->uv_edit_shape->uv_top_left.y = (1.0f - ui->uv_top_left.y) / 2.0f;
		ui->uv_edit_shape->uv_bottom_right.x = (ui->uv_bottom_right.x + 1.0f) / 2.0f;
		ui->uv_edit_shape->uv_bottom_right.y = (1.0f - ui->uv_bottom_right.y) / 2.0f;
		
		//clean states
		ui->uv_top_left = PWM_vec3(-1.0f, 1.0f, 0);
		ui->uv_bottom_right = PWM_vec3(1.0f, -1.0f, 0);
		ui->uv_edit_state = 0;
		
		//reload shape
		ui_update_add_shape(ui, engine);
		
		ui->state = MODE_SELECT;
	}
	
	pwlayer_set_projection(&ui->layer_uv_edit, pwpersp_mat4(&persp));
	pwlayer_render(&ui->layer_uv_edit);
}

int transform_indices(UI *ui, PWMat4 transform){
	//TODO
	return 0;
}

int remove_indices(UI *ui){
	//TODO
	return 0;
}


