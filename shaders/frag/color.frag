#version 420

in vec3 pos_E, norm_E, pos_W;
in vec2 tex_coords;
out vec4 frag_color;

layout (std140) uniform world_ubo {
	vec4 opts;
	vec4 color;
};

void main() {
	frag_color = color;
}
