//April 28, 2022
//Lithio (The deveoloper's pseudonym)

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int list_size(List *list){
	int i = 0;
	while(list){
		list = list->next;
		++i;
	}
	
	return i;
}

List *list_init(){
	return NULL;
}

List *list_push_back(List *list, void *data){
	List *new_node = malloc(sizeof(List));
	if(!new_node){
		return list;
	}
	new_node->data = data;
	new_node->next = list;
	new_node->prev = NULL;
	if(list){
		list->prev = new_node;
	}
	return new_node;
}

List *list_push_front(List *list, void *data){
	List *start_node = list;
	List *prev = NULL;
	List *new_node = malloc(sizeof(List));
	if(!new_node){
		return list;
	}
	new_node->data = data;
	new_node->next = NULL;
	//must traverse n times to reach the end of the list
	while(list){
		prev = list;
		list = list->next;
	}
	new_node->prev = prev;
	if(!prev){
		return new_node;
	}
	prev->next = new_node;
	return start_node;
}

void *list_pop_front(List *list){
	void *data = NULL;
	
	while(list){
		if(list->next == NULL) break;
		list = list->next;
	}
	if(list){
		data = list->data;
		
		if(list->prev){
			list->prev->next = NULL;
		}
		
		free(list);
	}
	return data;
}

void *list_peek_back(List *list){
	if(list){
		return list->data;
	}
	return NULL;
}

void *list_peek_front(List *list){
	if(!list){
		return NULL;
	}
	while(list->next){
		list = list->next;
	}
	return list->data;
}

void *list_at(List *list, int n){
	int i = 0;
	while(i < n && list){
		list = list->next;
		++i;
	}
	if(i == n && list){
		return list->data;
	}
	return NULL;
}

List *list_insert_at(List *list, int n, void *data){
	List *new_node;
	List *start = list;
	List *prev = NULL;
	int i;
	
	if(!new_node){
		return list;
	}
	
	i = 0;
	while(i < n && list){
		prev = list;
		list = list->next;
		++i;
	}
	if(i != n){
		return start;
	}
	
	//list points to the nth element
	new_node = malloc(sizeof(List));
	new_node->data = data;
	new_node->next = list;
	new_node->prev = prev;
	
	//if we're not inserting at the very end
	if(list){
		list->prev = new_node;
	}
	//if we're not inserting at the very start
	if(prev){
		prev->next = new_node;
		return start;
	}
		
	return new_node;
}

List *list_remove_at(List *list, int n){
	List *start = list;
	int i = 0;
	while(i < n && list){
		list = list->next;
		++i;
	}
	
	if(!list || i != n){
		return start;
	}
	//if we're not removing the last element
	if(list->next){
		list->next->prev = list->prev;
	}
	
	//if we're not removing the first element
	if(list->prev){
		list->prev->next = list->next;
	}
	else{
		start = list->next;
	}
	
	free(list);
	return start;
}

List *list_remove(List *start, List *node){
	List *next;
	if(!node){
		return start;
	}
	next = node->next;
	if(next){
		next->prev = node->prev;
	}
	if(node->prev){
		node->prev->next = next;
	}
	free(node);
	if(node == start){
		return next;
	}
	return start;
}

List *list_clear(List *list){
	List *next;
	while(list){
		next = list->next;
		free(list);
		list = next;
	}
	return list;
}

void list_print(List *list, void (*print_func)(void*)){
	if(!list) printf("empty");
	
	while(list){
		if(print_func) print_func(list->data);
		else printf("%p", list->data);
		printf(" ");
		
		list = list->next;
	}
}
