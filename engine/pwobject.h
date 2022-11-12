//Lithio (The developer's pseudonym)
//June 20, 2022

//PWObject for the M3PW engine

//collider type
//0 no collision checks
//1 collides with all types but 0
//2 does not collide with its own type

#ifndef M3PW_PWOBJECT_H
#define M3PW_PWOBJECT_H

#include "pwcollider.h"
#include "pwmath.h"

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
	PWMat4 *transform;           //local transformation
	int collider_type;           //see above
	PWCollider collider;        //optional collider type
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

//check collision between two objects or with a collision object
int pwobject_x_object(PWObject *o1, PWObject *o2);
int pwobject_x_collider(PWObject *o, PWCollider *c);

#endif
