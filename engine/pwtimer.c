
#include "pwtimer.h"



void pwtimer_reset(PWTimer *t){
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	clock_gettime(CLOCK_MONOTONIC, &t->last_time);
	#elif defined(_WIN32)
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	t->frequency = 1.0 / frequency.QuadPart;
	QueryPerformanceCounter(&t->start);
	#endif
}

int pwtimer_elapsed_i(PWTimer *t){	
	int elapsed;
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	elapsed = (tp.tv_sec - t->last_time.tv_sec) * 1000000 + (tp.tv_nsec - t->last_time.tv_nsec) / 1000;
	#elif defined(_WIN32)
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	unsigned __int64 cycles = current.QuadPart - t->start.QuadPart;
	elapsed = cycles * t->frequency;
	#endif
	return elapsed;
}

float pwtimer_elapsed_f(PWTimer *t){
	float elapsed;
	#if (defined(unix) || defined(__unix) || defined(__unix__))
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	elapsed = (tp.tv_sec - t->last_time.tv_sec) + (tp.tv_nsec - t->last_time.tv_nsec) / 1000000000.0f;
	#elif defined(_WIN32)
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	unsigned __int64 cycles = current.QuadPart - t->start.QuadPart;
	elapsed = cycles * t->frequency;
	#endif
	return elapsed;
}

