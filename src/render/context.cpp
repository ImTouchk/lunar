#include <lunar/render/context.hpp>
#include <lunar/render/program.hpp>
#include <lunar/debug/log.hpp>
#include <lunar/debug/assert.hpp>

namespace lunar::Render
{
	LUNAR_HANDLE_IMPL(GpuTexture);
	LUNAR_HANDLE_IMPL(GpuProgram);
	LUNAR_HANDLE_IMPL(GpuTexture);
	LUNAR_HANDLE_IMPL(GpuBuffer);
	LUNAR_HANDLE_IMPL(GpuMesh);
	LUNAR_HANDLE_IMPL(GpuCubemap);
	LUNAR_HANDLE_IMPL(Window);

	Window RenderContext_T::createWindow
	(
		int                     width,
		int                     height,
		bool                    fullscreen,
		const std::string_view& name,
		int                     msaa,
		bool                    vsync
	)
	{
		windows.emplace_back(this, width, height, fullscreen, name, msaa, vsync);
		return make_handle(windows);
	}

	GpuTexture RenderContext_T::createTexture
	(
		int               width,
		int               height,
		void*             data,
		TextureFormat     srcFormat,
		TextureDataFormat dataFormat,
		TextureFormat     dstFormat,
		TextureType       type,
		TextureFiltering  filtering,
		TextureWrapping   wrapping,
		TextureFlags      flags
	)
	{
		textures.emplace_back(
			this, 
			width, height, 
			data, srcFormat, dataFormat, 
			dstFormat, type, 
			filtering, wrapping, flags
		);
		return make_handle(textures);
	}

	GpuProgram RenderContext_T::createProgram
	(
		GpuProgramType programType,
		const std::initializer_list<GpuProgramStageData>& stages
	)
	{
		programs.emplace_back(this, programType, stages);
		return make_handle(programs);
	}

	GpuProgram RenderContext_T::createProgram
	(
		GpuProgramType programType,
		const std::span<GpuProgramStageData>& stages
	)
	{
		programs.emplace_back(this, programType, stages);
		return make_handle(programs);
	}

	GpuBuffer RenderContext_T::createBuffer
	(
		GpuBufferType type,
		GpuBufferUsageFlags usageFlags,
		size_t size,
		void* data
	)
	{
		buffers.emplace_back(this, type, usageFlags, size, data);
		return make_handle(buffers);
	}

	GpuMesh RenderContext_T::createMesh
	(
		GpuVertexArrayObject vertexArray,
		GpuBuffer            vertexBuffer,
		GpuBuffer            indexBuffer,
		MeshTopology         topology
	)
	{
		meshes.emplace_back(this, vertexArray, vertexBuffer, indexBuffer, topology);
		return make_handle(meshes);
	}

	GpuCubemap RenderContext_T::createCubemap
	(
		int   width,
		int   height,
		void* data,
		bool  isSourceHdr
	)
	{
		cubemaps.emplace_back(this, width, height, data, isSourceHdr);
		return make_handle(cubemaps);
	}

	GpuVertexArrayObject RenderContext_T::createVertexArray()
	{
		vertexArrays.emplace_back(this);
		return make_handle(vertexArrays);
	}

	void RenderContext_T::loadDefaultMeshes()
	{
		DEBUG_ASSERT(meshes.size() == 0, "Default meshes must be built before any other meshes.");
		DEBUG_ASSERT(defaultMeshesBuilt == false, "Default meshes were already built.");

		const auto cube_vertices = std::vector<Vertex>
		{
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 1, { 0, 0, 1.f}, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 0, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
		};

		const auto cube_indices = std::vector<uint32_t>
		{
			0, 1, 2, 3, 4, 5,
			6, 7, 8, 9, 10, 11,
			12, 13, 14, 15, 16, 17,
			18, 19, 20, 21, 22, 23,
			24, 25, 26, 27, 28, 29,
			30, 31, 32, 33, 34, 35
		};

		const auto quad_vertices = std::vector<Vertex>
		{
			Vertex { { -1.f, 1.f, 0.f }, 0.f, { 0.f, 0.f, 0.f }, 1.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { { -1.f,-1.f, 0.f }, 0.f, { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { {  1.f, 1.f, 0.f }, 1.f, { 0.f, 0.f, 0.f }, 1.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { {  1.f,-1.f, 0.f }, 1.f, { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f, 0.f } },
		};

		const auto quad_indices = std::vector<uint32_t>
		{
			0, 1, 2,
			2, 1, 3
		};

		GpuMeshBuilder()
			.useRenderContext(this)
			.defaultVertexArray()
			.fromVertexArray(cube_vertices)
			.fromIndexArray(cube_indices)
			.build();

		GpuMeshBuilder()
			.useRenderContext(this)
			.defaultVertexArray()
			.fromVertexArray(quad_vertices)
			.fromIndexArray(quad_indices)
			.build();

		defaultMeshesBuilt = true;
		DEBUG_LOG("Built primitive GPU meshes.");
	}

	void RenderContext_T::loadDefaultPrograms()
	{
		DEBUG_ASSERT(programs.size() == 0, "Default shaders must be built before any other shaders.");
		DEBUG_ASSERT(defaultProgramsBuilt == false, "Default shaders were already built.");

		GpuProgramBuilder()
			.graphicsShader()
			.addVertexSource(Fs::fromData("shader-src/screen.vert"))
			.addFragmentSource(Fs::fromData("shader-src/unlit.frag"))
			.build(this);

		GpuProgramBuilder()
			.graphicsShader()
			.addVertexSource(Fs::fromData("shader-src/pbr/cubemap.vert"))
			.addFragmentSource(Fs::fromData("shader-src/pbr/equirect_to_cubemap.frag"))
			.build(this);

		GpuProgramBuilder()
			.graphicsShader()
			.addVertexSource(Fs::fromData("shader-src/pbr/cubemap.vert"))
			.addFragmentSource(Fs::fromData("shader-src/pbr/irradiance.frag"))
			.build(this);

		GpuProgramBuilder()
			.graphicsShader()
			.addVertexSource(Fs::fromData("shader-src/pbr/cubemap.vert"))
			.addFragmentSource(Fs::fromData("shader-src/pbr/prefilter.frag"))
			.build(this);

		GpuProgramBuilder()
			.graphicsShader()
			.addVertexSource(Fs::fromData("shader-src/pbr/brdf.vert"))
			.addFragmentSource(Fs::fromData("shader-src/pbr/brdf.frag"))
			.build(this);

		defaultProgramsBuilt = true;
		DEBUG_LOG("Built default GPU programs.");
	}

	/*
		Global render context
	*/

	namespace imp
	{
		GlobalRenderContext& GetGlobalRenderContext()
		{
			static GlobalRenderContext context = {};
			return context;
		}
	}
}
