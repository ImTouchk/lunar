#include "utils/debug.hpp"
#include "utils/range.hpp"

#include "render/window.hpp"
#include "render/renderer.hpp"
#include "io/filesystem.hpp"
#include "utils/thread_pool.hpp"

#include "math/vec.hpp"

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#include <stdexcept>
#include <fstream>
#include <vector>

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

int main()
{
#   ifdef WIN32
    // Enables color formatting for the CLion terminal.
    system(("chcp " + std::to_string(CP_UTF8)).c_str());
#   endif

    CDebug::Log("Starting game engine... (Version: 0.0.1) (Rendering backends: {}/{}).", STRINGIFY(WINDOW_BACKEND), STRINGIFY(RENDERER_BACKEND));

    try
    {
        CThreadPool::Initialize();
        CFilesystem::LoadPackage("default.vpak");

        GameWindow game_window;
        game_window.create(WindowCreateInfo
        {
           .isResizable = true,
           .isFullscreen = false,
           .width = 1280,
           .height = 720,
           .pTitle = "Vulkan app"
        });

        GameRenderer game_renderer;
        game_renderer.create(RendererCreateInfo
        {
            .pWindow = &game_window
        });

        auto vertex_code = CVirtualPath("Default/shader.vert").get_bytes();
        auto fragment_code = CVirtualPath("Default/shader.frag").get_bytes();

        GraphicsShaderCreateInfo shader_create_info =
        {
            .vertexCode   = vertex_code,
            .fragmentCode = fragment_code
        };

        auto shaders = game_renderer.create_shaders(&shader_create_info, 1);
        std::vector<Vertex> vertices =
        {
            { 0.0, -0.5, 0.0 },
            { 0.5,  0.5, 0.0 },
            {-0.5,  0.5, 0.0 }
        };

        //std::vector<Index> indices = { 0, 1, 2, };

        //auto mesh = game_renderer.create_object(MeshCreateInfo
        //{
        //    .type     = MeshType::eUnknown,
        //    .vertices = vertices,
        //    .indices  = indices,
        //    .shader   = shaders[0],
        //});

        while(game_window.is_active())
        {
            if(!game_window.is_minimized())
            {
                game_renderer.draw();
            }

            game_window.update();

#           ifdef WIN32
            fflush(stdout); // necessary for CLion to print stuff before the program ends
#           endif
        }

        game_renderer.destroy();
        game_window.destroy();

        CFilesystem::UnloadPackage("Default");
        CThreadPool::Stop();
    } catch(std::runtime_error& error)
    {
        CDebug::Error("Uncaught exception: {}. Program halted.", error.what());
        return -1;
    }

    return 0;
}
