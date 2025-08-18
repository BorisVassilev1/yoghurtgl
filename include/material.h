#pragma once

#include <glm/glm.hpp>
#include <ostream>

/**
 * @file material.h
 * @brief Material definition
 */

namespace ygl {

class Renderer;

/**
 * @brief Material data class. Alligned to 16 bytes so it can be copied to the same structure on the GPU, defined in
 * ../shaders/include/rendering.glsl
 */
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
	int	  transparency_map;
	float use_transparency_map;

   public:
	Material();

	Material(glm::vec3 albedo);
	/**
	 * @brief Constructor with all possible parameters.
	 */
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic, int normal_map, float use_normal_map, int roughness_map, float use_roughness_map,
			 int ao_map, float use_ao_map, int metallic_map, float use_metallic_map, int albedo_map,
			 float use_albedo_map, int emission_map, float use_emission_map, int transparency_map,
			 float use_transparency_map);

	/**
	 * @brief Constructor for untextured materials
	 */
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic);

	/**
	 * @brief Constructor for textured materials, but only if you are going to set the texture fields yourself.
	 */
	Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior, glm::vec3 transparency_color,
			 float refraction_chance, glm::vec3 specular_color, float refraction_roughness, float specular_roughness,
			 float metallic, float texStrength);

	bool drawImGui(ygl::Renderer *renderer);

	friend std::ostream &operator<<(std::ostream &os, const Material &rhs);
};
}	  // namespace ygl
