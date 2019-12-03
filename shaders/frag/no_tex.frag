#version 420

//uniform mat4 view, proj;
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

float falloff = 500.0f;

in vec3 pos_E, norm_E, pos_W, col;
in vec2 tex_coords;
out vec4 frag_color;

void main() {
	//float r = opts[1];
	//float g = opts[2];
	//float b = opts[3];
	//float r = norm_E.r;
	//float g = norm_E.g;
	//float b = norm_E.b;
	float r = col.r;
	float g = col.g;
	float b = col.b;
	frag_color = vec4(r, g, b, 1.0);
}
