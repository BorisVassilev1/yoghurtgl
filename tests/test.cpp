#include <yoghurtgl.h>
#include <ecs.h>
#include <renderer.h>
#include <transformation.h>
#include <sstream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_CASE("Test Scene creation") {
	ygl::Scene scene;

	SUBCASE("Register Component") { scene.registerComponent<ygl::Transformation>(); }

	SUBCASE("Create Entity") { ygl::Entity e = scene.createEntity(); }

	SUBCASE("Add Component") {
		scene.registerComponent<ygl::Transformation>();

		ygl::Entity			e = scene.createEntity();
		ygl::Transformation t = ygl::Transformation(glm::vec3(0, 1, 0));

		scene.addComponent<ygl::Transformation>(e, t);

		ygl::Transformation &t1 = scene.getComponent<ygl::Transformation>(e);
		CHECK(t == t1);
	}

	SUBCASE("Add Component Twice") {
		scene.registerComponent<ygl::Transformation>();

		ygl::Entity e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation(glm::vec3(1.)));

		CHECK(scene.hasComponent<ygl::Transformation>(e));
		CHECK_THROWS(scene.addComponent(e, ygl::Transformation(glm::vec3(1.))));
	}

	SUBCASE("Delete Entity") {
		scene.registerComponent<ygl::Transformation>();

		ygl::Entity e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation(glm::vec3(1.)));

		scene.destroyEntity(e);

		CHECK_FALSE(scene.hasComponent<ygl::Transformation>(e));
		CHECK_THROWS(scene.getComponent<ygl::Transformation>(e));
	}
}

class Translator : public ygl::ISystem {
   public:
	using ygl::ISystem::ISystem;
	void init() override {
		this->scene->registerComponentIfCan<ygl::Transformation>();
		this->scene->setSystemSignature<Translator, ygl::Transformation>();
	}

	void doWork() override {
		for (ygl::Entity e : this->entities) {
			ygl::Transformation &t = this->scene->getComponent<ygl::Transformation>(e);
			t.position += glm::vec3(1.);
		}
	}

	void serialize(std::ostream &out) override { static_cast<void>(out); }
	void deserialize(std::istream &in) override { static_cast<void>(in); }
};

TEST_CASE("Scene System") {
	ygl::Scene scene;

	SUBCASE("Add System") { scene.registerSystem<Translator>(); }

	SUBCASE("Test System") {
		scene.registerSystem<Translator>();
		ygl::Entity e = scene.createEntity();

		bool res;
		res = scene.getSystem<Translator>()->entities.contains(e);
		CHECK_FALSE(res);

		scene.addComponent(e, ygl::Transformation());
		res = scene.getSystem<Translator>()->entities.contains(e);
		CHECK(res);

		scene.getSystem<Translator>()->doWork();
		ygl::Transformation t = scene.getComponent<ygl::Transformation>(e);
		CHECK(t.position == glm::vec3(1.));
	}
}

TEST_CASE("Serialization") {
	SUBCASE("Basic Serializable") {
		ygl::Transformation t(glm::vec3(2.));
		ygl::Transformation u;

		std::stringstream ss;

		t.serialize(ss);
		u.deserialize(ss);

		CHECK(t == u);

		ygl::RendererComponent r(1, 2, 3);
		ygl::RendererComponent r1;

		ss.clear();
		r.serialize(ss);
		r1.deserialize(ss);

		CHECK(r == r1);
	}

	SUBCASE("Scene serialization") {
		ygl::Scene scene;

		scene.registerSystem<Translator>();
		scene.registerComponent<ygl::RendererComponent>();

		ygl::Entity e;
		e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation());
		scene.addComponent(e, ygl::RendererComponent());

		e = scene.createEntity();
		scene.addComponent(e, ygl::Transformation(glm::vec3(1.)));

		std::stringstream ss;
		scene.serialize(ss);

		ygl::Scene other;
		other.registerComponent<ygl::RendererComponent>();
		other.registerComponent<ygl::Transformation>();
		other.deserialize(ss);

		// assert(scene == other);
	}
}
