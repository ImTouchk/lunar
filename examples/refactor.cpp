#include <lunar/render.hpp>
#include <lunar/file/image_file.hpp>

#include <lunar/core/component.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/time.hpp>

using namespace lunar;
using namespace lunar::Render;

class SimpleMovement : public Component_T
{
	void update() override
	{
		auto& transform  = getTransform();
		auto* camera     = getGameObject()->getComponent<Camera>();
		float delta_time = Time::DeltaTime();
		float speed      = Input::GetAction("sprint") ? 8 * 4 : 8;
		auto  axis       = Input::GetAxis();
		auto  rotation   = Input::GetRotation();
		auto& right      = camera->getRight();
		auto& front      = camera->getFront();

		transform.position += right * axis.x * delta_time * speed;
		transform.position += front * axis.y * delta_time * speed;
		transform.rotation += glm::vec3{ rotation.x, rotation.y, 0.f };
		transform.rotation = glm::clamp(transform.rotation, { -90.f, -90.f, 0.f }, { 90.f, 90.f, 90.f });

		if(transform.position.x != 0 || transform.position.y != 0 || transform.position.z != 0)
			DEBUG_LOG("x: {}, y: {}, z: {}", transform.position.x, transform.position.y, transform.position.z);
	}
};

int main()
{
	RenderContext context = std::make_shared<RenderContext_T>();
	Window        window  = WindowBuilder()
		.size(800, 600)
		.samples(4)
		.build(context, "Hello, world!");

	auto          scene   = Scene("Test");
	GameObject    object  = scene.createGameObject("Test Object");
	Camera&       camera  = object->addComponent<Camera>();

	object->addComponent<SimpleMovement>();

	scene.setMainCamera(&camera);

	auto img   = Fs::ImageFile(Fs::fromData("skybox/sky.hdr"));
	GpuCubemap cubemap = context->createCubemap(img.width, img.height, img.bytes, true);
	DEBUG_LOG("Cubemap loaded");
	
	auto components = object->getComponents();
	DEBUG_LOG("Game object {}", object->getName()); 
	DEBUG_LOG("Components (count: {}): ", components.size());
	for (auto& component : components)
		DEBUG_LOG("Component '{}'", component->getClassName());

	window->registerAction("toggle_menu", { { "keyboard.esc" } });
	Input::SetGlobalHandler(&window.get());

	GpuMesh mesh = GpuMeshBuilder()
		.useRenderContext(context.get())
		.fromMeshFile(Fs::fromData("models/house.gltf"))
		.build();

	GameObject object2   = scene.createGameObject("House");
	auto&      renderer  = object2->addComponent<MeshRenderer>();

	renderer.mesh    = mesh;
	renderer.program = context->getProgram(GpuDefaultPrograms::eBasicPbrShader);


	while (window->isActive())
	{
		if (window->getActionDown("toggle_menu"))
			window->toggleCursorLocked();

		scene.update();

		context->begin(&window.get());
		context->clear(1.f, 1.f, 1.f, 1.f);

		context->useCamera(camera);
		context->draw(cubemap);
		context->draw(scene);

		context->end();

		window->update();
		window->pollEvents();
	}


	return 1;
}
