#pragma once
#include <lunar/core/component.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/api.hpp>

namespace Render
{
	class LUNAR_API MeshRenderer : public Core::Component
	{
	public:
		MeshRenderer() = default;

		void update() override;
		bool isUpdateable() override;

		GraphicsShader shader = {};
	};

	//class LUNAR_API MeshRenderer : public Core::Component
	//{
	//public:
	//	MeshRenderer(const ShaderBuilder& shaderBuilder);
	//	MeshRenderer() = default;

	//	const char* getType() override;
	//	bool isUpdateable() override;
	//	void update() override;

	//	GraphicsShader& getShader();

	//private:
	//	GraphicsShader shader;
	//};
}
