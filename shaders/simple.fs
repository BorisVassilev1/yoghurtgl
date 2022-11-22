#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;
in mat3 vTBN;

out vec4 fragColor;

void main() {
	vec3 normalizedVertexNormal = normalize(vVertexNormal);
	vec3	 finalNormal;
	vec2	 vTexCoord = vTexCoord * 3.;
	Material mat	   = materials[material_index];

	if (mat.use_normal_map != 0.0) {
		vec3 normal = texture(normalMap, vTexCoord).xyz;

		normal = normalize(normal * 2. - 1.);

		finalNormal = normalize(vTBN * normal);
	} else {
		finalNormal = normalizedVertexNormal;
	}

	vec3 albedo = mat.albedo;
	if (mat.texture_influence != 0.0) {
		albedo = mix(mat.albedo, texture(albedoMap, vTexCoord).xyz, mat.texture_influence) *
				 mix(vec3(1.), vec3(texture(aoMap, vTexCoord).x), mat.use_ao_map);
	}

	if(!gl_FrontFacing) finalNormal = -finalNormal;
	vec3 color = calcAllLights(vVertexPos, finalNormal, finalNormal, vTexCoord, albedo);

	// light = vec3(materials[material_index].albedo);
	// color = vVertexNormal;
	// color = vVertexPos;

	// color = color / (color + vec3(1.0));
	// color = pow(color, vec3(1.0 / 2.2));

	fragColor = vec4(color, 1.0);
}