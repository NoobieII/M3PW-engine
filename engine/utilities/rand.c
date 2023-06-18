#include "rand.h"

#include <stdlib.h>
#include <time.h>

void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
long genrand_int31(void);
double genrand_real1(void); /* generates a random number on [0,1]-real-interval */
double genrand_real2(void); /* generates a random number on [0,1)-real-interval */
double genrand_real3(void); /* generates a random number on (0,1)-real-interval */
double genrand_res53(void); /* generates a random number on [0,1) with 53-bit resolution*/

void init_rng(){
	unsigned long init[2];
	int *ptr;
	unsigned long t;
	
	ptr = malloc(sizeof(int));
	t = time(0);
	
	init[0] = t;
	init[1] = (unsigned long)(ptr);
	
	free(ptr);
	
	init_by_array(init, 2);
}
