//NoobieII
//20 Dec 2022

#ifndef M3PW_ANIMATION_H
#define M3PW_ANIMATION_H

#include "pwbezier.h"
#include "pwmath.h"
#include "pwmodel.h"

typedef struct PWAnimation{
	int num_parts;
	float period;
	
	float *phase;
	PWVec3 *rotation_axis;
	PWBezier *rotation;
	PWBezier *scale_x;
	PWBezier *scale_y;
	PWBezier *scale_z;
	PWBezier *translation_x;
	PWBezier *translation_y;
	PWBezier *translation_z;
	PWMat4 **transforms; //reference to model->transforms
	
	PWModel *model;
} PWAnimation;

//return 0 if successful, -1 if not successful
int pwanimation_load(PWAnimation *a, const char *filename, PWModel *model);
void pwanimation_reset(PWAnimation *a);
void pwanimation_update(PWAnimation *a, int deltatime);
void pwanimation_close(PWAnimation *a);

#endif
