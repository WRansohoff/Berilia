#version 420

// Just for parity with the normal vertex shader.
in vec3 pos_E, norm_E, pos_W, norm_W;
in vec2 tex_coords;
out vec4 frag_color;

void main() {
	frag_color = vec4(0.0, 0.0, 0.0, 1.0);
}
