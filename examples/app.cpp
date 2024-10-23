#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>

#include <lunar/api.hpp>
#include <lunar/render.hpp>
#include <lunar/debug.hpp>
#include <lunar/utils/argument_parser.hpp>

#include <lunar/exp/utils/lexer.hpp>
#include <lunar/exp/ui/dom.hpp>
#include <lunar/file/file_tracker.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto dom = UI::Exp::Dom();
    dom.parseSourceFile(Fs::baseDirectory().append("test.html"));
     
    DEBUG_LOG("{}", dom.toPrettyString());

    auto game_window = Render::WindowBuilder()
        .setDefaultRenderContext()
        .setWidth(1280)
        .setHeight(720)
        .create();

    while (!game_window.shouldClose())
    {
        game_window.pollEvents();
    }

    return 1;

    //auto render_ctx = Render::createSharedContext();
    //auto game_window = Render::Window(
    //    Render::WindowBuilder()
    //        .renderContext(render_ctx)
    //        .fromConfigFile(Fs::baseDirectory().append("window.cfg"))
    //);
    //
    //auto& main_scene = Core::getActiveScene();
    //auto& skibidi = main_scene.getGameObject("Skibidi Toilet");
    //
    //auto& mesh_renderer = skibidi.addComponent<Render::MeshRenderer>();
    //mesh_renderer
    //    .getShader()
    //    .init(
    //        Render::ShaderBuilder()
    //            .renderContext(render_ctx)
    //            .fromVertexBinaryFile("default.vert")
    //            .fromFragmentBinaryFile("default.frag")
    //    );

    //while (!game_window.shouldClose())
    //{
    //    Render::Window::pollEvents();
    //    
    //    render_ctx->draw(main_scene, game_window);
    //}

    //return 1;
}
