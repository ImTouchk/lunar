#include <lunar/render/render_context.hpp>
#include <lunar/render/internal/render_gl.hpp>
#include <lunar/render/window.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_glfw.h>
#	include <imgui_impl_opengl3.h>
#endif

namespace Render
{
	std::shared_ptr<RenderContext> CreateDefaultContext()
	{
		return std::make_shared<GLContext>();
	}

	void GLContext::init()
	{
#		ifdef LUNAR_IMGUI
		ImGui_ImplOpenGL3_Init();
#		endif
	}

	void GLContext::destroy()
	{

	}

	void GLContext::draw(Core::Scene& scene, RenderTarget* target)
	{
		if (target->getType() != RenderTargetType::eWindow)
			return; // TODO

		auto& target_window = *static_cast<Render::Window*>(target);
		if (target_window.isMinimized())
			return;

		glClearColor(1.0f, 1.f, 1.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(target_window.handle);
	}

	void Window::_glInitialize()
	{
		glfwMakeContextCurrent(handle);
		int version = gladLoadGL(glfwGetProcAddress);
		if (version == 0)
			DEBUG_ERROR("Failed to initialize OpenGL context.");
		else
			DEBUG_LOG("Loaded OpenGL version {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

		int w, h;
		glfwGetFramebufferSize(handle, &w, &h);

		glViewport(0, 0, w, h);
		glEnable(GL_DEPTH_TEST);
	}
}