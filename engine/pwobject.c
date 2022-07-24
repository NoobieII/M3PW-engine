//Lithio (The developer's pseudonym)
//June 20, 2022

#include <stdlib.h>
#include <string.h>
#include "pwengine.h"
#include "pwobject.h"

void pwobject_base_init(PWObject *o, PWEngine *e, const char *name, int is_active){
	o->update = NULL;
	o->close = NULL;
	
	if(!e){
		return;
	}
	
	o->is_active = is_active;
	o->is_destroy = 0;
	o->engine = e;
	o->data = NULL;
	if(!name){
		o->name = NULL;
		return;
	}
	o->name = (char*) malloc(strlen(name) + 1);
	strcpy(o->name, name);
	
	pwengine_add_object(e, o);
}

int  pwobject_is_active(PWObject *o){
	return o->is_active;
}

void pwobject_set_active(PWObject *o, int active){
	o->is_active = active;
}

int  pwobject_is_destroy(PWObject *o){
	return o->is_destroy;
}

void pwobject_destroy(PWObject *o){
	o->is_destroy = 1;
}

void pwobject_update(PWObject *o){
	if(!o->update){
		printf("pwobject_update: object %s has no update()\n", o->name);
		return;
	}
	o->update(o);
}

void pwobject_close(PWObject *o){
	printf("a");fflush(stdout);
	if(o->name){
		free(o->name);
	}
	printf("a");fflush(stdout);
	if(!o->close){
		printf("pwobject_close: object %s has no close()\n", o->name);
		return;
	}
	printf("a");fflush(stdout);
	o->close(o->data);
	printf("b");fflush(stdout);
	free(o->data);
	printf("c");fflush(stdout);
}
