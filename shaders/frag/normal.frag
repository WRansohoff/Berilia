#version 420

#define MAX_PHONG_LIGHTS 100
#define INT_MAX_M1 2147483646

struct phong {
	vec4 light_pos_W;
	vec4 light_amb;
	vec4 light_dif;
	vec4 light_spec;
	vec4 light_vals; // 0 = specular exponent, 1 = falloff, 2 = type.
	vec4 light_vals2; // if type is directional, (0,1,2) = direction, 3 = angle.
};

layout (std140) uniform phong_ubo {
	vec4 light_opts;
	phong lights[MAX_PHONG_LIGHTS];
};

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
uniform sampler2D shadow_depth_map_sampler;
coherent restrict uniform layout(r32i, binding = 0) iimage2D surface_normal_image;
uniform vec2 px_scale;

// specular power.
float specular_exponent = 1000.0;
float falloff = 2000.0f;
// Surface properties.
vec3 Ka = vec3(1.0, 1.0, 1.0);
vec3 Kd = vec3(1.0, 1.0, 1.0);
vec3 Ks = vec3(1.0, 1.0, 1.0);

in vec3 pos_E, norm_E, pos_W, norm_W;
in vec2 tex_coords;
in vec4 st_shadow;
out vec4 frag_color;

void main() {
	int tex_type = int(opts[0]);
	// Texture sampling.
	vec4 final_tex;

	vec4 texel = texture(texture_sampler, tex_coords);

	// Store the surface normal in our image.
	// This, uh...may cause GPU hangs. I haven't worked out the kinks yet, and driver support for
	// this functionality, especially the atomic operations, seems spotty at best.
	/*
	int x_ind = 5 * int(gl_FragCoord.x - 0.5);
	int y_ind = int(gl_FragCoord.y - 0.5);
	float near = opts[0];
	float far = opts[1];
	float norm_coord = (gl_FragCoord.z - 0.5) * 2.0;
	norm_coord = (2.0 * near) / (far + near - norm_coord * (far - near));
	int sn_d = int(gl_FragCoord.z * INT_MAX_M1);
	int sn_x = int(abs(norm_E.x) * INT_MAX_M1);
	int sn_y = int(abs(norm_E.y) * INT_MAX_M1);
	int sn_z = int(abs(norm_E.z) * INT_MAX_M1);
	// Make a little spinlock/semaphore
	bool sem_avail = false;
	do {
		bool can_lock = (imageAtomicExchange(surface_normal_image, ivec2(x_ind + 4, y_ind), 1) != 1);
		if (can_lock) {
			int cur_depth = imageLoad(surface_normal_image, ivec2(x_ind + 3, y_ind)).r;
			if (sn_d < cur_depth) {
				imageAtomicExchange(surface_normal_image, ivec2(x_ind, y_ind), sn_x);
				imageAtomicExchange(surface_normal_image, ivec2(x_ind + 1, y_ind), sn_y);
				imageAtomicExchange(surface_normal_image, ivec2(x_ind + 2, y_ind), sn_z);
				imageAtomicExchange(surface_normal_image, ivec2(x_ind + 3, y_ind), sn_d);
			}
			sem_avail = true;
			imageAtomicExchange(surface_normal_image, ivec2(x_ind + 4, y_ind), 0);
		}
	} while (!sem_avail);
	memoryBarrier();
	*/

	// Lighting calculations.
	vec3 lighting_color = vec3(0,0,0);
	int num_lights = int(light_opts[0]);
	for (int i = 0; i < num_lights; i++) {
		float spot_factor = 1.0f;
		vec3 light_pos_E = vec3(V * lights[i].light_pos_W);
		vec3 to_surface = normalize(-pos_E);
		vec3 dist_to_light_E = light_pos_E - pos_E;
		vec3 dist_to_light_W = lights[i].light_pos_W.xyz - pos_W;
		vec3 dir_to_light_E = normalize(dist_to_light_E);
		vec3 dir_to_light_W = normalize(dist_to_light_W);
		vec3 spot_dir_W = normalize(lights[i].light_vals2.rgb);
		vec3 spot_dir_E = vec3(V * vec4(spot_dir_W, 1.0));

		// If it's not a point light, check if the light hits.
		if (lights[i].light_vals[2] == 1) {
			float ang = lights[i].light_vals2[3];
			float spotlight_edge = cos(ang);
			float spot_dot = dot(spot_dir_W, dir_to_light_W);
			if (spot_dot < spotlight_edge) {
				spot_factor = 0.0f;
			}
			else {
				spot_factor = (spot_dot - spotlight_edge) / (1.0 - spotlight_edge);
				spot_factor = clamp(spot_factor, 0.0, 1.0);
			}
		}

		if (spot_factor > 0.0f) {
			float specular_exponent = lights[i].light_vals.x;
			float falloff = lights[i].light_vals.y;

			vec3 Ia = lights[i].light_amb.rgb * Ka;

			float diffuse_dot = dot(dir_to_light_E, norm_E);
			//diffuse_dot = max(diffuse_dot, 0.0);
			diffuse_dot = abs(diffuse_dot);
			Kd = texel.rgb;
			vec3 Id = lights[i].light_dif.rgb * Kd * diffuse_dot;

			// Use Blinn-Phong for specular calculations.
			vec3 half_way = normalize(to_surface + dir_to_light_E);
			float specular_dot = dot(half_way, norm_E);
			specular_dot = max(specular_dot, 0.0);
			float specular_factor = pow(specular_dot, specular_exponent);
			vec3 Is = lights[i].light_spec.rgb * Ks * specular_factor;

			vec3 light_color = (Ia + Id + Is) * spot_factor;
			// Linear light falloff.
			float light_str = falloff - length(light_pos_E - pos_E);
			if (light_str <= 0.0) {
				light_str = 0.0;
			}
			else {
				light_str = light_str / falloff;
			}
			if (lights[i].light_vals[3] > 0) {
				float epsilon = 0.0;
				float sh_val = 1.0;
				float shadow = texture(shadow_depth_map_sampler, st_shadow.xy).r;
				if (shadow + epsilon < st_shadow.z) {
					sh_val = 0.2;
				}
				//light_color *= sh_val;
				//light_color *= shadow;
				if (st_shadow.z > 0) {
					light_color = vec3(shadow, shadow, shadow);
				}
				//light_str *= shadow;
			}
			light_color = light_color * light_str;
			lighting_color = lighting_color + light_color;
		}
	}

	//frag_color = (texel + vec4(normalize(lighting_color), 0))/2;
	//frag_color = vec4(lighting_color/num_lights, 1.0);
	//frag_color = vec4(0.5, 1.0, 0.5, 1.0);
	//frag_color = V[2];
	frag_color = texel * vec4((lighting_color), 1.0);
	//frag_color = texture(shadow_depth_map_sampler, st_shadow.xy);
	//frag_color = texture(shadow_depth_map_sampler, tex_coords);
	//frag_color = frag_color * vec4(sh_val, sh_val, sh_val, 1.0);
	//frag_color = frag_color * vec4(st_shadow.z, st_shadow.z, st_shadow.z, 1.0);
	//frag_color = vec4(0.2, 0.2, 0.2, 1.0);
	//frag_color = frag_color * vec4(gs, gs, gs, 1.0);
	//frag_color = st_shadow;
}
