#pragma once
#include <vector>
#include <functional>

enum class WindowEvent
{
    eAny = 0,
    eCreated,
    eDestroyed,
    eResized,
};

struct WindowCreateInfo
{
    bool is_fullscreen;
    int width, height;
    const char* title;
};

// Wrapper over native windowing system; does not include any rendering capabilities.
class GameWindow
{
public:
    GameWindow() = default;
    ~GameWindow() = default;

    void create(WindowCreateInfo createInfo);
    void destroy();

    bool should_close() const;
    void update();

private:
    void* native_handle = nullptr;
    int width = 0;
    int height = 0;
    bool is_active = false;
};
