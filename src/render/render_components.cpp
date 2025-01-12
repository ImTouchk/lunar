#include <lunar/render/render_components.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		auto model = glm::mat4(1.f);
		model      = glm::scale(glm::mat4(1.f), transform.scale);
		model      = glm::rotate(model, transform.rotation.z, { 0.f, 0.f, 1.f });
		model      = glm::rotate(model, transform.rotation.y, { 0.f, 1.f, 0.f });
		model      = glm::rotate(model, transform.rotation.x, { 1.f, 0.f, 0.f });
		model      = glm::translate(model, transform.position);
		return model;
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
