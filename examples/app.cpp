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

#include <lunar/core/time.hpp>

#include <imgui.h>

class Test2Comp : public Core::Component
{
public:
    Test2Comp(std::string str) : name(str) {}

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

class SimpleMovement : public Core::Component
{
    void update() override
    {
        auto& transform  = getTransform();
        auto* camera     = getGameObject().getComponent<Render::Camera>();
        float delta_time = Time::DeltaTime();
        float speed      = Input::GetAction("sprint") ? 8 * 4 : 8;
        auto  axis       = Input::GetAxis();
        auto  rotation   = Input::GetRotation();
        auto& right      = camera->getRight();
        auto& front      = camera->getFront();

        transform.position += right * axis.x * delta_time * speed;
        transform.position += front * axis.y * delta_time * speed;
        transform.rotation += glm::vec3 { rotation.x, rotation.y, 0.f };
        transform.rotation  = glm::clamp(transform.rotation, { -90.f, -90.f, 0.f }, { 90.f, 90.f, 90.f });
    }

    void drawDebugUI(Render::RenderContext& ctx) override
    {
        ImGui::SetCurrentContext(ctx.getImGuiContext());
        ImGui::Text("Test!");
    }

    Core::ComponentClassFlags _getClassFlags() override
    {
        return Core::ComponentClassFlagBits::eUpdateable |
            Core::ComponentClassFlagBits::eUiDrawable;
    }
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
    
    scene->setMainCamera(scene->getGameObject("Skibidi Toilet").addComponent<Render::Camera>());
    scene->getGameObject("Skibidi Toilet")
        .addComponent<SimpleMovement>();

    scene->createGameObject("Bruh Moment", &scene->getGameObject("Test Object"));

    DEBUG_LOG("{}", scene->getName());

    auto render_ctx = Render::CreateDefaultContext();

    auto game_window = Render::WindowBuilder()
        .setRenderContext(render_ctx)
        .loadFromConfigFile(Fs::baseDirectory().append("window.cfg"))
        .create();

    render_ctx->init();

    //auto script_vm = Script::VirtualMachineBuilder()
        //.useNativePackageLoader()
        //.enableVerbose()
        //.create();

    Terra::transpileCode(
        Fs::dataDirectory().append("test.tvs"),
        Terra::TranspilerOutput::eVulkanGLSL
    );

    auto& mesh_renderer = scene->getGameObject("Test Object").addComponent<Render::MeshRenderer>();

    auto vertices = std::vector<Render::Vertex>
    {
        Render::Vertex { { -1.f,-1.f, 0 }, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
        Render::Vertex { { -1.f, 1.f, 0 }, 0, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
        Render::Vertex { {  1.f,-1.f, 0 }, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
        Render::Vertex { {  1.f, 1.f, 0 }, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
    };

    auto indices = std::vector<uint32_t> { 0, 1, 2, 2, 1, 3 };

    auto mesh_builder = Render::MeshBuilder();

    //mesh_builder
    //    .useRenderContext(render_ctx)
    //    .setVertices(vertices)
    //    .setIndices(indices)
    //    .build();

    mesh_renderer.mesh   = mesh_builder
        .useRenderContext(render_ctx)
        .fromGltfFile(Fs::dataDirectory().append("models/house.gltf"))
        .build()
        .getResult();

    mesh_renderer.shader = Render::GraphicsShaderBuilder()
        .useRenderContext(render_ctx)
        .fromVertexSourceFile(Fs::dataDirectory().append("shader-src/pbr.vert"))
        .fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/pbr.frag"))
        .build();

    scene->getGameObject("Light")
        .addComponent<Render::Light>();

    auto cubemap_builder = Render::CubemapBuilder();
    auto cubemap = cubemap_builder
        .useRenderContext(render_ctx)
        .fromHDRFile(Fs::dataDirectory().append("skybox/sky.hdr"))
        .build()
        .getResult();

    std::vector<uint32_t> bytes = {};
    for (size_t x = 0; x < 16; x++) {
        for (size_t y = 0; y < 16; y++) {            
            //if ((x % 2) ^ (y % 2))
            //    bytes.push_back(0xFFFF00FF);
            //else
            //    bytes.push_back(0xFF000000);
            bytes.push_back(0xFFFFFFFFFF);
        }
    }
    
    auto texture_builder = Render::TextureBuilder();
    auto texture = texture_builder
        .fromByteArray(Render::TextureFormat::eRGBA, 16, 16, bytes.data())
        .setByteFormat(Render::TextureByteFormat::eUnsignedByte)
        .setFiltering(Render::TextureFiltering::eNearest)
        .build()
        .getResult();

    mesh_renderer.mesh.material.albedo = texture;
    game_window.registerAction("toggle_menu", { { "keyboard.esc" } });
    game_window.registerAction("sprint", { { "keyboard.shift" } });

    scene->addEventListener<Core::Events::SceneObjectDeleted>([](auto& e) {
        
        DEBUG_LOG("Event triggered | deleted {}", e.gameObject.getName());
    });
    
    Input::SetGlobalHandler(game_window);

    while (!game_window.shouldClose())
    {
        Render::Window::pollEvents();
        Time::Update();

        if (Input::GetActionUp("toggle_menu"))
            game_window.toggleCursor();

        scene->update();

        render_ctx->begin(game_window);
        render_ctx->clear(1.f, 1.f, 1.f, 1.f);
        render_ctx->setCamera(*scene->getMainCamera());
        render_ctx->draw(cubemap);
        render_ctx->draw(scene);

        Debug::DrawSceneHierarchyPanel(*render_ctx, *scene); 
        Debug::DrawGeneralInfoPanel(*render_ctx);

        render_ctx->end();

        game_window.update();
    }

    return 1;
}
