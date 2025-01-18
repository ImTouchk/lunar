#pragma once
#include <concepts>

namespace Render
{
	class RenderTarget
	{
	public:
		virtual int getRenderWidth() const = 0;
		virtual int getRenderHeight() const = 0;
	};

	template<typename T>
	concept IsRenderTarget = std::derived_from<T, RenderTarget>;

	template<typename T, typename U> requires IsRenderTarget<T> && IsRenderTarget<U>
	inline bool IsRenderTargetOfType(U* object)
	{
		return typeid(*object).hash_code() == typeid(T).hash_code();
	}
}
