//Lithio (The developer's pseudonym)
//June 21, 2022

//Textbox class made sometime in 2018
//Adapted to C

#ifndef TEXTBOX_H
#define TEXTBOX_H

typedef struct TextBox {
	int size;
	int pos;
	int len;
	char *str;
	char *str_censor;
} TextBox;

void textbox_init(TextBox *t, int size);
void textbox_close(TextBox *t);

void textbox_bks(TextBox *t);
void textbox_del(TextBox *t);
void textbox_add_char(TextBox *t, char ch);
void textbox_move_right(TextBox *t);
void textbox_move_left(TextBox *t);
void textbox_replace(TextBox *t, const char *str);
void textbox_clear(TextBox *t);
int  textbox_get_pos(TextBox *t);
const char* textbox_get_str(TextBox *t);
const char* textbox_get_str_censor(TextBox *t);

#endif
