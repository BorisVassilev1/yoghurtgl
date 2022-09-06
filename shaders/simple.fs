#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;

void main() {
	vec3 light = calcAllLights(vVertexPos, vVertexNormal, vTexCoord);

	// light = vec3(materials[material_index].albedo);
	// light = vVertexNormal;

	fragColor = vec4(light, 1.0);
}