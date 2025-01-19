#include <glad/gl.h>
#include <GLFW/glfw3.h>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_glfw.h>
#	include <imgui_impl_opengl3.h>
#endif

#include <lunar/render/render_context.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/internal/render_gl.hpp>
#include <lunar/render/window.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/render/mesh.hpp>
#include <lunar/render/texture.hpp>
#include <lunar/render/cubemap.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Render
{
	void GLContext::begin(RenderTarget* target)
	{
		currentTarget = target;

		if (sceneUbo == 0)
			init();

		int width  = target->getRenderWidth();
		int height = target->getRenderHeight();

		if (IsRenderTargetOfType<Texture>(target))
		{
			auto& texture = static_cast<Texture&>(*target);

			glBindFramebuffer(GL_FRAMEBUFFER, captureFbo);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.glGetHandle(), 0);
		}

		glViewport(0, 0, width, height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

#		ifdef LUNAR_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#		endif
	}

	void GLContext::clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void GLContext::end()
	{
#		ifdef LUNAR_IMGUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#		endif

		if (IsRenderTargetOfType<Texture>(currentTarget))
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		if (IsRenderTargetOfType<Window>(currentTarget))
		{
			auto& target_window = *static_cast<Render::Window*>(currentTarget);
			glfwSwapBuffers(target_window.handle);
		}
	}

	void GLContext::draw(const Cubemap& cubemap)
	{
		const auto& cube       = getPrimitiveMesh(MeshPrimitive::eCube);
		auto        view       = camera->getViewMatrix();
		auto        projection = camera->getProjectionMatrix(
			currentTarget->getRenderWidth(),
			currentTarget->getRenderHeight()
		);

		skyboxShader->use();
		skyboxShader->uniform("view", view);
		skyboxShader->uniform("projection", projection);
		skyboxShader->bind(0, cubemap);
		draw(cube);

		this->cubemap = &cubemap;
	}

	void GLContext::draw(const Mesh& mesh, const GraphicsShader& shader, const Material& material)
	{

	}

	void GLContext::draw(const Mesh& mesh)
	{
		glBindVertexArray(mesh._glVao);
		glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, 0);
	}

	void GLContext::draw(const Texture& texture)
	{
		const auto& quad = getPrimitiveMesh(MeshPrimitive::eQuad);

		unlitShader->use();
		unlitShader->bind("source", 0, texture);
		draw(quad);
	}

	void GLContext::draw(Core::Scene& scene)
	{
		scene.renderUpdate(*this);

		auto scene_data = SceneGpuData
		{
			.view       = camera->getViewMatrix(),
			.projection = camera->getProjectionMatrix(
							currentTarget->getRenderWidth(), 
							currentTarget->getRenderHeight()
						),
			.model      = {},
			.cameraPos  = camera->getTransform().position
		};

		auto light_data = getSceneLightData(scene);

		for (auto& object : scene.getGameObjects())
		{
			MeshRenderer* mesh_renderer = object.getComponent<MeshRenderer>();
			if (mesh_renderer == nullptr)
				continue;

			const auto& transform = object.getTransform();
			auto&       mesh      = mesh_renderer->mesh;
			auto&       shader    = mesh_renderer->shader;
			auto&       material  = mesh.material;

			scene_data.model = mesh_renderer->getModelMatrix();

			light_data.ao        = material.ao;
			light_data.metallic  = material.roughness;
			light_data.roughness = material.roughness;

			shader.use();

			glBindBuffer(GL_UNIFORM_BUFFER, sceneUbo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 256, &scene_data);

			glBindBuffer(GL_UNIFORM_BUFFER, lightsUbo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, 512, &light_data);

			glBindBufferBase(GL_UNIFORM_BUFFER, 0, sceneUbo);
			glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightsUbo);

			shader.bind("albedoMap", 0, material.albedo);
			shader.bind(1, *cubemap);

			// TODO: else ?

			draw(mesh);
		}
	}
}
