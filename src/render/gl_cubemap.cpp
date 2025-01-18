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

namespace Render
{
	static const glm::mat4 CAPTURE_PROJ    = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	static const glm::mat4 CAPTURE_VIEWS[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f))
	};

	bool CubemapBuilder::_glBuild()
	{
		auto& gl_ctx = static_cast<GLContext&>(*context.get());
		auto& handle = result._glHandle;

		if (isHdr)
			equirectToCubemap();
		else
			normalCubemap();

		buildIrradianceMap();
		buildPrefilterMap();
		buildBrdfTexture();

		return true;
	}

	void CubemapBuilder::buildBrdfTexture()
	{
		const auto& quad_mesh = context->getPrimitiveMesh(MeshPrimitive::eQuad);

		auto& gl_ctx       = static_cast<GLContext&>(*context.get());
		auto& handle       = result._glHandle;
		auto& brdf_texture = result._glBrdf;

		GLuint capture_fbo = gl_ctx.glGetCaptureFramebuffer();
		GLuint capture_rbo = gl_ctx.glGetCaptureRenderbuffer();

		glGenTextures(1, &brdf_texture);
		glBindTexture(GL_TEXTURE_2D, brdf_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto shader_builder = GraphicsShaderBuilder();
		auto brdf_shader    = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/pbr/brdf.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/pbr/brdf.frag"))
			.build();

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_texture, 0);

		glViewport(0, 0, 512, 512);
		brdf_shader.use();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glBindVertexArray(quad_mesh._glVao);
		glDrawElements(GL_TRIANGLES, quad_mesh.indicesCount, GL_UNSIGNED_INT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CubemapBuilder::buildPrefilterMap()
	{
		const auto& cube_mesh = context->getPrimitiveMesh(MeshPrimitive::eCube);

		auto& gl_ctx        = static_cast<GLContext&>(*context.get());
		auto& handle        = result._glHandle;
		auto& prefilter_map = result._glPrefilter;

		GLuint capture_fbo = gl_ctx.glGetCaptureFramebuffer();
		GLuint capture_rbo = gl_ctx.glGetCaptureRenderbuffer();

		glGenTextures(1, &prefilter_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
		for (size_t i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		auto shader_builder   = GraphicsShaderBuilder();
		auto prefilter_shader = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/pbr/cubemap.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/pbr/prefilter.frag"))
			.build();

		prefilter_shader.use();
		prefilter_shader.uniform("projection", CAPTURE_PROJ);
		prefilter_shader.uniform("environmentMap", 0);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		constexpr size_t MAX_MIP_LEVELS = 5;
		for (size_t mip = 0; mip < MAX_MIP_LEVELS; mip++)
		{
			size_t mip_w = 128 * glm::pow(0.5, mip);
			size_t mip_h = 128 * glm::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_w, mip_h);
			glViewport(0, 0, mip_w, mip_h);

			GLfloat roughness = (GLfloat)mip / (GLfloat)(MAX_MIP_LEVELS - 1);
			prefilter_shader.uniform("roughness", roughness);

			for (size_t i = 0; i < 6; i++)
			{
				prefilter_shader.uniform("view", CAPTURE_VIEWS[i]);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glBindVertexArray(cube_mesh._glVao);
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CubemapBuilder::buildIrradianceMap()
	{
		const auto& cube_mesh = context->getPrimitiveMesh(MeshPrimitive::eCube);
		
		auto& gl_ctx         = static_cast<GLContext&>(*context.get());
		auto& handle         = result._glHandle;
		auto& irradiance_map = result._glIrradiance;

		GLuint capture_fbo = gl_ctx.glGetCaptureFramebuffer();
		GLuint capture_rbo = gl_ctx.glGetCaptureRenderbuffer();

		glGenTextures(1, &irradiance_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
		for (size_t i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		auto shader_builder = GraphicsShaderBuilder();
		auto irradiance_shader = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/pbr/cubemap.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/pbr/irradiance.frag"))
			.build();

		irradiance_shader.use();
		irradiance_shader.uniform("projection", CAPTURE_PROJ);
		irradiance_shader.uniform("environmentMap", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

		glViewport(0, 0, 32, 32);
		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		for (size_t i = 0; i < 6; i++)
		{
			irradiance_shader.uniform("view", CAPTURE_VIEWS[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cube_mesh._glVao);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CubemapBuilder::equirectToCubemap()
	{
		auto& gl_ctx          = static_cast<GLContext&>(*context.get());
		auto& handle          = result._glHandle;
		auto  texture_builder = TextureBuilder();
		auto  staging         = texture_builder
			.useRenderContext(context)
			.fromByteArray(TextureFormat::eRGB, width[0], height[0], rawBytes[0])
			.setByteFormat(TextureByteFormat::eFloat)
			.setDstFormat(TextureFormat::eRGB16F)
			.setWrapping(TextureWrapping::eClampToEdge)
			.setFiltering(TextureFiltering::eLinear)
			.build()
			.getResult();

		GLuint capture_fbo = gl_ctx.glGetCaptureFramebuffer();
		GLuint capture_rbo = gl_ctx.glGetCaptureRenderbuffer();

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);

		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
		for (size_t i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto shader_builder = GraphicsShaderBuilder();
		auto equirect_shader = shader_builder
			.fromVertexSourceFile(Fs::dataDirectory().append("shader-src/pbr/cubemap.vert"))
			.fromFragmentSourceFile(Fs::dataDirectory().append("shader-src/pbr/equirect_to_cubemap.frag"))
			.build();

		const auto& cube_mesh = context->getPrimitiveMesh(MeshPrimitive::eCube);

		equirect_shader.use();
		equirect_shader.uniform("projection", CAPTURE_PROJ);
		equirect_shader.bind("equirectangularMap", 0, staging);

		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		for (size_t i = 0; i < 6; i++)
		{
			equirect_shader.uniform("view", CAPTURE_VIEWS[i]);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, handle, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cube_mesh._glVao);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	void CubemapBuilder::normalCubemap()
	{
		auto& handle = result._glHandle;

		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
		for (size_t i = 0; i < files.size(); i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, rawBytes[i]);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
}
