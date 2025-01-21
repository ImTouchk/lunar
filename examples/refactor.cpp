#include <lunar/render.hpp>
#include <lunar/file/image_file.hpp>

#include <lunar/core/component.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>

int main()
{
	using namespace lunar;
	using namespace lunar::Render;

	RenderContext context = std::make_shared<RenderContext_T>();
	Window        window  = WindowBuilder()
		.size(800, 600)
		.samples(4)
		.build(context, "Hello, world!");

	auto          scene   = Scene_T("Test");
	GameObject    object  = scene.createGameObject("Test Object");

	scene.setMainCamera(&object->addComponent<Camera>());

	auto img   = Fs::ImageFile(Fs::fromData("skybox/sky.hdr"));
	GpuCubemap cubemap = context->createCubemap(img.width, img.height, img.bytes, true);
	
	auto components = object->getComponents();
	DEBUG_LOG("Game object {}", object->getName()); 
	DEBUG_LOG("Components (count: {}): ", components.size());
	for (auto& component : components)
		DEBUG_LOG("Component '{}'", component->getClassName());

	while (window->isActive())
	{

		window->update();
		window->pollEvents();
	}


	return 1;
}
