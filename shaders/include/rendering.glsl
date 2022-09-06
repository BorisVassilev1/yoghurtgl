layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec4 color;
layout (location = 4) in vec3 tangent;

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
};

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
};

struct AmbientLight {
 	vec3 color;
	float intensity;
};

struct Light {
	mat4 transform;
	vec3 color;
	float intensity;
	int type;
};


struct Material {
	vec3  albedo;
	float specular_chance;
	// 16

	vec3  emission;
	float ior;
	// 32

	vec3  transparency_color;
	float refraction_chance;
	// 48

	vec3  specular_color;
	float refraction_roughness;
	// 56

	float specular_roughness;
	uint  texture_sampler;
	float texture_influence;
};	   // 64 bytes all


layout(std140, binding=0) uniform Matrices {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 cameraWorldMatrix;
};

layout(std140, binding=1) uniform Materials {
	Material materials[100];
};

layout(std140, binding=2) uniform Lights{
	Light lights[100];
	uint lightsCount;
};

uniform uint material_index = 0;

uniform bool	  use_texture;
layout(binding=0) uniform sampler2D texture_sampler;
layout(binding=1) uniform sampler2D normalMap;

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

vec3 calcAllLights(in vec3 position, in vec3 normal, in vec2 texCoord) {
	vec3 light = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < lightsCount; i++) {
		light += calcLight(lights[i], position, normal, texCoord, 2.0, materials[material_index]);
	}
	return light;
}