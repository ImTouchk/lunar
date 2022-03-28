#include "core/input.hpp"
#include "core/glfw_keys.hpp"
#include "utils/identifier.hpp"
#include "utils/fast_mutex.hpp"
#include "render/window.hpp"

#include <any>

namespace CInput
{
	struct CKeyState
	{
		unsigned key;
		bool currentState;
		bool lastState;
	};

	std::vector<CKeyState> KEY_STATES  = {};
	CMutex                 STATE_MUTEX = {};

	void KeyEventHandler(void*, const std::any& data)
	{
		CLockGuard lock(STATE_MUTEX);

		const auto event_data = std::any_cast<std::pair<int, bool>>(data);
		auto*      key_data   = find_by_identifier(KEY_STATES, event_data.first);

		if (key_data == nullptr)
		{
			KEY_STATES.push_back(CKeyState
			{
				.key          = static_cast<unsigned>(event_data.first),
				.currentState = event_data.second,
				.lastState    = false
			});

			return;
		}

		key_data->lastState = key_data->currentState;
		key_data->currentState = key_data->lastState;
	}

	void Initialize()
	{
		CGameWindow::GetPrimary()
			.subscribe(WindowEvent::eKeyStateChanged, KeyEventHandler);
	}

	void Destroy()
	{
		CGameWindow::GetPrimary()
			.unsubscribe(WindowEvent::eKeyStateChanged, KeyEventHandler);
	}

	bool IsKeyPressed(CKey key)
	{
		CLockGuard lock(STATE_MUTEX);
		auto* key_data = find_by_identifier(KEY_STATES, static_cast<unsigned>(key));
		if(key_data == nullptr)
		{
			return false;
		}
		return key_data->currentState;
	}
}