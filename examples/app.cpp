#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>

#include <lunar/api.hpp>
#include <lunar/render.hpp>
#include <lunar/debug.hpp>
#include <lunar/utils/argument_parser.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto render_ctx = Render::createSharedContext();
    auto game_window = Render::Window(
        render_ctx, 
        Fs::baseDirectory()
            .append("window.cfg")
    );
    
    auto& main_scene = Core::getActiveScene();
    auto& skibidi = main_scene.getGameObject("Skibidi Toilet");
    
    auto& mesh_renderer = skibidi.addComponent<Render::MeshRenderer>();
    mesh_renderer
        .getShader()
        .init(
            Render::ShaderBuilder()
                .renderContext(render_ctx)
                .fromVertexBinaryFile("default.vert")
                .fromFragmentBinaryFile("default.frag")
        );

    while (!game_window.shouldClose())
    {
        Render::Window::pollEvents();

        render_ctx->draw(main_scene, game_window);
    }

    //shader.destroy();
    game_window.close();
    return 1;
}
