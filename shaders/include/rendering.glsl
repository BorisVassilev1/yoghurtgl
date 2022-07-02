layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec4 color;

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
	vec3 albedo;
	float	  specular_chance;
	// 16
	
	vec3 emission;
	float	  ior;
	// 32
	
	vec3 transparency_color;
	float	  refraction_chance;
	// 48
	
	float refraction_roughness;
	float specular_roughness;
	// 56 

	int texture_sampler; // useless but here for alignment
	float texture_influence;
}; // 64 bytes all

layout(std140, binding=0) uniform Matrices {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 cameraWorldMatrix;
};

layout(std140, binding=0) buffer Materials {
	Material materials[];
};

layout(std140, binding=1) buffer Lights{
	Light lights[];
};

uniform unsigned int material_index = 0;