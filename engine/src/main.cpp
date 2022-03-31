#include "utils/debug.hpp"
#include "utils/range.hpp"

#include "core/time.hpp"
#include "core/input.hpp"
#include "render/window.hpp"
#include "render/renderer.hpp"
#include "io/filesystem.hpp"
#include "utils/thread_pool.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

        CGameWindow game_window;
        game_window.create(WindowCreateInfo
        {
           .isResizable  = true,
           .isFullscreen = false,
           .isMainWindow = true,
           .width        = 1280,
           .height       = 720,
           .pTitle       = "Vulkan app"
        });

        CInput::Initialize();

        //CRenderer game_renderer;
        //game_renderer.create(RendererCreateInfo
        //{
        //    .pWindow = &game_window
        //});

        //auto vertex_code = CVirtualPath("Default/shader.vert").get_bytes();
        //auto fragment_code = CVirtualPath("Default/shader.frag").get_bytes();

        //GraphicsShaderCreateInfo shader_create_info =
        //{
        //    .vertexCode   = vertex_code,
        //    .fragmentCode = fragment_code
        //};

        //auto shaders = game_renderer.create_shaders(&shader_create_info, 1);

        //std::vector<Vertex> vertices =
        //{
        //    { { -1.f, -1.f, -1.f }, { 0.f, 0.f, 0.f }, { 1.f, 0.f } },
        //    { {  1.f, -1.f, -1.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f } },
        //    { {  1.f,  1.f, -1.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f } },
        //    { { -1.f,  1.f, -1.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f } },
        //    { { -1.f, -1.f,  1.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f } },
        //    { {  1.f, -1.f,  1.f }, { 0.f, 0.f, 0.f }, { 1.f, 0.f } },
        //    { {  1.f,  1.f,  1.f }, { 0.f, 0.f, 0.f }, { 0.f, 1.f } },
        //    { { -1.f,  1.f,  1.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f } },
        //};

        //std::vector<Index> indices = 
        //{
        //    0, 1, 3, 3, 1, 2,
        //    1, 5, 2, 2, 5, 6,
        //    5, 4, 6, 6, 4, 7,
        //    4, 0, 7, 7, 0, 3,
        //    3, 2, 7, 7, 2, 6,
        //    4, 5, 0, 0, 5, 1,
        //};

        //int width, height, channels;
        //void* pixels;

        //auto tex_bytes = CVirtualPath("Default/statue.jpg").get_bytes();

        //pixels = stbi_load_from_memory((stbi_uc*)tex_bytes.data(), tex_bytes.size(), &width, &height, &channels, 4);
        //channels = 4;

        //TextureCreateInfo texture_create_info =
        //{
        //    .flags    = {},
        //    .width    = (unsigned)width,
        //    .height   = (unsigned)height,
        //    .channels = (unsigned)channels,
        //    .pData    = pixels
        //};

        //auto mesh = game_renderer.create_object(MeshCreateInfo
        //{
        //    .type     = MeshType::eDynamic,
        //    .vertices = vertices,
        //    .indices  = indices,
        //    .shader   = shaders[0],
        //});

        //auto texture = game_renderer.create_texture(std::move(texture_create_info));
        //shaders[0].use_texture(texture);

        //float rot = 0.f;

        while(game_window.is_active())
        {
            CTime::Update();

            //auto width = static_cast<float>(game_window.get_width());
            //auto height = static_cast<float>(game_window.get_height());
            //auto aspect_ratio = width / height;

            //auto view = glm::mat4(1.f);
            //auto model = glm::mat4(1.0f);
            //auto projection = glm::mat4(1.f);

            //projection = glm::perspective(glm::radians(45.f), aspect_ratio, 0.1f, 100.f);
            //view = glm::translate(view, { 0.f, 0.f, -5.f });
            //model = glm::rotate(model, glm::radians(rot), glm::vec3(1.f, 1.f, 1.f));

            //auto result = projection * view * model;

            //mesh.set_transform(std::move(result));

            //rot += 1.f * CTime::DeltaTime();
            //if (rot >= 5.f)
            //    rot = 0.f;

            //if(!game_window.is_minimized())
            //{
            //    game_renderer.draw();
            //}

            game_window.update();

#           ifdef WIN32
            fflush(stdout); // necessary for CLion to print stuff before the program ends
#           endif
        }

        //game_renderer.destroy();
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
