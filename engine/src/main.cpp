#include "utils/debug.hpp"
#include "render/window.hpp"
#include "render/renderer.hpp"

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif
#include <stdexcept>
#include <fstream>
#include <vector>

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

std::vector<char> ReadFile(const std::string& name)
{
    std::ifstream file(name, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Renderer-File-OpenFail");
    }

    size_t file_size;
    file_size = file.tellg();
    file.seekg(0);

    auto buffer = std::vector<char>(file_size);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

int main()
{
#   ifdef WIN32
    // Enables color formatting for the CLion terminal.
    system(("chcp " + std::to_string(CP_UTF8)).c_str());
#   endif

    CDebug::Log("Starting game engine... (Version: 0.0.1) (Rendering backends: {}/{}).", STRINGIFY(WINDOW_BACKEND), STRINGIFY(RENDERER_BACKEND));

    try
    {
        GameWindow game_window;
        game_window.create(WindowCreateInfo
        {
           .isResizable = true,
           .isFullscreen = false,
           .width = 1280,
           .height = 720,
           .pTitle = "Hello, world!"
        });

        GameRenderer game_renderer;
        game_renderer.create(RendererCreateInfo
        {
            .pWindow = &game_window
        });

        auto vert_code = ReadFile("vert.spv");
        auto frag_code = ReadFile("frag.spv");

        GraphicsShaderCreateInfo shader_create_info =
        {
            .vertexCode = vert_code,
            .fragmentCode = frag_code
        };

        game_renderer.create_shaders(&shader_create_info, 1);

        while(game_window.is_active())
        {
            if(!game_window.is_minimized())
            {
                game_renderer.draw();
                // do rendering
            }

            game_window.update();
        }

        game_renderer.destroy();
        game_window.destroy();
    } catch(std::runtime_error& error)
    {
        CDebug::Error("Uncaught exception: {}. Program halted.", error.what());
        return -1;
    }

    return 0;
}
