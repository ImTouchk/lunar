#pragma once
#ifdef GLFW_WINDOW_BACKEND
#	include "glfw_keys.hpp"
#endif

namespace CInput
{
	void Initialize();
	void Destroy();

	bool IsKeyPressed(CKey key);
}
