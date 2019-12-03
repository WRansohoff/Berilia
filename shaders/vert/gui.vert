#version 420

in vec2 xy;
out vec2 st;

void main() {
	st = (xy + 1.0) * 0.5;
	gl_Position = vec4(xy.x, xy.y, 0.0, 1.0);
}
