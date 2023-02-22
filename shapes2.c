//4 dec 2022

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc shapes2.c engine/pwmath.c -m32 -msse -lmingw32 -o shapes2 -static

//TODO
//output the 3d text file to "a"

#include "engine/pwmath.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct PWVertexData{
	PWVec3 position;
	PWVec2 uv;
	float tid;
	unsigned int color;
	PWVec3 normal;
} PWVertexData;

char *convert(char *s, float x)
{
    char *buf = malloc(100);
    char *p;
    int ch;

	//original used %.10f
    sprintf(buf, "%.6f", x);
    p = buf + strlen(buf) - 1;
    while (*p == '0' && *p-- != '.');
    *(p+1) = '\0';
    if (*p == '.') *p = '\0';
    strcpy(s, buf);
    free (buf);
    return s;
}

void print_model(PWVertexData *vertices, int vertex_count, unsigned short *indices, int index_count){
	int i;
	FILE *out;
	char str[3][20];
	
	static int a = 0;
	
	out = fopen("shape.txt", "w");
	//out = stdout;
	
	fprintf(out, "PWR1\n\nVERTICES %d\n", vertex_count);
	for(i = 0; i < vertex_count; ++i){
		fprintf(out, "%d ", i);
		fprintf(out, "position %s %s %s ", convert(str[0], vertices[i].position.x), convert(str[1], vertices[i].position.y), convert(str[2], vertices[i].position.z));
		fprintf(out, "uv %s %s ", convert(str[0], vertices[i].uv.x), convert(str[1], vertices[i].uv.y));
		fprintf(out, "color %x ", vertices[i].color);
		vertices[i].normal = PWM_normalize3(vertices[i].normal);
		fprintf(out, "normal %s %s %s\n", convert(str[0], vertices[i].normal.x), convert(str[1], vertices[i].normal.y), convert(str[2], vertices[i].normal.z));
		//fprintf(out, "%d position %f %f %f uv %f %f color %x normal %f %f %f\n", i, vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, vertices[i].uv.x, vertices[i].uv.y, vertices[i].color, vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z);
	}
	
	fprintf(out, "\nINDICES %d\n", index_count);
	for(i = 0; i < index_count; i += 3){
		fprintf(out, "%d %d %d\n", indices[i], indices[i+1], indices[i+2]);
	}
	
	a ^= 1;
	if(a){
		fprintf(out, "%*s\n", rand() % 100 + 1, "");
	}
	
	fclose(out);
}

void vf_log(const char *format, ...){
	va_list args;
	FILE *session; 
	session = fopen("shape.log", "a");
	va_start(args, format);
	vfprintf(session, format, args);
	va_end(args);
	fclose(session);
}

int read_model(const char *filename, PWVertexData **vertices, int *vertex_count, unsigned short **indices, int *index_count){
	PWVec3 position;
	PWVec2 uv;
	unsigned int color;
	PWVec3 normal;
	char str[16];
	FILE *in;
	int i, j;
	
	int vertex;
	
	//NOTE: no .obj file, PWR0 support
	
	in = fopen(filename, "rt");
	if(!in){
		return -1;
	}
	
	fgets(str, 16, in);
	if(strcmp(str, "PWR1\n") == 0){
		fscanf(in, "%*s %d", vertex_count); //expect VERTICES <vertex_count>
		
		//expect <vertex number> <vertex data>
		*vertices = (PWVertexData*) malloc(sizeof(PWVertexData) * (*vertex_count));
		
		for(i = 0; i < (*vertex_count); ++i){
			fscanf(in, "%*d %*s %f%f%f %*s %f%f %*s %x %*s %f%f%f", &position.x, &position.y, &position.z, &uv.x, &uv.y, &color, &normal.x, &normal.y, &normal.z);
			(*vertices)[i].position = position;
			(*vertices)[i].uv = uv;
			(*vertices)[i].color = color;
			(*vertices)[i].normal = normal;
		}
		
		fscanf(in, "%*s %d", index_count); //expect INDICES <index_count>
		*indices = (unsigned short*) malloc(sizeof(unsigned short) * (*index_count));
		
		for(i = 0; i < (*index_count); ++i){
			fscanf(in, "%d", &vertex); //we need to use an int type with fscanf
			(*indices)[i] = vertex;
		}
		
	}
	else{
		fclose(in);
		return -1;
	}
	
	fclose(in);
	return 0;
}

void reverse_indices(unsigned short *indices, int index_count){
	int i;
	unsigned short tmp;
	for(i = 0; i < index_count; i += 3){
		tmp = indices[i + 1];
		indices[i + 1] = indices[i + 2];
		indices[i + 2] = tmp;
	}
}

//indices should be (end0-start0 + end1-start1 - 2) * 3 long
void connect_points(unsigned short *indices, int start0, int end0, int start1, int end1){
	float a, b, a_len, b_len, a_inc, b_inc;
	int i;
	
	a_len = end1 - start1;
	b_len = end0 - start0;
	
	if(a_len > 0){
		if(fabs(a_len) > fabs(b_len)){
			a_inc = 1.0f;
		}
		else{
			a_inc = fabs(1.0f * a_len / b_len);
		}
	}
	else{
		if(fabs(a_len) > fabs(b_len)){
			a_inc = -1.0f;
		}
		else{
			a_inc = -fabs(1.0f * a_len / b_len);
		}
	}
	
	if(b_len > 0){
		if(fabs(b_len) > fabs(a_len)){
			b_inc = 1.0f;
		}
		else{
			b_inc = fabs(1.0f * b_len / a_len);
		}
	}
	else{
		if(fabs(b_len) > fabs(a_len)){
			b_inc = -1.0f;
		}
		else{
			b_inc = -fabs(1.0f * b_len / a_len);
		}
	}
	
	if(a_len > 0){
		a = start1;
	}
	else{
		a = (float)(start1) + a_inc;
	}
	if(b_len > 0){
		b = start0;
	}
	else{
		b = (float)(start0) + b_inc;
	}
	
	i = 0;
	while(i < abs(end1 - start1) + abs(end0 - start0) - 2){
		if((int)(a + a_inc) != (int)(a)){
			indices[i * 3 + 0] = (int)(a);
			indices[i * 3 + 1] = (int)(b);
			indices[i * 3 + 2] = (int)(a + a_inc);
			++i;
			
			if((int)(b + b_inc) != (int)(b)){
				indices[i * 3 + 0] = (int)(a + a_inc);
				indices[i * 3 + 1] = (int)(b);
				indices[i * 3 + 2] = (int)(b + b_inc);
				++i;
			}
		}
		else if((int)(b + b_inc) != (int)(b)){
			indices[i * 3 + 0] = (int)(a);
			indices[i * 3 + 1] = (int)(b);
			indices[i * 3 + 2] = (int)(b + b_inc);
			++i;
		}
		a += a_inc;
		b += b_inc;
	}
}

