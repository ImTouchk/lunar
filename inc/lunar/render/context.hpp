#pragma once
#include <lunar/api.hpp>
#include <lunar/utils/collections.hpp>
#include <lunar/render/common.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/imp.hpp>
#include <lunar/render/program.hpp>
#include <lunar/render/mesh.hpp>

#include <vector>
#include <span>

#include <imgui.h>

namespace lunar::Render
{
	//class LUNAR_API Camera;
	class LUNAR_API RenderContext_T
	{
	public:
		RenderContext_T()  noexcept;
		~RenderContext_T() noexcept;

		template <typename T>
		inline void begin(Handle<T> target) { begin(&target.get()); }
		void        begin(RenderTarget* target);
		void        draw(GpuProgram program, GpuTexture texture);
		void        draw(GpuMesh mesh);
		void        end();

		GpuMesh     getMesh(MeshPrimitive primitive);
		GpuProgram  getProgram(size_t number);
		GpuProgram  getProgram(GpuDefaultPrograms program);

		GpuVertexArrayObject createVertexArray();
		GpuBuffer            createBuffer
		(
			GpuBufferType type, 
			GpuBufferUsageFlags usageFlags, 
			size_t size, 
			void* data
		);
		GpuProgram           createProgram
		(
			GpuProgramType programType, 
			const std::initializer_list<GpuProgramStageData>& stages
		);
		GpuProgram           createProgram
		(
			GpuProgramType programType,
			const std::span<GpuProgramStageData>& stages
		);
		GpuTexture           createTexture
		(
			int               width,
			int               height,
			void*             data,
			TextureFormat     srcFormat,
			TextureDataFormat dataFormat = TextureDataFormat::eUnsignedByte,
			TextureFormat     dstFormat  = TextureFormat::eRGBA,
			TextureType       type       = TextureType::e2D,
			TextureFiltering  filtering  = TextureFiltering::eLinear,
			TextureWrapping   wrapping   = TextureWrapping::eRepeat,
			TextureFlags      flags      = TextureFlagBits::eNone
		);
		GpuMesh              createMesh
		(
			GpuVertexArrayObject vertexArray,
			GpuBuffer            vertexBuffer,
			GpuBuffer            indexBuffer,
			MeshTopology         topology
		);
		GpuCubemap           createCubemap
		(
			int   width,
			int   height,
			void* data,
			bool  isSourceHdr
		);

#ifdef LUNAR_OPENGL
		GLuint glGetFramebuffer();
		GLuint glGetRenderbuffer();
#endif

	private:
		vector<GpuVertexArrayObject_T> vertexArrays         = {};
		vector<GpuBuffer_T>            buffers              = {};
		vector<GpuProgram_T>           programs             = {};
		vector<GpuTexture_T>           textures             = {};
		vector<GpuMesh_T>              meshes               = {};
		vector<GpuCubemap_T>           cubemaps             = {};
		RenderTarget*                  target               = nullptr;
		ImGuiContext*                  imguiContext         = nullptr;
		bool                           inFrameScope         = false;
		bool                           defaultProgramsBuilt = false;
		bool                           defaultMeshesBuilt   = false;
		//const Camera*                  renderCamera         = nullptr;

		void loadDefaultMeshes();
		void loadDefaultPrograms();

#		ifdef LUNAR_OPENGL
		GLuint                         frameBuffer  = 0;
		GLuint                         renderBuffer = 0;
#		endif
	};
}
