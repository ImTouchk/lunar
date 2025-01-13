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

class UISceneHierarchy : public Core::Component
{
private:
    void renderComponent(Core::Component* component)
    {
        auto title = std::format("Component: {}", component->_getClassName());
        if (ImGui::CollapsingHeader(title.c_str()))
        {
            ImGui::Indent();

            ImGui::Unindent();
        }
    }

    void renderObjectTree(Core::GameObject& object)
    {
        auto title    = std::format("GameObject: {} (ID: {})", object.getName(), object.getId());

        ImGui::TreePush(object.getName().c_str());
        if (ImGui::CollapsingHeader(title.c_str()))
        {
            ImGui::Indent();

            if (ImGui::Button("Delete"))
            {
                auto* scene = object.getParentScene();
                scene->deleteGameObject(object.getId());
            }

            ImGui::SeparatorText("Transform");

            auto& transform = object.getTransform();
                
            ImGui::InputFloat3("Position", &transform.position.x);
            ImGui::InputFloat3("Rotation", &transform.rotation.x);
            ImGui::InputFloat3("Scale",    &transform.scale.x);

            ImGui::SeparatorText("Components");
            auto components = object.getComponents();
            for (auto& component : components)
                renderComponent(component.get());

            auto children = object.getChildren();
            ImGui::SeparatorText("Children");
            for (auto& child : children)
                renderObjectTree(*child);

            if (children.size() == 0)
                ImGui::Text("This object has no children.");

            ImGui::Unindent();
        }
        ImGui::TreePop();
    }
    
public:
    void renderUpdate(Render::RenderContext& context) override
    {
        auto& scene   = getScene();
        auto& objects = scene.getGameObjects();
        auto  title   = std::format("Scene: {}", scene.getName());

        ImGui::SetCurrentContext(context.getImGuiContext());
        ImGui::Begin(title.c_str());

        ImGui::Text("ID: %d", scene.getId());
        ImGui::Text("Objects: %d", objects.size());
        
        ImGui::SeparatorText("GameObjects");
        for (auto& object : objects)
            if (object.getParentId() == -1)
                renderObjectTree(object);

        ImGui::End();
    }

    Core::ComponentClassFlags _getClassFlags() 
    { 
        return 
            Core::ComponentClassFlagBits::eNone |
            Core::ComponentClassFlagBits::eRenderable; 
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


    scene->getGameObject("Skibidi Toilet").addComponent<UISceneHierarchy>();
    scene->setMainCamera(scene->getGameObject("Skibidi Toilet").addComponent<Render::Camera>());
    scene->createGameObject("Bruh Moment", &scene->getGameObject("Test Object"));

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


    std::vector<uint32_t> bytes = {};
    for (size_t x = 0; x < 16; x++) {
        for (size_t y = 0; y < 16; y++) {            
            if ((x % 2) ^ (y % 2))
                bytes.push_back(0xFFFF00FF);
            else
                bytes.push_back(0xFF000000);
        }
    }
    
    auto texture_builder = Render::TextureBuilder();
    auto texture = texture_builder
        .fromByteArray(Render::TextureFormat::eRGBA, 16, 16, bytes.data())
        .setFiltering(Render::TextureFiltering::eNearest)
        .build()
        .getResult();

    mesh_renderer.mesh.material.colorMap = texture;

    while (!game_window.shouldClose())
    {
        render_ctx->draw(scene, game_window);
        Render::Window::pollEvents();
    }

    return 1;
}
