#pragma once
#include <any>

class GameWindow;

struct RendererCreateInfo
{
    GameWindow* pWindow;
};

class GameRenderer
{
public:
    GameRenderer() = default;
    ~GameRenderer() = default;

    void create(RendererCreateInfo createInfo);
    void destroy();

    void draw();

private:
    std::any backend_data = 0;
    GameWindow* window_handle = nullptr;
    bool is_active = false;
    bool is_window_minimized = false;
};