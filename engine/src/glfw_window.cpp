#include "render/window.hpp"
#include "utils/debug.hpp"

#include <vector>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <GLFW/glfw3.h>

struct GlfwLifeguard
{
    GlfwLifeguard() noexcept
    {
        int result;
        result = glfwInit();
        if(!result)
            std::exit(1);
    }

    ~GlfwLifeguard() noexcept
    {
        glfwTerminate();
    }
};

GlfwLifeguard GLFW_LIFEGUARD = {};

void GLFW_KEY_CALLBACK(GLFWwindow* handle, int key, int scancode, int action, int mods)
{
    auto window = (GameWindow*)glfwGetWindowUserPointer(handle);
    auto data = std::pair<int, bool>(key, action);
    GameWindow::HandleEvent(window, WindowEvent::eKeyStateChanged, data);
}

void GLFW_CURSOR_POS_CALLBACK(GLFWwindow* handle, double x, double y)
{
    auto window = (GameWindow*)glfwGetWindowUserPointer(handle);
    auto data = std::pair<double, double>(x, y);
    GameWindow::HandleEvent(window, WindowEvent::eMouseMoved, data);
}

void GLFW_FRAMEBUFFER_SIZE_CALLBACK(GLFWwindow* handle, int width, int height)
{
    auto window = (GameWindow*)glfwGetWindowUserPointer(handle);
    auto data = std::pair<int, int>(width, height);

    if(width == 0 && height == 0)
    {
        GameWindow::HandleEvent(window, WindowEvent::eMinimized, nullptr);
    }
    else if(window->is_minimized())
    {
        GameWindow::HandleEvent(window, WindowEvent::eRestored, nullptr);
    }
    else
    {
        GameWindow::HandleEvent(window, WindowEvent::eResized, data);
    }
}

void GameWindow::HandleEvent(GameWindow* window, WindowEvent event, const std::any& data)
{
    for(const auto& subscriber : window->subscribers)
    {
        if(subscriber.event != event && subscriber.event != WindowEvent::eAny)
            continue;

        subscriber.handler(window, data);
    }

    switch(event)
    {
        case WindowEvent::eResized:
        {
            auto new_size = std::any_cast<std::pair<int, int>>(data);
            window->width = new_size.first;
            window->height = new_size.second;
            break;
        }

        case WindowEvent::eMinimized:
        {
            window->is_inactive = true;
            break;
        }

        case WindowEvent::eRestored:
        {
            window->is_inactive = false;
            break;
        }
    }
}

void GameWindow::create(WindowCreateInfo createInfo)
{
    assert(is_created == false);

    int new_width = createInfo.width;
    int new_height = createInfo.height;
    GLFWmonitor* pMonitor = nullptr;

    if(createInfo.isFullscreen)
    {
        pMonitor = glfwGetPrimaryMonitor();

        if(new_width == 0 && new_height == 0)
        {
            const GLFWvidmode* pVideoMode = glfwGetVideoMode(pMonitor);
            new_width = pVideoMode->width;
            new_height = pVideoMode->height;
        }
    }

    glfwWindowHint(GLFW_RESIZABLE, createInfo.isResizable);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    native_handle = (void*)glfwCreateWindow(new_width, new_height, createInfo.pTitle, pMonitor, nullptr);
    if(!native_handle)
    {
        CDebug::Error("Could not create a new game window (glfwCreateWindow returned null).");
        throw std::runtime_error("GameWindow-CreateError");
    }

    auto handle = reinterpret_cast<GLFWwindow*>(native_handle);

    if(glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetWindowUserPointer(handle, this);
    glfwSetKeyCallback(handle, GLFW_KEY_CALLBACK);
    glfwSetCursorPosCallback(handle, GLFW_CURSOR_POS_CALLBACK);
    glfwSetFramebufferSizeCallback(handle, GLFW_FRAMEBUFFER_SIZE_CALLBACK);

    is_created = true;
    width = new_width;
    height = new_height;

    GameWindow::HandleEvent(this, WindowEvent::eCreated, nullptr);
}

void GameWindow::destroy()
{
    assert(is_created == true);
    glfwDestroyWindow((GLFWwindow*)native_handle);
}

bool GameWindow::is_active() const
{
    assert(is_created == true);
    return !glfwWindowShouldClose((GLFWwindow*)native_handle);
}

bool GameWindow::is_minimized() const
{
    assert(is_created == true);
    return is_inactive;
}

void GameWindow::update()
{
    assert(is_created == true);
    glfwSwapBuffers((GLFWwindow*)native_handle);
    glfwPollEvents();
}

void* GameWindow::handle()
{
    return native_handle;
}

void GameWindow::subscribe(WindowEvent event, WindowEventSubscriber handler)
{
    for(const auto& subscriber : subscribers)
    {
        auto pointer_a = subscriber.handler.target<void*>();
        auto pointer_b = handler.target<void*>();

        if(subscriber.event == event && pointer_a == pointer_b)
        {
            CDebug::Error("GameWindow event handler is already subscribed to the same event.");
            throw std::runtime_error("GameWindow-EventHandler-AlreadySubscribed");
        }
    }

    subscribers.push_back(SubscriberData
    {
        .event = event,
        .handler = handler
    });

    CDebug::Log("Handler subscribed to GameWindow event.");
}

void GameWindow::unsubscribe(WindowEvent event, WindowEventSubscriber handler)
{
    for(auto iterator = subscribers.begin(); iterator != subscribers.end(); iterator++)
    {
        auto pointer_a = iterator->handler.target<void*>();
        auto pointer_b = handler.target<void*>();

        if(iterator->event == event && pointer_a == pointer_b)
        {
            iterator = subscribers.erase(iterator);
            return;
        }
    }

    CDebug::Error("GameWindow.unsubscribe called but handler does not exist.");
    throw std::runtime_error("GameWindow-EventHandler-SubscriberNotFound");
}
