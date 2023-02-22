//NoobieII
//20 Dec 2022

#include <stdio.h>
#include <string.h>

#include "pwanimation.h"

#define INPUT_LEN 300

int pwanimation_load(PWAnimation *a, const char *filename, PWModel *model){
	FILE *in;
	int i;
	char str[INPUT_LEN];
	
	int result;
	float f[32];
	
	if(!model){
		return -1;
	}
	
	in = fopen(filename, "r");
	if(!in){
		return -1;
	}
	
	fgets(str, INPUT_LEN, in);
	if(strncmp(str, "PWANIMATION", 11) != 0){
		fclose(in);
		return -1;
	}
	
	//expect num_parts <num_parts>
	//expect period <period>
	fgets(str, INPUT_LEN, in);
	sscanf(str, "%*s%d", &a->num_parts);
	fgets(str, INPUT_LEN, in);
	sscanf(str, "%*s%f", &a->period);
	
	if(a->period == 0){
		a->period = 1.0f;
	}
	
	if(a->num_parts < 0 || a->num_parts > 100){
		fclose(in);
		return -1;
	}
	
	//allocate the memory
	a->phase = (float*) malloc(sizeof(float) * a->num_parts);
	a->rotation_axis = (PWVec3*) malloc(sizeof(PWVec3) * a->num_parts);
	a->rotation = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->scale_x = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->scale_y = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->scale_z = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->translation_x = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->translation_y = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	a->translation_z = (PWBezier*) malloc(sizeof(PWBezier) * a->num_parts);
	
	for(i = 0; i < a->num_parts; ++i){
		fgets(str, INPUT_LEN, in);
		
		//expect phase <float> rotation_axis <vec3> rotation <bezier> scale_x <bezier> scale_y <bezier> scale_z <bezier> translation_x <bezier> translation_y <bezier> translation_z <bezier>
		result = sscanf(
			str,
			"%*s%f%*s%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f"
			"%*s%f%f%f%f",
			&f[0], &f[1], &f[2], &f[3],
			&f[4], &f[5], &f[6], &f[7],
			&f[8], &f[9], &f[10], &f[11],
			&f[12], &f[13], &f[14], &f[15],
			&f[16], &f[17], &f[18], &f[19],
			&f[20], &f[21], &f[22], &f[23],
			&f[24], &f[25], &f[26], &f[27],
			&f[28], &f[29], &f[30], &f[31]);
		
		if(result != 32){
			printf("pwanimation_load: invalid contents in file %s\n%s\n", filename, str);
			a->num_parts = 0;
			break;
		}
		
		a->phase[i] = f[0];
		a->rotation_axis[i] = PWM_vec3(f[1], f[2], f[3]);
		a->rotation[i] = pwbezier3(f[4], f[5], f[6], f[7]);
		a->scale_x[i] = pwbezier3(f[8], f[9], f[10], f[11]);
		a->scale_y[i] = pwbezier3(f[12], f[13], f[14], f[15]);
		a->scale_z[i] = pwbezier3(f[16], f[17], f[18], f[19]);
		a->translation_x[i] = pwbezier3(f[20], f[21], f[22], f[23]);
		a->translation_y[i] = pwbezier3(f[24], f[25], f[26], f[27]);
		a->translation_z[i] = pwbezier3(f[28], f[29], f[30], f[31]);
		
		//set the phase
		a->rotation[i].t = a->phase[i];
		a->scale_x[i].t = a->phase[i];
		a->scale_y[i].t = a->phase[i];
		a->scale_z[i].t = a->phase[i];
		a->translation_x[i].t = a->phase[i];
		a->translation_y[i].t = a->phase[i];
		a->translation_z[i].t = a->phase[i];
	}
	
	a->transforms = model->transforms;
	a->model = model;
	
	return 0;
}

void pwanimation_reset(PWAnimation *a){
	int i;
	
	for(i = 0; i < a->num_parts; ++i){
		a->rotation[i].t = a->phase[i];
		a->scale_x[i].t = a->phase[i];
		a->scale_y[i].t = a->phase[i];
		a->scale_z[i].t = a->phase[i];
		a->translation_x[i].t = a->phase[i];
		a->translation_y[i].t = a->phase[i];
		a->translation_z[i].t = a->phase[i];
	}
}

void pwanimation_update(PWAnimation *a, int deltatime){
	int i;
	PWMat4 mat4;
	
	float t = deltatime * 0.001 / a->period;
	
	for(i = 0; i < a->num_parts; ++i){
		mat4 = PWM_ini(1.0, NULL);
		
		//rotation, scale, then translation
		
		mat4 = PWM_rotation(pwbezier_add(&a->rotation[i], t), a->rotation_axis[i]);
		mat4 = PWM_mul(PWM_scale(PWM_vec3(pwbezier_add(&a->scale_x[i], t), pwbezier_add(&a->scale_y[i], t), pwbezier_add(&a->scale_z[i], t))), mat4);
		mat4 = PWM_mul(PWM_translation(PWM_vec3(pwbezier_add(&a->translation_x[i], t), pwbezier_add(&a->translation_y[i], t), pwbezier_add(&a->translation_z[i], t))), mat4);
		*(a->transforms[i]) = mat4;
	}
}

void pwanimation_close(PWAnimation *a){
	free(a->phase);
	free(a->rotation_axis);
	free(a->rotation);
	free(a->scale_x);
	free(a->scale_y);
	free(a->scale_z);
	free(a->translation_x);
	free(a->translation_y);
	free(a->translation_z);
}
