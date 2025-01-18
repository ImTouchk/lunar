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
		auto& position  = transform.position;
		auto& rotation  = transform.rotation;

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

	const glm::vec3& Camera::getFront() const
	{
		return front;
	}

	const glm::vec3& Camera::getRight() const
	{
		return right;
	}

	void Camera::drawDebugUI(RenderContext& context)
	{
		ImGui::SetCurrentContext(context.getImGuiContext());
		ImGui::InputFloat3("Front", &front.x);
		ImGui::InputFloat3("Right", &right.x);
		ImGui::InputFloat3("Up",    &up.x);
		ImGui::InputFloat("FOV",    &fov, 1.f);
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

	void MeshRenderer::drawDebugUI(RenderContext& context)
	{
		ImGui::SetCurrentContext(context.getImGuiContext());
		ImGui::SeparatorText("Mesh");
		ImGui::Text("Triangles: %d", mesh.indicesCount / 3);
		ImGui::Text("Indices: %d", mesh.indicesCount);
		ImGui::SeparatorText("Material");
		ImGui::BulletText("Color map");
		ImGui::Image(mesh.material.albedo._glHandle, { 128, 128 });
		ImGui::DragFloat("Roughness", &mesh.material.roughness, 0.01f, 0.01f, 1.f);
		ImGui::DragFloat("Metallic", &mesh.material.metallic, 0.01f, 0.01f, 1.f);
		ImGui::DragFloat("AO", &mesh.material.ao, 0.01f, 0.01f, 1.f);
	}

	void Light::drawDebugUI(RenderContext& context)
	{
		ImGui::SetCurrentContext(context.getImGuiContext());
		ImGui::ColorEdit3("Color", &color[0]);
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
