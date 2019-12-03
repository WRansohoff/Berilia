#version 420
#define INT_MAX 2147483647

restrict uniform layout(r32i, binding = 0) iimage2D surface_normal_image;
uniform vec2 px_scale;

in vec2 st;

void main() {
	int x_ind = 5 * int(st.s / px_scale.x);
	int y_ind = int(st.y / px_scale.y);
	//imageStore(surface_normal_image, ivec2(x_ind, y_ind), ivec4(0,0,0,INT_MAX));
	//imageStore(surface_normal_image, ivec2(x_ind, y_ind), ivec4(0,1,0,1));
	imageStore(surface_normal_image, ivec2(x_ind, y_ind), ivec4(0, 0, 0, 0));
	imageStore(surface_normal_image, ivec2(x_ind + 1, y_ind), ivec4(0, 0, 0, 0));
	imageStore(surface_normal_image, ivec2(x_ind + 2, y_ind), ivec4(0, 0, 0, 0));
	imageStore(surface_normal_image, ivec2(x_ind + 3, y_ind), ivec4(INT_MAX, 0, 0, 0));
	imageStore(surface_normal_image, ivec2(x_ind + 4, y_ind), ivec4(0, 0, 0, 0));
	discard;
}
