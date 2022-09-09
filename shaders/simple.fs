#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;
in mat3 vTBN;

out vec4 fragColor;

void main() {
	vec3 finalNormal;
	if (materials[material_index].use_normal_map != 0.0) {
		vec3 normal = texture(normalMap, vTexCoord).xyz;

		normal = normalize(normal * 2. - 1.);

		finalNormal = normalize(vTBN * normal);
	} else {
		finalNormal = vVertexNormal;
	}
	vec3 light = calcAllLights(vVertexPos, finalNormal, vTexCoord);

	// light = vec3(materials[material_index].albedo);
	// light = vVertexNormal;

	fragColor = vec4(light, 1.0);
}