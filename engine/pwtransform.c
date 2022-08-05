//Lithio (The developer's pseudonym)
//July 29, 2022

#include "pwtransform.h"

#define ROTATION 1
#define SCALE 2
#define TRANSLATION 4

void pwtransform_init(PWTransform *t, PWMat4 *ref){
	t->type = 0;
	t->ref = ref;
}

void pwtransform_rotation(PWTransform *t, PWVec3 axis, PWBezier bezier){
	t->type |= ROTATION;
	t->axis = axis;
	t->bezier_r = bezier;
}

void pwtransform_scale1(PWTransform *t, PWVec3 s0, PWVec3 s1){
	t->type |= SCALE;
	t->bezier_sx = pwbezier1(s0.x, s1.x);
	t->bezier_sy = pwbezier1(s0.y, s1.y);
	t->bezier_sz = pwbezier1(s0.z, s1.z);
}

void pwtransform_scale2(PWTransform *t, PWVec3 s0, PWVec3 s1, PWVec3 s2){
	t->type |= SCALE;
	t->bezier_sx = pwbezier2(s0.x, s1.x, s2.x);
	t->bezier_sy = pwbezier2(s0.y, s1.y, s2.y);
	t->bezier_sz = pwbezier2(s0.z, s1.z, s2.z);
}

void pwtransform_scale3(PWTransform *t, PWVec3 s0, PWVec3 s1, PWVec3 s2, PWVec3 s3){
	t->type |= SCALE;
	t->bezier_sx = pwbezier3(s0.x, s1.x, s2.x, s3.x);
	t->bezier_sy = pwbezier3(s0.y, s1.y, s2.y, s3.y);
	t->bezier_sz = pwbezier3(s0.z, s1.z, s2.z, s3.z);
}

void pwtransform_translation1(PWTransform *t, PWVec3 p0, PWVec3 p1){
	t->type |= TRANSLATION;
	t->bezier_tx = pwbezier1(p0.x, p1.x);
	t->bezier_ty = pwbezier1(p0.y, p1.y);
	t->bezier_tz = pwbezier1(p0.z, p1.z);
}

void pwtransform_translation2(PWTransform *t, PWVec3 p0, PWVec3 p1, PWVec3 p2){
	t->type |= TRANSLATION;
	t->bezier_tx = pwbezier2(p0.x, p1.x, p2.x);
	t->bezier_ty = pwbezier2(p0.y, p1.y, p2.y);
	t->bezier_tz = pwbezier2(p0.z, p1.z, p2.z);
}

void pwtransform_translation3(PWTransform *t, PWVec3 p0, PWVec3 p1, PWVec3 p2, PWVec3 p3){
	t->type |= TRANSLATION;
	t->bezier_tx = pwbezier3(p0.x, p1.x, p2.x, p3.x);
	t->bezier_ty = pwbezier3(p0.y, p1.y, p2.y, p3.y);
	t->bezier_tz = pwbezier3(p0.z, p1.z, p2.z, p3.z);
}

void pwtransform_evaluate(PWTransform *t, float val){
	PWMat4 m1, m2;
	
	if(t->type == 0){
		return;
	}
	
	m2 = PWM_ini(1.0, NULL);
	if(t->type & ROTATION){
		PWM_rotation_ref(&m1, pwbezier_eval(&t->bezier_r, val), t->axis);
		PWM_mul_ref(t->ref, &m1, &m2);
		m2 = m1;
	}
	if(t->type & SCALE){
		PWM_scale_ref(
			&m1,
			PWM_vec3(
				pwbezier_eval(&t->bezier_sx, val),
				pwbezier_eval(&t->bezier_sy, val),
				pwbezier_eval(&t->bezier_sz, val)
			)
		);
		PWM_mul_ref(t->ref, &m1, &m2);
		m2 = m1;
	}
	if(t->type & TRANSLATION){
		PWM_translation_ref(
			&m1,
			PWM_vec3(
				pwbezier_eval(&t->bezier_tx, val),
				pwbezier_eval(&t->bezier_ty, val),
				pwbezier_eval(&t->bezier_tz, val)
			)
		);
		PWM_mul_ref(t->ref, &m1, &m2);
	}
}
