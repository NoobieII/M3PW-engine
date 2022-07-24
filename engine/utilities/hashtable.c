//Lithio
//April 30, 2022
//June 13, 2022, remove functions implemented because they weren't yet!

//Hash table implementation.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "list.h"

typedef struct HashTableBin{
	union{
		int intt[2];
		char *str;
	};
	void *data;
} HashTableBin;

HashTable *hashtable_init(){
	HashTable *table;
	unsigned int i;
	
	table = (HashTable*) malloc(sizeof(HashTable));
	if(!table){
		return NULL;
	}
	
	//seed using address	
	i = *(unsigned int*)(&table);
	srand(i);
	
	//a_int < 2^w
	//b_int < 2^(w-M)
	//a_str[i] < 2^(2w)
	table->a_int = ((rand() & 0xFF) << 8) ^ (rand() & 0xFF);
	table->b_int = rand() & (HASHTABLE_SIZE - 1);
	
	for(i = 0; i < 16; ++i){
		table->a_str[i] = ((rand() & 0xFF) << 8) ^ (rand() & 0xFF);
		table->a_str[i] <<= 16;
		table->a_str[i] ^= ((rand() & 0xFF) << 8) ^ (rand() & 0xFF);
	}
	table->bin_index = 0;
	table->bin = NULL;
	for(i = 0; i < HASHTABLE_SIZE; ++i){
		table->bins[i] = NULL;
	}
	return table;
}

HashTable *hashtable_clear(HashTable *table){
	int i;
	List *list;
	HashTableBin *bin;
	
	for(i = 0; i < HASHTABLE_SIZE; ++i){
		list = table->bins[i];
		while(list){
			//free memory for the string key, if used - intt[1] is unused
			bin = (HashTableBin*) list->data;
			if(bin->intt[1]){
				free(bin->str);
			}
			free(list->data);
			list = list->next;
		}
	}
	free(table);
	return NULL;
}

//returns NULL if the bin with the key doesn't exist
void *hashtable_at_int(HashTable *table, int key){
	List *list;
	HashTableBin *bin;
	
	list = table->bins[hash_int(table, key)];
	
	while(list){
		bin = (HashTableBin*) list->data;
		if(bin->intt[0] == key){
			break;
		}
		list = list->next;
	}
	if(!list){
		return NULL;
	}
	return bin->data;
}

void *hashtable_at_str(HashTable *table, const char *str){
	List *list;
	HashTableBin *bin;
	
	list = table->bins[hash_str(table, str)];
	
	while(list){
		bin = (HashTableBin*) list->data;
		if(strcmp(bin->str, str) == 0){
			break;
		}
		list = list->next;
	}
	if(!list){
		return NULL;
	}
	return bin->data;
}

void *hashtable_begin(HashTable *table){
	void *data = NULL;
	
	if(!table){
		return data;
	}
	table->bin_index = 0;
	table->bin = NULL;
	
	while(table->bin_index < HASHTABLE_SIZE){
		if(table->bins[table->bin_index]){
			table->bin = table->bins[table->bin_index];
			data = ((HashTableBin*)table->bin->data)->data;
			break;
		}
		++(table->bin_index);
	}
	return data;
}
	
	
void *hashtable_next(HashTable *table){
	void *data = NULL;
	if(!table || table->bin_index >= HASHTABLE_SIZE){
		return data;
	}
	
	if(!table->bin){
		return NULL;
	}
	table->bin = table->bin->next;
	if(table->bin){
		data = ((HashTableBin*)table->bin->data)->data;
	}
	else{
		++(table->bin_index);
		
		while(table->bin_index < HASHTABLE_SIZE){
			if(table->bins[table->bin_index]){
				table->bin = table->bins[table->bin_index];
				data = ((HashTableBin*)table->bin->data)->data;
				break;
			}
			++(table->bin_index);
		}
	}
	return data;
}

int hashtable_key_i(HashTable *table){
	return ((HashTableBin*)table->bin->data)->intt[0];
}

const char *hashtable_key_s(HashTable *table){
	return ((HashTableBin*)table->bin->data)->str;
}

