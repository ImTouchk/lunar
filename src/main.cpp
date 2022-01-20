#include <GLFW/glfw3.h>

int main()
{
    GLFWwindow* pWindow;
    if(!glfwInit())
        return -1;

    pWindow = glfwCreateWindow(800, 600, "Hello, world!", nullptr, nullptr);
    if(!pWindow)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(pWindow);
    while(!glfwWindowShouldClose(pWindow))
    {
        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
