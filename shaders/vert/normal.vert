#version 420

layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec2 vt;

layout (std140) uniform cam_ubo {
	mat4 T;
	mat4 V;
	mat4 P;
};

layout (std140) uniform shadow_cam_ubo {
	mat4 s_T;
	mat4 s_V;
	mat4 s_P;
};

// world ubo values.
layout (std140) uniform world_ubo {
	vec4 opts;
	vec4 color;
};

out vec3 pos_E, norm_E, pos_W, norm_W;
out vec2 tex_coords;
out vec4 st_shadow;

void main() {
	pos_W = vec3(T * vec4(vp.x, vp.y, vp.z, 1.0));
	pos_E = vec3(V * vec4(pos_W, 1.0));
	norm_W = vec3(T * vec4(vn.x, vn.y, vn.z, 0.0));
	norm_E = vec3(V * vec4(norm_W, 0.0));
	tex_coords = vt;
	gl_Position = (P * vec4(pos_E, 1.0));
	//gl_Position = vec4(normalize(pos_W), 1.0);

	// Shadow depth coords.
	st_shadow = s_P * s_V * T * vec4(vp, 1.0);
	st_shadow.xyz /= st_shadow.w;
	st_shadow.xyz += 1.0;
	st_shadow.xyz *= 0.5;
}
