#version 330

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
uniform vec4 add_colour;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, texture_coordinates);
	frag_colour = texel * add_colour;
}
