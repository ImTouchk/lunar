#include <lunar/render/imp.hpp>
#include <lunar/render/context.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/debug/assert.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace lunar::Render
{
	static const glm::mat4 CUBEMAP_PROJ    = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	static const glm::mat4 CUBEMAP_VIEWS[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f),  glm::vec3(0.0f, -1.0f,  0.0f))
	};

	void RenderContext_T::begin(RenderTarget* target)
	{
		//this->renderCamera = &camera;
		this->target       = target;
		this->inFrameScope = true;

		int v_width  = target->getRenderWidth();
		int v_height = target->getRenderHeight();

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
	}

	void RenderContext_T::draw(GpuMesh mesh)
	{
		GpuVertexArrayObject vertex_arr = mesh->getVertexArray();
		vertex_arr->bind();
		glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0);
	}

	void RenderContext_T::draw(GpuProgram program, GpuTexture texture)
	{
		program->use();

	}

	void RenderContext_T::end()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		this->inFrameScope = false;
		//this->renderCamera = nullptr;
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
