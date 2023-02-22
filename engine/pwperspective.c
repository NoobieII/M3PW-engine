//NoobieII
//14 Jan 2023

#include <math.h>
#include <stdio.h>
#include "pwperspective.h"

PWPersp pwpersp(){
	PWPersp p;
	p.rot = p.txn = PWM_vec3(0,0,0);
	p.a = 4.0f/3.0f;
	p.pov = PWM_vec3(0,0,0);
	p.dist = 6;
	return p;
}

void pwpersp_add_global_rotation(PWPersp *p, PWVec3 rotation){
	PWQuat qrot, rot_total;
	PWVec3 rot;
	
	rot = PWM_mul3(p->rot, PI/180);
	rotation = PWM_mul3(rotation, PI/180);
	qrot = PWM_quat_from_euler(rotation.x, rotation.y, rotation.z);
	rot_total = PWM_quat_from_euler(rot.x, rot.y, rot.z); 
	rot_total = PWM_quat_mul(rot_total, qrot);
	rot = PWM_quat_to_euler(rot_total);
	rot = PWM_mul3(rot, 180/PI);
	p->rot = rot;
}

void pwpersp_add_local_rotation(PWPersp *p, PWVec3 rotation){
	PWQuat qrot, rot_total;
	PWVec3 rot;
	
	rot = PWM_mul3(p->rot, PI/180);
	rotation = PWM_mul3(rotation, PI/180);
	qrot = PWM_quat_from_euler(rotation.x, rotation.y, rotation.z);
	rot_total = PWM_quat_from_euler(rot.x, rot.y, rot.z); 
	rot_total = PWM_quat_mul(qrot, rot_total);
	rot = PWM_quat_to_euler(rot_total);
	rot = PWM_mul3(rot, 180/PI);
	p->rot = rot;
}

void pwpersp_add_global_translation(PWPersp *p, PWVec3 translation){
	p->txn = PWM_add3(translation, p->txn);
}

void pwpersp_add_local_translation(PWPersp *p, PWVec3 translation){
	PWMat4 mat4;
	//printf("p->rot = %f %f %f translation %f %f %f\r", p->rot.x, p->rot.y, p->rot.z, translation.x, translation.y, translation.z);
	mat4 = PWM_quat_mat4(PWM_quat_from_euler(p->rot.x, p->rot.y, p->rot.z));
	mat4 = PWM_inv(mat4);
	translation = PWM_mul_vec3(mat4, translation);
	p->txn = PWM_add3(translation, p->txn);
}

PWMat4 pwpersp_mat4(PWPersp *p){
	PWMat4 proj;
	PWVec3 dir;
	proj = PWM_ini(1.0, NULL);
	proj = PWM_mul(PWM_quat_mat4(PWM_quat_from_euler(p->rot.x*PI/180, p->rot.y*PI/180, p->rot.z*PI/180)), proj);
	proj = PWM_mul(PWM_translation(PWM_mul3(p->txn, -1)), proj);
	proj = PWM_mul(PWM_translation(PWM_mul3(p->pov, -1)), proj);
	proj = PWM_mul(PWM_perspective(60, 4/3.0, 2, 2*p->dist-2), proj);
	p->proj = proj;
	dir = PWM_vec3(0, 0, -1);
	dir = PWM_mul_vec3(PWM_inv(PWM_quat_mat4(PWM_quat_from_euler(p->rot.x*PI/180, p->rot.y*PI/180, p->rot.z*PI/180))), dir);
	return proj;
}

//generate ray from mouse position
PWRay pwpersp_ray(PWPersp *p, PWVec2 pos){
	PWVec3 v1, v2;
	
	v1 = PWM_vec3(pos.x * 2 * 4/3.0 * tan(PI/360*60), pos.y * 2 * tan(PI/360*60), -2);
	v2 = PWM_vec3(pos.x * (2*p->dist-2) * 4/3.0 * tan(PI/360*60), pos.y * (2*p->dist-2) * tan(PI/360*60), -2*p->dist+2);
	
	v1 = PWM_mul_vec3(PWM_translation(p->pov), v1);
	v2 = PWM_mul_vec3(PWM_translation(p->pov), v2);
	v1 = PWM_mul_vec3(PWM_translation(p->txn), v1);
	v2 = PWM_mul_vec3(PWM_translation(p->txn), v2);
	v1 = PWM_mul_vec3(PWM_inv(PWM_quat_mat4(PWM_quat_from_euler(p->rot.x*PI/180, p->rot.y*PI/180, p->rot.z*PI/180))), v1);
	v2 = PWM_mul_vec3(PWM_inv(PWM_quat_mat4(PWM_quat_from_euler(p->rot.x*PI/180, p->rot.y*PI/180, p->rot.z*PI/180))), v2);
	
	p->ray = PWM_ray(v1, PWM_normalize3(PWM_sub3(v2, v1)));
	p->p_near = v1;
	p->p_far = v2;
	return p->ray;
}

