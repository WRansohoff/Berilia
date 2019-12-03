#version 420

layout (std140) uniform cam_ubo {
	mat4 T;
	mat4 V;
	mat4 P;
};

layout(std140) uniform world_ubo {
	vec4 opts;
	vec4 color;
};

uniform sampler2D texture_sampler;

in vec3 pos_E, norm_E, pos_W;
in vec2 tex_coords;
out vec4 frag_color;

void main() {
	int tex_type = int(opts[0]);
	vec4 texel = texture(texture_sampler, tex_coords);
	frag_color = texel;
}
