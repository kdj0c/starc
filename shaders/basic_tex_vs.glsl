#version 420

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 vt; // per-vertex texture co-ords
layout (location = 2) in vec4 color;

uniform vec2 position_offset;
uniform vec2 position_scaling;
out vec2 uv;
out vec4 col;

void main() {
	uv = vt;
	col = color;
	gl_Position = vec4 ((vertex_position + position_offset) * position_scaling, 0.0, 1.0);
}
