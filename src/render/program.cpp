#include <lunar/render/program.hpp>
#include <lunar/render/context.hpp>
#include <lunar/file/text_file.hpp>

namespace lunar::Render
{
	GpuProgramBuilder& GpuProgramBuilder::graphicsShader()
	{
		this->programType = GpuProgramType::eGraphics;
		return *this;
	}

	GpuProgramBuilder& GpuProgramBuilder::addStageFromSource(GpuProgramStageType type, const Fs::Path& path)
	{
		auto file = Fs::TextFile(path);
		auto stage = this->stages.size();

		this->stageBytes[stage] = std::move(file.content);
		this->stages.emplace_back(GpuProgramStageData
		{
			.stageType  = type,
			.bufferSize = this->stageBytes[stage].size(),
			.pBuffer    = this->stageBytes[stage].data()
		});

		return *this;
	}

	GpuProgramBuilder& GpuProgramBuilder::addVertexSource(const Fs::Path& path)
	{
		return addStageFromSource(GpuProgramStageType::eVertex, path);
	}

	GpuProgramBuilder& GpuProgramBuilder::addFragmentSource(const Fs::Path& path)
	{
		return addStageFromSource(GpuProgramStageType::eFragment, path);
	}

	GpuProgram GpuProgramBuilder::build(RenderContext context)
	{
		return build(context.get());
	}
	
	GpuProgram GpuProgramBuilder::build(RenderContext_T* context)
	{
		return context->createProgram(programType, stages);
	}

	GpuProgram RenderContext_T::getProgram(GpuDefaultPrograms program)
	{
		return getProgram((size_t)program);
	}

	GpuProgram RenderContext_T::getProgram(size_t number)
	{
		return RefHandle<GpuProgram_T>(programs, number);
	}
}
