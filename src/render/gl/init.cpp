#include <lunar/render/context.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug/log.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace lunar::Render
{
	RenderContext_T::RenderContext_T() noexcept
	{
		IMGUI_CHECKVERSION();
		imguiContext = ImGui::CreateContext();
		
		loadDefaultPrograms();
		loadDefaultMeshes();

		glGenFramebuffers(1, &frameBuffer);
		glGenRenderbuffers(1, &renderBuffer);
	}

	RenderContext_T::~RenderContext_T() noexcept
	{
		cubemaps.clear();
		meshes.clear();
		programs.clear();
		buffers.clear();
		vertexArrays.clear();

		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteRenderbuffers(1, &renderBuffer);
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
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		return;

//#		ifdef LUNAR_IMGUI
//		ImGui::SetCurrentContext(renderCtx->getImGuiContext());
//		ImGui::StyleColorsDark();
//
//		ImGuiIO& io = ImGui::GetIO();
//		io.FontGlobalScale = 1.f;
//
//		ImGui_ImplGlfw_InitForOpenGL(handle, true);
//		ImGui_ImplOpenGL3_Init();
//#		endif
	}
}
