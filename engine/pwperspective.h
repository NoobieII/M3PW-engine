//NoobieII
//14 Jan 2023

//Calculations used in 3d perspective

#ifndef M3PW_PERSPECTIVE_H
#define M3PW_PERSPECTIVE_H

#include "pwmath.h"

typedef struct PWPersp{
	PWVec3 rot;
	PWVec3 txn;
	
	float a;
	PWVec3 pov;
	float dist;
	
	//calculated after pwpersp_mat4() is called
	PWMat4 proj;
	PWPlane front_plane; //looking out from near towards far
	
	//calculated after pwpersp_ray() is called
	PWRay ray;
	PWVec3 p_near;
	PWVec3 p_far;
} PWPersp;

//generate distance 6 (5 to 50) perspective at 0 rotation/translation
PWPersp pwpersp();

//add angle (degrees)/translation to perspective
void pwpersp_add_global_rotation(PWPersp *p, PWVec3 rotation);
void pwpersp_add_local_rotation(PWPersp *p, PWVec3 rotation);

void pwpersp_add_global_translation(PWPersp *p, PWVec3 translation);
void pwpersp_add_local_translation(PWPersp *p, PWVec3 translation);

//generate projection matrix from perspective
//tn(-pov), rotation, translation, perspective.
//near is 2, far is 2*dist-2
PWMat4 pwpersp_mat4(PWPersp *p);

//generate ray from mouse position
PWRay pwpersp_ray(PWPersp *p, PWVec2 pos);

#endif
