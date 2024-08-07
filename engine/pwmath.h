//Lithio
//January 28, 2022
//February 19, 2022
//May 20, 2022 added functionality for either float/double
//             and more functions!
//May 31, 2022 added PWM_mul_ref, PWM_mul_vec3, PWM_mul_vec3_ref functions
//Jul 08, 2022 added cross product functions, dot product functions, addition, subtraction, ray struct, plane struct, ray/line/triangle collision, ray/plane collision
//Jul 14, 2022 added more intersection functions...
//             TODO: test functions, rewrite PWM_aabb
//             DONE  PWM_aabb
//             DONE  PWM_plane3
//             DONE  PWM_plane_x_triangle
//             DONE  PWM_plane_x_plane
//             DONE  PWM_plane_x_aabb
//                   
//12 Nov 2022 
//Finished writing the collision functions from the the book. Functionality with
//doubles will probably be removed soon, as well as row major format support.

#ifndef PWMATRIX_H
#define PWMATRIX_H

//NOTE Jul 08, 2022
//This math library has been expanded a lot in recent months and a lot of
//functions are designed only for used with column major (float) matrices.

//If column major matrices are preferred, uncomment the below line!
#define PWMATRIX_COLUMN_MAJOR

//if doubles are preferred, uncomment the below line!
//#define PWMATRIX_USE_DOUBLES

#include <xmmintrin.h>

#ifdef PWMATRIX_USE_DOUBLES
#define PWMATRIX_TYPE double
#else
#define PWMATRIX_TYPE float
#endif

#ifdef PWMATRIX_USE_DOUBLES
#define PI 3.1415926535897932384626433
#else
#define PI 3.1415926535897932384626433f
#endif

//enable omission of intersection with triangles that are clockwise
//#define PWM_CULLING

#define PWM_FRONT  0
#define PWM_BACK   1
#define PWM_PLANAR 2
#define PWM_CLIPPED 3
#define PWM_CULLED  4
#define PWM_VISIBLE 5

//Matrices and vectors will be row major by default
typedef struct __attribute__((aligned(16))) PWMat4{
	PWMATRIX_TYPE elements[16];
} PWMat4;

typedef struct PWVec2{
	PWMATRIX_TYPE x, y;
} PWVec2;

typedef struct PWVec3{
	PWMATRIX_TYPE x, y, z;
} PWVec3;

typedef struct PWVec4{
	PWMATRIX_TYPE x, y, z, w;
} PWVec4;

typedef struct PWQuat{
	float x, y, z, w;
} PWQuat;

//collision structs

typedef struct PWRay{
	PWVec3 origin;            //m_vcOrig
	PWVec3 dir;               //m_vcDir
} PWRay;

typedef struct PWPlane{
	PWVec3 normal;  //
	PWVec3 p;       //point on plane
	float distance; 
} PWPlane;

//axis aligned bounding box
typedef struct PWAabb{
	PWVec3 min;               
	PWVec3 max;
} PWAabb;

//oriented bounding box
typedef struct PWObb{
	PWVec3 center;
	PWVec3 a[3];
	float fa[3];
} PWObb;

//polygon class
typedef struct PWPolygon{
	PWPlane plane;            //m_Plane
	int vertex_count;         //m_NumP
	int index_count;          //m_NumI
	PWAabb aabb;              //m_Aabb
	unsigned int flag;        //m_Flag
	PWVec3 *vertices;         //m_pPoints
	unsigned short *indices;  //m_pIndis
} PWPolygon;

//function comments in doxygen's format because I feel fancy today...
/**
 * @brief Initializes a matrix using either diagonal value or a string
 *
 * @details Pass a null value for str if using a diagonal value. All 4 diagonal
 *          elements of the matrix will be set to the diagonal value.
 *          
 *          If str is not null, diagonal will be ignored. str can initialize
 *          matrices a matrix of any dimension by initializing rows. The
 *          following example initializes a 2x2 identity matrix.
 *          PWM_ini(0, "1 0; 0 1");
 *
 * @param diagonal Diagonal value
 * @param str      Initialization string
 *
 */
PWMat4 PWM_ini(PWMATRIX_TYPE diagonal, const char *str);

PWMat4 PWM_orthographic(
	PWMATRIX_TYPE left,
	PWMATRIX_TYPE right,
	PWMATRIX_TYPE bottom,
	PWMATRIX_TYPE top,
	PWMATRIX_TYPE near,
	PWMATRIX_TYPE far
);

