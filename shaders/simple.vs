#version 430

#include <rendering.glsl>

out vec4 vertColor;

void main() {
	vertColor = color;
	gl_Position =  vec4(position.xyz, 1.0);
}