#include <lunar/render/render_context.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/mesh.hpp>

namespace Render
{
	SceneLightData& RenderContext::getSceneLightData(Core::Scene& scene)
	{
		sceneLightData = SceneLightData
		{
			.metallic  = 0.f,
			.roughness = 0.f,
			.ao        = 1.f,
			.count     = 0,
		};

		size_t light_count  = 0;
		auto   scene_lights = std::vector<Light*>();
		for (auto& object : scene.getGameObjects())
		{
			Light* light = object.getComponent<Light>();
			if (light == nullptr)
				continue;
			
			sceneLightData.positions[light_count] = glm::vec4(light->getTransform().position, 1.f);
			sceneLightData.colors[light_count]    = glm::vec4(light->color, 1.f);

			light_count++;

			if (light_count >= 10)
				break;
		}

		sceneLightData.count = light_count;
		return sceneLightData;
	}

	ImGuiContext* RenderContext::getImGuiContext()
	{
		return imguiContext;
	}

	const Mesh& RenderContext::getPrimitiveMesh(MeshPrimitive primitive) const
	{
		return primitiveMeshes[(size_t)primitive];
	}

	void RenderContext::unloadPrimitives()
	{
		delete[] primitiveMeshes;
	}

	void RenderContext::loadPrimitives()
	{
		primitiveMeshes = new Mesh[(size_t)MeshPrimitive::_count];

		auto& cube         = _getPrimitiveMesh(MeshPrimitive::eCube);
		auto& quad         = _getPrimitiveMesh(MeshPrimitive::eQuad);
		auto  mesh_builder = MeshBuilder();

		/*
			TODO: Currently there is no way of passing the <RenderContext> pointer 
			to the mesh builder, need to figure out how. For the moment, it's not a
			huge issue due to how OpenGL contextes work.
		*/ 

		cube = mesh_builder
			.setVertices(GetCubeVertices())
			.setIndices(GetCubeIndices())
			.build()
			.getResult();

		quad = mesh_builder
			.setVertices(GetQuadVertices())
			.setIndices(GetQuadIndices())
			.build()
			.getResult();
	}

	Mesh& RenderContext::_getPrimitiveMesh(MeshPrimitive primitive)
	{
		return primitiveMeshes[(size_t)primitive];
	}

	RenderTarget* RenderContext::getCurrentTarget()
	{
		DEBUG_ASSERT(currentTarget != nullptr, "Frame drawing has not begun yet.");
		return currentTarget;
	}

	void RenderContext::setCamera(const Camera& camera)
	{
		this->camera = &camera;
	}
}