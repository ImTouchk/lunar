#include <lunar/render/context.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug/log.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace lunar::Render
{
	RenderContext_T::RenderContext_T() noexcept
	{
		/*
			If the vector exceeds capacity, all the window user pointers passed
			to GLFW will suddenly be invalidated and will probably make the
			program crash.

			I think 5 windows is a reasonable amount of windows to set as the maximum.
		*/
		windows.reserve(5);

		/* 
			Calling the function assures the static variable inside of it (i.e.: the global
			context) gets initialized.
		*/
		imp::GetGlobalRenderContext();

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
		textures.clear();
		buffers.clear();
		vertexArrays.clear();
		windows.clear();

		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteRenderbuffers(1, &renderBuffer);

		glfwDestroyWindow(headless);
	}
}
