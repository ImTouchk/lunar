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

namespace Render
{
	std::shared_ptr<RenderContext> CreateDefaultContext()
	{
		return std::make_shared<GLContext>();
	}

	GLContext::GLContext()
	{
		init();
	}

	void GLContext::init()
	{
#ifdef LUNAR_IMGUI
		IMGUI_CHECKVERSION();
		_imguiContext = ImGui::CreateContext();
#endif
	}

	void GLContext::destroy()
	{
		glDeleteBuffers(1, &ubo);
	}

	void GLContext::draw(Core::Scene& scene, Camera& camera, RenderTarget* target)
	{
		if (target->getType() != RenderTargetType::eWindow)
			return; // TODO

		auto& target_window = *static_cast<Render::Window*>(target);
		if (target_window.isMinimized())
			return;

		if (ubo == 0)
		{
			glGenBuffers(1, &ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneGpuData), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
			DEBUG_LOG("Created uniform buffer object.");
		}

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glClearColor(1.0f, 1.f, 1.f, 1.f);

#		ifdef LUNAR_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#		endif

		auto ubo_data = SceneGpuData
		{
			.projection = camera.getProjectionMatrix(1280, 720),
			.view       = camera.getViewMatrix(),
			.model      = {},
		};

		for (auto& object : scene.getGameObjects())
		{
			MeshRenderer* mesh_renderer = object.getComponent<MeshRenderer>();
			if (mesh_renderer == nullptr)
				continue;

			const auto& transform = object.getTransform();
			auto&       mesh      = mesh_renderer->mesh;
			auto&       shader    = mesh_renderer->shader;
			auto&       material  = mesh.material;

			if (shader._glHandle != 0)
				glUseProgram(shader._glHandle);
			// TODO: else ? 

			ubo_data.model = mesh_renderer->getModelMatrix();
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneGpuData), &ubo_data);

			if (material.colorMap._glHandle != 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, material.colorMap._glHandle);
			}
			// TODO: else ?

			glBindVertexArray(mesh._glVao);
			glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, 0);
		}


#		ifdef LUNAR_IMGUI
		ImGui::ShowDemoWindow();
		scene.renderUpdate(*this);

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#		endif

		glfwSwapBuffers(target_window.handle);
	}

	bool TextureBuilder::_glBuild()
	{
		auto& handle = result._glHandle;
	
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		GLint min_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
		GLint mag_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST : GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)dstFormat, width, height, 0, (GLint)srcFormat, GL_UNSIGNED_BYTE, rawBytes);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		return true;
	}

	GraphicsShader GraphicsShaderBuilder::_glBuild()
	{
		auto shader = GraphicsShader();
		
		auto compile_shader = [](GLenum type, const Fs::Path& path) -> GLint
		{
			auto file = Fs::TextFile(path);
			const char* data = file.content.data();

			GLint data_size;
			data_size = file.content.size();

			GLint shader;
			shader = glCreateShader(type);
			glShaderSource(shader, 1, &data, &data_size);
			glCompileShader(shader);

			GLint success;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				GLchar error_message[512];
				glGetShaderInfoLog(shader, 512, nullptr, error_message);
				DEBUG_ERROR("Failed to compile shader: {}", error_message);
				shader = -1;
			}

			return shader;
		};

		GLint vertex_shader   = compile_shader(GL_VERTEX_SHADER, vertexPath);
		GLint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragmentPath);
		if (vertex_shader == -1 || fragment_shader == -1)
		{
			return shader;
		}

		shader._glHandle = glCreateProgram();
		glAttachShader(shader._glHandle, vertex_shader);
		glAttachShader(shader._glHandle, fragment_shader);
		glLinkProgram(shader._glHandle);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		GLint success;
		glGetProgramiv(shader._glHandle, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar error_message[512];
			glGetProgramInfoLog(shader._glHandle, 512, nullptr, error_message);
			DEBUG_ERROR("Failed to link shader program: {}", error_message);
		}

		return shader;
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
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_x));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_y));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

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
		glEnable(GL_DEPTH_TEST);

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