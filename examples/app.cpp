#include <lunar/utils/argument_parser.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/debug/log.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto main_scene = Core::Scene(Fs::dataDirectory().append("main_scene.json"));
    for (auto& obj : main_scene.getGameObjects())
    {
        obj.update();

        auto* transform = obj.getComponent<Core::TransformComponent>();
        if (transform != nullptr)
        {
            DEBUG_LOG("{}, {}, {}", transform->position.x, transform->position.y, transform->position.z);
        }
        else
            DEBUG_LOG("No transform component");

        if (obj.getComponent<Core::ScriptComponent>() != nullptr)
        {
            auto& script = obj.getComponentRef<Core::ScriptComponent>();
            DEBUG_LOG("Script: {}", script.getScriptName());
            script.update();

        }
    }

    //auto test = Core::GameObject("Test Object");
    //test.addComponent(Core::ScriptComponent("dev/mugur/TestScript"));

    return 1;
}
