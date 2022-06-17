#version 430

#include <rendering.glsl>

in vec4 vertColor;
out vec4 fragColor;

void main()
{
	//fragColor = vec4(vertColor.xyzw) + material_index * 0.001;
	fragColor = vec4(1);
}