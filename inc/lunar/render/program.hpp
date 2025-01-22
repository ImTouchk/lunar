#pragma once
#include <lunar/api.hpp>
#include <lunar/render/imp.hpp>
#include <lunar/file/filesystem.hpp>

#include <string>
#include <vector>
#include <array>

namespace lunar::Render
{
	enum class LUNAR_API GpuDefaultPrograms : size_t
	{
		eTextureUnlit         = 0,
		eEquirectToCubemap    = 1,
		eIrradianceMapBuilder = 2,
		ePrefilterMapBuilder  = 3,
		eBrdfBuilder          = 4,
		eSkyboxShader         = 5,
		eBasicPbrShader       = 6
	};

	struct LUNAR_API GpuProgramBuilder
	{
	public:
		GpuProgramBuilder()  noexcept = default;
		~GpuProgramBuilder() noexcept = default;
		
		GpuProgramBuilder& graphicsShader();
		GpuProgramBuilder& addVertexSource(const Fs::Path& path);
		GpuProgramBuilder& addFragmentSource(const Fs::Path& path);
		GpuProgramBuilder& addStageFromSource(GpuProgramStageType, const Fs::Path&);
		GpuProgram         build(RenderContext_T* context);
		GpuProgram         build(RenderContext context);

	private:
		GpuProgramType                   programType = GpuProgramType::eUnknown;
		std::vector<GpuProgramStageData> stages      = {};
		std::array<std::string, 4>       stageBytes  = {};
	};
}
