#define LUNAR_VULKAN
#include <lunar/render/render_context.hpp>
#include <lunar/utils/argument_parser.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/debug/log.hpp>
#include <lunar/render/window.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);
    
    auto game_window = Render::Window(
        Render::createSharedContext(), 
        Fs::baseDirectory()
            .append("window.cfg")
    );

    auto& main_scene = Core::getActiveScene();
    while (!game_window.shouldClose())
    {
        Render::Window::pollEvents();
    }

    
    return 1;
}