//remove occurence of triangles with the same indices as start_indices.
//call this before creating the new vertices/indices in an extrude
void remove_indices(unsigned short *start_indices, int num_points, unsigned short *group_indices, int *group_index_count){
	int i;
	int j;
	int num_matches;
	
	for(i = 0; i < *group_index_count; i += 3){
		//make comparison with all of start_indices
		num_matches = 0;
		for(j = 0; j < num_points; ++j){
			if(group_indices[i + 0] == start_indices[j]){
				num_matches++;
				break;
			}
		}
		for(j = 0; j < num_points; ++j){
			if(group_indices[i + 1] == start_indices[j]){
				num_matches++;
				break;
			}
		}
		for(j = 0; j < num_points; ++j){
			if(group_indices[i + 2] == start_indices[j]){
				num_matches++;
				break;
			}
		}
		
		//mark this triangle to be removed
		if(num_matches == 3){
			group_indices[i + 0] = 0;
			group_indices[i + 1] = 0;
		}
	}
	
	//remove indices
	j = 0;
	for(i = 0; i < *group_index_count; i += 3){
		if(group_indices[i + 0] != 0 || group_indices[i + 1] != 0){
			group_indices[j++] = group_indices[i + 0];
			group_indices[j++] = group_indices[i + 1];
			group_indices[j++] = group_indices[i + 2];
		}
	}
	*group_index_count = j;
}
	

//let vertices point to three vertices reserved for the plane
void draw_plane(PWVertexData *vertices, PWPlane *p){
	PWVec3 v[2];
	PWVec3 n[3];
	int i = 0;
	
	for(i = 0; i < 3; ++i){
		vertices[i].color = 0x8000ff00;
		vertices[i].normal = PWM_vec3(0, 0, 1);
		vertices[i].uv = PWM_vec2(0, 0);
		vertices[i].tid = 0;
	}
	
	n[0] = PWM_vec3(1.0f, 0.0f, 0.0f);
	n[1] = PWM_vec3(0.0f, 1.0f, 0.0f);
	n[2] = PWM_vec3(0.0f, 0.0f, 1.0f);
	
	//to retrieve two orthogonal vectors, we must get two vectors linearly
	//independant from the plane normal. We will use the basis vectors.
	
	//get a vector that is linearly independant from plane normal
	i = 0;
	while(PWM_norm3(PWM_cross3(p->normal, n[i])) < 0.001f) i++;

	//v[0] is now orthogonal to plane normal and n[i]
	v[0] = PWM_cross3(p->normal, n[i]);
	
	//v[0] is now an orthonormal vector to the plane's normal
	v[0] = PWM_sub3(v[0], PWM_mul3(n[i], PWM_dot3(v[0], p->normal)));
	v[0] = PWM_normalize3(v[0]);
	
	//v[1] is orthonormal to plane normal and v[0]
	v[1] = PWM_cross3(p->normal, v[0]);
	
	vertices[0].position = PWM_add3(PWM_mul3(p->normal, -p->distance), PWM_mul3(PWM_add3(PWM_mul3(v[0], 0), PWM_mul3(v[1], 1)), 100));
	vertices[1].position = PWM_add3(PWM_mul3(p->normal, -p->distance), PWM_mul3(PWM_add3(PWM_mul3(v[0], -1), PWM_mul3(v[1], -1)), 100));
	vertices[2].position = PWM_add3(PWM_mul3(p->normal, -p->distance), PWM_mul3(PWM_add3(PWM_mul3(v[0], 1), PWM_mul3(v[1], -1)), 100));
}

//vertices and indices are arrays of pointers to the model's vertices and indices
void ask_for_transform(
	int vertex_count,
	PWVertexData **vertices,
	int index_count,
	unsigned short **indices,
	int group_vertex_count,
	PWVertexData *group_vertices,
	int group_index_count,
	unsigned short *group_indices
){
	PWVertexData *backup_vertices;
	unsigned short *backup_indices;
	int transformation_type;
	PWMat4 transformation;
	PWVec3 rotation_axis;
	float rotation;
	PWVec3 scale;
	PWVec3 translation;
	
	int origin_set = 0;
	PWVec3 origin;
	
	int start_index;
	int end_index;
	
	int i;
	int j;
	int k;
	int tmp;
	
	backup_vertices = (PWVertexData*) malloc(sizeof(PWVertexData) * vertex_count);
	backup_indices = (unsigned short*) malloc(sizeof(unsigned short) * index_count);
	for(i = 0; i < vertex_count; ++i){
		backup_vertices[i] = *(vertices[i]);
	}
	for(i = 0; i < index_count; ++i){
		backup_indices[i] = *(indices[i]);
	}
	
	//print it on screen
	print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
	
	//ask for any transformation
	transformation = PWM_ini(1.0, NULL);
	do{
		printf("Any transformation? (0) No, (1) Rotation, (2) Scale, (3) Translation, (4) Reset transformation, (5) Set origin, (6) Set origin from vertices: ");
		scanf("%d", &transformation_type); vf_log("%d ", transformation_type);
		
		switch(transformation_type){
		case 1:
			printf("What is the axis of rotation? ");
			scanf("%f %f %f", &rotation_axis.x, &rotation_axis.y, &rotation_axis.z); vf_log("%f %f %f ", rotation_axis.x, rotation_axis.y, rotation_axis.z);
			
			printf("What is the angle of rotation (degrees)? ");
			scanf("%f", &rotation); vf_log("%f ", rotation);
			
			transformation = PWM_rotation(rotation, rotation_axis);
			break;
		case 2:
			printf("What is the scale? ");
			scanf("%f %f %f", &scale.x, &scale.y, &scale.z); vf_log("%f %f %f ", scale.x, scale.y, scale.z);
			transformation = PWM_scale(scale);
			
			//reverse indices if reflected an odd number of times
			if(scale.x * scale.y * scale.z < 0.0f){
				for(i = 0; i < index_count; i += 3){
					tmp = *(indices[i + 1]);
					*(indices[i + 1]) = *(indices[i + 2]);
					*(indices[i + 2]) = tmp;
				}
			}
			break;
		case 3:
			printf("What is the translation? ");
			scanf("%f %f %f", &translation.x, &translation.y, &translation.z); vf_log("%f %f %f ", translation.x, translation.y, translation.z);
			transformation = PWM_translation(translation);
			break;
		case 4: //reset
			for(i = 0; i < vertex_count; ++i){
				*(vertices[i]) = backup_vertices[i];
			}
			for(i = 0; i < index_count; ++i){
				*(indices[i]) = backup_indices[i];
			}
			transformation = PWM_ini(1.0, NULL);
			origin_set = 0;
			break;
		case 5:
			printf("What is the origin? ");
			scanf("%f %f %f", &origin.x, &origin.y, &origin.z); vf_log("%f %f %f ", origin.x, origin.y, origin.z); 
			origin_set = 1;
			break;
		case 6:
			printf("Type in the start and end index of the shape: ");
			scanf("%d %d", &start_index, &end_index); vf_log("%d %d ", start_index, end_index);
			
			origin = PWM_vec3(0, 0, 0);
			k = 0;
			for(i = start_index; i < end_index; ++i){
				//check if previous vertices have the same position
				for(j = start_index; j < i; ++j){
					if(group_vertices[j].position.x == group_vertices[i].position.x
						&& group_vertices[j].position.y == group_vertices[i].position.y
						&& group_vertices[j].position.z == group_vertices[i].position.z){
						
						break;
					}
				}
				if(j == i){
					origin = PWM_add3(group_vertices[i].position, origin);
					++k;
				}
			}
			origin = PWM_mul3(origin, 1.0f / k);
			
			
			
			printf("Origin is %f %f %f\n", origin.x, origin.y, origin.z);
			origin_set = 1;
			break;
		default:
			break;
		}
		
		//perform the transformation
		if(transformation_type == 1 || transformation_type == 2 || transformation_type == 3){
			if(origin_set){
				transformation = PWM_mul(PWM_translation(origin), transformation);
				transformation = PWM_mul(transformation, PWM_inv(PWM_translation(origin)));
			}
			for(i = 0; i < vertex_count; ++i){
				(vertices[i])->position = PWM_mul_vec3(transformation, (vertices[i])->position);
				(vertices[i])->normal = PWM_mul_vec3_notranslate(transformation, (vertices[i])->normal);
			}
		}
		
		//show the model
		print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		
		
	} while(transformation_type != 0);
	
	free(backup_vertices);
	free(backup_indices);
}

