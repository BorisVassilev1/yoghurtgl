#version 430

#include <rendering.glsl>

in vec4 teColor;
in vec2 teTexCoord;
in vec3 teVertexNormal;
in vec3 teVertexPos;

out vec4 fragColor;

void main() {
	// vec3 light = calcAllLights(teVertexPos, teVertexNormal, teTexCoord);

	vec3 light = vec3(texture(texture_sampler, teTexCoord));

	fragColor = vec4(light, 1.0);
}