PWMat4 PWM_perspective(
	PWMATRIX_TYPE fov,
	PWMATRIX_TYPE aspect_ratio,
	PWMATRIX_TYPE near,
	PWMATRIX_TYPE far
);

PWMat4 PWM_translation(PWVec3 translation);
PWMat4 PWM_rotation(PWMATRIX_TYPE angle, PWVec3 axis);
PWMat4 PWM_scale(PWVec3 scale);

void PWM_translation_ref(PWMat4 *result, PWVec3 translation);
void PWM_rotation_ref(PWMat4 *result, PWMATRIX_TYPE angle, PWVec3 axis);
void PWM_scale_ref(PWMat4 *result, PWVec3 scale);

PWMat4 PWM_add(PWMat4 lhs, PWMat4 rhs);
PWMat4 PWM_sub(PWMat4 lhs, PWMat4 rhs);
PWMat4 PWM_mul(PWMat4 lhs, PWMat4 rhs);
void PWM_mul_ref(PWMat4 *result, PWMat4 *lhs, PWMat4 *rhs);
PWVec3 PWM_mul_vec3(PWMat4 lhs, PWVec3 rhs);
void PWM_mul_vec3_ref(PWVec3 *result, PWMat4 *lhs, PWVec3 *rhs);
PWVec3 PWM_mul_vec3_notranslate(PWMat4 lhs, PWVec3 rhs);
void PWM_mul_vec3_notranslate_ref(PWVec3 *result, PWMat4 *lhs, PWVec3 *rhs);
PWVec4 PWM_mul_vec4(PWMat4 lhs, PWVec4 rhs);
void PWM_mul_vec4_ref(PWVec3 *result, PWMat4 *lhs, PWVec4 *rhs);

//inverse of a matrix, returns 0 if worked, -1 if not
PWMat4 PWM_inv(PWMat4 m);
int PWM_inv_ref(PWMat4 *result, PWMat4 *m);

PWMat4 PWM_transpose(PWMat4 m);

void PWM_transpose_ref(PWMat4 *m);

PWVec2 PWM_vec2(PWMATRIX_TYPE x, PWMATRIX_TYPE y);
PWVec3 PWM_vec3(PWMATRIX_TYPE x, PWMATRIX_TYPE y, PWMATRIX_TYPE z);
PWVec4 PWM_vec4(PWMATRIX_TYPE x, PWMATRIX_TYPE y, PWMATRIX_TYPE z, PWMATRIX_TYPE w);

//add/subtract 
//using ref is faster for PWVec3 and higher
//safe if result == v1 == v2
PWVec2 PWM_add2(PWVec2 v1, PWVec2 v2);
void PWM_add2_ref(PWVec2 *result, PWVec2 *v1, PWVec2 *v2);
PWVec3 PWM_add3(PWVec3 v1, PWVec3 v2);
void PWM_add3_ref(PWVec3 *result, PWVec3 *v1, PWVec3 *v2);
PWVec4 PWM_add4(PWVec4 v1, PWVec4 v2);
void PWM_add4_ref(PWVec4 *result, PWVec4 *v1, PWVec4 *v2);


PWVec2 PWM_sub2(PWVec2 v1, PWVec2 v2);
void PWM_sub2_ref(PWVec2 *result, PWVec2 *v1, PWVec2 *v2);
PWVec3 PWM_sub3(PWVec3 v1, PWVec3 v2);
void PWM_sub3_ref(PWVec3 *result, PWVec3 *v1, PWVec3 *v2);
PWVec4 PWM_sub4(PWVec4 v1, PWVec4 v2);
void PWM_sub4_ref(PWVec4 *result, PWVec4 *v1, PWVec4 *v2);

//multiplication
//better performance using ref
//safe if result == v
PWVec2 PWM_mul2(PWVec2 v, PWMATRIX_TYPE f);
void PWM_mul2_ref(PWVec2 *result, PWVec2 *v, PWMATRIX_TYPE f);
PWVec3 PWM_mul3(PWVec3 v, PWMATRIX_TYPE f);
void PWM_mul3_ref(PWVec3 *result, PWVec3 *v, PWMATRIX_TYPE f);
PWVec4 PWM_mul4(PWVec4 v, PWMATRIX_TYPE f);
void PWM_mul4_ref(PWVec4 *result, PWVec4 *v, PWMATRIX_TYPE f);

