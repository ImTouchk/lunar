#pragma once
#include <concepts>

namespace Render
{
	enum class RenderTargetType
	{
		eUnknown = 0,
		eWindow,
		eTexture
	};

	class RenderTarget
	{
	public:
		RenderTarget(RenderTargetType type) 
			: _renderTargetType(type) 
		{

		}

		RenderTargetType getType() const 
		{
			return _renderTargetType; 
		}

		virtual int getRenderWidth() const = 0;
		virtual int getRenderHeight() const = 0;

	protected:
		RenderTargetType _renderTargetType;
	};

	template<typename T>
	concept IsRenderTarget = std::derived_from<T, RenderTarget>;
}
