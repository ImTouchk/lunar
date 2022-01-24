#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#include <stdexcept>

#include "utils/debug.hpp"
#include "render/window.hpp"

int main()
{
#   ifdef WIN32
    // Enables color formatting for the CLion terminal.
    system(("chcp " + std::to_string(CP_UTF8)).c_str());
#   endif

    try
    {
        GameWindow game_window;
        game_window.create(WindowCreateInfo
        {
           .isResizable = true,
           .isFullscreen = false,
           .width = 1280,
           .height = 720,
           .pTitle = "Hello, world!"
        });

        while(game_window.is_active())
        {
            if(!game_window.is_minimized())
            {
                // do rendering
            }

            game_window.update();
        }

        game_window.destroy();
    } catch(std::runtime_error& error)
    {
        CDebug::Error("Uncaught exception: {}. Program halted.", error.what());
        return -1;
    }

    return 0;
}
