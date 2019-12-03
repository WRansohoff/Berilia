#version 420

uniform sampler2D texture_sampler;
uniform sampler2D depth_tex_sampler;
coherent restrict uniform layout(r32i, binding = 0) iimage2D surface_normal_image;

uniform vec2 px_scale;

// world ubo values.
layout (std140) uniform world_ubo {
	vec4 opts;
	vec4 color;
};

in vec2 st;
out vec4 frag_color;

void main() {
	vec2 offsets[] = vec2[](
		vec2(-px_scale.s, px_scale.t),
		vec2(0.0, px_scale.t),
		vec2(px_scale.s, px_scale.t),
		vec2(-px_scale.s, 0.0),
		vec2(0.0, 0.0),
		vec2(px_scale.s, 0.0),
		vec2(-px_scale.s, -px_scale.t),
		vec2(0.0, -px_scale.t),
		vec2(px_scale.s, -px_scale.t)
	);

	float depths[] = float[](
		texture(depth_tex_sampler, st + offsets[0]).x,
		texture(depth_tex_sampler, st + offsets[1]).x,
		texture(depth_tex_sampler, st + offsets[2]).x,
		texture(depth_tex_sampler, st + offsets[3]).x,
		texture(depth_tex_sampler, st + offsets[4]).x,
		texture(depth_tex_sampler, st + offsets[5]).x,
		texture(depth_tex_sampler, st + offsets[6]).x,
		texture(depth_tex_sampler, st + offsets[7]).x,
		texture(depth_tex_sampler, st + offsets[8]).x
	);
	float near = opts[0];
	float far = opts[1];
	for (int i = 0; i < 9; ++i) {
		depths[i] = (depths[i] - 0.5) * 2.0;
		depths[i] = (2.0 * near) / (far + near - depths[i] * (far - near));
		//depths[i] = pow(depths[i], 1024.0);
	}

	float depth = gl_FragCoord.z;
	depth = pow(depth, 1024.0);

	//vec4 texel = texture(texture_sampler, st);
	//vec4 texel = texture(depth_tex_sampler, st);
	float depth_val = (8 * depths[4] - depths[0] - depths[1] - depths[2] - depths[3] - depths[5] - depths[6] - depths[7] - depths[8]);
	vec4 texel = vec4(0.0, 0.0, 0.0, 0.0);
	if (depth_val >= 0.5) {
		texel = vec4(1.0, 1.0, 1.0, 1.0);
	}
	else {
		texel = texture(texture_sampler, st);
	}
	int x_ind = 5 * int(st.s / px_scale.x);
	int y_ind = int(st.y / px_scale.y);
	ivec4 sni_r = imageLoad(surface_normal_image, ivec2(x_ind, y_ind));
	ivec4 sni_g = imageLoad(surface_normal_image, ivec2(x_ind + 1, y_ind));
	ivec4 sni_b = imageLoad(surface_normal_image, ivec2(x_ind + 2, y_ind));
	const float img_sn_scale = 2147483646.0;
	float sn_r = float(sni_r.r) / img_sn_scale;
	float sn_g = float(sni_g.r) / img_sn_scale;
	float sn_b = float(sni_b.r) / img_sn_scale;
	//ivec4 sni_a = imageLoad(surface_normal_image, ivec2(x_ind + 3, y_ind));
	//texel = vec4(sni_r.r, sni_g.r, sni_b.r, sni_a.r);
	texel = vec4(sn_r, sn_g, sn_b, 1);
	//texel = vec4(depth_val, depth_val, depth_val, 1.0);
	//texel = vec4(depth, depth, depth, 1.0);
	//texel = vec4(depths[4], depths[4], depths[4], 1.0);
	frag_color = texel;
}
