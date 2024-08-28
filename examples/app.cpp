#include <lunar/render/render_context.hpp>
#include <lunar/utils/argument_parser.hpp>
#include <lunar/script/script_vm.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/gameobject.hpp>
#include <lunar/debug/log.hpp>
#include <lunar/render/window.hpp>
#include <lunar/file/binary_file.hpp>

#include <lunar/render.hpp>

int main(int argc, char* argv[])
{
    auto args = Utils::ArgumentParser(argc, argv);

    auto render_ctx = Render::createSharedContext();
    auto game_window = Render::Window(
        render_ctx, 
        Fs::baseDirectory()
            .append("window.cfg")
    );

    auto shader = Render::GraphicsShader(
        Render::ShaderBuilder()
            .fromVertexBinary("default.vert")
            .fromFragmentBinary("default.frag")
            .renderContext(render_ctx)
    );

    DEBUG_LOG("{}", (void*)static_cast<VkPipeline>(shader.getVkPipeline()));
    
    auto& main_scene = Core::getActiveScene();
    while (!game_window.shouldClose())
    {
        Render::Window::pollEvents();
    }

    //shader.destroy();
    game_window.close();
    return 1;
}
