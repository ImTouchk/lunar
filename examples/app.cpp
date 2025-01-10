#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>

#include <lunar/api.hpp>
#include <lunar/render.hpp>
#include <lunar/debug.hpp>
#include <lunar/utils/argument_parser.hpp>

#include <lunar/script/script_vm.hpp>

#include <lunar/exp/utils/lexer.hpp>
#include <lunar/exp/ui/dom.hpp>
#include <lunar/file/file_tracker.hpp>

#include <lunar/render/terra.hpp>

class Test2Comp : public Core::Component
{
public:
    Test2Comp(std::string str) : name(str) {}

    bool isUpdateable() override { return true; }
    void update() override {
        DEBUG_LOG("{}", name);
    }

    static Test2Comp Deserialize(const nlohmann::json& json)
    {
        return Test2Comp(json["name"]);
    }

    static nlohmann::json Serialize(const Test2Comp& comp)
    {
        auto json_object = nlohmann::json();
        json_object["name"] = comp.name;
        return json_object;
    }

    std::string name;
};

class TestComp : public Core::Component
{
public:
    TestComp(float x, float y) : x(x), y(y) {}

    bool isUpdateable() override { return true; }

    void update() override {
        const auto& ty = typeid(this);
        DEBUG_LOG("{} (hash: {}): {}, {}", ty.name(), ty.hash_code(), x, y);
    }

    static TestComp Deserialize(const nlohmann::json& json)
    {
        return TestComp(json["x"], json["y"]);
    }

    static nlohmann::json Serialize(const TestComp& object)
    {
        auto json_object = nlohmann::json();
        json_object["x"] = object.x;
        json_object["y"] = object.y;
        return json_object;
    }

    float x, y;
};

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto dom = UI::Exp::Dom();
    dom.parseSourceFile(Fs::baseDirectory().append("test.html"));
     
    DEBUG_LOG("\n{}", dom.toPrettyString());

    auto scene = Core::SceneBuilder()
        .useCoreSerializers()
        .useClassSerializer<TestComp>("testComp")
        .useClassSerializer<Test2Comp>("test2Comp")
        .fromJsonFile(Fs::dataDirectory().append("main_scene.json"))
        .create();

    scene->getGameObject("Skibidi Toilet")
        .addComponent<Test2Comp>("big boss");

    scene->getGameObject("Skibidi Toilet")
        .getComponent<Test2Comp>()
            ->update();

    scene->getGameObject("Skibidi Toilet")
        .addComponent<TestComp>(1.f, 1.f)
        .update();

    scene->getGameObject("Test Object")
        .getComponent<TestComp>()
            ->update();

    DEBUG_LOG("{}", scene->getName());

    auto render_ctx = Render::CreateDefaultContext();

    auto game_window = Render::WindowBuilder()
        .setRenderContext(render_ctx)
        .loadFromConfigFile(Fs::baseDirectory().append("window.cfg"))
        .create();

    //auto script_vm = Script::VirtualMachineBuilder()
        //.useNativePackageLoader()
        //.enableVerbose()
        //.create();

    Terra::transpileCode(
        Fs::dataDirectory().append("test.tvs"),
        Terra::TranspilerOutput::eVulkanGLSL
    );

    auto& mesh_renderer = scene->getGameObject("Test Object")
        .addComponent<Render::MeshRenderer>();

    auto vertices = std::vector<Render::Vertex>
    {
        Render::Vertex { { -1.f, 1.f, 0 }, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
        Render::Vertex { {  1.f, 1.f, 0 }, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
        Render::Vertex { {  0.f, 0.f, 0 }, 0, { 0, 0, 0 }, 0, { 0.f, 0.f, 1.f, 1.f } },
    };

    auto indices = std::vector<uint32_t> { 0, 1, 2 };

    auto mesh_builder = Render::MeshBuilder();

    mesh_builder
        .useRenderContext(render_ctx)
        .setVertices(vertices)
        .setIndices(indices)
        .build();

    mesh_renderer.mesh   = mesh_builder.getResult();
    mesh_renderer.shader = Render::GraphicsShaderBuilder()
        .useRenderContext(render_ctx)
        .fromVertexSourceFile(Fs::dataDirectory().append("shader-src/default_gl.vert"))
        .fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/default_gl.frag"))
        .build();

    while (!game_window.shouldClose())
    {
        render_ctx->draw(scene, game_window);
        Render::Window::pollEvents();
    }

    return 1;
}
