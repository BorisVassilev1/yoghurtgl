#version 430

#include <rendering.glsl>

in vec4 outColor;
in vec2 outTexCoord;
in vec3 mvVertexNormal;
in vec3 mvVertexPos;

out vec4 fragColor;

uniform bool	  use_texture;
uniform sampler2D texture_sampler;

vec3 calcLight(Light light, in vec3 position, in vec3 normal, in vec2 texCoord, in float attenuationExp, Material mat) {
	vec3 lightPosition = (light.transform * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	vec3 lightForward  = (light.transform * vec4(0.0, 0.0, 1.0, 0.0)).xyz;

	vec3 albedo = mat.albedo;
	if (use_texture) {
		albedo =
			mat.texture_influence * texture(texture_sampler, texCoord).xyz + (1 - mat.texture_influence) * mat.albedo;
	}

	if (light.type == 0) return light.color * albedo * light.intensity;

	vec3 toLight = lightPosition - position;
	if (light.type == 1) toLight = -lightForward;

	float dist = length(toLight);

	float diffuse = 0, specular = 0;
	vec3  diffuseColor, specularColor;

	{
		diffuse = max(dot(normalize(toLight), normal), 0.0);

		diffuseColor = diffuse * albedo * light.color * light.intensity;
	}

	{
		vec3 cameraPos = cameraWorldMatrix[3].xyz;
		vec3 toCamera  = normalize(cameraPos - position);
		vec3 fromLight = -toLight;

		float face_hit	= dot(fromLight, normal);
		vec3  reflected = normalize(reflect(fromLight, normal));

		specular = max(dot(toCamera, reflected), 0.0) * float(face_hit < 0);

		specular = pow(specular, 20);

		specularColor = specular * light.color * light.intensity;
	}

	float att;
	if (light.type == 2) att = pow(dist, attenuationExp);
	else att = 1.0;

	return (diffuseColor + specularColor) / att;
}

void main() {
	vec3 light = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < lights.length(); i++) {
		light += calcLight(lights[i], mvVertexPos, mvVertexNormal, outTexCoord, 2.0, materials[material_index]);
	}

	// light = vec3(materials[material_index].albedo);

	fragColor = vec4(light, 1.0);
}