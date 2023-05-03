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
		scene.registerComponent<ygl::Transformation>();

		ygl::Entity e = scene.createEntity();
		ygl::Transformation t = ygl::Transformation(glm::vec3(0, 1, 0));

		scene.addComponent<ygl::Transformation>(e, t);
		
		ygl::Transformation &t1 = scene.getComponent<ygl::Transformation>(e);
		t1.updateWorldMatrix();
		t.updateWorldMatrix();
		CHECK(t == t1);	
	}
}
