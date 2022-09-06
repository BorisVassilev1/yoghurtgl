#version 430

#include <rendering.glsl>

in vec4 teColor;
in vec2 teTexCoord;
in vec3 teVertexNormal;
in vec3 teVertexPos;
in mat3 teTBN;

out vec4 fragColor;
uniform mat4 worldMatrix;

void main() {
	vec3 normal = texture(normalMap, teTexCoord).xyz;
	normal = normalize(normal * 2. - 1.);

	vec3 finalNormal = normalize(teTBN * normal);

	vec3 light = calcAllLights(teVertexPos, finalNormal, teTexCoord);

	// vec3 light = vec3(texture(normalMap, teTexCoord));
	// vec3 light = finalNormal;

	fragColor = vec4(light, 1.0);
}