//4 dec 2022

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc shapes.c engine/pwmath.c -m32 -msse -lmingw32 -o shapes -static

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
	
	out = fopen("test.txt", "w");
	//out = stdout;
	
	fprintf(out, "PWR1\n\nVERTICES %d\n", vertex_count);
	for(i = 0; i < vertex_count; ++i){
		fprintf(out, "%d ", i);
		fprintf(out, "position %s %s %s ", convert(str[0], vertices[i].position.x), convert(str[1], vertices[i].position.y), convert(str[2], vertices[i].position.z));
		fprintf(out, "uv %s %s color %x ", convert(str[0], vertices[i].uv.x), convert(str[1], vertices[i].uv.y), vertices[i].color);
		fprintf(out, "normal %s %s %s\n", convert(str[0], vertices[i].normal.x), convert(str[1], vertices[i].normal.y), convert(str[2], vertices[i].normal.z));
		//fprintf(out, "%d position %f %f %f uv %f %f color %x normal %f %f %f\n", i, vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, vertices[i].uv.x, vertices[i].uv.y, vertices[i].color, vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z);
	}
	
	fprintf(out, "\nINDICES %d\n", index_count);
	for(i = 0; i < index_count; i += 3){
		fprintf(out, "%d %d %d\n", indices[i], indices[i+1], indices[i+2]);
	}
	
	fclose(out);
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

void vf_log(const char *format, ...){
	va_list args;
	FILE *session; 
	session = fopen("shapes.log", "a");
	va_start(args, format);
	vfprintf(session, format, args);
	va_end(args);
	fclose(session);
}