void ask_for_uv(int vertex_count, PWVertexData **vertices, int index_count, unsigned short **indices);

int main(void){
	int num_groups;               //all shapes
	int group_vertex_count;       //all shapes
	PWVertexData *group_vertices = NULL; //all shapes
	int group_index_count;        //all shapes
	unsigned short *group_indices = NULL; //all shapes' indices
	char input[500];
	
	int connect_shape;     //connecting vertices
	int start_index[2];
	int end_index[2];
	int num_new_triangles; //connecting vertices
	int num_new_indices;   //connecting vertices
	int connection_ok;     //connecting vertices
	int num_reconnections; //connecting vertices
	
	//creating a primitive
	
	int shape;      //prompted after asking to connect shape
	
	int degree;            //cone and curve
	int num_points;        //cone and curve
	float length;          //cone and curve
	PWVec3 *cross_section_points; //cone and curve
	PWVec2 *modulation;           //cone and curve
	float *length_points;         //cone and curve
	int i;
	int j;
	int k;
	int l;
	
	
	float *width_points;       //grid
	float *height_points;      //grid
	int num_width;             //grid
	int num_height;            //grid
	PWVec2 mesh_size;          //grid
	
	int vertex_count;          //for current shape
	int index_count;           //for current shape
	PWVertexData **vertices = NULL;
	unsigned short **indices = NULL;  //for current shape
	
	int transformation_type; //after creating the shape
	PWVec3 rotation_axis;    //after creating the shape
	float rotation;          //after creating the shape
	PWVec3 scale;            //after creating the shape
	PWVec3 translation;      //after creating the shape
	PWMat4 transformation;   //after creating the shape
	
	float uv_left;   //adjustment of UVs after transformation
	float uv_top;    //..
	float uv_right;  //..
	float uv_bottom; //..
	
	unsigned short *start_indices; //extrude option
	PWVec3 extrude_direction;      //extrude option
	PWVec3 polygon_center;
	int extrude_ok;                //extrude option
	
	int select_ok;
	PWVec3 select_point;
	PWVec3 select_normal;
	PWPlane select_plane; //select option, by plane separation
	
	
	num_groups = 0;
	group_vertex_count = 0;
	group_vertices = NULL;
	group_index_count = 0;
	group_indices = NULL;
	
	//clear the log file
	fclose(fopen("shape.log", "w"));
	
	while(!feof(stdin)){
		if(num_groups > 0){
			printf("Connect vertices together? (0) no, (1) connection, (2) primitive: ");
			scanf("%d", &connect_shape); vf_log("\n%d ", connect_shape);
			
			if(connect_shape == 1){
				printf("Type in the start and end index for one shape: ");
				scanf("%d %d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
				
				printf("Type in the start and end index for another shape: ");
				scanf("%d %d", &start_index[1], &end_index[1]); vf_log("%d %d ", start_index[1], end_index[1]);
				
				num_new_triangles = end_index[0] - start_index[0] + end_index[1] - start_index[1] - 2;
				if(num_new_triangles < 1){
					//num_new_triangles = 0;
					continue;
				}
				index_count = num_new_triangles * 3;
				
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
				indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
				
				group_index_count += index_count;
				
				//create the indices
				connect_points(&group_indices[group_index_count - index_count], start_index[1], end_index[1], start_index[0], end_index[0]);
				
				//print the file
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				
				//ask if it is okay
				num_reconnections = 0;
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				
				if(connection_ok){
					continue;
				}
				
				connect_points(&group_indices[group_index_count - index_count], start_index[0], end_index[0], start_index[1], end_index[1]); //same as last but triangles reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				connect_points(&group_indices[group_index_count - index_count], end_index[1], start_index[1], start_index[0], end_index[0]); //vertices are reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				connect_points(&group_indices[group_index_count - index_count], start_index[0], end_index[0], end_index[1], start_index[1]); //triangles and vertices reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				
				if(!connection_ok){
					group_index_count -= index_count;
					group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * group_index_count);
					indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * group_index_count);
					print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				}
				
				continue;
			}
			else if(connect_shape == 2){
				
				printf("Type in the start and end index of the shape to fill in: ");
				scanf("%d %d", &start_index[0], &end_index[0]);  vf_log("%d %d ", start_index[0], end_index[0]);
				
				num_new_triangles = end_index[0]- start_index[0] - 2;
				index_count = 3 * num_new_triangles;
				
				if(num_new_triangles < 1){
					continue;
				}
				
				//create the primitive
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
				indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
				
				group_index_count += index_count;
				for(i = 0; i < num_new_triangles; ++i){
					group_indices[group_index_count - index_count + i*3 + 0] = start_index[0];
					group_indices[group_index_count - index_count + i*3 + 1] = start_index[0] + i + 1;
					group_indices[group_index_count - index_count + i*3 + 2] = start_index[0] + i + 2;
				}
				
				
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				
				//reverse the direction of the triangle
				reverse_indices(&group_indices[group_index_count], index_count);
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					
					continue;
				}
				
				//remove the primitive
				group_index_count -= index_count;
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count));
				indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count));
				continue;
			}
		}
		
		printf("Add a shape\n (1) cone\n (2) curve\n (3) grid\n"
			" (4) copy vertices\n (5) load file\n"
			" (6) select vertices by index range\n (7) extrude\n"
			" (8) select vertices by plane\n (9) select vertices individually\n:");
		scanf("%d", &shape);  vf_log("%d ", shape);
		
		if(shape == 0){
			//make the program think that all shapes are currently selected
			vertex_count = group_vertex_count;
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[i];
			}
			index_count = group_index_count;
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[i];
			}
			printf("All vertices selected.\n");
		}
		else if(shape == 1){
			printf("How many points on the length? ");
			scanf("%d", &degree); vf_log("%d ", degree);
			
			printf("How many points in the cross section? ");
			scanf("%d", &num_points); vf_log("%d ", num_points);
			
			printf("What is the length of the shape? ");
			scanf("%f", &length); vf_log("%f ", length);
				
			if(degree < 2){
				degree = 2;
			}
			
			cross_section_points = (PWVec3*) malloc(sizeof(PWVec3) * num_points);
			modulation = (PWVec2*) malloc(sizeof(PWVec2) * (degree - 1));
			length_points = (float*) malloc(sizeof(float) * degree);
			
			
			vertex_count = (degree - 1) * num_points + 1;
			index_count = (1 + 2 * (degree - 2)) * (num_points - 1) * 3;
			
			
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
			vertices = (PWVertexData**) realloc(vertices, sizeof(PWVertexData*) * (group_vertex_count + vertex_count));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
			indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
			
			//select the vertices/indices
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[group_vertex_count + i];
			}
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[group_index_count + i];
			}
			group_vertex_count += vertex_count;
			group_index_count += index_count;
			
			//get the cross section points from user
			printf("Input %d ordered pairs for the cross section shape: ", num_points);
			for(i = 0; i < num_points; ++i){
				scanf("%f %f", &cross_section_points[i].x, &cross_section_points[i].y); vf_log("%f %f ", cross_section_points[i].x, cross_section_points[i].y);
			}
			
			//get the modulation in the x/y direction
			printf("Enter %d ordered pairs for the modulation along the length: ", degree - 1);
			for(i = 0; i < degree - 1; ++i){
				scanf("%f %f", &modulation[i].x, &modulation[i].y); vf_log("%f %f ", modulation[i].x, modulation[i].y);
			}
			
			printf("Enter %d values for the placement of the points along the length: ", degree);
			for(i = 0; i < degree; ++i){
				scanf("%f", &length_points[i]); vf_log("%f ", length_points[i]);
			}
			
			//fill in the rest of the points now
			
			//origin
			vertices[0]->position = PWM_vec3(0, 0, -length_points[0] * length);
			vertices[0]->uv = PWM_vec2(0.5, 0);
			vertices[0]->tid = 0;
			vertices[0]->color = 0xffffffff;
			vertices[0]->normal = PWM_vec3(0, 0, 1);
			
			//behind the origin (cone body)
			for(i = 0; i < degree - 1; ++i){
				for(j = 0; j < num_points; ++j){
					vertices[i * num_points + j + 1]->position.x = cross_section_points[j].x * modulation[i].x;
					vertices[i * num_points + j + 1]->position.y = cross_section_points[j].y * modulation[i].y;
					vertices[i * num_points + j + 1]->position.z = -length_points[i + 1] * length;
					vertices[i * num_points + j + 1]->uv = PWM_vec2(j / (num_points - 1.0), (i + 1.0) / (degree - 1.0));
					vertices[i * num_points + j + 1]->tid = 0.0;
					vertices[i * num_points + j + 1]->color = 0xffffffff;
					vertices[i * num_points + j + 1]->normal = PWM_normalize3(PWM_vec3(vertices[i * num_points + j + 1]->position.x, vertices[i * num_points + j + 1]->position.y, 0));
				}
			}
			
			//index the points
			//top triangles
			for(i = 0; i < num_points - 1; ++i){
				*(indices[i * 3 + 0]) = 0;
				*(indices[i * 3 + 1]) = i + 1;
				*(indices[i * 3 + 2]) = i + 2;
			}
			k = i * 3;
			
			//squares not connected to the origin
			//1 num_points+1 2, 2 num_points+1 num_points+2, etc...
			for(i = 0; i < degree - 2; ++i){
				for(j = 0; j < num_points - 1; ++j){
					*(indices[(i * (num_points - 1) + j) * 6 + 0 + k]) = i * num_points + j + 1;
					*(indices[(i * (num_points - 1) + j) * 6 + 1 + k]) = (i + 1) * num_points + j + 1;
					*(indices[(i * (num_points - 1) + j) * 6 + 2 + k]) = i * num_points + j + 2;
					
					*(indices[(i * (num_points - 1) + j) * 6 + 3 + k]) = i * num_points + j + 2;
					*(indices[(i * (num_points - 1) + j) * 6 + 4 + k]) = (i + 1) * num_points + j + 1;
					*(indices[(i * (num_points - 1) + j) * 6 + 5 + k]) = (i + 1) * num_points + j + 2;
				}
			}
			
			for(i = 0; i < index_count; ++i){
				*(indices[i]) += group_vertex_count - vertex_count;
			}
			
			free(cross_section_points);
			free(modulation);
			free(length_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count);
			printf("The indices for the tip of the cone is  [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count - vertex_count+ 1);
			printf("The indices for the edge of the cone is [%d %d]\n", group_vertex_count - num_points, group_vertex_count);
		}
		//curve
		else if(shape == 2){
			printf("How many points on the length? ");
			scanf("%d", &degree); vf_log("%d ", degree);
			
			printf("How many points in the cross section? ");
			scanf("%d", &num_points); vf_log("%d ", num_points);
			
			printf("What is the length of the shape? ");
			scanf("%f", &length); vf_log("%f ", length);
		
			if(degree < 2){
				degree = 2;
			}
			
			cross_section_points = (PWVec3*) malloc(sizeof(PWVec3) * num_points);
			modulation = (PWVec2*) malloc(sizeof(PWVec2) * degree);
			length_points = (float*) malloc(sizeof(float) * degree);
			
			
			vertex_count = degree * num_points;
			index_count = 2 * (degree - 1) * (num_points - 1) * 3;
			
			
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
			vertices = (PWVertexData**) realloc(vertices, sizeof(PWVertexData*) * (group_vertex_count + vertex_count));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
			indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
			
			
			//select the vertices/indices
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[group_vertex_count + i];
			}
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[group_index_count + i];
			}
			group_vertex_count += vertex_count;
			group_index_count += index_count;
			
			//get the cross section points from user
			printf("Input %d ordered pairs for the cross section shape: ", num_points);
			for(i = 0; i < num_points; ++i){
				scanf("%f %f", &cross_section_points[i].x, &cross_section_points[i].y); vf_log("%f %f ", cross_section_points[i].x, cross_section_points[i].y);
			}
			
			//get the modulation in the x/y direction
			printf("Enter %d ordered pairs for the modulation along the length: ", degree);
			for(i = 0; i < degree; ++i){
				scanf("%f %f", &modulation[i].x, &modulation[i].y); vf_log("%f %f ", modulation[i].x, modulation[i].y);
			}
			
			printf("Enter %d values for the placement of the points along the length: ", degree);
			for(i = 0; i < degree; ++i){
				scanf("%f", &length_points[i]); vf_log("%f ", length_points[i]);
			}
			
			//fill in the rest of the points now
			for(i = 0; i < degree; ++i){
				for(j = 0; j < num_points; ++j){
					vertices[i * num_points + j]->position.x = cross_section_points[j].x * modulation[i].x;
					vertices[i * num_points + j]->position.y = cross_section_points[j].y * modulation[i].y;
					vertices[i * num_points + j]->position.z = -length_points[i] * length;
					vertices[i * num_points + j]->uv = PWM_vec2(j / (num_points - 1.0), i / (degree - 1.0));
					vertices[i * num_points + j]->tid = 0.0;
					vertices[i * num_points + j]->color = 0xffffffff;
					vertices[i * num_points + j]->normal = PWM_normalize3(PWM_vec3(vertices[i * num_points + j]->position.x, vertices[i * num_points + j]->position.y, 0));
				}
			}
			
			//index the points
			
			//squares not connected to the origin
			//1 num_points+1 2, 2 num_points+1 num_points+2, etc...
			for(i = 0; i < degree - 1; ++i){
				for(j = 0; j < num_points - 1; ++j){
					*(indices[(i * (num_points - 1) + j) * 6 + 0]) = i * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 1]) = (i + 1) * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 2]) = i * num_points + j + 1;
					
					*(indices[(i * (num_points - 1) + j) * 6 + 3]) = i * num_points + j + 1;
					*(indices[(i * (num_points - 1) + j) * 6 + 4]) = (i + 1) * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 5]) = (i + 1) * num_points + j + 1;
				}
			}
			
			for(i = 0; i < index_count; ++i){
				*(indices[i]) += group_vertex_count - vertex_count;
			}
			
			free(cross_section_points);
			free(modulation);
			free(length_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count);
			printf("The indices for the curve front is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count - vertex_count + num_points);
			printf("The indices for the curve end is   [%d %d]\n", group_vertex_count - num_points, group_vertex_count);
		} //else if(shape == 2)
			
		//create a rectangular mesh
		else if(shape == 3){
			printf("How many points along the width of the mesh: ");
			scanf("%d", &num_width); vf_log("%d ", num_width);
			
			printf("How many points along the height of the mesh: ");
			scanf("%d", &num_height); vf_log("%d ", num_height);
			
			printf("Enter the width and height: ");
			scanf("%f %f", &mesh_size.x, &mesh_size.y); vf_log("%f %f ", mesh_size.x, mesh_size.y);
			
			if(num_width < 2) num_width = 2;
			if(num_height < 2) num_height = 2;
			
			//memory allocation
			width_points = (float*) malloc(sizeof(float) * num_width);
			height_points = (float*) malloc(sizeof(float) * num_height);
			
			printf("Enter %d values 0.0~1.0 for the position of the points on the width: ", num_width);
			for(i = 0; i < num_width; ++i){
				scanf("%f", &width_points[i]); vf_log("%f ", width_points[i]);
			}
			
			printf("Enter %d values 0.0~1.0 for the position of the points on the height: ", num_height);
			for(i = 0; i < num_height; ++i){
				scanf("%f", &height_points[i]); vf_log("%f ", height_points[i]);
			}
			
			
			vertex_count = num_width * num_height;
			index_count = (num_width - 1) * (num_height - 1) * 6;
			
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
			vertices = (PWVertexData**) realloc(vertices, sizeof(PWVertexData*) * (group_vertex_count + vertex_count));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
			indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
			
			//select the vertices/indices
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[group_vertex_count + i];
			}
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[group_index_count + i];
			}
			group_vertex_count += vertex_count;
			group_index_count += index_count;
			
			//get the vertex point values
			printf("Enter the %d x %d = %d values for the depth of the mesh: ", num_width, num_height, num_width * num_height);
			for(i = 0; i < num_height; ++i){
				for(j = 0; j < num_width; ++j){
					vertices[i * num_width + j]->position.x = mesh_size.x * (width_points[j] - 0.5);
					vertices[i * num_width + j]->position.y = mesh_size.y * (0.5 - height_points[i]);
					scanf("%f", &(vertices[i * num_width + j]->position.z)); vf_log("%f ", vertices[i * num_width + j]->position.z);
					vertices[i * num_width + j]->uv = PWM_vec2(j / (num_width - 1.0), i / (num_height - 1.0));
					vertices[i * num_width + j]->tid = 0.0;
					vertices[i * num_width + j]->color = 0xffffffff;
					vertices[i * num_width + j]->normal = PWM_vec3(0, 0, 1);
				}
			}
			
			//get the index values
			for(i = 0; i < num_height - 1; ++i){
				for(j = 0; j < num_width - 1; ++j){
					*(indices[(i * (num_width - 1) + j) * 6 + 0]) = i * num_width + j;
					*(indices[(i * (num_width - 1) + j) * 6 + 1]) = (i + 1) * num_width + j;
					*(indices[(i * (num_width - 1) + j) * 6 + 2]) = i * num_width + j + 1;
					*(indices[(i * (num_width - 1) + j) * 6 + 3]) = i * num_width + j + 1;
					*(indices[(i * (num_width - 1) + j) * 6 + 4]) = (i + 1) * num_width + j;
					*(indices[(i * (num_width - 1) + j) * 6 + 5]) = (i + 1) * num_width + j + 1;
				}
			}
			
			for(i = 0; i < index_count; ++i){
				*(indices[i]) += group_vertex_count - vertex_count;
			}
			
			free(width_points);
			free(height_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count);
		} //else if shape == 3
		else if(shape == 4){
			printf("Type in the start and end index of the shape to copy: ");
			scanf("%d %d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
			
			if(end_index[0] - start_index[0] < 1){
				continue;
			}
			
			//calculate vertex/index count
			vertex_count = end_index[0] - start_index[0];
			index_count = 0;
			for(i = 0; i < group_index_count; i += 3){
				if(group_indices[i + 0] >= start_index[0] && group_indices[i + 0] < end_index[0]
					&& group_indices[i + 1] >= start_index[0] && group_indices[i + 1] < end_index[0]
					&& group_indices[i + 2] >= start_index[0] && group_indices[i + 2] < end_index[0]){
					
					index_count += 3;
				}
			}
			
			//allocate
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
			vertices = (PWVertexData**) realloc(vertices, sizeof(PWVertexData*) * (group_vertex_count + vertex_count));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
			indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
			
			
			//select the vertices/indices address value
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[group_vertex_count + i];
			}
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[group_index_count + i];
			}
			
			//add to group vertex/index count
			group_vertex_count += vertex_count;
			group_index_count += index_count;
			
			//assign the selected vertices/indices
			for(i = 0; i < vertex_count; ++i){
				*(vertices[i]) = group_vertices[i + start_index[0]];
			}
			//copy the indices (triplets) which are all within [start, end)
			j = 0;
			for(i = 0; i < group_index_count; i += 3){
				if(group_indices[i + 0] >= start_index[0] && group_indices[i + 0] < end_index[0]
					&& group_indices[i + 1] >= start_index[0] && group_indices[i + 1] < end_index[0]
					&& group_indices[i + 2] >= start_index[0] && group_indices[i + 2] < end_index[0]){
					
					*(indices[j++]) = group_indices[i + 0] - start_index[0];
					*(indices[j++]) = group_indices[i + 1] - start_index[0];
					*(indices[j++]) = group_indices[i + 2] - start_index[0];
				}
			}
			
			for(i = 0; i < index_count; ++i){
				*(indices[i]) += group_vertex_count - vertex_count;
			}
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count);
		}
		else if(shape == 5){
			printf("Type in the file name: ");
			scanf("%s", input); vf_log("%s ", input);
			
			free(vertices);
			free(indices);
			free(group_vertices);
			free(group_indices);
			group_vertices = NULL;
			group_vertex_count = 0;
			group_indices = NULL;
			group_index_count = 0;
			
			if(read_model(input, &group_vertices, &group_vertex_count, &group_indices, &group_index_count) != 0){
				printf("Invalid file.\n");
				vertices = NULL;
				indices = NULL;
				continue;
			}
			
			
			vertices = (PWVertexData**) malloc(sizeof(PWVertexData*) * group_vertex_count);
			indices = (unsigned short**) malloc(sizeof(unsigned short*) * group_index_count);
			continue;
		}
		//select indices
		else if(shape == 6){
			//get start/end index
			printf("Type in the start and end index of the shape: ");
			scanf("%d %d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
			
			if(end_index[0] <= start_index[0]){
				continue;
			}
			
			vertex_count = end_index[0] - start_index[0];
			index_count = 0;
			for(i = 0; i < end_index[0] - start_index[0]; ++i){
				vertices[i] = &group_vertices[start_index[0] + i];
			}
			
			j = 0;
			for(i = 0; i < group_index_count; i += 3){
				if(group_indices[i + 0] >= start_index[0] && group_indices[i + 0] < end_index[0]
					&& group_indices[i + 1] >= start_index[0] && group_indices[i + 1] < end_index[0]
					&& group_indices[i + 2] >= start_index[0] && group_indices[i + 2] < end_index[0]){
					
					index_count += 3;
					indices[j++] = &group_indices[i + 0];
					indices[j++] = &group_indices[i + 1];
					indices[j++] = &group_indices[i + 2];
				}
			}
		}//if(shape == 6)
		else if(shape == 7){
			printf("How many points on the length? ");
			scanf("%d", &degree); vf_log("%d ", degree);
			
			printf("How many points in the cross section? ");
			scanf("%d", &num_points); vf_log("%d ", num_points);
			
			printf("What is the length of the shape? ");
			scanf("%f", &length); vf_log("%f ", length);
			
			if(degree < 2){
				degree = 2;
			}
			
			cross_section_points = (PWVec3*) malloc(sizeof(PWVec3) * num_points);
			modulation = (PWVec2*) malloc(sizeof(PWVec2) * degree);
			length_points = (float*) malloc(sizeof(float) * degree);
			start_indices = (unsigned short*) malloc(sizeof(unsigned short) * num_points);
			
			
			
			
			
			//get the cross section points from user
			printf("Input %d indices for the cross section shape: ", num_points);
			for(i = 0; i < num_points; ++i){
				//scanf("%d", &k); vf_log("%d ", k);
				scanf("%hd", &start_indices[i]); vf_log("%hd ", start_indices[i]);
			}
			
			//remove the indices
			remove_indices(start_indices, num_points, group_indices, &group_index_count);
			
			//now reallocate
			vertex_count = degree * num_points;
			index_count = 2 * (degree - 1) * (num_points - 1) * 3;
			
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
			vertices = (PWVertexData**) realloc(vertices, sizeof(PWVertexData*) * (group_vertex_count + vertex_count));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
			indices = (unsigned short**) realloc(indices, sizeof(unsigned short*) * (group_index_count + index_count));
			
			//select the vertices/indices
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[group_vertex_count + i];
			}
			for(i = 0; i < index_count; ++i){
				indices[i] = &group_indices[group_index_count + i];
			}
			group_vertex_count += vertex_count;
			group_index_count += index_count;
			
			for(i = 0; i < num_points; ++i){
				//write the position new vertices
				vertices[i]->position = group_vertices[start_indices[i]].position;
			}
			
			//determine the direction of the extrude
			extrude_direction = PWM_vec3(0, 0, 0);
			for(i = 0; i < num_points - 2; ++i){
				extrude_direction = PWM_add3(PWM_cross3(PWM_sub3(vertices[i + 2]->position, vertices[i + 1]->position), PWM_sub3(vertices[i + 1]->position, vertices[i]->position)), extrude_direction);
				
			}
			extrude_direction = PWM_normalize3(extrude_direction);
			printf("Extrude direction is <%.2f, %.2f, %.2f>\n", extrude_direction.x, extrude_direction.y, extrude_direction.z); 
			
			//get the modulation in the x/y direction
			printf("Enter %d values for the modulation along the length: ", degree);
			for(i = 0; i < degree; ++i){
				scanf("%f", &modulation[i].x); vf_log("%f ", modulation[i].x);
				modulation[i].y = modulation[i].x;
			}
			
			printf("Enter %d values for the placement of the points along the length: ", degree);
			for(i = 0; i < degree; ++i){
				scanf("%f", &length_points[i]); vf_log("%f ", length_points[i]);
			}
			
			//polygon center is the average of the vertices selected
			polygon_center = PWM_vec3(0, 0, 0);
			k = 0;
			for(i = 0; i < num_points; ++i){
				//check if previous vertices have the same position
				for(j = 0; j < i; ++j){
					if(vertices[j]->position.x == vertices[i]->position.x
						&& vertices[j]->position.y == vertices[i]->position.y
						&& vertices[j]->position.z == vertices[i]->position.z){
						
						break;
					}
				}
				if(j == i){
					polygon_center = PWM_add3(vertices[i]->position, polygon_center);
					++k;
				}
			}
			polygon_center = PWM_mul3(polygon_center, 1.0f / k);
			printf("Shape origin is %f %f %f\n", polygon_center.x, polygon_center.y, polygon_center.z);
			
			//fill in the rest of the points now
			for(i = 0; i < num_points; ++i){
				vertices[i]->uv = PWM_vec2(i / (num_points - 1.0), 0);
				vertices[i]->tid = 0.0;
				vertices[i]->color = 0xffffffff;
				vertices[i]->normal = PWM_normalize3(PWM_sub3(vertices[i]->position, polygon_center));
			}
			
			for(i = 1; i < degree; ++i){
				for(j = 0; j < num_points; ++j){
					vertices[i * num_points + j]->position = PWM_add3(polygon_center, PWM_mul3(PWM_sub3(vertices[j]->position, polygon_center), modulation[i].x));
					vertices[i * num_points + j]->position = PWM_add3(vertices[i * num_points + j]->position, PWM_mul3(extrude_direction, length_points[i] * length));
					vertices[i * num_points + j]->uv = PWM_vec2(j / (num_points - 1.0), i / (degree - 1.0));
					vertices[i * num_points + j]->tid = 0.0;
					vertices[i * num_points + j]->color = 0xffffffff;
					vertices[i * num_points + j]->normal = PWM_normalize3(PWM_sub3(vertices[j]->position, polygon_center));
				}
			}
			
			//1 num_points+1 2, 2 num_points+1 num_points+2, etc...
			for(i = 0; i < degree - 1; ++i){
				for(j = 0; j < num_points - 1; ++j){
					*(indices[(i * (num_points - 1) + j) * 6 + 0]) = i * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 1]) = (i + 1) * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 2]) = i * num_points + j + 1;
					
					*(indices[(i * (num_points - 1) + j) * 6 + 3]) = i * num_points + j + 1;
					*(indices[(i * (num_points - 1) + j) * 6 + 4]) = (i + 1) * num_points + j;
					*(indices[(i * (num_points - 1) + j) * 6 + 5]) = (i + 1) * num_points + j + 1;
				}
			}
			
			for(i = 0; i < index_count; ++i){
				*(indices[i]) += group_vertex_count - vertex_count;
			}
			
			print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
			
			printf("Is the extrude visible? (0) No, (1) Yes: ");
			scanf("%d", &extrude_ok); vf_log("%d ", extrude_ok);
			
			//reverse indices
			if(!extrude_ok){
				for(i = 0; i < index_count; i += 3){
					k = *(indices[i + 1]);
					*(indices[i + 1]) = *(indices[i + 2]);
					*(indices[i + 2]) = k;
				}
			}
			
			print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
			
			printf("Is the extrude in the right direction? (0) No, (1) Yes: ");
			scanf("%d", &extrude_ok); vf_log("%d ", extrude_ok);
			
			//recalculate position, reverse indices
			if(!extrude_ok){
				extrude_direction = PWM_mul3(extrude_direction, -1.0f);
				for(i = 1; i < degree; ++i){
					for(j = 0; j < num_points; ++j){
						vertices[i * num_points + j]->position = PWM_add3(polygon_center, PWM_mul3(PWM_sub3(vertices[j]->position, polygon_center), modulation[i].x));
						vertices[i * num_points + j]->position = PWM_add3(vertices[i * num_points + j]->position, PWM_mul3(extrude_direction, length_points[i] * length));
					}
				}
				for(i = 0; i < index_count; i += 3){
					k = *(indices[i + 1]);
					*(indices[i + 1]) = *(indices[i + 2]);
					*(indices[i + 2]) = k;
				}
			}
			
			free(cross_section_points);
			free(modulation);
			free(length_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count);
			printf("The indices for the curve front is [%d %d]\n", group_vertex_count - vertex_count, group_vertex_count - vertex_count + num_points);
			printf("The indices for the curve end is   [%d %d]\n", group_vertex_count - num_points, group_vertex_count);
		}//else if shape == 7
		else if(shape == 8){
			//draw a plane
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + 3));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + 6));
			group_vertex_count += 3;
			group_index_count += 6;
			group_indices[group_index_count - 6] = group_vertex_count - 3;
			group_indices[group_index_count - 5] = group_vertex_count - 2;
			group_indices[group_index_count - 4] = group_vertex_count - 1;
			group_indices[group_index_count - 3] = group_vertex_count - 3;
			group_indices[group_index_count - 2] = group_vertex_count - 1;
			group_indices[group_index_count - 1] = group_vertex_count - 2;
			select_plane = PWM_plane(PWM_vec3(0, 0, 1), PWM_vec3(0, 0, 10000));
			draw_plane(&group_vertices[group_vertex_count - 3], &select_plane);
			
			printf("Remove vertices from selection that lie behind the plane\n");
			
			//set vertices to ALL
			vertex_count = group_vertex_count - 3;
			for(i = 0; i < vertex_count; ++i){
				vertices[i] = &group_vertices[i];
			}
			index_count = 0;
			
			//make selected vertices red
			for(i = 0; i < vertex_count; ++i){
				vertices[i]->color = 0xff0000ff;
			}
			
			print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
			
			select_ok = 0;
			do{
				
				printf("Enter a 3D point on the plane: ");
				scanf("%f%f%f", &select_point.x, &select_point.y, &select_point.z); vf_log("%f %f %f ", select_point.x, select_point.y, select_point.z);
				
				printf("Enter the plane normal: ");
				scanf("%f%f%f", &select_normal.x, &select_normal.y, &select_normal.z); vf_log("%f %f %f ", select_normal.x, select_normal.y, select_normal.z);
				select_normal = PWM_normalize3(select_normal);
				
				select_plane = PWM_plane(select_normal, select_point);
				
				//remove vertices from selection that lie behind the plane
				for(i = 0; i < vertex_count; ++i){
					if(PWM_plane_classify(&select_plane, vertices[i]->position) == PWM_BACK){
						vertices[i]->color = 0xffffffff;
						vertices[i] = NULL;
					}
				}
				//move all vertices to the front of array, removing the gaps
				j = 0;
				for(i = 0; i < vertex_count; ++i){
					if(vertices[i]){
						vertices[j++] = vertices[i];
					}
				}
				vertex_count = j;
				
				draw_plane(&group_vertices[group_vertex_count - 3], &select_plane);
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				
				printf("Is selection ok? (0) No, (1) Yes (2) Reset: ");
				scanf("%d", &select_ok); vf_log("%d ", select_ok);
				
				
				if(select_ok == 2){
					//put all vertices back in the selection
					vertex_count = group_vertex_count - 3;
					for(i = 0; i < vertex_count; ++i){
						vertices[i] = &group_vertices[i];
						vertices[i]->color = 0xff0000ff;
					}
					select_ok = 0;
				}
				
			}while(!select_ok);
			
			//remove plane
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count - 3));
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count - 6));
			group_vertex_count -= 3;
			group_index_count -= 6;
			
			//make all vertices white again
			for(i = 0; i < vertex_count; ++i){
				vertices[i]->color = 0xffffffff;
			}
			
			print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		}//else if shape == 8
		else if(shape == 9){
			//TODO
			
			//draw a plane
			group_vertices = realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + 3));
			group_indices = realloc(group_indices, sizeof(unsigned short) * (group_index_count + 6));
			group_indices[group_index_count + 0] = group_vertex_count + 0;
			group_indices[group_index_count + 1] = group_vertex_count + 1;
			group_indices[group_index_count + 2] = group_vertex_count + 2;
			group_indices[group_index_count + 3] = group_vertex_count + 0;
			group_indices[group_index_count + 4] = group_vertex_count + 2;
			group_indices[group_index_count + 5] = group_vertex_count + 1;
			
			select_plane = PWM_plane(PWM_vec3(0, 0, 1), PWM_vec3(0, 0, 10000));
			
			//make all vertices NULL
			for(i = 0; i < group_vertex_count; ++i){
				vertices[i] = NULL;
			}
			
			do{
				//select vertices individually
				printf("Select vertices: (0) Done, (1) By plane, (2) Add indices, (3) Remove indices, (4) Select all, (5) Deselect all: ");
				scanf("%d", &select_ok); vf_log("%d ", select_ok);
				
				if(select_ok == 1){
					printf("Enter a 3D point on the plane: ");
					scanf("%f%f%f", &select_point.x, &select_point.y, &select_point.z); vf_log("%f %f %f ", select_point.x, select_point.y, select_point.z);
					
					printf("Enter the plane normal: ");
					scanf("%f%f%f", &select_normal.x, &select_normal.y, &select_normal.z); vf_log("%f %f %f ", select_normal.x, select_normal.y, select_normal.z);
					select_normal = PWM_normalize3(select_normal);
					
					select_plane = PWM_plane(select_normal, select_point);
					
					//remove the values on vertices selected
					for(i = 0; i < group_vertex_count; ++i){
						if(vertices[i]){
							if(PWM_plane_classify(&select_plane, vertices[i]->position) == PWM_BACK){
								vertices[i]->color = 0xffffffff;
								vertices[i] = NULL;
							}
						}
					}
				}
				if(select_ok == 2){
					printf("Enter the start and end index of the shape: ");
					scanf("%d%d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
					
					//select the vertices, but also select other vertices whose position is the same
					for(k = 0; k < group_vertex_count; ++k){
						for(i = start_index[0]; i < end_index[0]; ++i){
							if(group_vertices[i].position.x == group_vertices[k].position.x && group_vertices[i].position.y == group_vertices[k].position.y && group_vertices[i].position.z == group_vertices[k].position.z){
							
								vertices[k] = &group_vertices[k];
								vertices[k]->color = 0xff0000ff;
								break;
							}
						}
					}
				}
				if(select_ok == 3){
					printf("Enter the start and end index of the shape: ");
					scanf("%d%d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
					
					//deselect the vertices, but also other vertices whose position is the same
					for(k = 0; k < group_vertex_count; ++k){
						for(i = start_index[0]; i < end_index[0]; ++i){
							if(group_vertices[i].position.x == group_vertices[k].position.x && group_vertices[i].position.y == group_vertices[k].position.y && group_vertices[i].position.z == group_vertices[k].position.z){
							
								vertices[k] = NULL;
								group_vertices[k].color = 0xffffffff;
								break;
							}
						}
					}
				}
				if(select_ok == 4){
					//select all
					for(i = 0; i < group_vertex_count; ++i){
						vertices[i] = &group_vertices[i];
						group_vertices[i].color = 0xff0000ff;
					}
				}
				if(select_ok == 5){
					//deselect all
					for(i = 0; i < group_vertex_count; ++i){
						vertices[i] = NULL;
						group_vertices[i].color = 0xffffffff;
					}
				}
				
				draw_plane(&group_vertices[group_vertex_count], &select_plane);
				print_model(group_vertices, group_vertex_count + 3, group_indices, group_index_count + 6);
			} while(select_ok != 0);
			
			
			//select all indices with triangles containing the vertex indices
			j = 0;
			for(i = 0; i < group_index_count; i += 3){
				//check if first, second and third index is one of the selected vertex indices
				l = 0;
				for(k = 0; k < group_vertex_count; ++k){
					if(!vertices[k]) continue;
					
					if(group_indices[i + 0] == k) ++l;
					if(group_indices[i + 1] == k) ++l;
					if(group_indices[i + 2] == k) ++l;
				}
				
				if(l == 3){
					indices[j++] = &group_indices[i + 0];
					indices[j++] = &group_indices[i + 1];
					indices[j++] = &group_indices[i + 2];
				}
			}
			index_count = j;
			
			//move all vertices to the front of array, removing the gaps
			j = 0;
			for(i = 0; i < group_vertex_count; ++i){
				if(vertices[i]){
					vertices[j++] = vertices[i];
				}
			}
			vertex_count = j;
			
			//remove plane
			group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * group_vertex_count);
			group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * group_index_count);
			
			//make all vertices white again
			for(i = 0; i < group_vertex_count; ++i){
				group_vertices[i].color = 0xffffffff;
			}
			
			print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		}
		else if(shape == 10){
			//remove indices
		}
		
		//print it on screen
		print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		
		ask_for_transform(
			vertex_count,
			vertices,
			index_count,
			indices,
			group_vertex_count,
			group_vertices,
			group_index_count,
			group_indices
		);
		
		
		//specify the texture coordinate of the shape, only if a new shape is created
		if(shape != 0 && shape != 8){
			printf("uv left    boundary: ");
			scanf("%f", &uv_left); vf_log("%f ", uv_left);
			printf("uv top     boundary: ");
			scanf("%f", &uv_top); vf_log("%f ", uv_top);
			printf("uv right   boundary: ");
			scanf("%f", &uv_right); vf_log("%f ", uv_right);
			printf("uv bottom  boundary: ");
			scanf("%f", &uv_bottom); vf_log("%f ", uv_bottom);
			
			for(i = 0; i < vertex_count; ++i){
				vertices[i]->uv.x *= (uv_right - uv_left);
				vertices[i]->uv.x += uv_left;
				vertices[i]->uv.y *= (uv_bottom - uv_top);
				vertices[i]->uv.y += uv_top;
			}
		}
		
		print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		num_groups++;
	}
	
	return 0;
}