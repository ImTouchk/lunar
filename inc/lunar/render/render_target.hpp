#pragma once

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

	protected:
		RenderTargetType _renderTargetType;
	};
}
