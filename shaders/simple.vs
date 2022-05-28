#version 430

#include <rendering.glsl>

out vec4	 vertColor;
uniform mat4 worldMatrix;

void main() {
	vertColor = color;

	vec4 mvPos	= worldMatrix * vec4(position.xyz, 1.0);
	gl_Position = projectionMatrix * viewMatrix * mvPos;

	// gl_Position = worldMatrix * vec4(position.xyz, 1.0);
}