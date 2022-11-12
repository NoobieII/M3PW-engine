//NoobieII
//22 Oct 22

//implementation of the collision detection type

#include "pwcollider.h"

enum COLLIDER_TYPE {RAY, PLANE, AABB, OBB, MESH};

inline void pwcollider_init_ray(PWCollider *c, PWRay r){
	c->type = RAY;
	c->ray = r;
}

inline void pwcollider_init_plane(PWCollider *c, PWPlane p){
	c->type = PLANE;
	c->plane = p;
}

inline void pwcollider_init_aabb(PWCollider *c, PWAabb aabb){
	c->type = AABB;
	c->aabb = aabb;
}

inline void pwcollider_init_obb(PWCollider *c, PWObb obb){
	c->type = OBB;
	c->obb = obb;
}

inline void pwcollider_init_mesh(PWCollider *c, int size, PWVec3 *mesh){
	c->type = MESH;
	c->size = size;
	c->mesh = mesh;
}

//check if two collider objects collide
int pwcollider_check(PWCollider *c1, PWCollider *c2){
	switch(c1->type){
	case RAY:
		switch(c2->type){
		case RAY:
			return 0;
			break;
		case PLANE:
			break;
		case AABB:
			break;
		case OBB:
			break;
		case MESH:
			break;
		}
		break;
	case PLANE:
		switch(c2->type){
		case RAY:
			break;
		case PLANE:
			break;
		case AABB:
			break;
		case OBB:
			break;
		case MESH:
			break;
		}
		break;
	case AABB:
		switch(c2->type){
		case RAY:
			break;
		case PLANE:
			break;
		case AABB:
			break;
		case OBB:
			break;
		case MESH:
			break;
		}
		break;
	case OBB:
		switch(c2->type){
		case RAY:
			break;
		case PLANE:
			break;
		case AABB:
			break;
		case OBB:
			break;
		case MESH:
			break;
		}
		break;
	case MESH:
		switch(c2->type){
		case RAY:
			break;
		case PLANE:
			break;
		case AABB:
			break;
		case OBB:
			break;
		case MESH:
			break;
		}
		break;
	}
	return 0;
}
