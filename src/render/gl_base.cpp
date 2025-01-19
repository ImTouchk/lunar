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
#include <lunar/file/text_file.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Render
{
	std::shared_ptr<RenderContext> CreateDefaultContext()
	{
		return std::make_shared<RenderContext>();
	}

	RenderContext::RenderContext()
	{
		init();
	}

	RenderContext::~RenderContext()
	{
		if (primitiveMeshes != nullptr)
			destroy();
	}

	GLuint RenderContext::glGetCaptureFramebuffer()
	{
		return captureFbo;
	}

	GLuint RenderContext::glGetCaptureRenderbuffer()
	{
		return captureRbo;
	}

	void RenderContext::init()
	{
#ifdef LUNAR_IMGUI
		if (imguiContext == nullptr)
		{
			IMGUI_CHECKVERSION();
			imguiContext = ImGui::CreateContext();
		}
#endif

		if (glfwGetCurrentContext() == NULL)
			return;
		
		glGenBuffers(2, &sceneUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, sceneUbo);
		glBufferData(GL_UNIFORM_BUFFER, 256, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_UNIFORM_BUFFER, lightsUbo);
		glBufferData(GL_UNIFORM_BUFFER, 512, NULL, GL_STATIC_DRAW);

		glGenFramebuffers(1, &captureFbo);
		glGenRenderbuffers(1, &captureRbo);

		auto shader_builder = GraphicsShaderBuilder();
		unlitShader   = new GraphicsShader();
		*unlitShader  = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/screen.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/unlit.frag"))
			.build();

		skyboxShader  = new GraphicsShader();
		*skyboxShader = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/skybox.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/skybox.frag"))
			.build();

		loadPrimitives();

		DEBUG_LOG("Context data loaded.");
	}


	void RenderContext::destroy()
	{
		delete unlitShader;
		delete skyboxShader;

		unloadPrimitives();
		glDeleteBuffers(2, &sceneUbo);
		glDeleteRenderbuffers(1, &captureRbo);
		glDeleteFramebuffers(1, &captureFbo);
	}

	void MeshBuilder::_glBuild()
	{
		glGenVertexArrays(1, &result._glVao);
		glBindVertexArray(result._glVao);

		glGenBuffers(2, &result._glVbo);

		glBindBuffer(GL_ARRAY_BUFFER, result._glVbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_x));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_y));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result._glEbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		result.indicesCount = indices.size();
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

#		ifdef LUNAR_IMGUI
		ImGui::SetCurrentContext(renderCtx->getImGuiContext());
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = 1.f;

		ImGui_ImplGlfw_InitForOpenGL(handle, true);
		ImGui_ImplOpenGL3_Init();
#		endif
	}
}