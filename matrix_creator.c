//NoobieII
//12 Dec 2022

//WINDOWS
//cd C:/Users/Ryan/github/M3PW-engine
//gcc matrix_creator.c engine/pwmath.c -m32 -msse -lmingw32 -o matrix_creator -static

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine/pwmath.h"

char *convert(char *s, double x)
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

void print_mat4(PWMat4 mat4){
	int i;
	char str[20];
	
	for(i = 0; i < 16; ++i){
		printf("%s ", convert(str, (double)(mat4.elements[i])));
	}
	printf("\n");
}

int main(void){
	PWMat4 mat4;
	char input[200];
	mat4 = PWM_ini(1.0, NULL);
	
	char str[200];
	float f;
	PWVec3 v;
	
	printf("Welcome to the matrix creator. Use the following commands:\n");
	printf("rotate <angle> <axis>\n");
	printf("scale <scale>\n");
	printf("translate <translation>\n");
	printf("reset\n");
	printf("quit\n");
	printf("Current transform below:\n");
	
	while(1){
		print_mat4(mat4);
		fflush(stdout);
		
		fgets(input, 200, stdin);
		sscanf(input, "%s", str);
		
		if(strcmp(str, "rotate") == 0){
			if(sscanf(input, "%*s%f%f%f%f", &f, &v.x, &v.y, &v.z) == 4){
				mat4 = PWM_mul(PWM_rotation(f, v), mat4);
			}
		}
		if(strcmp(str, "scale") == 0){
			if(sscanf(input, "%*s%f%f%f", &v.x, &v.y, &v.z) == 3){
				mat4 = PWM_mul(PWM_scale(v), mat4);
			}
		}
		if(strcmp(str, "translate") == 0){
			if(sscanf(input, "%*s%f%f%f", &v.x, &v.y, &v.z) == 3){
				mat4 = PWM_mul(PWM_translation(v), mat4);
			}
		}
		if(strcmp(str, "reset") == 0){
			mat4 = PWM_ini(1.0, NULL);
		}
		if(strcmp(str, "quit") == 0){
			break;
		}
	}
		
	return 0;
}