//Lithio
//April 30, 2022
//Modified on June 13, 2022 for retrieving the key using the iterator

//Hash table in C, using linked lists for bins.

#ifndef M3PW_HASH_TABLE_H
#define M3PW_HASH_TABLE_H

#include <stdlib.h>

//let the number of bins in the hash table be 2^M
//let the machine word be 16 bits

//w = 16

#define HASHTABLE_M 8
#define HASHTABLE_SIZE (1 << HASHTABLE_M)

typedef struct List List;

typedef struct HashTable{
	size_t a_int;
	size_t b_int;
	size_t a_str[16];
	//used for begin/next function
	int bin_index;
	List *bin;
	List *bins[1 << HASHTABLE_M];
} HashTable;

//returns a new instance, with optional function to handle data when removed
//if no function is given, memory is not managed.
HashTable *hashtable_init();

//returns a null pointer
HashTable *hashtable_clear(HashTable *table);

//returns NULL if the bin with the key doesn't exist
void *hashtable_at_int(HashTable *table, int key);
void *hashtable_at_str(HashTable *table, const char *str);

//returns beginning/next element, NULL if empty or at end
void *hashtable_begin(HashTable *table);
void *hashtable_next(HashTable *table);

int         hashtable_key_i(HashTable *table);
const char *hashtable_key_s(HashTable *table);


//returns a pointer to the hashtable
HashTable *hashtable_set_at_int(HashTable *table, int key, void *data);
HashTable *hashtable_set_at_str(HashTable *table, const char *str, void *data);
HashTable *hashtable_remove_at_int(HashTable *table, int key);
HashTable *hashtable_remove_at_str(HashTable *table, const char *str);

int hashtable_size(HashTable *table);

//if print function not given, pointer address is printed
void hashtable_print(HashTable *table, void (*print_function)(void*));

size_t hash_int(HashTable *table, int key);
size_t hash_str(HashTable *table, const char *str);

#endif
