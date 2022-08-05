//Lithio (The developer's pseudonym)
//July 29, 2022

//transforms calculate the transform matrix depending on the interval
//transformations are performed in order: rotation, scale, translation

#ifndef M3PW_TRANSFORM_H
#define M3PW_TRANSFORM_H

#include "pwbezier.h"
#include "pwmath.h"

typedef struct PWTransform{
	int type;
	PWVec3 axis;
	PWBezier bezier_r;
	PWBezier bezier_sx;
	PWBezier bezier_sy;
	PWBezier bezier_sz;
	PWBezier bezier_tx;
	PWBezier bezier_ty;
	PWBezier bezier_tz;
	PWMat4 *ref;
} PWTransform;

//initializer
void pwtransform_init(PWTransform *t, PWMat4 *ref);

//bezier curve is for the rotation angle, scale and translation
void pwtransform_rotation(PWTransform *t, PWVec3 axis, PWBezier bezier);
void pwtransform_scale1(PWTransform *t, PWVec3 s0, PWVec3 s1);
void pwtransform_scale2(PWTransform *t, PWVec3 s0, PWVec3 s1, PWVec3 s2);
void pwtransform_scale3(PWTransform *t, PWVec3 s0, PWVec3 s1, PWVec3 s2, PWVec3 s3);
void pwtransform_translation1(PWTransform *t, PWVec3 p0, PWVec3 p1);
void pwtransform_translation2(PWTransform *t, PWVec3 p0, PWVec3 p1, PWVec3 p2);
void pwtransform_translation3(PWTransform *t, PWVec3 p0, PWVec3 p1, PWVec3 p2, PWVec3 p3);

//evaluate the transform and set it to ref
void pwtransform_evaluate(PWTransform *t, float val);

#endif
