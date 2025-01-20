#pragma once
#include <lunar/core/handle.hpp>
#include <lunar/api.hpp>

namespace lunar
{
	class LUNAR_API Scene_T;
	class LUNAR_API GameObject_T;
	class LUNAR_API Component_T;
	LUNAR_HANDLE(Scene);
	LUNAR_HANDLE(GameObject);
	LUNAR_SHARED_HANDLE(Component);
}
