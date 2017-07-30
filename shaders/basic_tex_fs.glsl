#version 420
#extension GL_ARB_explicit_uniform_location : require

in vec2 uv;
uniform sampler2DArray basic_texture;
layout (location = 1) uniform int layer;
uniform vec4 add_colour;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, vec3(uv, layer));
	frag_colour = texel * add_colour;
}
