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

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto dom = UI::Exp::Dom();
    dom.parseSourceFile(Fs::baseDirectory().append("test.html"));
     
    DEBUG_LOG("\n{}", dom.toPrettyString());

    auto scene = Core::SceneBuilder()
        .useDefaultComponentParsers()
        .fromJsonFile(Fs::dataDirectory().append("main_scene.json"))
        .create();

    DEBUG_LOG("{}", scene.get()->getName());

    auto render_ctx = Render::CreateDefaultContext();

    auto game_window = Render::WindowBuilder()
        .setRenderContext(render_ctx)
        .setSize(1280, 720)
        .create();

    auto secondary_window = Render::WindowBuilder()
        .setRenderContext(render_ctx)
        .setSize(800, 600)
        .create();

    //auto script_vm = Script::VirtualMachineBuilder()
        //.useNativePackageLoader()
        //.enableVerbose()
        //.create();
    


    while (!game_window.shouldClose())
    {
        Render::Window::pollEvents();
        render_ctx->render(*scene.get());
        render_ctx->output(game_window);
        //render_ctx->output(&secondary_window);
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
