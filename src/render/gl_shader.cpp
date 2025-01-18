#include <lunar/render/internal/render_gl.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/render/texture.hpp>
#include <lunar/render/cubemap.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/file/text_file.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Render
{
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
					DEBUG_ERROR("While compiling '{}'", path.generic_string());
					shader = -1;
				}

				return shader;
			};

		GLint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertexPath);
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

		GLint uniform_count = 0;
		glGetProgramiv(shader._glHandle, GL_ACTIVE_UNIFORMS, &uniform_count);

		shader.uniforms = std::vector<GraphicsShader::LayoutInfo>(uniform_count);

		constexpr GLsizei BUF_SIZE = 128;
		GLchar  u_name[BUF_SIZE];
		GLsizei u_name_len;

		for (size_t i = 0; i < uniform_count; i++)
		{
			GLint  _size;
			GLenum _type;
			glGetActiveUniform(shader._glHandle, (GLuint)i, BUF_SIZE, &u_name_len, &_size, &_type, u_name);
			
			auto name_view = std::string_view(u_name, u_name_len);
			auto name_hash = Lunar::imp::fnv1a_hash(name_view);

			auto& layout_info = shader.uniforms[i];
			layout_info.location = glGetUniformLocation(shader._glHandle, u_name);
			layout_info.nameHash = name_hash;
		}

		return shader;
	}

	void GraphicsShader::use()
	{
		glUseProgram(_glHandle);
	}

	int GraphicsShader::getUniformLocation(const std::string_view& name)
	{
		size_t name_hash = Lunar::imp::fnv1a_hash(name);
		for (size_t i = 0; i < uniforms.size(); i++)
			if (name_hash == uniforms[i].nameHash)
				return uniforms[i].location;

		return -1;
	}

	void GraphicsShader::uniform(const std::string_view& name, const glm::vec2& v2)
	{
		int location = getUniformLocation(name);
		glUniform2fv(location, 1, &v2.r);
	}

	void GraphicsShader::uniform(const std::string_view& name, const glm::vec3& v3)
	{
		int location = getUniformLocation(name);
		glUniform3fv(location, 1, &v3.r);
	}

	void GraphicsShader::uniform(const std::string_view& name, const glm::vec4& v4)
	{
		int location = getUniformLocation(name);
		glUniform4fv(location, 1, &v4.r);
	}

	void GraphicsShader::uniform(const std::string_view& name, const glm::mat4& m4)
	{
		int location = getUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m4));
	}

	void GraphicsShader::uniform(const std::string_view& name, const float f)
	{
		int location = getUniformLocation(name);
		glUniform1f(location, f);
	}

	void GraphicsShader::uniform(const std::string_view& name, const int i)
	{
		int location = getUniformLocation(name);
		if (location == -1)
			return;

		glUniform1i(location, i);
	}

	void GraphicsShader::bind(const std::string_view& name, size_t location, const Texture& texture)
	{
		int name_loc = getUniformLocation(name);
		if (name_loc == -1)
			return;

		glUniform1i(name_loc, location);
		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture(GL_TEXTURE_2D, texture._glHandle);
	}

	void GraphicsShader::bind(size_t location, const Cubemap& cubemap)
	{
		uniform("environmentMap", (int)location);
		uniform("irradianceMap",  (int)location + 1);
		uniform("prefilterMap",   (int)location + 2);
		uniform("brdf",           (int)location + 3);

		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap._glHandle);

		glActiveTexture(GL_TEXTURE1 + location);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap._glIrradiance);

		glActiveTexture(GL_TEXTURE2 + location);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap._glPrefilter);

		glActiveTexture(GL_TEXTURE3 + location);
		glBindTexture(GL_TEXTURE_2D, cubemap._glBrdf);
	}
}
