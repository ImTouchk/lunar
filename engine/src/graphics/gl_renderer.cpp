#include "gl_renderer.hpp"
#include "render/renderer.hpp"
#include "render/window.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "utils/identifier.hpp"

#ifdef GLFW_WINDOW_BACKEND
#	include <GLFW/glfw3.h>
#endif

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>
#include <any>

ShaderWrapper::ShaderWrapper(Identifier handle)
	: identifier(handle)
{
}

void ShaderWrapper::use_texture(TextureWrapper& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture.handle());
}

unsigned ShaderWrapper::handle() const
{
	return identifier;
}

MeshWrapper::MeshWrapper(GL::RendererInternalData& internalData, Identifier handle)
	: identifier(handle),
	rendererData(internalData)
{
}

void MeshWrapper::set_transform(glm::mat4&& transform)
{
	find_by_identifier_safe(rendererData.meshes, identifier)
		.transform = transform;
}

TextureWrapper::TextureWrapper(GL::RendererInternalData& internalData, Identifier handle)
	: identifier(handle),
	renderer_data(internalData)
{
}

unsigned TextureWrapper::handle() const
{
	return identifier;
}

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
	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);
	auto& meshes        = internal_data->meshes;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.f, 0.f);

	for(size_t i = 0; i < meshes.size(); i++)
	{
		auto& mesh   = meshes[i];
		auto  shader = mesh.shader.handle();

		glUseProgram(shader);
		glUniformMatrix4fv(glGetUniformLocation(shader, "transform"), 1, GL_FALSE, glm::value_ptr(mesh.transform));

		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indices, GL_UNSIGNED_SHORT, 0);
	}

	glBindVertexArray(0);
}

std::vector<ShaderWrapper> CRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count)
{
	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);
	auto  results       = std::vector<ShaderWrapper>();

	auto compile_shader = [](GLenum type, const std::vector<char>& code) -> GLint
	{
		const char* data = code.data();

		GLint data_size;
		data_size = code.size();

		GLint shader;
		shader = glCreateShader(type);
		glShaderSource(shader, 1, &data, &data_size);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			GLchar error_message[512];
			glGetShaderInfoLog(shader, 512, nullptr, error_message);
			CDebug::Error("OpenGL Renderer | Shader code compilation error: {}", error_message);
			shader = -1;
		}

		return shader;
	};

	for(auto i : range(0, count - 1))
	{
		GLuint vertex_shader;
		GLuint fragment_shader;

		vertex_shader = compile_shader(GL_VERTEX_SHADER, pCreateInfos[i].vertexCode);
		fragment_shader = compile_shader(GL_FRAGMENT_SHADER, pCreateInfos[i].fragmentCode);

		if(vertex_shader == -1 || fragment_shader == -1)
		{
			CDebug::Error("OpenGL Renderer | Skipping shader {}...", i);
			continue;
		}

		GLint id;
		id = glCreateProgram();
		glAttachShader(id, vertex_shader);
		glAttachShader(id, fragment_shader);
		glLinkProgram(id);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		GLint success;
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if(!success)
		{
			GLchar error_message[512];
			glGetProgramInfoLog(id, 512, nullptr, error_message);
			CDebug::Error("OpenGL Renderer | Shader link error: {}", error_message);
			CDebug::Error("OpenGL Renderer | Skipping shader {}...", i);
			continue;
		}

		internal_data->shaders.push_back(static_cast<GL::Shader>(id));
		results.push_back(ShaderWrapper(static_cast<Identifier>(id)));
	}

	return results;
}

MeshWrapper CRenderer::create_object(MeshCreateInfo&& meshCreateInfo)
{
	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);

	GL::DrawableObjectData object_data =
	{
		.identifier = get_unique_number(),
		.type       = meshCreateInfo.type,
		.shader     = meshCreateInfo.shader,
		.indices    = static_cast<GLuint>(meshCreateInfo.indices.size()),
		.vao        = 0,
		.vbo        = 0,
		.ebo        = 0,
		.transform  = glm::mat4(1.f)
	};

	glGenBuffers(2, &object_data.vbo);

	glGenVertexArrays(1, &object_data.vao);
	glBindVertexArray(object_data.vao);

	glBindBuffer(GL_ARRAY_BUFFER, object_data.vbo);
	glBufferData(GL_ARRAY_BUFFER, meshCreateInfo.vertices.size() * sizeof(Vertex), meshCreateInfo.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_data.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshCreateInfo.indices.size() * sizeof(Index), meshCreateInfo.indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_uv));

	glBindVertexArray(0);

	auto identifier = object_data.identifier;

	internal_data->meshes.push_back(std::move(object_data));
	return MeshWrapper { *internal_data, identifier };
}

TextureWrapper CRenderer::create_texture(TextureCreateInfo&& createInfo)
{
	auto* internal_data = std::any_cast<GL::RendererInternalData>(&backend_data);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, createInfo.width, createInfo.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, createInfo.pData);
	glGenerateMipmap(GL_TEXTURE_2D);

	return TextureWrapper { *internal_data, static_cast<Identifier>(texture) };
}
