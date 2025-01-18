#pragma once
#include <lunar/core/component.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/render/mesh.hpp>
#include <lunar/render/texture.hpp>
#include <lunar/api.hpp>

#include <glm/glm.hpp>

namespace Render
{
	class LUNAR_API Camera : public Core::Component
	{
	public:
		Camera() = default;

		void             update() override;
		glm::mat4        getViewMatrix() const;
		glm::mat4        getProjectionMatrix(int renderWidth, int renderHeight) const;
		const glm::vec3& getFront() const;
		const glm::vec3& getRight() const;

		void             drawDebugUI(Render::RenderContext&) override;

	private:
		glm::vec3      front      = { 0.f, 0.f, -1.f };
		glm::vec3      right      = { 1.f, 0.f, 0.f };
		glm::vec3      up         = { 0.f, 1.f, 0.f };
		glm::vec3      view       = {};
		float          fov        = 60.f;
	};

	class LUNAR_API MeshRenderer : public Core::Component
	{
	public:
		MeshRenderer() = default;

		glm::mat4 getModelMatrix() const;
		void      drawDebugUI(Render::RenderContext&) override;


		GraphicsShader shader = {};
		Mesh           mesh   = {};
	};

	class LUNAR_API Light : public Core::Component
	{
	public:
		void drawDebugUI(Render::RenderContext&) override;

		glm::vec3 color = { 1.f, 0.f, 0.f };
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
