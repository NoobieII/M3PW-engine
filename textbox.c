#include "textbox.h"
#include <stdlib.h>
#include <string.h>

void textbox_init(TextBox *t, int size){
	t->size = size;
	t->pos = 0;
	t->len = 0;
	t->str = (char*) malloc(size);
	t->str[0] = '\0';
	t->str_censor = (char*) malloc(size);
	t->str_censor[0] = '\0';
}

void textbox_close(TextBox *t){
	free(t->str);
}

void textbox_bks(TextBox *t){
	if(t->pos == 0){
		return;
	}
	
	t->str[t->pos - 1] = 0;
	t->str_censor[t->pos - 1] = 0;
	
	//if there is nothing in front of the cursor to move back one space
	if(t->pos-- == t->len--){
		return;
	}
	
	//copy what is in front of the cursor back one space
	memcpy(t->str + t->pos, t->str + t->pos + 1, t->len - t->pos);
	
	//remove the end of the string
	t->str[t->len] = 0;
}

void textbox_del(TextBox *t){
	if(t->pos == t->len){
		return;
	}
	
	t->str[t->pos] = 0;
	t->str_censor[t->len - 1] = 0;
	t->len--;
	
	//if the most front character has been deleted
	if(t->pos == t->len){
		return;
	}
	//copy the front of the string back one space
	memcpy(t->str + t->pos, t->str + t->pos + 1, t->len - t->pos);
	
	//remove the end of the string
	t->str[t->len] = 0;
}
	
void textbox_add_char(TextBox *t, char ch){
	//do not add a char if length of string is maximum
	if(t->len == t->size - 1){
		return;
	}
	
	t->str_censor[t->len] = '*';
	
	//copy memory to one spot forward if position is not length
	if(t->pos < t->len){
		memcpy(t->str + t->pos + 1, t->str + t->pos, t->len - t->pos);
	}
	t->str[t->pos] = ch;
	t->pos++;
	t->len++;
	t->str[t->len] = '\0';
	t->str_censor[t->len] = '\0';
}

void textbox_move_right(TextBox *t){
	if(t->pos < t->len){
		t->pos++;
	}
}

void textbox_move_left(TextBox *t){
	if(t->pos > 0){
		t->pos--;
	}
}

void textbox_replace(TextBox *t, const char *str){
	strncpy(t->str, str, t->size - 1);
	t->pos = strlen(t->str);
	t->len = t->pos;
	memset(t->str_censor, '*', t->len);
	t->str_censor[t->len] = '\0';
}

void textbox_clear(TextBox *t){
	t->str[0] = '\0';
	t->str_censor[t->len] = '\0';
	t->len = 0;
	t->pos = 0;
}

int textbox_get_pos(TextBox *t){
	return t->pos;
}

const char* textbox_get_str(TextBox *t){
	return t->str;
}

const char* textbox_get_str_censor(TextBox *t){
	return t->str_censor;
}

