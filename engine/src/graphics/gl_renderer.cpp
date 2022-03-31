#include "gl_renderer.hpp"
#include "render/renderer.hpp"
#include "render/window.hpp"

#ifdef GLFW_WINDOW_BACKEND
#	include <GLFW/glfw3.h>
#endif

#include <glad/glad.h>
#include <vector>
#include <any>

void CRenderer::create(RendererCreateInfo&& createInfo)
{
	window_handle = createInfo.pWindow;
	backend_data = GL::RendererInternalData();

	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);
	internal_data->meshes = {};
	internal_data->shaders = {};

#ifdef GLFW_WINDOW_BACKEND
	glfwMakeContextCurrent((GLFWwindow*)window_handle->get_handle());
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Renderer-OpenGL-LoadFail");
	}
#else
	static_assert(false, "Not implemented");
#endif

	glEnable(GL_DEPTH_TEST);

	window_handle->subscribe(WindowEvent::eResized, [](void*, const std::any& eventData)
	{
		auto new_size = std::any_cast<std::pair<int, int>>(eventData);
		glViewport(0, 0, new_size.first, new_size.second);
	});
}

void CRenderer::destroy()
{
	
}

void CRenderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.f, 0.f, 0.f, 0.f);
}

std::vector<ShaderWrapper> CRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count)
{
	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);
	return {};
}
