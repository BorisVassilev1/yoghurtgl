#include <material.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "renderer.h"

ygl::Material::Material()
	: Material(glm::vec3(1., 1., 1.), .2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.0), 0.0, 0.3, 0.) {}

ygl::Material::Material(glm::vec3 albedo)
	: Material(albedo, 0.2, glm::vec3(0.), 0.99, glm::vec3(0.1), 0.0, glm::vec3(1.0), 0.0, 0.3, 0.) {}

ygl::Material::Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior,
						glm::vec3 transparency_color, float refraction_chance, glm::vec3 specular_color,
						float refraction_roughness, float specular_roughness, float metallic, int normal_map,
						float use_normal_map, int roughness_map, float use_roughness_map, int ao_map, float use_ao_map,
						int metallic_map, float use_metallic_map, int albedo_map, float use_albedo_map,
						int emission_map, float use_emission_map, int transparency_map, float use_transparency_map)
	: albedo(albedo),
	  specular_chance(specular_chance),
	  emission(emission),
	  ior(ior),
	  transparency_color(transparency_color),
	  refraction_chance(refraction_chance),
	  specular_color(specular_color),
	  refraction_roughness(refraction_roughness),
	  specular_roughness(specular_roughness),
	  metallic(metallic),
	  normal_map(normal_map),
	  use_normal_map(use_normal_map),
	  roughness_map(roughness_map),
	  use_roughness_map(use_roughness_map),
	  ao_map(ao_map),
	  use_ao_map(use_ao_map),
	  metallic_map(metallic_map),
	  use_metallic_map(use_metallic_map),
	  albedo_map(albedo_map),
	  use_albedo_map(use_albedo_map),
	  emission_map(emission_map),
	  use_emission_map(use_emission_map),
	  transparency_map(transparency_map),
	  use_transparency_map(use_transparency_map) {}

ygl::Material::Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior,
						glm::vec3 transparency_color, float refraction_chance, glm::vec3 specular_color,
						float refraction_roughness, float specular_roughness, float metallic)
	: albedo(albedo),
	  specular_chance(specular_chance),
	  emission(emission),
	  ior(ior),
	  transparency_color(transparency_color),
	  refraction_chance(refraction_chance),
	  specular_color(specular_color),
	  refraction_roughness(refraction_roughness),
	  specular_roughness(specular_roughness),
	  metallic(metallic),
	  normal_map(0),
	  use_normal_map(0.0),
	  roughness_map(0),
	  use_roughness_map(0),
	  ao_map(0),
	  use_ao_map(0),
	  metallic_map(0),
	  use_metallic_map(0),
	  albedo_map(0),
	  use_albedo_map(0),
	  emission_map(0),
	  use_emission_map(0),
	  transparency_map(0),
	  use_transparency_map(0) {}

ygl::Material::Material(glm::vec3 albedo, float specular_chance, glm::vec3 emission, float ior,
						glm::vec3 transparency_color, float refraction_chance, glm::vec3 specular_color,
						float refraction_roughness, float specular_roughness, float metallic, float texStrength)
	: albedo(albedo),
	  specular_chance(specular_chance),
	  emission(emission),
	  ior(ior),
	  transparency_color(transparency_color),
	  refraction_chance(refraction_chance),
	  specular_color(specular_color),
	  refraction_roughness(refraction_roughness),
	  specular_roughness(specular_roughness),
	  metallic(metallic),
	  normal_map(0),
	  use_normal_map(texStrength),
	  roughness_map(0),
	  use_roughness_map(texStrength),
	  ao_map(0),
	  use_ao_map(texStrength),
	  metallic_map(0),
	  use_metallic_map(texStrength),
	  albedo_map(0),
	  use_albedo_map(texStrength),
	  emission_map(0),
	  use_emission_map(texStrength),
	  transparency_map(0),
	  use_transparency_map(texStrength) {}

