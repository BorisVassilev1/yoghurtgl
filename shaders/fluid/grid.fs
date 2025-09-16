#define FRAG

#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;
in mat3 vTBN;

out vec4 fragColor;

layout(rgba32f, binding = 17) uniform readonly image3D volume;
layout(r32ui, binding = 18) uniform readonly uimage3D cellTypes;
uniform float cellSize = 1.0f;

void Unity_Blackbody_float(float Temperature, out vec3 Out)
{
    vec3 color = vec3(255.0f, 255.0f, 255.0f);
    color.x = 56100000.f * pow(Temperature,(-3.0f / 2.0f)) + 148.0f;
    color.y = 100.04f * log(Temperature) - 623.6f;
    if (Temperature > 6500.0f) color.y = 35200000.0f * pow(Temperature,(-3.0f / 2.0f)) + 184.0f;
    color.z = 194.18f * log(Temperature) - 1448.6f;
    color = clamp(color, 0.0f, 255.0f)/255.0f;
    if (Temperature < 1000.0f) color *= Temperature/1000.0f;
    Out = color;
}

void main() {
	vec3	 normalizedVertexNormal = normalize(vVertexNormal);
	vec3	 finalNormal;
	Material mat = materials[material_index];

	if (mat.use_normal_map != 0.0) {
		vec3 normal = texture(normalMap, vTexCoord).xyz;

		normal = normalize(normal * 2. - 1.);

		finalNormal = normalize(mix(vVertexNormal, vTBN * normal, mat.use_normal_map));
	} else {
		finalNormal = normalizedVertexNormal;
	}

	//float temperature = length(vParticleData1.xyz) * 200.0;
	vec3 albedo = vec3(0.0, 0.5, 0.0);
	//Unity_Blackbody_float(temperature, albedo.xyz);
	vec4 vol = imageLoad(volume, ivec3(floor(vVertexPos / cellSize) + vec3(20)));
	uvec4 type = imageLoad(cellTypes, ivec3(floor(vVertexPos / cellSize) + vec3(20)));

	albedo = clamp(vol.xyz + 0.5f, 0.0f, 1.0f);

	vec3 color = albedo;

	//if (!gl_FrontFacing) finalNormal = -finalNormal;
	//color = calcAllLights(vVertexPos, finalNormal, finalNormal, vTexCoord);
	//color = calcAllLightsCustom(vVertexPos, finalNormal, albedo, 0.4f, 0.0f, 1.0f, vec3(0.0f), 1.0f);

	fragColor = vec4(color, clamp(vol.w * abs(dot(vVertexNormal, normalize(vol.xyz))), 0.0, 1.0));
	//fragColor =	vec4(color, float(type.x == 1u)); 
	//fragColor = vec4(color, clamp(vol.w * 5, 0.1, 1.0));
	//fragColor = vec4(color, 0.4);
}
