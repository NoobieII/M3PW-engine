//NoobieII
//22 Oct 22

//Collision detection type
//0 ray
//1 plane
//2 aabb
//3 oriented bounding box
//4 mesh

#ifndef PWCOLLIDER_H
#define PWCOLLIDER_H

#include "pwmath.h"

typedef struct PWCollider{
	int type;
	int size;
	union{
		PWRay ray;
		PWPlane plane;
		PWAabb aabb;
		PWObb obb;
		PWVec3 *mesh;      //triangles only, no vertex indexing
	};
}PWCollider;

void pwcollider_init_ray(PWCollider *c, PWRay r);
void pwcollider_init_plane(PWCollider *c, PWPlane p);
void pwcollider_init_aabb(PWCollider *c, PWAabb aabb);
void pwcollider_init_obb(PWCollider *c, PWObb obb);
void pwcollider_init_mesh(PWCollider *c, int size, PWVec3 *mesh);

//check if two collider objects collide
int pwcollider_check(PWCollider *c1, PWCollider *c2);

#endif
