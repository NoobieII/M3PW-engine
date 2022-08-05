//Lithio (The developer's pseudonym)
//July 29, 2022

//Bezier curves up to degree 3

#ifndef M3PW_BEZIER_H
#define M3PW_BEZIER_H

typedef struct PWBezier{
	int n;      //degree of the bezier curve
	float p[4]; //POLYNOMIAL coefficients, not the points provided
	float t;    //current time value, within interval [0, 1]
} PWBezier;

PWBezier pwbezier0(float f0);
PWBezier pwbezier1(float f0, float f1);
PWBezier pwbezier2(float f0, float f1, float f2);
PWBezier pwbezier3(float f0, float f1, float f2, float f3);

void pwbezier_reset(PWBezier *b);
float pwbezier_eval(PWBezier *b, float t);
float pwbezier_add(PWBezier *b, float delta);

#endif
