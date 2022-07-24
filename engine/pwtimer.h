//Lithio (The developer's pseudonym)
//May 30, 2022

//basic timer for measuring things

#ifndef M3PW_PWTIMER_H
#define M3PW_PWTIMER_H

#include <time.h>

#if (defined(unix) || defined(__unix) || defined(__unix__))
#include "unistd.h"
#elif defined(_WIN32)
#include "windows.h"
#endif

typedef struct PWTimer{
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	struct timespec last_time;
	#elif defined(_WIN32)
	LARGE_INTEGER start;
	double frequency;
	#endif
} PWTimer;

//reset timer
void pwtimer_reset(PWTimer *t);

//elapsed time in microseconds since reset
int pwtimer_elapsed_i(PWTimer *t);

//elapsed time in seconds since reset
float pwtimer_elapsed_f(PWTimer *t);

#endif

