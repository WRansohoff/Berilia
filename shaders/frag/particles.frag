#version 420 core

in float opacity;
uniform sampler2D texture_sampler;
out vec4 frag_color;

vec4 particle_color = vec4(0.2, 1.0, 0.4, 1.0);

void main() {
	vec4 texel = texture(texture_sampler, gl_PointCoord);
	frag_color.a = opacity * texel.a;
	if (frag_color.a <= 0.1) {
		discard;
	}
	frag_color.rgb = particle_color.rgb * texel.rgb;
}
