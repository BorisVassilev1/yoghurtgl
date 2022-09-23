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
	Material mat = materials[material_index];
	
	vec3 finalNormal;
	if (mat.use_normal_map != 0.0) {
		vec3 normal = texture(normalMap, teTexCoord).xyz;

		normal = normalize(normal * 2. - 1.);

		finalNormal = normalize(teTBN * normal);
	} else {
		finalNormal = teVertexNormal;
	}

	vec3 albedo = mat.albedo;
	if (mat.texture_influence != 0.0) {
		albedo = mat.texture_influence * texture(albedoMap, texCoord).xyz * mix(vec3(1.), texture(aoMap, texCoord).xyz, mat.use_ao_map) +
				 (1 - mat.texture_influence) * mat.albedo;
	}

	vec3 light = calcAllLights(teVertexPos, finalNormal, teVertexNormal, teTexCoord, albedo);

	// vec3 light = vec3(texture(normalMap, teTexCoord));
	// vec3 light = finalNormal;

	light = pow(light, vec3(1.0/2.2));

	fragColor = vec4(light, 1.0);
}