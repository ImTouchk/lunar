#pragma once
#include <lunar/api.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <initializer_list>
#include <unordered_map>
#include <string_view>
#include <vector>

namespace lunar
{
	namespace imp
	{
		enum class LUNAR_API ActionType
		{
			eUnknown = 0,
			eKey     = 1,
			eMouse   = 2,
			eGamepad = 3,
		};

		struct LUNAR_API ActionData
		{
			ActionType type;
			size_t     value;
		};
	}

	class LUNAR_API InputHandler
	{
	public:
		using InputCombo = std::initializer_list<std::string_view>;
		
		virtual void      update()            = 0;
		virtual glm::vec2 getAxis()     const = 0;
		virtual glm::vec2 getRotation() const = 0;
		virtual bool      getAction(const std::string_view&) const = 0;
		virtual bool      getActionUp(const std::string_view&) const = 0;
		virtual bool      getActionDown(const std::string_view&) const = 0;
		void              registerAction(const std::string_view& name, const std::initializer_list<InputCombo>& combos);

	protected:
		struct InputComboValues 
		{ 
			bool active = false; 
			imp::ActionData* key[4] = { nullptr, nullptr, nullptr, nullptr }; 
		};

		struct InputKeys 
		{ 
			InputComboValues combos[4] = { {}, {}, {}, {} };
		};

		std::unordered_map<std::string_view, InputKeys> actions          = {};
		float                                           deadzoneLeft     = 0.f;
		float                                           deadzoneRight    = 0.f;
		float                                           mouseSensitivity = 1.f;
	};
}

namespace lunar::Input
{
	LUNAR_API InputHandler& GetGlobalHandler();
	LUNAR_API void          SetGlobalHandler(InputHandler&);
	LUNAR_API bool          GetAction(const std::string_view& name);
	LUNAR_API bool          GetActionUp(const std::string_view& name);
	LUNAR_API bool          GetActionDown(const std::string_view& name);
	LUNAR_API glm::vec2     GetAxis();
	LUNAR_API glm::vec2     GetRotation();
}
