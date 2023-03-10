#pragma once

#include <glm/glm.hpp>
#include <ostream>
namespace ygl {
struct alignas(16) Material {
	glm::vec3 albedo;
	float	  specular_chance;

	glm::vec3 emission;
	float	  ior;

	glm::vec3 transparency_color;
	float	  refraction_chance;

	glm::vec3 specular_color;
	float	  refraction_roughness;

	float specular_roughness;
	float metallic;
	int	  normal_map;
	float use_normal_map;

	int	  roughness_map;
	float use_roughness_map;
	int	  ao_map;
	float use_ao_map;

	int	  metallic_map;
	float use_metallic_map;
	int	  albedo_map;
	float use_albedo_map;

	int	  emission_map;
	float use_emission_map;

   public:
	Material();
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic, int normal_map, float use_normal_map, int roughness_map, float use_roughness_map,
			 int ao_map, float use_ao_map, int metallic_map, float use_metallic_map, int albedo_map,
			 float use_albedo_map, int emission_map, float use_emission_map);

	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic);

	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic, float texStrength);

	friend std::ostream &operator<<(std::ostream &os, const Material &rhs);
};
}	  // namespace ygl