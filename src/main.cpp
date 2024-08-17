#include <utils/argument_parser.hpp>
#include <script/script_vm.hpp>
#include <core/scene.hpp>
#include <core/gameobject.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto main_scene = Core::Scene(Fs::dataDirectory().append("main_scene.json"));
    for (auto& obj : main_scene.getGameObjects())
    {
        obj.update();
    }
    
    //auto test = Core::GameObject("Test Object");
    //test.addComponent(Core::ScriptComponent("dev/mugur/TestScript"));

    return 1;
}
