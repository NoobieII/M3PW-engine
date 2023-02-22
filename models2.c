//27 Dec 2022
//NoobieII

//Model file creator

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc models2.c engine/pwmath.c -m32 -msse -lmingw32 -o models2 -static

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/pwmath.h"

typedef struct PWVertexData{
	PWVec3 position;
	PWVec2 uv;
	float tid;
	unsigned int color;
	PWVec3 normal;
} PWVertexData;

int read_model(const char *filename, PWVertexData **vertices, int *vertex_count, unsigned short **indices, int *index_count);
char *convert(char *s, float x);
void print_model(PWVertexData *vertices, int vertex_count, unsigned short *indices, int index_count);
void draw_plane(PWVertexData *vertices, PWPlane *p);

//filesize in c, portable, using binary search

//not meant to be efficient, note that this is limited to 2GB files or smaller
int fsize(const char *filename){
	int result;
	unsigned int size = 1;
	int jump;
	FILE *f;
	char c;
	
	f = fopen(filename, "rb");
	if(!f){
		return -1;
	}
	
	// find upper bound
	do{
		fseek(f, size - 1, SEEK_SET);
		result = fread(&c, 1, 1, f);
		size <<= 1;
	} while(result == 1);
	
	size >>= 2;
	
	if(size == 0){
		fclose(f);
		return 0;
	}
	
	fseek(f, size - 1, SEEK_SET);
	result = fread(&c, 1, 1, f);
	if(result == 1 && fread(&c, 1, 1, f) != 1){
		return size;
	}
	
	size <<= 1;
	
	//begin binary search between size and size/2
	jump = size >> 1;
	
	do{
		fseek(f, size - 1, SEEK_SET);
		result = fread(&c, 1, 1, f);
		if(result == 1 && fread(&c, 1, 1, f) != 1){
			break;
		}
		
		jump >>= 1;
		if(result == 1){
			size += jump;
		}
		else{
			size -= jump;
		}
	} while(jump > 0);
	
	fclose(f);
	return size;
}

char *read_from_file(const char *filename){
	int filesize;
	FILE *file;
	char *data;
	
	filesize = fsize(filename);
	if(filesize == -1){
		return NULL;
	}
	data = (char*) malloc(filesize + 1);
	data[filesize] = '\0';
	file = fopen(filename, "rb");
	fread(data, 1, filesize, file);
	fclose(file);
	
	return data;
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
	char str[3][100];
	
	int prev_size;
	
	static int a = 0;
	
	//out = stdout;
	out = fopen("shape.txt", "w");
	
	fprintf(out, "PWR1\n\nVERTICES %d\n", vertex_count);
	for(i = 0; i < vertex_count; ++i){
		fprintf(out, "%d ", i);
		fprintf(out, "position %s %s %s ", convert(str[0], vertices[i].position.x), convert(str[1], vertices[i].position.y), convert(str[2], vertices[i].position.z));
		fprintf(out, "uv %s %s ", convert(str[0], vertices[i].uv.x), convert(str[1], vertices[i].uv.y));
		fprintf(out, "color %x ", vertices[i].color);
		
		fprintf(out, "normal %s %s %s\n", convert(str[0], vertices[i].normal.x), convert(str[1], vertices[i].normal.y), convert(str[2], vertices[i].normal.z));
	}
	
	fprintf(out, "\nINDICES %d\n", index_count);
	for(i = 0; i < index_count; i += 3){
		fprintf(out, "%d %d %d\n", indices[i], indices[i+1], indices[i+2]);
	}
	
	a ^= 1;
	if(a){
		fprintf(out, "%*s\n", rand() % 32 + 1, "");
	}
	
	if(out != stdout){
		fclose(out);
	}
}

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

void vf_log(const char *format, ...){
	va_list args;
	FILE *session; 
	session = fopen("model.log", "a");
	va_start(args, format);
	vfprintf(session, format, args);
	va_end(args);
	fclose(session);
}

