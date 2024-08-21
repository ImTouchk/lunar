#include <lunar/utils/argument_parser.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/debug/log.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto& main_scene = Core::getActiveScene();

    for (int i = 0; i < 3; i++)
    {
        for (auto& obj : main_scene.getGameObjects())
        {
            auto& transform = obj.getTransform();
            DEBUG_LOG("{}, {}, {}", transform.position.x, transform.position.y, transform.position.z);
            obj.update();
        }
    }

    //auto test = Core::GameObject("Test Object");
    //test.addComponent(Core::ScriptComponent("dev/mugur/TestScript"));

    return 1;
}
