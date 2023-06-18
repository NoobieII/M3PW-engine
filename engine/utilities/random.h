#ifndef RAND_H
#define RAND_H

/*random number functions using Marsenne Twister 19937*/

void init_rng(); //my way of adding 
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void); /* generates a random number on [0,0xffffffff]-interval */
long genrand_int31(void);   /* generates a random number on [0,0x7fffffff]-interval */
double genrand_real1(void); /* generates a random number on [0,1]-real-interval */
double genrand_real2(void); /* generates a random number on [0,1)-real-interval */
double genrand_real3(void); /* generates a random number on (0,1)-real-interval */
double genrand_res53(void); /* generates a random number on [0,1) with 53-bit resolution*/

#endif
