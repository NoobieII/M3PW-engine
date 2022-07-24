//April 28, 2022
//Lithio
//basic C linked list structure

#ifndef M3PW_LIST_H
#define M3PW_LIST_H

typedef struct List{
	void *data;
	struct List *next;
	struct List *prev;
} List;


int list_size(List *list);

//returns the start of the list
List *list_init();
List *list_push_back(List *list, void *data);
List *list_push_front(List *list, void *data);

//NULL is returned if no value exists
void *list_pop_front(List *list);
void *list_peek_back(List *list);
void *list_peek_front(List *list);
void *list_at(List *list, int n);

//returns the start of the list
List *list_insert_at(List *list, int n, void *data);
List *list_remove_at(List *list, int n);
List *list_remove(List *start, List *node);
List *list_clear(List *list);

//provide a function pointer - if null, will print the hex value of data
void list_print(List *list, void (*print_func)(void*));



#endif
