#pragma once
#include <lunar/core/component.hpp>
#include <lunar/render/imp.hpp>
#include <lunar/render/mesh.hpp>

namespace lunar
{
	class LUNAR_API Camera : public Component_T
	{
	public:
		Camera(GameObject parent);
		Camera() = default;

		void             start()  override;
		void             update() override;
		glm::mat4        getViewMatrix() const;
		glm::mat4        getProjectionMatrix(int renderWidth, int renderHeight) const;

	public:
		glm::vec3 front = { 0.f, 0.f, -1.f };
		glm::vec3 right = { 1.f, 0.f, 0.f };
		glm::vec3 up    = { 0.f, 1.f, 0.f };
		glm::vec3 view  = {};
		float     fov   = 60.f;
	};

	class LUNAR_API MeshRenderer : public Component_T
	{
	public:
		MeshRenderer() = default;

		glm::mat4 getModelMatrix() const;

		Render::GpuMesh    mesh    = nullptr;
		Render::GpuProgram program = nullptr;
	};
}
