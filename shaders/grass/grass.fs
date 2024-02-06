#version 430

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;


void main() {
	Material mat = materials[material_index];
	vec3 albedo = mix(mat.transparency_color, mat.albedo, vColor.x * vColor.x);

	vec3 normalizedVertexNormal = normalize(vVertexNormal);

	// if(!gl_FrontFacing) normalizedVertexNormal = -normalizedVertexNormal;
	
	vec3 color = calcAllLightsCustom(
		vVertexPos, 
		normalizedVertexNormal, 
		albedo, 
		mat.specular_roughness,
		mat.metallic,
		clamp(vColor.x * vColor.x + 0.2, 0, 1),
		vec3(0.0));

	// color = vec3(materials[material_index].albedo);
	// color = vVertexNormal;

	fragColor = vec4(color, 1.0);
}