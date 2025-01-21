#include <lunar/render/imp.hpp>
#include <lunar/render/context.hpp>
#include <lunar/render/components.hpp>
#include <lunar/debug/assert.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/core/component.hpp>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace lunar::Render
{
	void RenderContext_T::begin(RenderTarget* target)
	{
		//this->renderCamera = &camera;
		this->target       = target;
		this->inFrameScope = true;

		int v_width  = target->getRenderWidth();
		int v_height = target->getRenderHeight();

		if (typeid(*target).hash_code() == typeid(Window_T).hash_code())
		{
			Window_T* window = static_cast<Window_T*>(target);
			glfwMakeContextCurrent(window->glfwGetHandle());
		}

		if (typeid(*target).hash_code() == typeid(GpuTexture_T).hash_code())
		{
			GpuTexture_T* texture = static_cast<GpuTexture_T*>(target);
			
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, v_width, v_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				texture->glGetHandle(),
				0
			);
		}

		glViewport(0, 0, v_width, v_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (v_width != viewportWidth || v_height != viewportHeight)
			setViewportSize(v_width, v_height);
	}

	void RenderContext_T::clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void RenderContext_T::draw(GpuMesh mesh)
	{
		GpuVertexArrayObject vertex_arr = mesh->getVertexArray();
		vertex_arr->bind();
		glDrawElements((GLenum)mesh->getTopology(), mesh->getVertexCount(), GL_UNSIGNED_INT, 0);
		vertex_arr->unbind();
	}

	void RenderContext_T::draw(Scene& scene)
	{
		
	}

	void RenderContext_T::draw(GpuCubemap cubemap)
	{
		const GpuMesh cube_mesh  = getMesh(MeshPrimitive::eCube);
		GpuProgram    shader     = getProgram(GpuDefaultPrograms::eSkyboxShader);
		auto          view       = renderCamera->getViewMatrix();
		auto          projection = renderCamera->getProjectionMatrix(viewportWidth, viewportHeight);
		
		shader->use();
		shader->uniform("view", view);
		shader->uniform("projection", projection);
		shader->bind("environmentMap", 0, cubemap->environmentMap);
		shader->bind("irradianceMap", 1, cubemap->irradianceMap);
		shader->bind("prefilterMap", 2, cubemap->prefilterMap);
		shader->bind("brdfMap", 3, cubemap->brdfLut);
		draw(cube_mesh);
	}

	void RenderContext_T::end()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		this->inFrameScope = false;
		this->renderCamera = nullptr;
		this->target       = nullptr;
	}

	GLuint RenderContext_T::glGetFramebuffer()
	{
		return frameBuffer;
	}

	GLuint RenderContext_T::glGetRenderbuffer()
	{
		return renderBuffer;
	}
}