static bool drawTextureGui(ygl::Renderer *renderer, int &texture, float &use_texture, const char *name) {
	bool res = false;
	ImGui::InputInt(name, &texture);
	if (texture > 0 || use_texture > 0) {
		int index = renderer->getAssetManager()->getTexture(texture)->getID();
		ImGui::Image(index, ImVec2(128, 128));
		res = ImGui::SliderFloat((std::string("use_") + name).c_str(), (float *)&use_texture, 0., 1.);
	}
	return res;
}

bool ygl::Material::drawImGui(ygl::Renderer *renderer) {
	ImGui::Begin("Material Controls");
	bool res = false;

	res = res || ImGui::ColorEdit3("albedo", glm::value_ptr(albedo));
	res = res || ImGui::SliderFloat("specular_chance", &specular_chance, 0., 1.);
	res = res || ImGui::ColorEdit3("emission", glm::value_ptr(emission));
	res = res || ImGui::SliderFloat("ior", &ior, 0., 1.);
	res = res || ImGui::ColorEdit3("transparency_color", glm::value_ptr(transparency_color));
	res = res || ImGui::SliderFloat("refraction_chance", &refraction_chance, 0., 1.);
	res = res || ImGui::ColorEdit3("specular_color", glm::value_ptr(specular_color));
	res = res || ImGui::SliderFloat("refraction_roughness", &refraction_roughness, 0., 1.);
	res = res || ImGui::SliderFloat("specular_roughness", &specular_roughness, 0., 1.);
	res = res || ImGui::SliderFloat("metallic", &metallic, 0., 1.);
	res = res || drawTextureGui(renderer, normal_map, use_normal_map, "normal_map");
	res = res || drawTextureGui(renderer, roughness_map, use_roughness_map, "roughness_map");
	res = res || drawTextureGui(renderer, ao_map, use_ao_map, "ao_map");
	res = res || drawTextureGui(renderer, metallic_map, use_metallic_map, "metallic_map");
	res = res || drawTextureGui(renderer, albedo_map, use_albedo_map, "albedo_map");
	res = res || drawTextureGui(renderer, emission_map, use_emission_map, "emission_map");
	res = res || drawTextureGui(renderer, transparency_map, use_transparency_map, "transparency_map");

	ImGui::End();
	return res;
}

namespace ygl {
std::ostream &operator<<(std::ostream &os, const ygl::Material &rhs) {
	os << "albedo: (" << rhs.albedo.r << ", " << rhs.albedo.g << ", " << rhs.albedo.b
	   << ")\nspecular_chance: " << rhs.specular_chance << "\nemission: (" << rhs.emission.r << ", " << rhs.emission.g
	   << ", " << rhs.emission.b << ")\nior: " << rhs.ior << "\ntransparency_color: (" << rhs.transparency_color.r
	   << ", " << rhs.transparency_color.g << ", " << rhs.transparency_color.b
	   << ")\nrefraction_chance: " << rhs.refraction_chance << "\nspecular_color: (" << rhs.specular_color.r << ", "
	   << rhs.specular_color.g << ", " << rhs.specular_color.b
	   << ")\nrefraction_roughness: " << rhs.refraction_roughness << "\nspecular_roughness: " << rhs.specular_roughness
	   << "\nmetallic: " << rhs.metallic << "\nnormal_map: " << rhs.normal_map
	   << "\nuse_normal_map: " << rhs.use_normal_map << "\nroughness_map: " << rhs.roughness_map
	   << "\nuse_roughness_map: " << rhs.use_roughness_map << "\nao_map: " << rhs.ao_map
	   << "\nuse_ao_map: " << rhs.use_ao_map << "\nmetallic_map: " << rhs.metallic_map
	   << "\nuse_metallic_map: " << rhs.use_metallic_map << "\nalbedo_map: " << rhs.albedo_map
	   << "\nuse_albedo_map: " << rhs.use_albedo_map << "\nemission_map: " << rhs.emission_map
	   << "\nuse_emission_map: " << rhs.use_emission_map << "\ntransparency_map: " << rhs.transparency_map
	   << "\nuse_transparency_map: " << rhs.use_transparency_map;
	return os;
}
}	  // namespace ygl
