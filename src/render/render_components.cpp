#define GLM_ENABLE_EXPERIMENTAL
#include <lunar/render/render_components.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Render
{
	void Camera::update()
	{
		static const glm::vec3 worldUp = { 0.f, 1.f, 0.f };

		auto& transform = getTransform();
		auto& rotation  = transform.rotation;
		auto& position  = transform.position;

		glm::vec3 new_front = {
			glm::cos(glm::radians(rotation.x)) * glm::cos(glm::radians(rotation.y)),
			glm::sin(glm::radians(rotation.y)),
			glm::sin(glm::radians(rotation.x)) * glm::cos(glm::radians(rotation.y))
		};

		front = glm::normalize(new_front);
		right = glm::normalize(glm::cross(front, worldUp));
		up    = glm::normalize(glm::cross(right, front));
	}

	glm::mat4 Camera::getProjectionMatrix(int renderWidth, int renderHeight) const
	{
		return glm::perspective(
			glm::radians(fov),
			static_cast<float>(renderWidth) / static_cast<float>(renderHeight),
			.1f,
			1000000.f
		);
	}

	glm::mat4 Camera::getViewMatrix() const
	{
		const auto& transform = getTransform();
		return glm::lookAt(transform.position, transform.position + front, up);
	}

	glm::mat4 MeshRenderer::getModelMatrix() const
	{
		const auto& transform = getTransform();
		auto scale       = glm::scale(glm::mat4(1.f), transform.scale);
		auto translation = glm::translate(glm::mat4(1.f), transform.position);
		auto rot_quat    = glm::quat(glm::radians(transform.rotation));
		auto rotation    = glm::mat4(rot_quat);
		return translation * rotation * scale;
	}

	//MeshRenderer::MeshRenderer(const ShaderBuilder& shaderBuilder)
	//	: shader(shaderBuilder)
	//{

	//}

	//GraphicsShader& MeshRenderer::getShader()
	//{
	//	return shader;
	//}

	//const char* MeshRenderer::getType()
	//{
	//	return "core.meshRenderer";
	//}
}
