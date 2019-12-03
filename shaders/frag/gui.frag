#version 420

uniform sampler2D texture_sampler;

in vec2 st;
out vec4 frag_color;

void main() {
	vec4 texel = texture(texture_sampler, st);
	frag_color = texel;
}
