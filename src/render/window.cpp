#ifdef LUNAR_VULKAN
//#	define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#	define GLFW_INCLUDE_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#	include <vulkan/vulkan.hpp>
#endif

#ifdef LUNAR_OPENGL
#	include <glad/gl.h>
#endif	

#include <lunar/file/config_file.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>
#include <atomic>

namespace Render
{
	std::atomic<bool> _JOYSTICK_CONNECTED[16];

	WindowBuilder& WindowBuilder::setWidth(int width)
	{
		w = width;
		return *this;
	}

	WindowBuilder& WindowBuilder::setHeight(int height)
	{
		h = height;
		return *this;
	}

	WindowBuilder& WindowBuilder::setSize(int width, int height)
	{
		w = width;
		h = height;
		return *this;
	}

	WindowBuilder& WindowBuilder::setFullscreen(bool value)
	{
		fs = value;
		return *this;
	}

	WindowBuilder& WindowBuilder::setTitle(const std::string_view& title)
	{
		this->title = title;
		return *this;
	}

	WindowBuilder& WindowBuilder::loadFromConfigFile(const Fs::Path& path)
	{
		auto config = Fs::ConfigFile(path);
		setSize(
			config.get_or<int>("width", 800),
			config.get_or<int>("height", 600)
		);

		setFullscreen(config.get_or<int>("fullscreen", 0));
		return *this;
	}

	WindowBuilder& WindowBuilder::setRenderContext(std::shared_ptr<RenderContext>& context)
	{
		renderContext = context;
		return *this;
	}

	WindowBuilder& WindowBuilder::setDefaultRenderContext()
	{
		renderContext = CreateDefaultContext();
		return *this;
	}

	Window WindowBuilder::create()
	{
		return Window(w, h, fs, title.data(), renderContext);
	}

	std::atomic<int> GlfwUsers = 0;

	inline Window& Glfw_CastUserPtr(GLFWwindow* window)
	{
		return *reinterpret_cast<Window*>(
			glfwGetWindowUserPointer(window)
		);
	}

	void Glfw_JoystickCallback(int jid, int ev)
	{
		if (!glfwJoystickIsGamepad(jid))
			return;

		// TODO
		switch (ev)
		{
		case GLFW_CONNECTED:
		case GLFW_DISCONNECTED:
			_JOYSTICK_CONNECTED[jid] = ev == GLFW_CONNECTED;
		}
	}

	void Glfw_MouseBtnCallback(GLFWwindow* handle, int button, int action, int mods)
	{
		static_assert(sizeof(int) == sizeof(int32_t));

		auto& window = Glfw_CastUserPtr(handle);
		window.keys[button | (int)(1 << 31)] = (action == GLFW_PRESS) ? KeyState::ePressed : KeyState::eReleased;
	}

	void Glfw_KeyCallback(GLFWwindow* handle, int key, int scancode, int action, int mods)
	{
		auto& window = Glfw_CastUserPtr(handle);
		window.keys[key] = (action == GLFW_PRESS) ? KeyState::ePressed : KeyState::eReleased;
	}

	void Glfw_FramebufferSizeCb(GLFWwindow* handle, int width, int height)
	{
		auto& window = Glfw_CastUserPtr(handle);
#		ifdef LUNAR_VULKAN
		window._vkHandleResize(width, height);
#		endif

#		ifdef LUNAR_OPENGL
		// TODO: switch to window context (need a public window.getNative() function)
		glViewport(0, 0, width, height);
#		endif
	}

	void Glfw_CursorPosCb(GLFWwindow* handle, double x, double y)
	{
		auto& window = Glfw_CastUserPtr(handle);
		if (not window.mouseInside)
			return;

		auto current = glm::vec2 { x, y };

		if (window.isCursorLocked())
		{
			auto delta = (current - window.lastMouse) * window.mouseSensitivity;
			window.rotation += glm::vec2{ delta.x, -delta.y };
		}

		window.lastMouse = current;
	}

	void Glfw_CursorEnterCb(GLFWwindow* handle, int entered)
	{
		auto& window = Glfw_CastUserPtr(handle);
		window.mouseInside = entered;
	}

