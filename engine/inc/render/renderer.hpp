#pragma once
#include <any>

class GameWindow;

struct RendererCreateInfo
{
    GameWindow* pWindow;
};

class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    void create(RendererCreateInfo createInfo);
    void destroy();

private:
    std::any backend_data = 0;
    GameWindow* window_handle = nullptr;
    bool is_active = false;
};