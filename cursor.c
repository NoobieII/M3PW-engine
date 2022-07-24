//Lithio (The developer's pseudonym)
//June 21, 2022

#include "cursor.h"

//initialize cursor with number of possible positions
inline void cursor_init(Cursor *c, int max_val, int wrap){
	c->pos = 0;
	c->len = max_val;
	c->is_wrap = wrap;
}

//decrement 
inline void cursor_dec(Cursor *c){
	if(c->is_wrap){
		c->pos = (c->pos - 1 + c->len) % c->len;
	}
	else if(c->pos > 0){
		--c->pos;
	}
}

//increment
inline void cursor_inc(Cursor *c){
	if(c->is_wrap){
		c->pos = (c->pos + 1) % c->len;
	}
	else if(c->pos < c->len - 1){
		c->pos = c->pos + 1;
	}
}

inline int cursor_pos(Cursor *c){
	return c->pos;
}

//resize the number of selections possible,
//if the position overflows the new max, it is adjusted
inline void cursor_resize(Cursor *c, int max_val){
	if(max_val < 1){
		return;
	}
	
	c->len = max_val;
	if(c->pos >= max_val){
		c->pos = max_val - 1;
	}
}

inline void cursor_reset(Cursor *c){
	c->pos = 0;
}
