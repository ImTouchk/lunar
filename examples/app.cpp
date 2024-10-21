#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>

#include <lunar/api.hpp>
#include <lunar/render.hpp>
#include <lunar/debug.hpp>
#include <lunar/utils/argument_parser.hpp>

#include <lunar/exp/utils/lexer.hpp>
#include <lunar/file/file_tracker.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto lexer = Utils::Exp::LexerBuilder()
        .appendTextFile(Fs::baseDirectory().append("window.cfg"))
        .create();

    std::string_view a;
    int b;
    bool v;
    lexer.parseLine("{:s} = {:d}", &a, &b);
    lexer.parseLine("{:s} = {:d}", &a, &b);
    lexer.parseLine("{:s} = {:b}", &a, &v);

    DEBUG_LOG("{} {}", a, v);

    return 1;

    auto render_ctx = Render::createSharedContext();
    auto game_window = Render::Window(
        Render::WindowBuilder()
            .renderContext(render_ctx)
            .fromConfigFile(Fs::baseDirectory().append("window.cfg"))
    );

    auto secondary_window = Render::Window(
        Render::WindowBuilder()
            .renderContext(render_ctx)
            .fullscreen(false)
            .width(1280)
            .height(720)
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

    while (game_window.exists() || secondary_window.exists())
    {
        Render::Window::pollEvents();

        if (game_window.exists())
        {
            if (game_window.shouldClose())
            {
                game_window.destroy();
            }
            else
            {
                render_ctx->draw(main_scene, game_window);
            }
        }

        if (secondary_window.exists())
        {
            if (secondary_window.shouldClose())
            {
                secondary_window.destroy();
            }
            else
            {
                render_ctx->draw(main_scene, secondary_window);
            }
        }
    }

    return 1;
}
