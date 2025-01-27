#include <lunar/render/components.hpp>
#include <lunar/debug/log.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace lunar
{
	Camera::Camera(GameObject parent)
		: Component_T(parent)
	{
	}

	void Camera::start()
	{
		DEBUG_LOG("Hello, world!");
	}

	void Camera::update()
	{
		static const glm::vec3 worldUp = { 0.f, 1.f, 0.f };

		const auto& transform = getTransform();

		auto& position  = transform.position;
		auto& rotation  = transform.rotation;

		auto  new_front = glm::vec3
		{
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
		const auto& transform   = getTransform();
		auto        scale       = glm::scale(glm::mat4(1.f), transform.scale);
		auto        translation = glm::translate(glm::mat4(1.f), transform.position);
		auto        rot_quat    = glm::quat(glm::radians(transform.rotation));
		auto        rotation    = glm::mat4(rot_quat);
		return translation * rotation * scale;
	}
}
