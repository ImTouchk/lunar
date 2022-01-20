#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include "utils/debug.hpp"

int main()
{
#   ifdef WIN32
    // Enables color formatting for the CLion terminal.
    system(("chcp " + std::to_string(CP_UTF8)).c_str());
#   endif

    return 0;
}