//returns a pointer to the hashtable
HashTable *hashtable_set_at_int(HashTable *table, int key, void *data){
	size_t hash_val;
	List *list;
	HashTableBin *bin;
	if(!table){
		return table;
	}
	
	hash_val = hash_int(table, key);
	list = table->bins[hash_val];
	
	while(list){
		bin = (HashTableBin*) list->data;
		if(bin->intt[0] == key && bin->intt[1] == 0){
			bin->data = data;
			break;
		}
		list = list->next;
	}
	//add new element to hashtable
	if(!list){
		bin = (HashTableBin*) malloc(sizeof(HashTableBin));
		bin->intt[0] = key;
		bin->intt[1] = 0;
		bin->data = data;
		table->bins[hash_val] = list_push_back(table->bins[hash_val], bin);
	}
	
	return table;
}

HashTable *hashtable_set_at_str(HashTable *table, const char *str, void *data){
	size_t hash_val;
	List *list;
	HashTableBin *bin;
	if(!table){
		return table;
	}
	
	hash_val = hash_str(table, str);
	list = table->bins[hash_val];
	
	while(list){
		bin = (HashTableBin*) list->data;
		if(bin->intt[1] && strcmp(bin->str, str) == 0){
			bin->data = data;
			break;
		}
		list = list->next;
	}
	//add new element to hashtable
	if(!list){
		bin = (HashTableBin*) malloc(sizeof(HashTableBin));
		bin->str = (char*) malloc(strlen(str) + 1);
		strcpy(bin->str, str);
		bin->data = data;
		table->bins[hash_val] = list_push_back(table->bins[hash_val], bin);
	}
	
	return table;
}

HashTable *hashtable_remove_at_int(HashTable *table, int key){
	size_t hash_val;
	List *list;
	HashTableBin *bin;
	if(!table){
		return table;
	}
	
	hash_val = hash_int(table, key);
	list = table->bins[hash_val];
	
	while(list){
		bin = (HashTableBin*) list->data;
		//delete this element of the list
		if(bin->intt[0] == key && bin->intt[1] == 0){
			table->bins[hash_val] = list_remove(table->bins[hash_val], list);
			break;
		}
		list = list->next;
	}
	return table;
}

HashTable *hashtable_remove_at_str(HashTable *table, const char *str){
	size_t hash_val;
	List *list;
	HashTableBin *bin;
	if(!table){
		return table;
	}
	
	hash_val = hash_str(table, str);
	list = table->bins[hash_val];
	
	while(list){
		bin = (HashTableBin*) list->data;
		if(bin->intt[1] && strcmp(bin->str, str) == 0){
			table->bins[hash_val] = list_remove(table->bins[hash_val], list);
			break;
		}
		list = list->next;
	}
	return table;
}

int hashtable_size(HashTable *table){
	int i;
	int count = 0;
	
	for(i = 0; i < HASHTABLE_SIZE; ++i){
		count += list_size(table->bins[i]);
	}
	return count;
}

//if print function not given, pointer address is printed
void hashtable_print(HashTable *table, void (*print_function)(void*)){
	int i;
	int n = 0;
	List *list;
	
	for(i = 0; i < HASHTABLE_SIZE; ++i){
		list = table->bins[i];
		
		if(list){
			list_print(list, print_function);
			n++;
		}
	}
	if(!n){
		printf("empty");
	}
}

size_t hash_int(HashTable *table, int key){
	if(!table){
		return 0;
	}
	//hash = (a*key + b) mod 2^w div 2^(w-M)
	return ((table->a_int * key + table->b_int) & 0xFFFF) >> (16 - HASHTABLE_M);
}

size_t hash_str(HashTable *table, const char *str){
	size_t i;
	size_t ii;
	size_t hash = 0;
	size_t len;
	char c1, c2;
	
	if(!table || !str){
		return 0;
	}
	
	len = strlen(str);
	
	for(i = 0; i < (len + 15) >> 4 << 4; i+=2){
		c1 = 0;
		c2 = 0;
		if(i < len) c1 = str[i];
		if(i+1 < len) c2 = str[i+1];
		
		ii = i & 0x0f;
		hash += (c1 + table->a_str[ii])*(c2 + table->a_str[ii+1]);
	}
	hash = (hash & 0xFFFFFFFF) >> (2 * 16 - HASHTABLE_M);
	
	return hash;
}
