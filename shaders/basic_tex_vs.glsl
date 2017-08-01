#version 420

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vt; // per-vertex texture co-ords
uniform vec3 position_offset;
uniform vec3 position_scaling;
out vec2 uv;

void main() {
	uv = vt;
	gl_Position = vec4 ((vertex_position + position_offset) * position_scaling, 1.0);
}