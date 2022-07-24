//Lithio (The developer's pseudonym)
//June 20, 2022

//PWObject for the M3PW engine

#ifndef M3PW_PWOBJECT_H
#define M3PW_PWOBJECT_H

typedef struct PWEngine PWEngine;

typedef struct PWObject {
	void (*update)(PWObject*);   //function to be called every tick
	void (*close)(void*);        //cleanup for whatever data contains
	int id;
	int is_active;
	int is_destroy;
	char *name;
	PWEngine *engine;
	void *data;
} PWObject;

//all pwobject types will call this function
void pwobject_base_init(PWObject *o, PWEngine *e, const char *name, int is_active);

//use 0 or nonzero values, inactive objects won't have
//their update function called during subsequent engine ticks.
int  pwobject_is_active(PWObject *o);
void pwobject_set_active(PWObject *o, int is_active);

//call this function to set it to be killed in the next game tick
int  pwobject_is_destroy(PWObject *o);
void pwobject_destroy(PWObject *o);

//These functions will always be called by the engine,
//so don't call them in your own code!
void pwobject_update(PWObject *o);
void pwobject_close(PWObject *o);

#endif
