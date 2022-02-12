#pragma once
#include <any>
#include <vector>
#include <cstdint>

class GameWindow;

struct RendererCreateInfo
{
    GameWindow* pWindow;
};

struct GraphicsShaderCreateInfo
{
    std::vector<char>& vertexCode;
    std::vector<char>& fragmentCode;
};

using Shader = unsigned int;

class GameRenderer
{
public:
    GameRenderer() = default;
    ~GameRenderer() = default;

    void create(RendererCreateInfo createInfo);
    void destroy();

    std::vector<Shader> create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);

    void draw();

private:
    std::any backend_data = 0;
    GameWindow* window_handle = nullptr;
    bool is_active = false;
    bool is_window_minimized = false;
};