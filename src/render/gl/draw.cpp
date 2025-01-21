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
		GLuint    vao = (target == nullptr)
			? imp::GetGlobalRenderContext().glfw.vao
			: static_cast<Window_T*>(target)->getBackendData().globalVao;

		GpuBuffer vertex = mesh->getVertexBuffer();
		GpuBuffer index  = mesh->getIndexBuffer();

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex->glGetHandle());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->glGetHandle());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_x));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv_y));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);

		glDrawElements((GLenum)mesh->getTopology(), mesh->getIndicesCount(), GL_UNSIGNED_INT, 0);
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
