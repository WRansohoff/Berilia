#version 420

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vc;

//uniform mat4 view, proj;
layout (std140) uniform cam_ubo {
	mat4 T;
	mat4 V;
	mat4 P;
};

// world ubo values.
layout (std140) uniform world_ubo {
	vec4 opts;
	vec4 color;
};

out vec3 pos_E, col, pos_W;

void main() {
	pos_W = vec3(T * vec4(vp.x, vp.y, vp.z, 1.0));
	pos_E = vec3(V * vec4(pos_W, 1.0));
	col = vc;
	gl_Position = P * vec4(pos_E, 1.0);
}
