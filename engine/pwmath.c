#include "pwmath.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef PWMATRIX_USE_DOUBLES
PWMat4 PWM_ini(double diagonal, const char *str){
#else
PWMat4 PWM_ini(float diagonal, const char *str){
#endif
	PWMat4 m;
	int i, j;
	int n = 0;
	
	char nums[] = "-+.0123456789";
	char whit[] = " \n\r\t";
	
	//set all elements to zero
	for(i = 0; i < 16; ++i){
		m.elements[i] = 0;
	}
	
	//set diagonal elements if string is null
	if(!str){
		for(i = 0; i < 4; ++i){
			m.elements[i + i*4] = diagonal;
		}
	}
	else{
		//unused elements will have a diagonal
		for(i = 0; i < 4; ++i){
			m.elements[i + i*4] = 1.0;
		}
		//loop to get each row
		for(i = 0; i < 4; ++i){
			//loop to get each column element within row
			for(j = 0; j < 4; ++j){
				//skip to first number
				n += strcspn(str + n, nums);
				
				//read number and assign value
				#ifdef PWMATRIX_COLUMN_MAJOR
				#ifdef PWMATRIX_USE_DOUBLES
				sscanf(str + n, "%lf", m.elements + j * 4 + i);
				#else
				sscanf(str + n, "%f", m.elements + j * 4 + i);
				#endif
				#else
				#ifdef PWMATRIX_USE_DOUBLES
				sscanf(str + n, "%lf", m.elements + i * 4 + j);
				#else
				sscanf(str + n, "%f", m.elements + i * 4 + j);
				#endif
				#endif
				
				//skip past number
				n += strspn(str + n, nums);
				
				//skip past whitespace
				n += strspn(str + n, whit);
				
				//break if the character is not a number
				if(strcspn(str + n, nums) != 0){
					break;
				}
			}
			//if the character is not a semicolon ';', break
			if(str[n] != ';'){
				break;
			}
		}
	}
	
	//return matrix
	return m;
}

PWMat4 PWM_eye(int n){
	int i;
	PWMat4 m;
	for(i = 0; i < 16; ++i){
		m.elements[i] = 0.0;
	}
	
	//for(i = 0; i < 4 && i < n; ++i){
	for(i = 0; i < 4; ++i){
		m.elements[i + i * 4] = 1.0;
	}
	return m;
}

PWMat4 PWM_ones(int n){
	int i, j;
	PWMat4 m;
	
	m = PWM_eye(4);
	
	if(n > 4){
		n = 4;
	}
	
	for(i = 0; i < n; ++i){
		for(j = 0; j < n; ++j){
			m.elements[i * 4 + j] = 1.0;
		}
	}
	return m;
}

PWMat4 PWM_orthographic(
	PWMATRIX_TYPE left,
	PWMATRIX_TYPE right,
	PWMATRIX_TYPE bottom,
	PWMATRIX_TYPE top,
	PWMATRIX_TYPE near,
	PWMATRIX_TYPE far
){
	PWMat4 m = PWM_ini(0.0, NULL);
	m.elements[0] = 2.0 / (right - left);
	m.elements[5] = 2.0 / (top - bottom);
	m.elements[10] = -2.0 / (far - near);
	m.elements[15] = 1.0;
	#if defined PWMATRIX_COLUMN_MAJOR
	m.elements[12] = (right + left) / (left - right);
	m.elements[13] = (top + bottom) / (bottom - top);
	m.elements[14] = (far + near) / (near - far);
	#else
	m.elements[3]  = (right + left) / (left - right);
	m.elements[7]  = (top + bottom) / (bottom - top);
	m.elements[11] = (far + near) / (near - far);
	#endif
	
	return m;
}

PWMat4 PWM_perspective(
	PWMATRIX_TYPE fov,
	PWMATRIX_TYPE aspect_ratio,
	PWMATRIX_TYPE near,
	PWMATRIX_TYPE far
){
	PWMat4 m = PWM_ini(0.0, NULL);
	m.elements[0] = 1.0 / (aspect_ratio*tan(PI/360*fov));
	m.elements[5] = m.elements[0] * aspect_ratio;
	m.elements[10] = (far + near) / (near - far);
	#if defined PWMATRIX_COLUMN_MAJOR
	m.elements[11] = -1.0;
	m.elements[14] = 2.0 * far * near / (near - far);
	#else
	m.elements[11] = 2.0 * far * near / (near - far);
	m.elements[14] = -1.0;
	#endif
	
	return m;
}

PWMat4 PWM_translation(
	PWVec3 translation
){
	PWMat4 m = PWM_ini(1.0, NULL);
	#if defined PWMATRIX_COLUMN_MAJOR
	m.elements[12] = translation.x;
	m.elements[13] = translation.y;
	m.elements[14] = translation.z;
	#else
	m.elements[3] = translation.x;
	m.elements[7] = translation.y;
	m.elements[11] = translation.z;
	#endif
	return m;
}

PWMat4 PWM_rotation(
	PWMATRIX_TYPE angle,
	PWVec3 axis
){
	PWMat4 m = PWM_ini(1.0, NULL);
	angle = angle * PI / 180.0;
	
	PWMATRIX_TYPE c = cos(angle);
	PWMATRIX_TYPE s = sin(angle);
	PWMATRIX_TYPE omc = 1.0 - c;
	
	#if defined PWMATRIX_COLUMN_MAJOR
	m.elements[0] = axis.x * axis.x * omc + c;
	m.elements[1] = axis.x * axis.y * omc + s * axis.z;
	m.elements[2] = axis.x * axis.z * omc - s * axis.y;
	m.elements[4] = axis.y * axis.x * omc - s * axis.z;
	m.elements[5] = axis.y * axis.y * omc + c;
	m.elements[6] = axis.y * axis.z * omc + s * axis.x;
	m.elements[8] = axis.z * axis.x * omc + s * axis.y;
	m.elements[9] = axis.z * axis.y * omc - s * axis.x;
	m.elements[10] = axis.z * axis.z * omc + c;
	#else
	m.elements[0] = axis.x * axis.x * omc + c;
	m.elements[1] = axis.y * axis.x * omc - s * axis.z;
	m.elements[2] = axis.z * axis.x * omc + s * axis.y;
	m.elements[4] = axis.x * axis.y * omc + s * axis.z;
	m.elements[5] = axis.y * axis.y * omc + c;
	m.elements[6] = axis.z * axis.y * omc - s * axis.x;
	m.elements[8] = axis.x * axis.z * omc - s * axis.y;
	m.elements[9] = axis.y * axis.z * omc + s * axis.x;
	m.elements[10] = axis.z * axis.z * omc + c;
	#endif
	return m;
}

PWMat4 PWM_scale(
	PWVec3 scale
){
	PWMat4 m = PWM_ini(1.0, NULL);
	m.elements[0] = scale.x;
	m.elements[5] = scale.y;
	m.elements[10] = scale.z;
	return m;
}

inline void PWM_translation_ref(PWMat4 *result, PWVec3 translation){
	result->elements[0] = result->elements[6] = result->elements[10]
		= result->elements[15] = 1;
	result->elements[1] = result->elements[2] = result->elements[4]
		= result->elements[6] = result->elements[8] = result->elements[9] = 0;
	#if defined PWMATRIX_COLUMN_MAJOR
	result->elements[3] = result->elements[7] = result->elements[11] = 0;
	result->elements[12] = translation.x;
	result->elements[13] = translation.y;
	result->elements[14] = translation.z;
	#else
	result->elements[12] = result->elements[13] = result->elements[14] = 0;
	result->elements[3] = translation.x;
	result->elements[7] = translation.y;
	result->elements[11] = translation.z;
	#endif
}

inline void PWM_rotation_ref(PWMat4 *result, PWMATRIX_TYPE angle, PWVec3 axis){
	angle = angle * PI / 180.0;
	PWMATRIX_TYPE c = cos(angle);
	PWMATRIX_TYPE s = sin(angle);
	PWMATRIX_TYPE omc = 1.0 - c;
	
	result->elements[0] = result->elements[5] = result->elements[10]
		= result->elements[15] = 1;
	result->elements[3] = result->elements[7] = result->elements[11]
		= result->elements[12] = result->elements[13] = result->elements[14] = 0;
	
	#if defined PWMATRIX_COLUMN_MAJOR
	result->elements[0] = axis.x * axis.x * omc + c;
	result->elements[1] = axis.x * axis.y * omc + s * axis.z;
	result->elements[2] = axis.x * axis.z * omc - s * axis.y;
	result->elements[4] = axis.y * axis.x * omc - s * axis.z;
	result->elements[5] = axis.y * axis.y * omc + c;
	result->elements[6] = axis.y * axis.z * omc + s * axis.x;
	result->elements[8] = axis.z * axis.x * omc + s * axis.y;
	result->elements[9] = axis.z * axis.y * omc - s * axis.x;
	result->elements[10] = axis.z * axis.z * omc + c;
	#else
	result->elements[0] = axis.x * axis.x * omc + c;
	result->elements[1] = axis.y * axis.x * omc - s * axis.z;
	result->elements[2] = axis.z * axis.x * omc + s * axis.y;
	result->elements[4] = axis.x * axis.y * omc + s * axis.z;
	result->elements[5] = axis.y * axis.y * omc + c;
	result->elements[6] = axis.z * axis.y * omc - s * axis.x;
	result->elements[8] = axis.x * axis.z * omc - s * axis.y;
	result->elements[9] = axis.y * axis.z * omc + s * axis.x;
	result->elements[10] = axis.z * axis.z * omc + c;
	#endif
}

inline void PWM_scale_ref(PWMat4 *result, PWVec3 scale){
	result->elements[0] = scale.x;
	result->elements[1] = 0;
	result->elements[2] = 0;
	result->elements[3] = 0;
	result->elements[4] = 0;
	result->elements[5] = scale.y;
	result->elements[6] = 0;
	result->elements[7] = 0;
	result->elements[8] = 0;
	result->elements[9] = 0;
	result->elements[10] = scale.z;
	result->elements[11] = 0;
	result->elements[12] = 0;
	result->elements[13] = 0;
	result->elements[14] = 0;
	result->elements[15] = 1;
}

PWMat4 PWM_add(PWMat4 lhs, PWMat4 rhs){
	int i;
	PWMat4 m;
	
	for(i = 0; i < 16; ++i){
		m.elements[i] = lhs.elements[i] + rhs.elements[i];
	}
	
	return m;
}

PWMat4 PWM_sub(PWMat4 lhs, PWMat4 rhs){
	int i;
	PWMat4 m;
	
	for(i = 0; i < 16; ++i){
		m.elements[i] = lhs.elements[i] - rhs.elements[i];
	}
	
	return m;
}

PWMat4 PWM_mul(PWMat4 lhs, PWMat4 rhs){
	int i, j, k;
	PWMat4 m;
	
	for(i = 0; i < 4; ++i){
		for(j = 0; j < 4; ++j){
			#if defined PWMATRIX_COLUMN_MAJOR
			m.elements[j * 4 + i] = 0.0;
			#else
			m.elements[i * 4 + j] = 0.0;
			#endif
			
			//multiply with each element
			for(k = 0; k < 4; ++k){
				#if defined PWMATRIX_COLUMN_MAJOR
				m.elements[j * 4 + i] += lhs.elements[k * 4 + i] * rhs.elements[j * 4 + k];
				#else
				m.elements[i * 4 + j] += lhs.elements[i * 4 + k] * rhs.elements[k * 4 + j];
				#endif
			}
		}
	}
	
	return m;
}

void PWM_mul_ref(PWMat4 *result, PWMat4 *lhs, PWMat4 *rhs){
	int i, j, k;
	__m128 a, b, c, d;
	
	#if defined PWMATRIX_COLUMN_MAJOR
	#ifndef PWMATRIX_USE_DOUBLES
	for(i = 0; i < 4; ++i){
		a = _mm_set1_ps(rhs->elements[i*4 + 0]);
		b = _mm_loadu_ps(lhs->elements);
		d = _mm_mul_ps(a, b);
		
		a = _mm_set1_ps(rhs->elements[i*4 + 1]);
		b = _mm_loadu_ps(lhs->elements + 4);
		c = _mm_mul_ps(a, b);
		d = _mm_add_ps(c, d);
		
		a = _mm_set1_ps(rhs->elements[i*4 + 2]);
		b = _mm_loadu_ps(lhs->elements + 8);
		c = _mm_mul_ps(a, b);
		d = _mm_add_ps(c, d);
		
		a = _mm_set1_ps(rhs->elements[i*4 + 3]);
		b = _mm_loadu_ps(lhs->elements + 12);
		c = _mm_mul_ps(a, b);
		d = _mm_add_ps(c, d);
		
		_mm_storeu_ps(&result->elements[i*4], d);
	}
	#endif
	#else
		
	for(i = 0; i < 4; ++i){
		for(j = 0; j < 4; ++j){
			#if defined PWMATRIX_COLUMN_MAJOR
			result->elements[j * 4 + i] = 0.0;
			#else
			result->elements[i * 4 + j] = 0.0;
			#endif
			
			//multiply with each element
			for(k = 0; k < 4; ++k){
				#if defined PWMATRIX_COLUMN_MAJOR
				result->elements[j * 4 + i] += lhs->elements[k * 4 + i] * rhs->elements[j * 4 + k];
				#else
				result->elements[i * 4 + j] += lhs->elements[i * 4 + k] * rhs->elements[k * 4 + j];
				#endif
			}
		}
	}
	#endif
}

PWVec3 PWM_mul_vec3(PWMat4 lhs, PWVec3 rhs){
	PWVec3 v;
	#if defined PWMATRIX_COLUMN_MAJOR
	v.x = lhs.elements[0] * rhs.x + lhs.elements[4] * rhs.y + lhs.elements[8] * rhs.z + lhs.elements[12];
	v.y = lhs.elements[1] * rhs.x + lhs.elements[5] * rhs.y + lhs.elements[9] * rhs.z + lhs.elements[13];
	v.z = lhs.elements[2] * rhs.x + lhs.elements[6] * rhs.y + lhs.elements[10] * rhs.z + lhs.elements[14];
	#else
	v.x = lhs.elements[0] * rhs.x + lhs.elements[2] * rhs.y + lhs.elements[3] * rhs.z + lhs.elements[4];
	v.y = lhs.elements[5] * rhs.x + lhs.elements[6] * rhs.y + lhs.elements[7] * rhs.z + lhs.elements[8];
	v.z = lhs.elements[9] * rhs.x + lhs.elements[10] * rhs.y + lhs.elements[11] * rhs.z + lhs.elements[12];
	#endif
	return v;
}

inline void PWM_mul_vec3_ref(PWVec3 *result, PWMat4 *lhs, PWVec3 *rhs){
	__m128 a, b, c, d;
	//#if defined PWMATRIX_COLUMN_MAJOR
	#if 0
	#ifndef PWMATRIX_USE_DOUBLES
	a = _mm_set1_ps(rhs->x);
	b = _mm_load_ps(lhs->elements);
	d = _mm_mul_ps(a, b);
	a = _mm_set1_ps(rhs->y);
	b = _mm_load_ps(lhs->elements + 4);
	c = _mm_mul_ps(a, b);
	d = _mm_add_ps(d, c);
	a = _mm_set1_ps(rhs->z);
	b = _mm_load_ps(lhs->elements + 8);
	c = _mm_mul_ps(a, b);
	d = _mm_add_ps(d, c);
	b = _mm_load_ps(lhs->elements + 12);
	d = _mm_add_ps(d, b);
	result->x = ((__v4sf)d)[0];
	result->y = ((__v4sf)d)[1];
	result->z = ((__v4sf)d)[2];
	#endif
	#else
	#if defined PWMATRIX_COLUMN_MAJOR
	result->x = lhs->elements[0] * rhs->x + lhs->elements[4] * rhs->y + lhs->elements[8] * rhs->z + lhs->elements[12];
	result->y = lhs->elements[1] * rhs->x + lhs->elements[5] * rhs->y + lhs->elements[9] * rhs->z + lhs->elements[13];
	result->z = lhs->elements[2] * rhs->x + lhs->elements[6] * rhs->y + lhs->elements[10] * rhs->z + lhs->elements[14];
	#else
	result->x = lhs->elements[0] * rhs->x + lhs->elements[2] * rhs->y + lhs->elements[3] * rhs->z + lhs->elements[4];
	result->y = lhs->elements[5] * rhs->x + lhs->elements[6] * rhs->y + lhs->elements[7] * rhs->z + lhs->elements[8];
	result->z = lhs->elements[9] * rhs->x + lhs->elements[10] * rhs->y + lhs->elements[11] * rhs->z + lhs->elements[12];
	#endif
	#endif
}

PWVec3 PWM_mul_vec3_notranslate(PWMat4 lhs, PWVec3 rhs){
	PWVec3 result;
	#if defined PWMATRIX_COLUMN_MAJOR
	result.x = lhs.elements[0] * rhs.x + lhs.elements[4] * rhs.y + lhs.elements[8] * rhs.z;
	result.y = lhs.elements[1] * rhs.x + lhs.elements[5] * rhs.y + lhs.elements[9] * rhs.z;
	result.z = lhs.elements[2] * rhs.x + lhs.elements[6] * rhs.y + lhs.elements[10] * rhs.z;
	#else
	result.x = lhs.elements[0] * rhs.x + lhs.elements[2] * rhs.y + lhs.elements[3] * rhs.z;
	result.y = lhs.elements[5] * rhs.x + lhs.elements[6] * rhs.y + lhs.elements[7] * rhs.z;
	result.z = lhs.elements[9] * rhs.x + lhs.elements[10] * rhs.y + lhs.elements[11] * rhs.z;
	#endif
	return result;
}

inline void PWM_mul_vec3_notranslate_ref(PWVec3 *result, PWMat4 *lhs, PWVec3 *rhs){
	#if defined PWMATRIX_COLUMN_MAJOR
	result->x = lhs->elements[0] * rhs->x + lhs->elements[4] * rhs->y + lhs->elements[8] * rhs->z;
	result->y = lhs->elements[1] * rhs->x + lhs->elements[5] * rhs->y + lhs->elements[9] * rhs->z;
	result->z = lhs->elements[2] * rhs->x + lhs->elements[6] * rhs->y + lhs->elements[10] * rhs->z;
	#else
	result->x = lhs->elements[0] * rhs->x + lhs->elements[2] * rhs->y + lhs->elements[3] * rhs->z;
	result->y = lhs->elements[5] * rhs->x + lhs->elements[6] * rhs->y + lhs->elements[7] * rhs->z;
	result->z = lhs->elements[9] * rhs->x + lhs->elements[10] * rhs->y + lhs->elements[11] * rhs->z;
	#endif
}

PWMat4 PWM_inv(PWMat4 m){
	int i, j, k;
	double tmp[4];
	double t;
	
	//augmented matrix
	PWMat4 am;
	
	am = PWM_eye(4);
	
	#if defined PWMATRIX_COLUMN_MAJOR
	PWM_transpose_ref(&m);
	#endif
	
	//perform row echelon form on each row
	for(j = 0; j < 4; ++j){
		
		//find a row with nonzero leading element (break if there is none)
		for(i = j; m.elements[i * 4 + j] == 0.0 && i < 4; ++i);
		
		if(i == 4){
			break;
		}
		
		//normalilze the leading element to 1
		t =  m.elements[i * 4 + j];
		for(k = 0; k < 4; ++k){
			m.elements[i * 4 + k] /= t;
			
			//augmented matrix operation
			am.elements[i * 4 + k] /= t;
		}
		
		//move the row to the appropriate row
		if(i != j){
			memcpy(tmp, m.elements + j * 4, 4 * sizeof(PWMATRIX_TYPE));
			memcpy(m.elements + j * 4, m.elements + i * 4, 4 * sizeof(PWMATRIX_TYPE));
			memcpy(m.elements + i * 4, tmp, 4 * sizeof(PWMATRIX_TYPE));
			
			//augmented matrix operation
			memcpy(tmp, am.elements + j * 4, 4 * sizeof(PWMATRIX_TYPE));
			memcpy(am.elements + j * 4, am.elements + i * 4, 4 * sizeof(PWMATRIX_TYPE));
			memcpy(am.elements + i * 4, tmp, 4 * sizeof(PWMATRIX_TYPE));
		}
		//PWM_printa(m, am);
		
		//subtract all other rows to make their column 0
		for(i = j + 1; i < 4; ++i){
			t = m.elements[i * 4 + j];
			for(k = 0; k < 4; ++k){
				//                                              //scale factor
				m.elements[i * 4 + k] -= m.elements[j * 4 + k] * t;
				
				//aumented matrix operation
				am.elements[i * 4 + k] -= am.elements[j * 4 + k] * t;
			}
		}
		//PWM_printa(m, am);
	}
	
	//perform reduced row echelon form
	for(j = 1; j < 4; ++j){
		if(m.elements[j * 4 + j] == 0){
			am = PWM_ini(0, NULL);
			break;
		}
		//subtract the row by the normalized row scaled by leading element
		for(i = 0; i < j; ++i){
			t = m.elements[i * 4 + j];
			
			for(k = 0; k < 4; ++k){
				m.elements[i * 4 + k] -= m.elements[j * 4 + k] * t;
				
				am.elements[i * 4 + k] -= am.elements[j * 4 + k] * t;
			}
		}
		//PWM_printa(m, am);
	}
	
	#if defined PWMATRIX_COLUMN_MAJOR
	PWM_transpose_ref(&am);
	#endif
	
	//return augmented matrix
	return am;
}

int PWM_inv_ref(PWMat4 *result, PWMat4 *m){
	//MESA implementation of the GLU library
	PWMATRIX_TYPE det;
	PWMATRIX_TYPE *inv = result->elements;
	PWMATRIX_TYPE *in = m->elements;
	int i;
	
	inv[0] = in[5]  * in[10] * in[15] - 
		in[5]  * in[11] * in[14] - 
		in[9]  * in[6]  * in[15] + 
		in[9]  * in[7]  * in[14] +
		in[13] * in[6]  * in[11] - 
		in[13] * in[7]  * in[10];
	
	inv[4] = -in[4]  * in[10] * in[15] + 
		in[4]  * in[11] * in[14] + 
		in[8]  * in[6]  * in[15] - 
		in[8]  * in[7]  * in[14] - 
		in[12] * in[6]  * in[11] + 
		in[12] * in[7]  * in[10];
	
	inv[8] = in[4]  * in[9] * in[15] - 
		in[4]  * in[11] * in[13] - 
		in[8]  * in[5] * in[15] + 
		in[8]  * in[7] * in[13] + 
		in[12] * in[5] * in[11] - 
		in[12] * in[7] * in[9];
	
	inv[12] = -in[4]  * in[9] * in[14] + 
		in[4]  * in[10] * in[13] +
		in[8]  * in[5] * in[14] - 
		in[8]  * in[6] * in[13] - 
		in[12] * in[5] * in[10] + 
		in[12] * in[6] * in[9];
	
	inv[1] = -in[1]  * in[10] * in[15] + 
		in[1]  * in[11] * in[14] + 
		in[9]  * in[2] * in[15] - 
		in[9]  * in[3] * in[14] - 
		in[13] * in[2] * in[11] + 
		in[13] * in[3] * in[10];

	inv[5] = in[0]  * in[10] * in[15] - 
		in[0]  * in[11] * in[14] - 
		in[8]  * in[2] * in[15] + 
		in[8]  * in[3] * in[14] + 
		in[12] * in[2] * in[11] - 
		in[12] * in[3] * in[10];
	
	inv[9] = -in[0]  * in[9] * in[15] + 
		in[0]  * in[11] * in[13] + 
		in[8]  * in[1] * in[15] - 
		in[8]  * in[3] * in[13] - 
		in[12] * in[1] * in[11] + 
		in[12] * in[3] * in[9];
	
	inv[13] = in[0]  * in[9] * in[14] - 
		in[0]  * in[10] * in[13] - 
		in[8]  * in[1] * in[14] + 
		in[8]  * in[2] * in[13] + 
		in[12] * in[1] * in[10] - 
		in[12] * in[2] * in[9];
	
	inv[2] = in[1]  * in[6] * in[15] - 
		in[1]  * in[7] * in[14] - 
		in[5]  * in[2] * in[15] + 
		in[5]  * in[3] * in[14] + 
		in[13] * in[2] * in[7] - 
		in[13] * in[3] * in[6];

	inv[6] = -in[0]  * in[6] * in[15] + 
		in[0]  * in[7] * in[14] + 
		in[4]  * in[2] * in[15] - 
		in[4]  * in[3] * in[14] - 
		in[12] * in[2] * in[7] + 
		in[12] * in[3] * in[6];

	inv[10] = in[0]  * in[5] * in[15] - 
		in[0]  * in[7] * in[13] - 
		in[4]  * in[1] * in[15] + 
		in[4]  * in[3] * in[13] + 
		in[12] * in[1] * in[7] - 
		in[12] * in[3] * in[5];

	inv[14] = -in[0]  * in[5] * in[14] + 
		in[0]  * in[6] * in[13] + 
		in[4]  * in[1] * in[14] - 
		in[4]  * in[2] * in[13] - 
		in[12] * in[1] * in[6] + 
		in[12] * in[2] * in[5];

	inv[3] = -in[1] * in[6] * in[11] + 
		in[1] * in[7] * in[10] + 
		in[5] * in[2] * in[11] - 
		in[5] * in[3] * in[10] - 
		in[9] * in[2] * in[7] + 
		in[9] * in[3] * in[6];

	inv[7] = in[0] * in[6] * in[11] - 
		in[0] * in[7] * in[10] - 
		in[4] * in[2] * in[11] + 
		in[4] * in[3] * in[10] + 
		in[8] * in[2] * in[7] - 
		in[8] * in[3] * in[6];

	inv[11] = -in[0] * in[5] * in[11] + 
		in[0] * in[7] * in[9] + 
		in[4] * in[1] * in[11] - 
		in[4] * in[3] * in[9] - 
		in[8] * in[1] * in[7] + 
		in[8] * in[3] * in[5];

	inv[15] = in[0] * in[5] * in[10] - 
		in[0] * in[6] * in[9] - 
		in[4] * in[1] * in[10] + 
		in[4] * in[2] * in[9] + 
		in[8] * in[1] * in[6] - 
		in[8] * in[2] * in[5];

	det = in[0] * inv[0] + in[1] * inv[4] + in[2] * inv[8] + in[3] * inv[12];

	if (det == 0)
		return -1;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		inv[i] = inv[i] * det;

	return 0;
}


PWMat4 PWM_transpose(PWMat4 m){
	PWMat4 t;
	int i, j;
	
	for(i = 0; i < 4; ++i){
		for(j = 0; j < 4; ++j){
			t.elements[i * 4 + j] = m.elements[j * 4 + i];
		}
	}
	return t;
}

void PWM_transpose_ref(PWMat4 *m){
	int i, j;
	double tmp;
	
	for(i = 0; i < 4; ++i){
		for(j = i + 1; j < 4; ++j){
			tmp = m->elements[i * 4 + j];
			m->elements[i * 4 + j] = m->elements[j * 4 + i];
			m->elements[j * 4 + i] = tmp;
		}
	}
}

PWVec2 PWM_vec2(PWMATRIX_TYPE x, PWMATRIX_TYPE y){
	PWVec2 v;
	v.x = x;
	v.y = y;
	return v;
}

PWVec3 PWM_vec3(PWMATRIX_TYPE x, PWMATRIX_TYPE y, PWMATRIX_TYPE z){
	PWVec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

PWVec4 PWM_vec4(PWMATRIX_TYPE x, PWMATRIX_TYPE y, PWMATRIX_TYPE z, PWMATRIX_TYPE w){
	PWVec4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

//add/subtract
PWVec2 PWM_add2(PWVec2 v1, PWVec2 v2){
	PWVec2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

void PWM_add2_ref(PWVec2 *result, PWVec2 *v1, PWVec2 *v2){
	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
}

PWVec3 PWM_add3(PWVec3 v1, PWVec3 v2){
	PWVec3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

void PWM_add3_ref(PWVec3 *result, PWVec3 *v1, PWVec3 *v2){
	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
	result->z = v1->z + v2->z;
}

PWVec4 PWM_add4(PWVec4 v1, PWVec4 v2){
	PWVec4 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	result.w = v1.w + v2.w;
	return result;
}

void PWM_add4_ref(PWVec4 *result, PWVec4 *v1, PWVec4 *v2){
	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
	result->z = v1->z + v2->z;
	result->w = v1->w + v2->w;
}

PWVec2 PWM_sub2(PWVec2 v1, PWVec2 v2){
	PWVec2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

void PWM_sub2_ref(PWVec2 *result, PWVec2 *v1, PWVec2 *v2){
	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
}

PWVec3 PWM_sub3(PWVec3 v1, PWVec3 v2){
	PWVec3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

void PWM_sub3_ref(PWVec3 *result, PWVec3 *v1, PWVec3 *v2){
	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
	result->z = v1->z - v2->z;
}

PWVec4 PWM_sub4(PWVec4 v1, PWVec4 v2){
	PWVec4 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	result.w = v1.w - v2.w;
	return result;
}

void PWM_sub4_ref(PWVec4 *result, PWVec4 *v1, PWVec4 *v2){
	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
	result->z = v1->z - v2->z;
	result->w = v1->w - v2->w;
}

PWVec2 PWM_mul2(PWVec2 v, float f){
	PWVec2 result;
	result.x = v.x * f;
	result.y = v.y * f;
	return result;
}

void PWM_mul2_ref(PWVec2 *result, PWVec2 *v, float f){
	result->x = v->x * f;
	result->y = v->y * f;
}

PWVec3 PWM_mul3(PWVec3 v, float f){
	PWVec3 result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	return result;
}

void PWM_mul3_ref(PWVec3 *result, PWVec3 *v, float f){
	result->x = v->x * f;
	result->y = v->y * f;
	result->z = v->z * f;
}

PWVec4 PWM_mul4(PWVec4 v, float f){
	PWVec4 result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	result.w = v.w * f;
	return result;
}

void PWM_mul4_ref(PWVec4 *result, PWVec4 *v, float f){
	result->x = v->x * f;
	result->y = v->y * f;
	result->z = v->z * f;
	result->w = v->w * f;
}

PWMATRIX_TYPE PWM_dot2(PWVec2 v1, PWVec2 v2){
	return v1.x * v2.x + v1.y * v2.y;
}

inline PWMATRIX_TYPE PWM_dot2_ref(PWVec2 *v1, PWVec2 *v2){
	return v1->x * v2->x + v1->y * v2->y;
}

PWMATRIX_TYPE PWM_dot3(PWVec3 v1, PWVec3 v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline PWMATRIX_TYPE PWM_dot3_ref(PWVec3 *v1, PWVec3 *v2){
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

PWMATRIX_TYPE PWM_dot4(PWVec4 v1, PWVec4 v2){
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

inline PWMATRIX_TYPE PWM_dot4_ref(PWVec4 *v1, PWVec4 *v2){
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

PWVec3 PWM_cross3(PWVec3 lhs, PWVec3 rhs){
	PWVec3 result;
	result.x = lhs.y * rhs.z - lhs.z * rhs.y;
	result.y = lhs.z * rhs.x - lhs.x * rhs.z;
	result.z = lhs.x * rhs.y - lhs.y * rhs.x;
	return result;
}
	
void PWM_cross3_ref(PWVec3 *result, PWVec3 *lhs, PWVec3 *rhs){
	result->x = lhs->y * rhs->z - lhs->z * rhs->y;
	result->y = lhs->z * rhs->x - lhs->x * rhs->z;
	result->z = lhs->x * rhs->y - lhs->y * rhs->x;
}

PWVec3 PWM_normalize3(PWVec3 v){
	PWVec3 result;
	float squared = v.x * v.x + v.y * v.y + v.z * v.z;
	float invs;
	
	//inverse square root
	_mm_store_ss(&invs, _mm_rsqrt_ss(_mm_set_ss(squared)));
	
	result.x = v.x * invs;
	result.y = v.y * invs;
	result.z = v.z * invs;
	
	return result;
}

void PWM_normalize3_ref(PWVec3 *result, PWVec3 *v){
	float squared = v->x * v->x + v->y * v->y + v->z * v->z;
	float invs;
	
	//inverse square root
	_mm_store_ss(&invs, _mm_rsqrt_ss(_mm_set_ss(squared)));
	
	result->x = v->x * invs;
	result->y = v->y * invs;
	result->z = v->z * invs;
}

float PWM_norm3(PWVec3 v){
	float squared = v.x * v.x + v.y * v.y + v.z * v.z;
	float rt;
	_mm_store_ss(&rt, _mm_sqrt_ss(_mm_set_ss(squared)));
	return rt;
}

float PWM_norm3_ref(PWVec3 *v){
	float squared = v->x * v->x + v->y * v->y + v->z * v->z;
	float rt;
	_mm_store_ss(&rt, _mm_sqrt_ss(_mm_set_ss(squared)));
	return rt;
}

inline PWRay PWM_ray(PWVec3 origin, PWVec3 direction){
	PWRay ray;
	ray.origin = origin;
	ray.dir = direction;
	return ray;
}

inline PWPlane PWM_plane(PWVec3 normal, PWVec3 point){
	PWPlane plane;
	plane.normal = normal;
	plane.p = point;
	plane.distance = -PWM_dot3(normal, point);
	return plane;
}

inline PWPlane PWM_plane3(PWVec3 v0, PWVec3 v1, PWVec3 v2){
	PWPlane plane;
	PWVec3 edge1, edge2;
	
	PWM_sub3_ref(&edge1, &v1, &v0);
	PWM_sub3_ref(&edge2, &v2, &v0);
	
	PWM_cross3_ref(&plane.normal, &edge1, &edge2);
	
	PWM_normalize3_ref(&plane.normal, &plane.normal);
	plane.distance = fabs(PWM_dot3(plane.normal, v0));
	PWM_mul3_ref(&plane.p, &plane.normal, plane.distance);
	
	return plane;
}

inline PWAabb PWM_aabb(PWVec3 v1, PWVec3 v2){
	PWAabb aabb;
	if(v1.x < v2.x){ aabb.min.x = v1.x; aabb.max.x = v2.x; }
	else { aabb.max.x = v1.x; aabb.min.x = v2.x; }
	if(v1.y < v2.y){ aabb.min.y = v1.y; aabb.max.y = v2.y; }
	else { aabb.max.y = v1.y; aabb.min.y = v2.y; }
	if(v1.z < v2.z){ aabb.min.z = v1.z; aabb.max.z = v2.z; }
	else { aabb.max.z = v1.z; aabb.min.z = v2.z; }
	return aabb;
}

void PWM_ray_detransform(PWRay *r, PWMat4 *transform){
	PWMat4 inv;
	PWVec3 t;
	
	if(PWM_inv_ref(&inv, transform) == -1){
		printf("PWM_ray_detransform: matrix inverse not possible\n");
		return;
	}
	
	//apply inverse matrix
	r->origin = PWM_mul_vec3(inv, r->origin);
	r->dir    = PWM_mul_vec3_notranslate(inv, r->dir);
}

inline float PWM_plane_distance(PWPlane *p, PWVec3 v){
	return fabs(PWM_dot3(p->normal, v) - p->distance);
}

inline int PWM_plane_classify(PWPlane *p, PWVec3 v){
	float f = PWM_dot3(v, p->normal) - p->distance;
	if(f > 0.0001) return PWM_FRONT;
	if(f < -0.0001) return PWM_BACK;
	return PWM_PLANAR;
}

//algorithm from Moller and Trumbore.
int PWM_ray_x_triangle(PWRay *r, PWVec3 v0, PWVec3 v1, PWVec3 v2, float *t){
	PWVec3 edge1, edge2;
	PWVec3 pvec, tvec, qvec;
	float det, u, v;
	
	PWM_sub3_ref(&edge1, &v1, &v0);
	PWM_sub3_ref(&edge2, &v2, &v0);
	
	PWM_cross3_ref(&pvec, &r->dir, &edge2);
	
	det = PWM_dot3_ref(&edge1, &pvec);
	
	//cull triangles that go clockwise
	#ifdef PWM_CULLING
	if(det < 0.0001){
		return 0;
	}
	#endif
	
	//the ray and triangle is parallel
	if((det < 0.0001f) && (det > -0.0001)){
		return 0;
	}
	
	//u is distance to plane
	PWM_sub3_ref(&tvec, &r->origin, &v0);
	u = PWM_dot3_ref(&tvec, &pvec);
	if(u < 0 || u > det){
		return 0;
	}
	
	PWM_cross3_ref(&qvec, &tvec, &edge1);
	v = PWM_dot3_ref(&r->dir, &qvec);
	if(v < 0 || u + v > det){
		return 0;
	}
	
	if(t){
		*t = PWM_dot3_ref(&edge2, &qvec);
		*t /= det;
	}
	
	return 1;
}

int PWM_line_x_triangle(PWVec3 l0, PWVec3 l1, PWVec3 v0, PWVec3 v1, PWVec3 v2, float *t){
	PWRay r;
	PWVec3 dir;
	float d;
	float _t;
	
	PWM_sub3_ref(&dir, &l1, &l0);
	d = PWM_dot3(dir, dir);
	PWM_normalize3_ref(&dir, &dir);
	r = PWM_ray(l0, dir);
	
	if(PWM_ray_x_triangle(&r, v0, v1, v2, &_t) == 0){
		return 0;
	}
	if(_t*_t > d){
		return 0;
	}
	if(t){
		*t = _t;
	}
	
	return 1;
}

int PWM_ray_x_plane(PWRay *r, PWPlane *p, float *t, PWVec3 *hit){
	float d, vo, _t;
	
	d = PWM_dot3_ref(&p->normal, &r->dir);
	
	//ray parallel to the plane
	if(d < 0.0001 || d > -0.0001){
		return 0;
	}
	
	//shooting from behind the plane or away from plane
	#ifdef PWM_CULLING
	if(d > 0.0){
		return 0;
	}
	#endif
	
	vo = -(PWM_dot3_ref(&p->normal, &r->origin) + p->distance);
	_t = vo / d;
	
	if(_t < 0.0){
		return 0;
	}
	
	if(hit){
		PWM_mul3_ref(hit, &r->dir, _t);
		PWM_add3_ref(hit, &r->origin, hit);
	}
	
	if(t){
		*t = _t;
	}
	
	return 1;
}

int PWM_line_x_plane(PWVec3 l0, PWVec3 l1, PWPlane *p, float *t, PWVec3 *hit){
	PWRay r;
	PWVec3 dir;
	float d;
	float _t;
	
	PWM_sub3_ref(&dir, &l1, &l0);
	d = PWM_dot3(dir, dir);
	PWM_normalize3_ref(&dir, &dir);
	r = PWM_ray(l0, dir);
	
	if(PWM_ray_x_plane(&r, p, &_t, hit) == 0){
		return 0;
	}
	if(_t < 0.0 || _t*_t > d){
		return 0;
	}
	if(t){
		*t = _t;
	}
	return 1;
}

int PWM_ray_x_aabb(PWRay *r, PWAabb *aabb, PWVec3 *hit){
	float maxt[3];
	float *origin = (float*)(&r->origin);
	float *dir = (float*)(&r->dir);
	float *min = (float*)(&aabb->min);
	float *max = (float*)(&aabb->max);
	
	int inside = 1;
	int nplane;
	int i = 0;
	
	for(i = 0; i < 3; ++i){
		maxt[i] = -1.0;
	}
	
	//all components
	for(i = 0; i < 3; ++i){
		if(origin[i] < min[i]){
			((float*)(hit))[i] = min[i];
			inside = 0;
			if(dir[i] != 0.0){
				maxt[i] = (min[i] - origin[i]) / dir[i];
			}
		}
		else if(origin[i] > max[i]){
			((float*)(hit))[i] = max[i];
			inside = 0;
			if(dir[i] != 0.0){
				maxt[i] = (max[i] - origin[i]) / dir[i];
			}
		}
	}
	
	
	//ray origin inside the box
	if(inside){
		*hit = r->origin;
		return 1;
	}
	
	//find maximum value for maxt
	nplane = 0;
	
	for(i = 1; i < 3; ++i){
		if(maxt[nplane] < maxt[i]){
			nplane = i;
		}
	}
	
	if(maxt[nplane] < 0.0){
		return 0;
	}
	
	for(i = 0; i < 3; ++i){
		if(nplane != i){
			((float*)(hit))[i] = origin[i] +  maxt[nplane] * dir[i];
			if(((float*)(hit))[i] < min[i] - 0.0001 || ((float*)(hit))[i] > max[i] + 0.0001){
				return 0;
			}
		}
	}
	
	return 1;
}

//Slabs Method, by Moller and Haines
int PWM_ray_x_obb(PWRay *r, PWObb *obb, float *t){
	float e, f, t1, t2, tmp;
	float tmin = -99999.9;
	float tmax =  99999.9;
	PWVec3 p;
	int i;
	
	PWM_sub3_ref(&p, &obb->center, &r->origin);
	
	//1. Slap
	for(i = 0; i < 3; ++i){
		e = PWM_dot3(obb->a[i], p);
		f = PWM_dot3(obb->a[i], r->dir);
		
		//otherwise ray parallel to plane normal
		if(f < -0.0001 || f > 0.0001){
			t1 = (e + obb->fa[i]) / f;
			t2 = (e - obb->fa[i]) / f;
			
			if(t1 > t2){ tmp = t1; t1 = t2; t2 = tmp; }
			if(t1 > tmin) tmin = t1;
			if(t2 < tmax) tmax = t2;
			if(tmin > tmax) return 0;
			if(tmax < 0.0) return 0;
		}
		else if( ((-e - obb->fa[i]) > 0.0) || ((-e + obb->fa[i]) < 0.0) ){
			return 0;
		}
	}
	
	if(tmin < 0.0){
		if(t) *t = tmin;
		return 1;
	}
	if(t) *t = tmax;
	
	return 1;
}

int PWM_plane_x_triangle(PWPlane *p, PWVec3 v0, PWVec3 v1, PWVec3 v2){
	int n;
	
	n = PWM_plane_classify(p, v0);
	if( (n == PWM_plane_classify(p, v1)) && (n == PWM_plane_classify(p, v2)) ){
		return 0;
	}
	return 1;
}

int PWM_plane_x_plane(PWPlane *p1, PWPlane *p2, PWRay *r){
	PWVec3 cross;
	float sqrlength;
	float n00, n01, n11, det;
	float invdet, c0, c1;
	
	PWM_cross3_ref(&cross, &p1->normal, &p2->normal);
	sqrlength = PWM_dot3(cross, cross);
	
	if(sqrlength < 1e-08){
		return 0;
	}
	
	if(r){
		n00 = PWM_dot3(p1->normal, p1->normal);
		n01 = PWM_dot3(p1->normal, p2->normal);
		n11 = PWM_dot3(p2->normal, p2->normal);
		det = n00 * n11 - n01 * n01;
		
		if(fabs(det) < 1e-08){
			return 0;
		}
		
		invdet = 1.0 / det;
		c0 = (n11 * p1->distance - n01 * p2->distance) * invdet;
		c1 = (n00 * p2->distance - n01 * p1->distance) * invdet;
		
		r->dir = cross;
		r->origin = PWM_add3(PWM_mul3(p1->normal, c0), PWM_mul3(p2->normal, c1));
	}
	return 1;
}

int PWM_plane_x_aabb(PWPlane *p, PWAabb *aabb){
	PWVec3 min, max;
	
	//x coordinate
	if(p->normal.x >= 0.0){
		min.x = aabb->min.x;
		max.x = aabb->max.x;
	}
	else{
		min.x = aabb->max.x;
		max.x = aabb->min.x;
	}
	
	//y-coordinate
	if(p->normal.y >= 0.0){
		min.y = aabb->min.y;
		max.y = aabb->max.y;
	}
	else{
		min.y = aabb->max.y;
		max.y = aabb->min.y;
	}
	
	//z-coordinate
	if(p->normal.y >= 0.0){
		min.z = aabb->min.z;
		max.z = aabb->max.z;
	}
	else{
		min.z = aabb->max.z;
		max.z = aabb->min.z;
	}
	
	if((PWM_dot3(p->normal, min) + p->distance) > 0.0){
		return 0;
	}
	if((PWM_dot3(p->normal, max) + p->distance) >= 0.0){
		return 1;
	}
	return 0;
}

void PWM_print(PWMat4 m){
	int i, j;
	
	for(i = 0; i < 4; ++i){
		for(j = 0; j < 4; ++j){
			#if defined PWMATRIX_COLUMN_MAJOR
			printf("%8.4f ", m.elements[j*4+i]);
			#else
			printf("%8.4f ", m.elements[i*4+j]);
			#endif
		}
		printf("\n");
	}
	printf("\n");
}

void PWM_printa(PWMat4 m, PWMat4 a){
	int i, j;
	
	for(i = 0; i < 4; ++i){
		for(j = 0; j < 4; ++j){
			#if defined PWMATRIX_COLUMN_MAJOR
			printf("%8.2f ", m.elements[j*4+i]);
			#else
			printf("%8.2f ", m.elements[i*4+j]);
			#endif
		}
		printf("          ");
		for(j = 0; j < 4; ++j){
			#if defined PWMATRIX_COLUMN_MAJOR
			printf("%8.2f ", a.elements[j*4+i]);
			#else
			printf("%8.2f ", a.elements[i*4+j]);
			#endif
		}
		printf("\n");
	}
	printf("\n");
}
