#version 330

in vec2 texture_coordinates;
uniform sampler2DArray basic_texture;
uniform vec4 add_colour;
out vec4 frag_colour;

void main() {
	//vec4 texel = texture(basic_texture, vec3(texture_coordinates.x, texture_coordinates.y, 0));
	vec4 texel = texture (basic_texture, vec3(texture_coordinates, 0.));

	frag_colour = texel * add_colour;
	//frag_colour = vec4(1.f, 1.f, 1.f, 1.f);
}