//dot products
//dot product has same performance for copy and ref
PWMATRIX_TYPE PWM_dot2(PWVec2 v1, PWVec2 v2);
PWMATRIX_TYPE PWM_dot2_ref(PWVec2 *v1, PWVec2 *v2);
PWMATRIX_TYPE PWM_dot3(PWVec3 v1, PWVec3 v2);
PWMATRIX_TYPE PWM_dot3_ref(PWVec3 *v1, PWVec3 *v2);
PWMATRIX_TYPE PWM_dot4(PWVec4 v1, PWVec4 v2);
PWMATRIX_TYPE PWM_dot4_ref(PWVec4 *v1, PWVec4 *v2);

//cross products
//ref has about 30% faster performance
//result must not be equal to lhs nor rhs
PWVec3 PWM_cross3(PWVec3 lhs, PWVec3 rhs);
void PWM_cross3_ref(PWVec3 *result, PWVec3 *lhs, PWVec3 *rhs);

//make vector unit length using SSE rsqrt instruction
//ref has about 10% faster performance
//safe if v == result
PWVec3 PWM_normalize3(PWVec3 v);
void PWM_normalize3_ref(PWVec3 *result, PWVec3 *v);

//get vector length
//ref has about 20% slower performance (PWVec3 only tested)
float PWM_norm2(PWVec2 v);
float PWM_norm3(PWVec3 v);
float PWM_norm3_ref(PWVec3 *v);

//angle between 2 vectors in radians
float PWM_angle3(PWVec3 v1, PWVec3 v2);

//quaternions
PWQuat PWM_quat(float x, float y, float z, float w);
PWQuat PWM_quat_mul(PWQuat p, PWQuat q);
PWQuat PWM_quat_from_euler(float x, float y, float z);
PWVec3 PWM_quat_to_euler(PWQuat q);
PWMat4 PWM_quat_mat4(PWQuat q);

//direction must be normalized
PWRay PWM_ray(PWVec3 origin, PWVec3 direction);

//normal is normalized and point is closest point on plane to origin
PWPlane PWM_plane(PWVec3 normal, PWVec3 point);

//plane defined by three vectors of a triangle
PWPlane PWM_plane3(PWVec3 v0, PWVec3 v1, PWVec3 v2);

//axis aligned bounding box defined by two opposite corners
PWAabb PWM_aabb(PWVec3 v1, PWVec3 v2);

//detransform a ray
void PWM_ray_detransform(PWRay *r, PWMat4 *transform);

//distance from a vector point to plane
//classify a point with respect to the plane... returns PWM_FRONT, PWM_BACK, PWM_PLANAR
//classify a polygon with respect to the plane... returns PWM_CLIPPED, PWM_CULLED, PWM_VISIBLE
//return a projection of a vector on a plane.
float PWM_plane_distance(PWPlane *p, PWVec3 v);
int PWM_plane_classify(PWPlane *p, PWVec3 v);
int PWM_plane_classify_polygon(PWPlane *p, PWPolygon *polygon);
PWVec3 PWM_plane_proj(PWPlane *p, PWVec3 v);

//intersection functions (returns nonzero if intersects)

//ray with a plane. optionally store distance in t
//axis aligned bounded boxes... hit must not be NULL
//ray with 3 points of a triangle. optionally store distance in t
int PWM_ray_x_plane(PWRay *r, PWPlane *p, float *t, PWVec3 *hit);
int PWM_ray_x_aabb(PWRay *r, PWAabb *aabb, PWVec3 *hit);
int PWM_ray_x_obb(PWRay *r, PWObb *obb, float *t);
int PWM_ray_x_triangle(PWRay *r, PWVec3 v0, PWVec3 v1, PWVec3 v2, float *t);
int PWM_ray_x_polygon(PWRay *r, PWPolygon *polygon, float *t);

int PWM_line_x_plane(PWVec3 l0, PWVec3 l1, PWPlane *p, float *t, PWVec3 *hit);
int PWM_line_x_aabb(PWVec3 l0, PWVec3 l1, PWAabb *aabb, PWVec3 *hit);
int PWM_line_x_obb(PWVec3 l0, PWVec3 l1, PWObb *obb);
int PWM_line_x_triangle(PWVec3 l0, PWVec3 l1, PWVec3 v0, PWVec3 v1, PWVec3 v2, float *t);
int PWM_line_x_polygon(PWVec3 l0, PWVec3 l1, PWPolygon *polygon, float *t);

