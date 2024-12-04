#pragma once
#include <lunar/utils/identifiable.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/api.hpp>
#include <string_view>
#include <vector>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/vk_pipeline.hpp>
#endif


namespace Render
{
	class LUNAR_API GraphicsShader : public Identifiable
	{
	public:
		GraphicsShader() = default;
		~GraphicsShader() = default;

	private:
#		ifdef LUNAR_VULKAN
		vk::Pipeline       _vkPipeline = VK_NULL_HANDLE;
		vk::PipelineLayout _vkLayout   = VK_NULL_HANDLE;
		friend class VulkanContext;
#		endif

		friend struct GraphicsShaderBuilder;
	};

	struct LUNAR_API GraphicsShaderBuilder
	{
	public:
		GraphicsShaderBuilder() = default;
		~GraphicsShaderBuilder() = default;

		GraphicsShaderBuilder& useRenderContext(std::shared_ptr<RenderContext>& ctx);
		GraphicsShaderBuilder& fromVertexSourceFile(const Fs::Path& path);
		GraphicsShaderBuilder& fromFragmentSourceFile(const Fs::Path& path);
		GraphicsShader build();

	private:
		std::shared_ptr<RenderContext> context       = nullptr;
		Fs::Path                       vertexPath    = {};
		Fs::Path                       fragmentPath  = {};
	};

//	enum class LUNAR_API ShaderVariableValueT
//	{
//		eUnknown = 0,
//		eFloat,
//		eFloat3,
//		eFloat4,
//		eFloat4x4,
//	};
//
//	struct LUNAR_API ShaderBuilder
//	{
//	public:
//		ShaderBuilder() = default;
//		~ShaderBuilder() = default;
//		
//		ShaderBuilder& renderContext(std::shared_ptr<RenderContext>& context);
//		ShaderBuilder& fromVertexBinaryFile(const std::string_view& name);
//		ShaderBuilder& fromFragmentBinaryFile(const std::string_view& name);
//		ShaderBuilder& uniformVariable(const std::string_view& name, ShaderVariableValueT type);
//
//	private:
//		enum class ShaderVariableType
//		{
//			eUnknown = 0,
//			eUniform,
//			eVertexInput
//		};
//
//		struct
//		{
//			ShaderVariableType type;
//			ShaderVariableValueT valueType;
//			std::string_view name;
//		} _variables[20];
//		size_t _variableCount = 0;
//
//		std::shared_ptr<RenderContext> _renderCtx;
//		std::string _vertCode;
//		std::string _fragCode;
//		
//		friend class GraphicsShader;
//	};
//
//	class LUNAR_API GraphicsShader : public Identifiable
//	{
//	public:
//		GraphicsShader(const ShaderBuilder& builder);
//		GraphicsShader();
//		~GraphicsShader();
//		
//		void init(const ShaderBuilder& builder);
//		void destroy();
//
//#	ifdef LUNAR_VULKAN
//		vk::Pipeline& getVkPipeline();
//		vk::PipelineLayout& getVkLayout();
//#	endif
//
//	private:
//		bool initialized;
//		std::shared_ptr<RenderContext> renderCtx;
//
//#	ifdef LUNAR_VULKAN
//		void _vkInit(const ShaderBuilder& builder);
//		void _vkDestroy();
//
//		vk::PipelineLayout _vkPipelineLayout;
//		vk::Pipeline _vkPipeline;
//#	endif
//	};
}
