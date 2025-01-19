#pragma once
#include <lunar/api.hpp>
#include <lunar/render/texture.hpp>

namespace fastgltf { class Material; }

namespace Render
{
	struct LUNAR_API Material
	{
		Texture albedo    = {};
		float   metallic  = 0.2f;
		float   roughness = 0.2f;
		float   ao        = 0.1f;
	};

	struct LUNAR_API MaterialBuilder
	{
		MaterialBuilder() = default;

		MaterialBuilder&         useRenderContext(std::shared_ptr<RenderContext>& context);
		MaterialBuilder&         fromGltfObject(const fastgltf::Material& object);
		MaterialBuilder&         build();
		Material                 getResult();
		GpuMaterial              getGpuResult();

	private:
		std::shared_ptr<RenderContext> context = nullptr;
		const fastgltf::Material*      gltf   = nullptr;
		Material                       result = {};
	};
}
