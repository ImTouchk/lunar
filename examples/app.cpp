#include <lunar/utils/argument_parser.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/debug/log.hpp>
#include <lunar/render/window.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto& main_scene = Core::getActiveScene();

    if (args.has("--no-graphics") == -1)
    {
        auto& game_window = Render::getGameWindow();

        while (!game_window.shouldClose())
        {
            Render::Window::pollEvents();
        }
    }


    for (int i = 0; i < 3; i++)
    {
        for (auto& obj : main_scene.getGameObjects())
        {
            obj.update();
        }
    }


    return 1;
}
