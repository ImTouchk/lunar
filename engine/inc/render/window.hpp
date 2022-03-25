#pragma once
#include <any>
#include <vector>
#include <functional>

enum class WindowEvent
{
    eAny = 0,
    eCreated,
    eDestroyed,
    eResized,
    eKeyStateChanged,
    eMouseMoved,
};

struct WindowCreateInfo
{
    bool isResizable;
    bool isFullscreen;
    bool isMainWindow;
    int width, height;
    const char* pTitle;
};

using WindowEventSubscriber = std::function<void(void*, const std::any&)>;

// Wrapper over native windowing system; does not include any rendering capabilities.
class GameWindow
{
public:
    GameWindow() = default;
    ~GameWindow() = default;

    void create(WindowCreateInfo createInfo);
    void destroy();

    [[nodiscard]] bool is_active() const;
    [[nodiscard]] bool is_minimized() const;

    [[nodiscard]] int get_width() const;
    [[nodiscard]] int get_height() const;

    void update();

    [[nodiscard]] static GameWindow& GetPrimary();

    void* get_handle();

    void subscribe(WindowEvent event, WindowEventSubscriber handler);
    void unsubscribe(WindowEvent event, WindowEventSubscriber handler);

    static void HandleEvent(GameWindow* window, WindowEvent event, const std::any& data);

private:
    void* native_handle = nullptr;
    int width = 0;
    int height = 0;
    bool is_created = false;
    bool is_inactive = false;

    struct SubscriberData
    {
        WindowEvent event = WindowEvent::eAny;
        WindowEventSubscriber handler = nullptr;
    };

    std::vector<SubscriberData> subscribers = {};
};