int main(int argc, const char *argv[]){
	PWVertexData *vertices;
	int vertex_count;
	unsigned short *indices;
	int index_count;
	char *contents;
	
	PWVertexData **vertices_left;
	PWVertexData **vertices_selected;
	
	PWVertexData *vertices_reordered;
	unsigned short *indices_reordered;
	
	PWVec3 origin;
	
	int start_index;
	int end_index;
	PWMat4 detransform;
	PWMat4 group_transform;
	int group_level;
	PWMat4 origins[100];
	int num_group_end;
	
	int selection_ok;
	PWVec3 select_point;
	PWVec3 select_normal;
	PWPlane select_plane;
	
	int is_more_group;
	int group_count = 0;
	
	int i;
	int j = 0; //used to populate vertices reordered array
	int k;
	int l;
	
	FILE *model_out;
	char str[20];
	
	//read the file "shape.txt"
	if(read_model("shape.txt", &vertices, &vertex_count, &indices, &index_count) != 0){
		printf("Error reading shape.txt\n");
		return -1;
	}
	//make a backup of the shape
	contents = read_from_file("shape.txt");
	model_out = fopen("shape_backup.txt", "wb");
	fwrite(contents, fsize("shape.txt"), 1, model_out);
	fclose(model_out);
	free(contents);
	
	//empty model.log file
	fclose(fopen("model.log", "wt"));
	
	model_out = fopen("model.txt", "wt");
	fprintf(model_out, "PWMODEL\n");
	fprintf(model_out, "shape.txt\n");
	fprintf(model_out, "texture.png\n");
	fprintf(model_out, "num_parts INSERT_YER_NUMBER\n");
	
	vertices_left = malloc(sizeof(PWVertexData*) * vertex_count);
	vertices_selected = malloc(sizeof(PWVertexData*) * vertex_count);
	vertices_reordered = malloc(sizeof(PWVertexData) * vertex_count);
	indices_reordered = malloc(sizeof(unsigned short) * index_count);
	
	
	memset(indices_reordered, 0, sizeof(unsigned short) * index_count);
	
	//draw a plane
	vertices = realloc(vertices, sizeof(PWVertexData) * (vertex_count + 3));
	indices = realloc(indices, sizeof(unsigned short) * (index_count + 6));
	indices[index_count + 0] = vertex_count + 0;
	indices[index_count + 1] = vertex_count + 1;
	indices[index_count + 2] = vertex_count + 2;
	indices[index_count + 3] = vertex_count + 0;
	indices[index_count + 4] = vertex_count + 2;
	indices[index_count + 5] = vertex_count + 1;
	
	//copy vertices to vertices left
	for(i = 0; i < vertex_count; ++i){
		vertices_left[i] = &vertices[i];
	}
	
	origins[0] = PWM_ini(1.0f, NULL);
	group_level = 1;
	do{
		select_plane = PWM_plane(PWM_vec3(0, 0, 1), PWM_vec3(0, 0, 10000));
		
		printf("You are at group level %d\n", group_level);
		printf("Type in the start and end index for origin: ");
		
		scanf("%d%d", &start_index, &end_index); vf_log("%d %d ", start_index, end_index);
		
		//calculate origin of shape using average vertex position (no repeats)
		origin = PWM_vec3(0, 0, 0);
		l = 0;
		for(i = start_index; i < end_index; ++i){
			for(k = start_index; k < i; ++k){
				if(vertices[k].position.x == vertices[i].position.x
					&& vertices[k].position.y == vertices[i].position.y
					&& vertices[k].position.z == vertices[i].position.z){
					
					break;
				}
			}
			if(k == i){
				origin = PWM_add3(vertices[i].position, origin);
				++l;
			}
		}
		origin = PWM_mul3(origin, 1.0f / l);
		printf("Origin is at %f %f %f.\n", origin.x, origin.y, origin.z);
		
		detransform = PWM_inv(PWM_translation(origin));
		origins[group_level] = PWM_translation(origin);
		group_transform = PWM_mul(PWM_inv(origins[group_level - 1]), origins[group_level]);
		
		do{
			printf("Select vertices: (0) Done, (1) By plane, (2) Add indices, (3) Remove indices, (4) Select all, (5) Deselect all: ");
			scanf("%d", &selection_ok); vf_log("%d ", selection_ok);
			
			if(selection_ok == 1){
				printf("Enter a 3D point on the plane: ");
				scanf("%f%f%f", &select_point.x, &select_point.y, &select_point.z); vf_log("%f %f %f ", select_point.x, select_point.y, select_point.z);
				
				printf("Enter the plane normal: ");
				scanf("%f%f%f", &select_normal.x, &select_normal.y, &select_normal.z); vf_log("%f %f %f ", select_normal.x, select_normal.y, select_normal.z);
				select_normal = PWM_normalize3(select_normal);
				
				select_plane = PWM_plane(select_normal, select_point);
				
				//remove the values on vertices selected
				for(i = 0; i < vertex_count; ++i){
					if(vertices_selected[i]){
						if(PWM_plane_classify(&select_plane, vertices_selected[i]->position) == PWM_BACK){
							vertices_selected[i]->color = 0xffffffff;
							vertices_selected[i] = NULL;
						}
					}
				}
			}
			if(selection_ok == 2){
				printf("Enter the start and end index of the shape: ");
				scanf("%d%d", &start_index, &end_index); vf_log("%d %d ", start_index, end_index);
				
				//select the vertices, but also select other vertices whose position is the same
				for(k = 0; k < vertex_count; ++k){
					for(i = start_index; i < end_index; ++i){
						if(vertices_left[k] && vertices[i].position.x == vertices[k].position.x && vertices[i].position.y == vertices[k].position.y && vertices[i].position.z == vertices[k].position.z){
						
							vertices_selected[k] = vertices_left[k];
							vertices[k].color = 0xff0000ff;
							break;
						}
					}
				}
			}
			if(selection_ok == 3){
				printf("Enter the start and end index of the shape: ");
				scanf("%d%d", &start_index, &end_index); vf_log("%d %d ", start_index, end_index);
				
				//deselect the vertices, but also other vertices whose position is the same
				for(k = 0; k < vertex_count; ++k){
					for(i = start_index; i < end_index; ++i){
						if(vertices[i].position.x == vertices[k].position.x && vertices[i].position.y == vertices[k].position.y && vertices[i].position.z == vertices[k].position.z){
						
							vertices_selected[k] = NULL;
							vertices[k].color = 0xffffffff;
							break;
						}
					}
				}
			}
			if(selection_ok == 4){
				for(i = 0; i < vertex_count; ++i){
					if(vertices_left[i]){
						vertices_selected[i] = vertices_left[i];
						vertices[i].color = 0xff0000ff;
					}
				}
			}
			if(selection_ok == 5){
				for(i = 0; i < vertex_count; ++i){
					vertices_selected[i] = NULL;
					vertices[i].color = 0xffffffff;
				}
			}
			
			draw_plane(&vertices[vertex_count], &select_plane);
			print_model(vertices, vertex_count + 3, indices, index_count + 6);
		} while(selection_ok != 0);
		
		//ask if the group is to be broken down further
		printf("Is the group to be broken down further? (0) No, (1) Yes: ");
		scanf("%d", &is_more_group); vf_log("%d ", is_more_group);
		
		if(is_more_group){
			num_group_end = 0;
		}
		else{
			//else ask for how many groups to end
			printf("How many groups to end? ");
			scanf("%d", &num_group_end); vf_log("%d ", num_group_end);
			
			if(num_group_end >= group_level){
				num_group_end = group_level;
				
				//TODO make sure no vertices left
			}
		}
		vf_log("\n");
		
		
		//make all vertices white again
		for(i = 0; i < vertex_count; ++i){
			vertices[i].color = 0xffffffff;
		}
		
		//copy to reordered vertices
		start_index = j;
		for(i = 0; i < vertex_count; ++i){
			if(vertices_selected[i]){
				//copy instances of the vertex index number to the reordered indices
				for(k = 0; k < index_count; ++k){
					if(indices[k] == i){
						indices_reordered[k] = j;
					}
				}
				vertices_reordered[j++] = *vertices_selected[i];
			}
		}
		end_index = j;
		
		//minus contents of vertices left by vertices selected
		for(i = 0; i < vertex_count; ++i){
			if(vertices_selected[i]){
				vertices_left[i] = NULL;
			}
			vertices_selected[i] = NULL;
		}
		
		//update model file and shape file
		fprintf(model_out, "vertices %d %d ", start_index, end_index);
		fprintf(model_out, "detransform ");
		for(i = 0; i < 16; ++i){
			fprintf(model_out, "%s ", convert(str, detransform.elements[i]));
		}
		fprintf(model_out, "transform ");
		for(i = 0; i < 16; ++i){
			fprintf(model_out, "%s ", convert(str, group_transform.elements[i]));
		}
		fprintf(model_out, "num_group_end %d\n", num_group_end);
		fflush(model_out);
		
		print_model(vertices, vertex_count + 3, indices, index_count + 6);
		
		if(is_more_group){
			group_level++;
		}
		else{
			group_level -= (num_group_end - 1);
		}
		
		group_count++;
	} while(group_level > 1);
	
	//write the shape file rearranged
	print_model(vertices_reordered, vertex_count, indices_reordered, index_count);
	
	//write the animation file
	fclose(model_out);
	model_out = fopen("animation.txt", "wt");
	fprintf(model_out, "PWANIMATION\n");
	fprintf(model_out, "num_parts %d\n", group_count);
	fprintf(model_out, "period 1\n");
	for(i = 0; i < group_count; ++i){
		fprintf(model_out, "phase 0 rotation_axis 1 0 0 rotation 0 0 0 0 scale_x 1 1 1 1 scale_y 1 1 1 1 scale_z 1 1 1 1 translation_x 0 0 0 0 translation_y 0 0 0 0 translation_z 0 0 0 0\n");
	}
	fclose(model_out);
	
	
	free(vertices);
	free(indices);
	free(vertices_left);
	free(vertices_selected);
	free(vertices_reordered);
	free(indices_reordered);
	
	return 0;
}
