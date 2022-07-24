#ifndef CURSOR_H
#define CURSOR_H

/*cleaner way to implement cursors instead of using plain integers
 *
 *(December 12, 2018)
 *
 *March 16, 2019 note: max_val in constructor refers to the number of
 *possible cursor positions
 *
 *June 21, 2022: Adapted to be used in C
 */

typedef struct Cursor{
	int pos;
	int len;
	int is_wrap;
} Cursor;

//initialize cursor with number of possible positions. Always starts at 0
//closing the cursor is not needed.
void cursor_init(Cursor *c, int max_val, int wrap);

//decrement cursor
void cursor_dec(Cursor *c);

//increment
void cursor_inc(Cursor *c);

int cursor_pos(Cursor *c);
void cursor_resize(Cursor *c, int max_val);
void cursor_reset(Cursor *c);

#endif
