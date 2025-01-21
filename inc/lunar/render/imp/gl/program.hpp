#pragma once
#include <lunar/api.hpp>
#include <lunar/render/common.hpp>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <initializer_list>
#include <string_view>
#include <span>

namespace lunar::Render
{
	enum class LUNAR_API GpuProgramStageType : GLenum
	{
		eUnknown  = 0,
		eVertex   = GL_VERTEX_SHADER,
		eFragment = GL_FRAGMENT_SHADER,
		eGeometry = GL_GEOMETRY_SHADER,
		eCompute  = GL_COMPUTE_SHADER,
	};

	struct LUNAR_API GpuProgramStageData
	{
		GpuProgramStageType stageType  = GpuProgramStageType::eUnknown;
		size_t              bufferSize = 0;
		void*               pBuffer    = nullptr;
	};

	enum class LUNAR_API GpuProgramType
	{
		eUnknown  = 0,
		eGraphics,
		eCompute
	};

	class LUNAR_API GpuProgram_T
	{
	public:
		GpuProgram_T
		(
			RenderContext_T* context, 
			GpuProgramType programType,
			const std::initializer_list<GpuProgramStageData>& stages
		) noexcept;
		GpuProgram_T
		(
			RenderContext_T* context,
			GpuProgramType programType,
			const std::span<GpuProgramStageData>& stages
		) noexcept;
		GpuProgram_T(GpuProgram_T&&)      noexcept;
		GpuProgram_T(const GpuProgram_T&) noexcept = default;
		GpuProgram_T()                    noexcept = default;
		~GpuProgram_T();

		void           use();
		void           uniform(const std::string_view& name, const glm::vec4& v4);
		void           uniform(const std::string_view& name, const glm::mat4& m4);
		void           bind(const std::string_view& name, size_t location, GpuTexture texture);

		GpuProgram_T&  operator=(GpuProgram_T&&)      noexcept;
		GpuProgram_T&  operator=(const GpuProgram_T&) noexcept = default;

		GpuProgramType getProgramType() const;

		GLuint         glGetHandle();

	public:
		size_t         refCount    = 0;
	private:
		GLuint         handle      = 0;
		GpuProgramType programType = GpuProgramType::eUnknown;
	};

	using GpuProgram = Handle2<GpuProgram_T>;
}
