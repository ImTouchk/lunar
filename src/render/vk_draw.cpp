#include <lunar/render/internal/render_vk.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/window.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Render
{
	void VulkanContext::draw(Core::Scene& scene, RenderTarget* target)
	{
		// TODO: make Core::Scene const
		// TODO: implement for RenderTargetType::eTexture
		// TODO: check for initialization

		auto& target_window = *reinterpret_cast<Render::Window*>(target);
		auto& swap_extent = target_window.getVkSwapExtent();
		auto& cmd_buffer = cmdBufferWrapper.primaryCmdBuffer;

		auto current_frame = target_window.getVkFrame();

		vk::CommandBufferBeginInfo begin_info = {};
		cmd_buffer.begin(begin_info);

		vk::ClearValue clear_value = { .color = { {{ 1.f, 0.f, 0.f, 1.f }} } };
		vk::RenderPassBeginInfo render_pass_begin_info = {
			.renderPass  = getDefaultRenderPass(),
			.framebuffer = target_window.getVkSwapFramebuffer(current_frame),
			.renderArea = {
				.offset = { 0, 0 },
				.extent = swap_extent
			},
			.clearValueCount = 1,
			.pClearValues    = &clear_value
		};

		vk::Viewport viewport = {
			.x        = 0.f,
			.y        = 0.f,
			.width    = static_cast<float>(swap_extent.width),
			.height   = static_cast<float>(swap_extent.height),
			.minDepth = 0.f,
			.maxDepth = 1.f
		};

		vk::Rect2D scissor = {
			.offset = { 0, 0 },
			.extent = swap_extent
		};

		cmd_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
		cmd_buffer.setViewport(0, viewport);
		cmd_buffer.setScissor(0, scissor);

		for (auto& gameObject : scene.getGameObjects())
		{
			auto* mesh_renderer = gameObject.getComponent<MeshRenderer>();
			
			if (mesh_renderer == nullptr)
				continue;

			auto& shader = mesh_renderer->getShader();
			cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shader.getVkPipeline());
			cmd_buffer.draw(3, 1, 0, 0);
		}

		// draw call

		cmd_buffer.endRenderPass();
		cmd_buffer.end();

		target_window.endVkFrame();
	}
}
