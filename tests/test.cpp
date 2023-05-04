#include <yoghurtgl.h>
#include <ecs.h>
#include "renderer.h"
#include "transformation.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>


TEST_CASE("Test Scene creation") {
	ygl::Scene scene;

	SUBCASE("Register Component") {
		scene.registerComponent<ygl::Transformation>();
	}

	SUBCASE("Create Entity") {
		ygl::Entity e = scene.createEntity();
	}

	SUBCASE("Add Component") {
		scene.registerComponent<ygl::Transformation>();

		ygl::Entity e = scene.createEntity();
		ygl::Transformation t = ygl::Transformation(glm::vec3(0, 1, 0));

		scene.addComponent<ygl::Transformation>(e, t);
		
		ygl::Transformation &t1 = scene.getComponent<ygl::Transformation>(e);
		CHECK(t == t1);	
	}

	SUBCASE("Add Component Twice") {
		scene.registerComponent<ygl::Transformation>();
		
		ygl::Entity e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation(glm::vec3(1.)));
		CHECK_THROWS(scene.addComponent(e, ygl::Transformation(glm::vec3(1.))));
	}

	SUBCASE("Delete Entity") {
		scene.registerComponent<ygl::Transformation>();
		
		ygl::Entity e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation(glm::vec3(1.)));

		scene.destroyEntity(e);

		CHECK_THROWS(scene.getComponent<ygl::Transformation>(e));
	}
}
