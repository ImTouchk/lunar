#include <glad/gl.h>
#include <GLFW/glfw3.h>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_glfw.h>
#	include <imgui_impl_opengl3.h>
#endif

#include <lunar/render/render_context.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/internal/render_gl.hpp>
#include <lunar/render/window.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/render/mesh.hpp>
#include <lunar/render/texture.hpp>
#include <lunar/file/text_file.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fastgltf/core.hpp>

namespace Render
{
	int Texture::getRenderWidth() const
	{
		return width;
	}

	int Texture::getRenderHeight() const
	{
		return height;
	}

	void Texture::bind(size_t location)
	{
		glActiveTexture(GL_TEXTURE0 + location);
		glBindTexture(GL_TEXTURE_2D, _glHandle);
	}

	bool TextureBuilder::_glBuild()
	{
		auto& handle = result._glHandle;

		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, (GLint)wrapping);

		GLint min_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
		GLint mag_filter = (filtering == TextureFiltering::eNearest) ? GL_NEAREST : GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
		
		if (rawBytes != nullptr)
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)dstFormat, width, height, 0, (GLint)srcFormat, (GLenum)byteFormat, rawBytes);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, (GLint)dstFormat, width, height, 0, (GLint)srcFormat, (GLenum)byteFormat, nullptr);

		glGenerateMipmap(GL_TEXTURE_2D);

		return true;
	}

	MaterialBuilder& MaterialBuilder::useRenderContext(std::shared_ptr<RenderContext>& context)
	{
		this->context = context;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::fromGltfObject(const fastgltf::Material& object)
	{
		this->gltf = &object;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::build()
	{
		float ao = glm::clamp(gltf->ior, 1.f, 4.f);
		ao       = ao / 4.f;

		result.metallic  = gltf->pbrData.metallicFactor;
		result.roughness = gltf->pbrData.roughnessFactor;
		result.ao        = ao;

		if (gltf->pbrData.baseColorTexture.has_value())
		{
			// TODO:
			DEBUG_WARN("Not implemented");
		}
		else
		{
			auto     color = gltf->pbrData.baseColorFactor;
			
			uint32_t value = 0;
			value |= static_cast<uint32_t>(glm::round(color.x() * 255.f));
			value |= static_cast<uint32_t>(glm::round(color.y() * 255.f)) << 8;;
			value |= static_cast<uint32_t>(glm::round(color.z() * 255.f)) << 16;;
			value |= static_cast<uint32_t>(glm::round(color.w() * 255.f)) << 24;

			auto texture_builder = TextureBuilder();
			result.albedo = texture_builder
				.useRenderContext(context)
				.fromByteArray(TextureFormat::eRGBA, 1, 1, &value)
				.setDstFormat(TextureFormat::eRGBA)
				.setWrapping(TextureWrapping::eRepeat)
				.setFiltering(TextureFiltering::eNearest)
				.build()
				.getResult();
		}
		return *this;
	}

	GpuMaterial MaterialBuilder::getGpuResult()
	{
		auto material = GpuMaterial
		{
			.albedoMap = (int)result.albedo.glGetHandle(),
			.metallic  = result.metallic,
			.roughness = result.roughness,
			.ao        = result.ao,
		};

		result.albedo._glHandle = 0;

		return material;
	}

	Material MaterialBuilder::getResult()
	{
		return std::move(result);
	}
}