//plane intersection with a triangle
//plane intersection with plane, ray is optional if line intersection is needed
//intersection with plane and aabb

int PWM_plane_x_ray(PWPlane *p, PWRay *r, float *t, PWVec3 *hit);
int PWM_plane_x_line(PWPlane *p, PWVec3 l0, PWVec3 l1, float *t, PWVec3 *hit);
int PWM_plane_x_plane(PWPlane *p1, PWPlane *p2, PWRay *r);
int PWM_plane_x_aabb(PWPlane *p, PWAabb *aabb);
int PWM_plane_x_obb(PWPlane *p, PWObb *obb);
int PWM_plane_x_triangle(PWPlane *p, PWVec3 v0, PWVec3 v1, PWVec3 v2);

int PWM_aabb_x_ray(PWAabb *aabb, PWRay *r, PWVec3 *hit);
int PWM_aabb_x_line(PWAabb *aabb, PWVec3 l0, PWVec3 l1, PWVec3 *hit);
int PWM_aabb_x_plane(PWAabb *aabb, PWPlane *p);
int PWM_aabb_x_vec3(PWAabb *aabb, PWVec3 v);
int PWM_aabb_x_aabb(PWAabb *aabb1, PWAabb *aabb2);
//int PWM_aabb_x_obb(PWAabb *aabb, PWObb *obb);
//int PWM_aabb_x_triangle

int PWM_obb_x_ray(PWObb *obb, PWRay *r, float *t);
int PWM_obb_x_line(PWObb *obb, PWVec3 l0, PWVec3 l1);
int PWM_obb_x_plane(PWObb *obb, PWPlane *p); //todo, just inline
int PWM_obb_x_triangle(PWObb *obb, PWVec3 v0, PWVec3 v1, PWVec3 v2);

int PWM_polygon_x_ray(PWPolygon *polygon, PWRay *ray, float *t);
int PWM_polygon_x_line(PWPolygon *polygon, PWVec3 l0, PWVec3 l1, float *t);

//caution: the normal vectors of the viewing volume planes are pointing outward.
//the space enclosed by those planes can then be thought of as the insides of 
//the viewing volume.
//returns: PWM_CLIPPED, PWM_CULLED, PWM_VISIBLE
int PWM_aabb_cull(PWAabb *aabb, PWPlane *planes, int size);
int PWM_obb_cull(PWObb *obb, PWPlane *planes, int size);

//get an aabb from an obb
PWAabb PWM_obb_aabb(PWObb *obb);

//return obb1 as to obb2 transformed to the coordinate space of m
void PWM_obb_detransform(PWObb *obb1, PWObb *obb2, PWMat4 *m);

//helper functions
void PWM_obb_proj(PWObb *obb1, PWObb *obb2, PWVec3 v, float *min, float *max);
void PWM_obb_triproj(PWObb *obb, PWVec3 v0, PWVec3 v1, PWVec3 v2, PWVec3 v, float *min, float *max);

//get (6) planes from an aabb
void PWM_aabb_get_planes(PWAabb *aabb, PWPlane *planes);

//if a ray of length l is contained inside the aabb
int PWM_aabb_contains(PWAabb *aabb, PWRay *ray, float l);

PWPolygon PWM_polygon(PWVec3 *vertices, int vertex_count, unsigned short *indices, int index_count);
void PWM_polygon_calc_bounding_box(PWPolygon *polygon);
void PWM_polygon_clip_polygon(PWPolygon *polygon, PWPlane *plane, PWPolygon *front, PWPolygon *back);
void PWM_polygon_clip_aabb(PWPolygon *polygon, PWAabb *aabb);
int PWM_polygon_cull(PWPolygon *polygon, PWAabb *aabb);
void PWM_polygon_copy_of(PWPolygon *dest, PWPolygon *src);
void PWM_polygon_swap_faces(PWPolygon *polygon);

void PWM_polygon_close(PWPolygon *polygon);

//print contents of one or two matrices
void PWM_print(PWMat4 m);
void PWM_printa(PWMat4 m, PWMat4 a);

#endif
