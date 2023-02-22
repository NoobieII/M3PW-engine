//Lithio (The developer's pseudonym)
//July 29, 2022

#include "pwbezier.h"

inline PWBezier pwbezier0(float f0){
	PWBezier b;
	b.n = 0;
	b.p[0] = f0;
	b.t = 0.0f;
	return b;
}

inline PWBezier pwbezier1(float f0, float f1){
	PWBezier b;
	b.n = 1;
	b.p[0] = f1 - f0;
	b.p[1] = f1;
	b.t = 0.0f;
	return b;
}

inline PWBezier pwbezier2(float f0, float f1, float f2){
	PWBezier b;
	b.n = 2;
	b.p[0] = f0 + f2 - 2.0f * f1;
	b.p[1] = 2.0f * (f1 - f0);
	b.p[2] = f0;
	b.t = 0.0f;
	return b;
}

inline PWBezier pwbezier3(float f0, float f1, float f2, float f3){
	PWBezier b;
	b.n = 3;
	b.p[0] = f3 - f0 + 3.0f * (f1 - f2);
	b.p[1] = 3.0f * (f2 + f0 - 2.0f * f1);
	b.p[2] = 3.0f * (f1 - f0);
	b.p[3] = f0;
	b.t = 0.0f;
	return b;
}

inline void pwbezier_reset(PWBezier *b){
	b->t = 0.0f;
}

inline float pwbezier_eval(PWBezier *b, float t){
	float f;
	int i;
	
	if(b->n == 0){
		return b->p[0];
	}
	i = 0;
	f = b->p[i];
	while(i < b->n){
		f *= t;
		f += b->p[++i];
	}
	return f;
}

inline float pwbezier_add(PWBezier *b, float delta){
	b->t += delta;
	b->t -=(int)(b->t);
	return pwbezier_eval(b, b->t);
}
