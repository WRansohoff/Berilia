#version 420 core

layout(location = 0) in vec3 iv;
layout(location = 1) in float it;

layout (std140) uniform cam_ubo {
	mat4 T;
	mat4 V;
	mat4 P;
};

uniform vec3 ipos_W;
uniform float cur_time;

out float opacity;

void main() {
	float t = cur_time - it;
	t = mod(t, 3.5);

	vec3 pos_W = ipos_W;
	// gravity (x + dx*dt + dv*dt^2)
	vec3 g = vec3(0.0, -1.0, 0.0);
	pos_W += iv * t + 0.5 * g * t * t;
	opacity = 1.0 - (t / 3.5);

	gl_Position = P * V * vec4(pos_W, 1.0);
	gl_PointSize = 15.0 / gl_Position.z;
}
