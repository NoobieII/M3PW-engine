#version 120

attribute vec4 position;
attribute vec2 uv;
attribute float tid;
attribute vec4 colour;
attribute vec4 normal;

varying vec4 position_from_vert;
varying vec2 uv_from_vert;
varying float tid_from_vert;
varying vec4 color_from_vert;

varying float intensity_from_vert;

uniform mat4 pr_matrix;
uniform mat4 vw_matrix;
uniform mat4 ml_matrix;

uniform vec3  light_direction;
uniform float ambient_intensity;

void main()
{
	vec3 normie = normalize(normal.xyz);
	
	position_from_vert = ml_matrix * position;
	uv_from_vert = uv;
	tid_from_vert = tid;
	color_from_vert = colour;
	
	intensity_from_vert = max(min(dot(normie, light_direction) + ambient_intensity, 1), ambient_intensity);
	//intensity_from_vert = 1;
	gl_Position = pr_matrix * vw_matrix * ml_matrix * position;
}
