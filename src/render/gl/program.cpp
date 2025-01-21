#include <lunar/render/imp/gl/program.hpp>
#include <lunar/render/imp/gl/texture.hpp>
#include <lunar/utils/collections.hpp>
#include <lunar/debug/assert.hpp>
#include <lunar/debug/log.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace lunar::Render
{
	GLint CompileShader(const GpuProgramStageData& stageData)
	{
		const char* source_buf = (const char*)stageData.pBuffer;
		int         source_len = static_cast<int>(stageData.bufferSize);
		GLint       handle     = glCreateShader((GLenum)stageData.stageType);
		glShaderSource(handle, 1, &source_buf, &source_len);
		glCompileShader(handle);

		GLint success;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar error_message[512];
			glGetShaderInfoLog(handle, 512, nullptr, error_message);
			DEBUG_ERROR("Failed to compile shader: {}", error_message);
			return -1;
		}

		return handle;
	}

	GpuProgram_T::GpuProgram_T
	(
		RenderContext_T* context,
		GpuProgramType programType,
		const std::span<GpuProgramStageData>& stages
	) noexcept : programType(programType)
	{
		handle = glCreateProgram();

		auto shaders = vector<GLint>();
		shaders.reserve(stages.size());
		for (const auto& stage : stages)
		{
			GLint shader = CompileShader(stage);
			shaders.emplace_back(shader);
			glAttachShader(handle, shader);
		}

		glLinkProgram(handle);

		GLint success;
		glGetProgramiv(handle, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar error_message[512];
			glGetProgramInfoLog(handle, 512, nullptr, error_message);
			DEBUG_ERROR("Failed to link shader program: {}", error_message);
		}

		for (auto& shader : shaders)
			glDeleteShader(shader);
	}

	GpuProgram_T::GpuProgram_T
	(
		RenderContext_T* context,
		GpuProgramType programType,
		const std::initializer_list<GpuProgramStageData>& stages
	) noexcept : programType(programType)
	{
		handle = glCreateProgram();

		auto shaders = vector<GLint>();
		shaders.reserve(stages.size());
		for (const auto& stage : stages)
		{
			GLint shader = CompileShader(stage);
			shaders.emplace_back(shader);
			glAttachShader(handle, shader);
		}

		glLinkProgram(handle);

		GLint success;
		glGetProgramiv(handle, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar error_message[512];
			glGetProgramInfoLog(handle, 512, nullptr, error_message);
			DEBUG_ERROR("Failed to link shader program: {}", error_message);
		}

		for (auto& shader : shaders)
			glDeleteShader(shader);
	}

	GpuProgram_T::~GpuProgram_T()
	{
		if (handle != 0 && refCount <= 0)
			glDeleteProgram(handle);
	}

	void GpuProgram_T::use()
	{
		glUseProgram(handle);
	}

	void GpuProgram_T::uniform(const std::string_view& name, const glm::vec4& v4)
	{
		GLint loc = glGetUniformLocation(handle, name.data());
		glUniform3fv(loc, 1, &v4.r);
	}

	void GpuProgram_T::uniform(const std::string_view& name, const glm::mat4& m4)
	{
		GLint loc = glGetUniformLocation(handle, name.data());
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m4));
	}

	void GpuProgram_T::uniform(const std::string_view& name, const float f)
	{
		GLint loc = glGetUniformLocation(handle, name.data());
		glUniform1f(loc, f);
	}

	void GpuProgram_T::bind(const std::string_view& name, size_t location, GpuTexture texture)
	{
		GLint name_loc = glGetUniformLocation(handle, name.data());
		glUniform1i(name_loc, location);
		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture((GLenum)texture->getType(), texture->glGetHandle());
	}

	GLuint GpuProgram_T::glGetHandle()
	{
		return handle;
	}

	/*
		Move operators
	*/

	GpuProgram_T::GpuProgram_T(GpuProgram_T&& other) noexcept
	{
		this->handle      = std::move(other.handle);
		this->programType = std::move(other.programType);

		other.handle = 0;
	}

	GpuProgram_T& GpuProgram_T::operator=(GpuProgram_T&& other) noexcept
	{
		this->handle      = std::move(other.handle);
		this->programType = std::move(other.programType);

		other.handle = 0;
		return *this;
	}
}