int main(void){
	int num_groups;               //all shapes
	int group_vertex_count;       //all shapes
	PWVertexData *group_vertices; //all shapes
	int group_index_count;        //all shapes
	unsigned short *group_indices; //all shapes' indices
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
	
	
	float *width_points;       //grid
	float *height_points;      //grid
	int num_width;             //grid
	int num_height;            //grid
	PWVec2 mesh_size;          //grid
	
	int vertex_count;          //for current shape
	int index_count;           //for current shape
	PWVertexData *group;       //for current shape
	unsigned short *indices;   //for current shape
	
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
	
	PWVec3 extrude_points;   //cone and curve (unused)
	
	num_groups = 0;
	group_vertex_count = 0;
	group_vertices = NULL;
	group_index_count = 0;
	group_indices = NULL;
	
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
				num_new_indices = num_new_triangles * 3;
				
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + num_new_indices));
				indices = &group_indices[group_index_count];
				group_index_count += num_new_indices;
				
				//create the indices
				//TODO
				connect_points(indices, start_index[1], end_index[1], start_index[0], end_index[0]);
				
				//print the file
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				
				//ask if it is okay
				num_reconnections = 0;
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				
				
				
				
				if(connection_ok){
					continue;
				}
				
				connect_points(indices, start_index[0], end_index[0], start_index[1], end_index[1]); //same as last but triangles reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				connect_points(indices, end_index[1], start_index[1], start_index[0], end_index[0]); //vertices are reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				connect_points(indices, start_index[0], end_index[0], end_index[1], start_index[1]); //triangles and vertices reversed
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				
				if(!connection_ok){
					group_index_count -= num_new_indices;
					group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * group_index_count);
					print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				}
				
				continue;
			}
			else if(connect_shape == 2){
				
				printf("Type in the start and end index of the shape to fill in: ");
				scanf("%d %d", &start_index[0], &end_index[0]);  vf_log("%d %d ", start_index[0], end_index[0]);
				
				num_new_triangles = end_index[0]- start_index[0] - 2;
				num_new_indices = 3 * num_new_triangles;
				
				if(num_new_triangles < 1){
					continue;
				}
				
				//create the primitive
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + num_new_indices));
				indices = &group_indices[group_index_count];
				group_index_count += num_new_indices;
				for(i = 0; i < num_new_triangles; ++i){
					indices[i*3 + 0] = start_index[0];
					indices[i*3 + 1] = start_index[0] + i + 1;
					indices[i*3 + 2] = start_index[0] + i + 2;
				}
				
				
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					continue;
				}
				
				//reverse the direction of the triangle
				reverse_indices(indices, num_new_indices);
				print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
				printf("Is it okay? (0) No, (1) Yes: ");
				scanf("%d", &connection_ok); vf_log("%d ", connection_ok);
				if(connection_ok){
					
					continue;
				}
				
				//remove the primitive
				group_index_count -= num_new_indices;
				group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count));
				continue;
			}
		}
		
		printf("Add a shape (1) cone, (2) curve, (3) grid, (4) copy vertices, (5) load file: ");
		scanf("%d", &shape);  vf_log("%d ", shape);
		
		if(shape == 0){
			//make the program think that all shapes are currently selected
			group = group_vertices;
			group_vertices = NULL;
			vertex_count = group_vertex_count;
			group_vertex_count = 0;
			indices = group_indices;
			group_indices = NULL;
			index_count = group_index_count;
			group_index_count = 0;
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
			group = (PWVertexData*) malloc(sizeof(PWVertexData) * ((degree - 1) * num_points + 1));
			indices = (unsigned short*) malloc(sizeof(unsigned short) * ((1 + 2 * (degree - 2)) * (num_points - 1) * 3));
			
			vertex_count = (degree - 1) * num_points + 1;
			index_count = (1 + 2 * (degree - 2)) * (num_points - 1) * 3;
			
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
			group[0].position = PWM_vec3(0, 0, -length_points[0] * length);
			group[0].uv = PWM_vec2(0.5, 0);
			group[0].tid = 0;
			group[0].color = 0xffffffff;
			group[0].normal = PWM_vec3(0, 0, 1);
			
			//behind the origin (cone body)
			for(i = 0; i < degree - 1; ++i){
				for(j = 0; j < num_points; ++j){
					group[i * num_points + j + 1].position.x = cross_section_points[j].x * modulation[i].x;
					group[i * num_points + j + 1].position.y = cross_section_points[j].y * modulation[i].y;
					group[i * num_points + j + 1].position.z = -length_points[i + 1] * length;
					group[i * num_points + j + 1].uv = PWM_vec2(j / (num_points - 1.0), (i + 1.0) / (degree - 1.0));
					group[i * num_points + j + 1].tid = 0.0;
					group[i * num_points + j + 1].color = 0xffffffff;
					group[i * num_points + j + 1].normal = PWM_normalize3(PWM_vec3(group[i * num_points + j + 1].position.x, group[i * num_points + j + 1].position.y, 0));
				}
			}
			
			//index the points
			//top triangles
			for(i = 0; i < num_points - 1; ++i){
				indices[i * 3 + 0] = 0;
				indices[i * 3 + 1] = i + 1;
				indices[i * 3 + 2] = i + 2;
			}
			k = i * 3;
			
			//squares not connected to the origin
			//1 num_points+1 2, 2 num_points+1 num_points+2, etc...
			for(i = 0; i < degree - 2; ++i){
				for(j = 0; j < num_points - 1; ++j){
					indices[(i * (num_points - 1) + j) * 6 + 0 + k] = i * num_points + j + 1;
					indices[(i * (num_points - 1) + j) * 6 + 1 + k] = (i + 1) * num_points + j + 1;
					indices[(i * (num_points - 1) + j) * 6 + 2 + k] = i * num_points + j + 2;
					
					indices[(i * (num_points - 1) + j) * 6 + 3 + k] = i * num_points + j + 2;
					indices[(i * (num_points - 1) + j) * 6 + 4 + k] = (i + 1) * num_points + j + 1;
					indices[(i * (num_points - 1) + j) * 6 + 5 + k] = (i + 1) * num_points + j + 2;
				}
			}
			
			free(cross_section_points);
			free(modulation);
			free(length_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count, group_vertex_count + vertex_count);
			printf("The indices for the tip of the cone is  [%d %d]\n", group_vertex_count, group_vertex_count + 1);
			printf("The indices for the edge of the cone is [%d %d]\n", group_vertex_count + vertex_count - num_points, group_vertex_count + vertex_count);
		}
		//curve
		else if(shape == 2){
			printf("How many points on the length? ");
			scanf("%d", &degree); vf_log("%d ", degree);
			
			printf("How many points in the cross section? ");
			scanf("%d", &num_points); vf_log("%d ", num_points);
			
			printf("What is the length of the shape? ");
			scanf("%f", &length); vf_log("%d ", length);
		
			if(degree < 2){
				degree = 2;
			}
			
			cross_section_points = (PWVec3*) malloc(sizeof(PWVec3) * num_points);
			modulation = (PWVec2*) malloc(sizeof(PWVec2) * degree);
			length_points = (float*) malloc(sizeof(float) * degree);
			group = (PWVertexData*) malloc(sizeof(PWVertexData) * degree * num_points);
			indices = (unsigned short*) malloc(sizeof(unsigned short) * ((2 * (degree - 1)) * (num_points - 1) * 3));
			
			vertex_count = degree * num_points;
			index_count = 2 * (degree - 1) * (num_points - 1) * 3;
			
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
					group[i * num_points + j].position.x = cross_section_points[j].x * modulation[i].x;
					group[i * num_points + j].position.y = cross_section_points[j].y * modulation[i].y;
					group[i * num_points + j].position.z = -length_points[i] * length;
					group[i * num_points + j].uv = PWM_vec2(j / (num_points - 1.0), i / (degree - 1.0));
					group[i * num_points + j].tid = 0.0;
					group[i * num_points + j].color = 0xffffffff;
					group[i * num_points + j].normal = PWM_normalize3(PWM_vec3(group[i * num_points + j].position.x, group[i * num_points + j].position.y, 0));
				}
			}
			
			//index the points
			
			//squares not connected to the origin
			//1 num_points+1 2, 2 num_points+1 num_points+2, etc...
			for(i = 0; i < degree - 1; ++i){
				for(j = 0; j < num_points - 1; ++j){
					indices[(i * (num_points - 1) + j) * 6 + 0] = i * num_points + j;
					indices[(i * (num_points - 1) + j) * 6 + 1] = (i + 1) * num_points + j;
					indices[(i * (num_points - 1) + j) * 6 + 2] = i * num_points + j + 1;
					
					indices[(i * (num_points - 1) + j) * 6 + 3] = i * num_points + j + 1;
					indices[(i * (num_points - 1) + j) * 6 + 4] = (i + 1) * num_points + j;
					indices[(i * (num_points - 1) + j) * 6 + 5] = (i + 1) * num_points + j + 1;
				}
			}
			
			free(cross_section_points);
			free(modulation);
			free(length_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count, group_vertex_count + vertex_count);
			printf("The indices for the curve front is [%d %d]\n", group_vertex_count, group_vertex_count + num_points);
			printf("The indices for the curve end is   [%d %d]\n", group_vertex_count + vertex_count - num_points, group_vertex_count + vertex_count);
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
			group = (PWVertexData*) malloc(sizeof(PWVertexData) * num_width * num_height);
			indices = (unsigned short*) malloc(sizeof(unsigned short) * (num_width - 1) * (num_height - 1) * 6);
			
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
			
			//get the vertex point values
			printf("Enter the %d x %d = %d values for the depth of the mesh: ", num_width, num_height, num_width * num_height);
			for(i = 0; i < num_height; ++i){
				for(j = 0; j < num_width; ++j){
					group[i * num_width + j].position.x = mesh_size.x * (width_points[j] - 0.5);
					group[i * num_width + j].position.y = mesh_size.y * (0.5 - height_points[i]);
					scanf("%f", &group[i * num_width + j].position.z); vf_log("%f ", group[i * num_width + j].position.z);
					group[i * num_width + j].uv = PWM_vec2(j / (num_width - 1.0), i / (num_height - 1.0));
					group[i * num_width + j].tid = 0.0;
					group[i * num_width + j].color = 0xffffffff;
					group[i * num_width + j].normal = PWM_vec3(0, 0, 1);
				}
			}
			
			//get the index values
			for(i = 0; i < num_height - 1; ++i){
				for(j = 0; j < num_width - 1; ++j){
					indices[(i * (num_width - 1) + j) * 6 + 0] = i * num_width + j;
					indices[(i * (num_width - 1) + j) * 6 + 1] = (i + 1) * num_width + j;
					indices[(i * (num_width - 1) + j) * 6 + 2] = i * num_width + j + 1;
					indices[(i * (num_width - 1) + j) * 6 + 3] = i * num_width + j + 1;
					indices[(i * (num_width - 1) + j) * 6 + 4] = (i + 1) * num_width + j;
					indices[(i * (num_width - 1) + j) * 6 + 5] = (i + 1) * num_width + j + 1;
				}
			}
			
			free(width_points);
			free(height_points);
			
			printf("The indices for this shape is [%d %d]\n", group_vertex_count, group_vertex_count + vertex_count);
		} //else if shape == 3
		else if(shape == 4){
			printf("Type in the start and end index of the shape to copy: ");
			scanf("%d %d", &start_index[0], &end_index[0]); vf_log("%d %d ", start_index[0], end_index[0]);
			
			if(end_index[0] - start_index[0] < 1){
				continue;
			}
			
			//copy the vertices
			vertex_count = end_index[0] - start_index[0];
			group = (PWVertexData*) malloc(sizeof(PWVertexData) * vertex_count);
			for(i = 0; i < vertex_count; ++i){
				group[i] = group_vertices[i + start_index[0]];
			}
			
			//copy the indices (triplets) which are all within [start, end)
			index_count = 0;
			for(i = 0; i < group_index_count; i += 3){
				if(group_indices[i + 0] >= start_index[0] && group_indices[i + 0] < end_index[0]
					&& group_indices[i + 1] >= start_index[0] && group_indices[i + 1] < end_index[0]
					&& group_indices[i + 2] >= start_index[0] && group_indices[i + 2] < end_index[0]){
					
					index_count += 3;
				}
			}
			indices = (unsigned short*) malloc(sizeof(unsigned short) * index_count);
			j = 0;
			for(i = 0; i < group_index_count; i += 3){
				if(group_indices[i + 0] >= start_index[0] && group_indices[i + 0] < end_index[0]
					&& group_indices[i + 1] >= start_index[0] && group_indices[i + 1] < end_index[0]
					&& group_indices[i + 2] >= start_index[0] && group_indices[i + 2] < end_index[0]){
					
					indices[j++] = group_indices[i + 0] - start_index[0];
					indices[j++] = group_indices[i + 1] - start_index[0];
					indices[j++] = group_indices[i + 2] - start_index[0];
				}
			}
			printf("The indices for this shape is [%d %d]\n", group_vertex_count, group_vertex_count + vertex_count);
		}
		else if(shape == 5){
			printf("Type in the file name: ");
			//fflush(stdin);
			//fgets(input, 500, stdin);
			//input[strcspn(input, "\n")] = '\0';  vf_log("\n%s ", input);
			scanf("%s", input); vf_log("\n%s", input);
			
			if(read_model(input, &group, &vertex_count, &indices, &index_count) != 0){
				printf("Invalid file.\n");
				continue;
			}
			
			if(group_vertices){
				free(group_vertices);
				group_vertex_count = 0;
				group_vertices = NULL;
			}
			if(group_indices){
				free(group_indices);
				group_index_count = 0;
				group_indices = NULL;
			}
		}
		//shape value is 0~5 at this point. group contains the untransformed shape. indices contains the untransformed indices.
		
		//move current shape to the group
		group_vertices = (PWVertexData*) realloc(group_vertices, sizeof(PWVertexData) * (group_vertex_count + vertex_count));
		memcpy(&group_vertices[group_vertex_count], group, sizeof(PWVertexData) * vertex_count);
		
		group_indices = (unsigned short*) realloc(group_indices, sizeof(unsigned short) * (group_index_count + index_count));
		memcpy(&group_indices[group_index_count], indices, sizeof(unsigned short) * index_count);
		for(i = 0; i < index_count; ++i){
			group_indices[group_index_count + i] += group_vertex_count;
		}
		
		//print it on screen
		print_model(group_vertices, group_vertex_count + vertex_count, group_indices, group_index_count + index_count);
		
		//ask for any transformation
		transformation = PWM_ini(1.0, NULL);
		do{
			printf("Any transformation? (0) No, (1) Rotation, (2) Scale, (3) Translation, (4) Reset transformation: ");
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
					reverse_indices(&group_indices[group_index_count], index_count);
				}
				break;
			case 3:
				printf("What is the translation? ");
				scanf("%f %f %f", &translation.x, &translation.y, &translation.z); vf_log("%f %f %f ", translation.x, translation.y, translation.z);
				transformation = PWM_translation(translation);
				break;
			case 4: //reset
				memcpy(&group_vertices[group_vertex_count], group, sizeof(PWVertexData) * vertex_count);
				memcpy(&group_indices[group_index_count], indices, sizeof(unsigned short) * index_count);
				transformation = PWM_ini(1.0, NULL);
				break;
			default:
				break;
			}
			
			//perform the transformation
			if(transformation_type != 0 && transformation_type != 4){
				for(i = 0; i < vertex_count; ++i){
					group_vertices[group_vertex_count + i].position = PWM_mul_vec3(transformation, group_vertices[group_vertex_count + i].position);
				}
			}
			
			//show the model
			print_model(group_vertices, group_vertex_count + vertex_count, group_indices, group_index_count + index_count);
			
			
		} while(transformation_type != 0);
		
		
		//overwrite the shape with the transformed shape
		//memcpy(&group_vertices[group_vertex_count], group, sizeof(PWVertexData) * vertex_count);
		//memcpy(&group_indices[group_index_count], indices, sizeof(unsigned short) * index_count);
		
		//specify the texture coordinate of the shape, only if a new shape is created
		if(shape > 0){
			printf("uv left    boundary: ");
			scanf("%f", &uv_left); vf_log("%f ", uv_left);
			printf("uv top     boundary: ");
			scanf("%f", &uv_top); vf_log("%f ", uv_top);
			printf("uv right   boundary: ");
			scanf("%f", &uv_right); vf_log("%f ", uv_right);
			printf("uv bottom  boundary: ");
			scanf("%f", &uv_bottom); vf_log("%f ", uv_bottom);
			
			for(i = 0; i < vertex_count; ++i){
				group_vertices[group_vertex_count + i].uv.x *= (uv_right - uv_left);
				group_vertices[group_vertex_count + i].uv.x += uv_left;
				group_vertices[group_vertex_count + i].uv.y *= (uv_bottom - uv_top);
				group_vertices[group_vertex_count + i].uv.y += uv_top;
			}
		}
		print_model(group_vertices, group_vertex_count + vertex_count, group_indices, group_index_count + index_count);
		
		//untransformed shape no longer needed
		free(group);
		free(indices);
		
		group_vertex_count += vertex_count;
		group_index_count += index_count;
		
		print_model(group_vertices, group_vertex_count, group_indices, group_index_count);
		num_groups++;
	}
	
	return 0;
}