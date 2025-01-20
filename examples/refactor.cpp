#include <lunar/render.hpp>
#include <lunar/file/image_file.hpp>

#include <lunar/core/component.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/core/scene.hpp>

int main()
{
	using namespace lunar;
	using namespace lunar::Render;

	auto window_builder = WindowBuilder();
	auto window = window_builder
		.loadFromConfigFile(Fs::fromBase("window.cfg"))
		.create();

	RenderContext context = std::make_shared<RenderContext_T>();
	
	auto img   = Fs::ImageFile(Fs::fromData("skybox/sky.hdr"));
	auto scene = Scene_T("Test");
	GpuCubemap cubemap = context->createCubemap(img.width, img.height, img.bytes, true);
	GameObject obj     = scene.createGameObject("Fack you");
	
	auto components = obj->getComponents();
	DEBUG_LOG("Game object {}", obj->getName()); 
	DEBUG_LOG("Components (count: {}): ", components.size());

	while (!window.shouldClose())
	{
		window.pollEvents();

		context->begin(&window);

		context->end();

		window.update();
	}


	return 1;
}
