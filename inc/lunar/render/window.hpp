#pragma once
#include <GLFW/glfw3.h>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/file/config_file.hpp>
#include <lunar/core/input.hpp>
#include <lunar/api.hpp>
#include <unordered_map>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
#	ifdef LUNAR_VULKAN
	constexpr size_t FRAME_OVERLAP = 2;

	struct VulkanFrameData
	{
		struct
		{
			vk::Image image;
			vk::ImageView view;
			vk::Semaphore imageAvailable;
		} swapchain;

		struct
		{
			VulkanImage image;
			VulkanImage depthImage;
			vk::Extent2D extent;
			vk::Semaphore renderFinished;
		} internal;

		struct
		{
			vk::DescriptorSetLayout descriptorLayout;
			vk::DescriptorSet descriptorSet;
			VulkanBuffer buffer;
		} uniformBuffer;

		VulkanCommandBuffer commandBuffer;
	};
#	endif

	enum class LUNAR_API KeyState : int
	{
		eNone     = 0,
		eReleased = 1 << 0,
		eHeld     = 1 << 1,
		ePressed  = 1 << 2,
	};

	inline KeyState operator|(KeyState a, KeyState b) { return static_cast<KeyState>(static_cast<int>(a) | static_cast<int>(b)); }
	inline bool     operator&(KeyState a, KeyState b) { return (static_cast<int>(a) & static_cast<int>(b)) > 0; }

	class LUNAR_API Window : public Identifiable, public RenderTarget, public Core::InputHandler
	{
	public:
		Window(
			int width,
			int height,
			bool fullscreen,
			const char* title,
			std::shared_ptr<RenderContext> context
		);

		~Window();

		void      init(int width, int height, bool fullscreen, const char* title, std::shared_ptr<RenderContext>& context);
		void      destroy();
		void      close();
		void      update() override;
		bool      shouldClose() const;
		bool      isMinimized() const;
		bool      exists() const;
		void      lockCursor();
		void      unlockCursor();
		void      toggleCursor();
		bool      isCursorLocked() const;

		int       getRenderWidth() const override;
		int       getRenderHeight() const override;

		bool      getActionDown(const std::string_view& name) const override;
		bool      getActionUp(const std::string_view& name) const override;
		bool      getAction(const std::string_view& name) const override;
		glm::vec2 getAxis()     const override;
		glm::vec2 getRotation() const override;

		static void pollEvents();

#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR& getVkSurface();
		vk::SwapchainKHR& getVkSwapchain();
		VulkanFrameData& getVkFrameData(size_t idx);
		size_t getVkSwapImageCount();
		const vk::Extent2D& getVkSwapExtent() const;

		vk::Extent2D& getVkSwapExtent();
		vk::Image& getVkSwapImage(size_t idx);
		vk::Semaphore& getVkImageAvailable(size_t idx);
		vk::Semaphore& getVkImagePresentable(size_t idx);
		VulkanCommandBuffer& getVkCommandBuffer(size_t idx);


		size_t getVkCurrentFrame() const;
		void endVkFrame();
#		endif
	protected:
		GLFWwindow*                       handle      = nullptr;
		std::shared_ptr<RenderContext>    renderCtx   = nullptr;
		bool                              initialized = false;
		std::unordered_map<int, KeyState> keys        = {};
		glm::vec2                         axis        = { 0, 0 };
		glm::vec2                         rotation    = { 0, 0 };
		glm::vec2                         lastMouse   = { 0, 0 };
		bool                              mouseInside = false;
		bool                              mouseLocked = false;

		bool checkActionValue(const std::string_view& name, KeyState required) const;

		friend void Glfw_FramebufferSizeCb(GLFWwindow*, int, int);
		friend void Glfw_KeyCallback(GLFWwindow*, int, int, int, int);
		friend void Glfw_MouseBtnCallback(GLFWwindow*, int, int, int);
		friend void Glfw_CursorPosCb(GLFWwindow*, double, double);
		friend void Glfw_CursorEnterCb(GLFWwindow*, int);

#		ifdef LUNAR_OPENGL
		void _glInitialize();
		friend class GLContext;
#		endif

#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR _vkSurface;
		vk::SurfaceFormatKHR _vkSurfaceFmt;
		vk::PresentModeKHR _vkPresentMode;
		
		vk::SwapchainKHR _vkSwapchain;
		vk::Extent2D _vkSwapExtent;
		size_t _vkSwapImgCount;

		VulkanFrameData _vkFrameData[FRAME_OVERLAP];
		VulkanCommandPool _vkCommandPool;
		VulkanGrowableDescriptorAllocator _vkDescriptorAlloc;
		size_t _vkCurrentFrame;

		VulkanContext& _getVkContext();

		void _vkInitialize();
		void _vkInitSwap();
		void _vkDestroy();
		void _vkDestroySwap();
		void _vkUpdateSwapExtent();
		void _vkHandleResize(int width, int height);

		friend class VulkanContext;
#		endif
	};

	struct LUNAR_API WindowBuilder
	{
	public:
		WindowBuilder& setWidth(int width);
		WindowBuilder& setHeight(int height);
		WindowBuilder& setSize(int width, int height);
		WindowBuilder& setFullscreen(bool value = true);
		WindowBuilder& setTitle(const std::string_view& title);
		WindowBuilder& setRenderContext(std::shared_ptr<RenderContext>& context);
		WindowBuilder& setDefaultRenderContext();
		WindowBuilder& loadFromConfigFile(const Fs::Path& path);
		Window create();

	private:
		int w = 800, h = 600;
		bool fs = false;
		std::string_view title = "lunar";
		std::shared_ptr<RenderContext> renderContext = nullptr;
	};
}
