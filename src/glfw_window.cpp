#include "window.hpp"

#include <vector>
#include <stdexcept>
#include <functional>
#include <GLFW/glfw3.h>

struct GLFWLifeguard
{
    GLFWLifeguard() noexcept
    {
        int result;
        result = glfwInit();
        if(!result)
            std::exit(1);
    }

    ~GLFWLifeguard() noexcept
    {
        glfwTerminate();
    }
};

GLFWLifeguard GLFW_LIFEGUARD = {};

void GameWindow::create(WindowCreateInfo createInfo)
{

}