	Window::Window(
		int width,
		int height,
		bool fullscreen,
		const char* title,
		std::shared_ptr<RenderContext> context
	) : handle(nullptr),
		renderCtx(context),
		initialized(false),
		RenderTarget(),
		Identifiable()
	{
		init(width, height, fullscreen, title, renderCtx);
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::init(int width, int height, bool fullscreen, const char* title, std::shared_ptr<RenderContext>& context)
	{
		if (initialized)
			return;

		if (GlfwUsers == 0 && glfwInit() == GLFW_FALSE)
			DEBUG_ERROR("Failed to initialize glfw.");
		else if (++GlfwUsers == 1)
		{
			int major, minor, rev;
			glfwGetVersion(&major, &minor, &rev);
			DEBUG_LOG("Loaded glfw version {}.{}.{}", major, minor, rev);
		}

#		ifdef LUNAR_VULKAN
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#		endif

#		ifdef LUNAR_OPENGL
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // for Mac
#		endif

		handle = glfwCreateWindow(
			width, height,
			title,
			(fullscreen)
				? glfwGetPrimaryMonitor() // TODO: monitor selection
				: nullptr,
			nullptr
		);

		if (handle == nullptr)
			DEBUG_ERROR("Failed to create a new window.");
		else
			DEBUG_LOG("Window created successfully.");

		glfwSetWindowUserPointer(handle, this);
		glfwSetFramebufferSizeCallback(handle, Glfw_FramebufferSizeCb);
		glfwSetKeyCallback(handle, Glfw_KeyCallback);
		glfwSetMouseButtonCallback(handle, Glfw_MouseBtnCallback);
		glfwSetCursorPosCallback(handle, Glfw_CursorPosCb);
		glfwSetCursorEnterCallback(handle, Glfw_CursorEnterCb);
		glfwSetJoystickCallback(Glfw_JoystickCallback);

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		//glfwSwapInterval(2);
		
		initialized = true;

#		ifdef LUNAR_VULKAN
		_vkInitialize();
#		endif

#		ifdef LUNAR_OPENGL
		_glInitialize();
#		endif
	}

	void Window::destroy()
	{
		if (!initialized)
			return;

#		ifdef LUNAR_VULKAN
		_vkDestroy();
#		endif	

		glfwDestroyWindow(handle);
		if (--GlfwUsers == 0)
		{
			glfwTerminate();
			DEBUG_LOG("Terminated glfw library.");
		}

		initialized = false;
		handle = nullptr;
	}

	void Window::close()
	{
		DEBUG_INIT_CHECK();
		glfwSetWindowShouldClose(handle, GLFW_TRUE);
	}

	bool Window::exists() const
	{
		return handle != nullptr;
	}

	bool Window::shouldClose() const
	{
		DEBUG_INIT_CHECK();
		return glfwWindowShouldClose(handle);
	}

	bool Window::isMinimized() const
	{
		DEBUG_INIT_CHECK();
		int w, h;
		glfwGetFramebufferSize(handle, &w, &h);
		return w == 0 || h == 0;
	}

	void Window::lockCursor()
	{
		glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouseLocked = true;
	}

	void Window::unlockCursor()
	{
		glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		mouseLocked = false;
	}

	void Window::toggleCursor()
	{
		switch (mouseLocked)
		{
		case true:  unlockCursor(); return;
		case false: lockCursor();   return;
		}
	}

	bool Window::isCursorLocked() const
	{
		return mouseLocked;
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	int Window::getRenderWidth() const
	{
		int w, _;
		glfwGetFramebufferSize(handle, &w, &_);
		return w;
	}

	int Window::getRenderHeight() const
	{
		int _, h;
		glfwGetFramebufferSize(handle, &_, &h);
		return h;
	}

	inline int GetKeyId(Core::imp::ActionData* key)
	{
		switch (key->type)
		{
		case Core::imp::ActionType::eKey:     return key->value;
		case Core::imp::ActionType::eMouse:   return key->value | (1 << 31);
		case Core::imp::ActionType::eGamepad: return key->value | (1 << 30);
		default: return key->value;
		}
	}

	inline bool IsKeyPressed(GLFWwindow* handle, Core::imp::ActionData* key)
	{
		switch (key->type)
		{
		case Core::imp::ActionType::eKey:     return glfwGetKey(handle, key->value) == GLFW_PRESS;
		case Core::imp::ActionType::eMouse:   return glfwGetMouseButton(handle, key->value) == GLFW_PRESS;
		case Core::imp::ActionType::eGamepad:
		{
			GLFWgamepadstate state;
			glfwGetGamepadState(GLFW_JOYSTICK_1, &state); // TOOD: multiple joysticks?
			return state.buttons[key->value] == GLFW_PRESS;
		}
		default:
			return false;
		}
	}

	bool Window::checkActionValue(const std::string_view& name, KeyState required) const
	{
		DEBUG_ASSERT(actions.contains(name), "Input action does not exist");
		auto& options = actions.at(name);

		for (size_t i = 0; i < 4; i++)
		{
			auto& combo = options.combos[i];
			if (not combo.active)
				continue;

			bool combo_fulfilled = true;
			for (size_t j = 0; j < 4; j++)
			{
				auto& key = combo.key[j];
				if (key == nullptr)
					continue;

				int  id    = GetKeyId(key);
				auto value = keys.find(id);
				if (value == keys.end() || !(value->second & required))
					combo_fulfilled = false;
			}

			if (combo_fulfilled)
				return true;
		}

		return false;
	}

	bool Window::getAction(const std::string_view& name) const
	{
		return checkActionValue(name, KeyState::ePressed | KeyState::eHeld | KeyState::eReleased);
	}

	bool Window::getActionUp(const std::string_view& name) const
	{
		return checkActionValue(name, KeyState::eReleased);
	}

	bool Window::getActionDown(const std::string_view& name) const
	{
		return checkActionValue(name, KeyState::ePressed);
	}

	glm::vec2 Window::getAxis() const
	{
		return axis;
	}

	glm::vec2 Window::getRotation() const
	{
		return rotation;
	}

	inline float GetAxisValue(GLFWgamepadstate& state, size_t n, float deadzone)
	{
		float value = state.axes[n];
		if (value < -deadzone || value > deadzone)
			return value;
		else
			return 0.f;
	}

	inline void SetAxisValue(GLFWwindow* handle, int lower, int upper, float& value)
	{
		int first  = glfwGetKey(handle, lower);
		int second = glfwGetKey(handle, upper);

		value = 0.f;
		if (first == GLFW_PRESS)
			value += -1.f;
		if (second == GLFW_PRESS)
			value += 1.f;
	}

	void Window::update()
	{
		rotation = { 0, 0 };

		for (auto& [key, value] : keys)
		{
			switch (value)
			{
			case KeyState::ePressed:  value = KeyState::eHeld; break;
			case KeyState::eReleased: value = KeyState::eNone; break;
			default: break;
			}
		}

		SetAxisValue(handle, GLFW_KEY_A, GLFW_KEY_D, axis.x);
		SetAxisValue(handle, GLFW_KEY_S, GLFW_KEY_W, axis.y);

		//for (size_t i = 0; i < 16; i++)
		//{
		//	if (not _JOYSTICK_CONNECTED[i])
		//		continue;

		//	GLFWgamepadstate state;
		//	glfwGetGamepadState(i, &state);
		//	
		//	for (int j = 0; j < 16; j++)
		//		keys[j | (1 << 30)] = state.buttons[j] == GLFW_PRESS ? ;

		//	axis = 
		//	{
		//		GetAxisValue(state, GLFW_GAMEPAD_AXIS_LEFT_X, deadzoneLeft),
		//		GetAxisValue(state, GLFW_GAMEPAD_AXIS_LEFT_Y, deadzoneRight)
		//	};

		//	rotation =
		//	{
		//		GetAxisValue(state, GLFW_GAMEPAD_AXIS_RIGHT_X, deadzoneLeft),
		//		GetAxisValue(state, GLFW_GAMEPAD_AXIS_RIGHT_Y, deadzoneRight)
		//	};
		//}
	}
}
