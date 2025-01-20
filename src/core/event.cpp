#include <lunar/core/event.hpp>
#include <lunar/debug.hpp>

namespace lunar
{
	void EventHandler::addEventListener(size_t type, EventListener listener)
	{
		auto& e_type_data = listeners[type];
		for (size_t i = 0; i < imp::MAX_LISTENERS; i++)
		{
			auto& cb = e_type_data.callbacks[i];
			if (!cb)
			{
				cb = listener;
				return;
			}
		}

		DEBUG_ASSERT(true, "Too many callbacks added to event!");
		throw;
	}

	template<typename T, typename... U>
	size_t getAddress(std::function<T(U...)>& f)
	{
		typedef T(fnType)(U...);
		fnType** fn_ptr = f.template target<fnType*>();
		return (size_t)*fn_ptr;
	}

	void EventHandler::removeEventListener(size_t type, EventListener listener)
	{
		size_t address = getAddress(listener);

		auto& e_type_data = listeners[type];
		for (size_t i = 0; i < imp::MAX_LISTENERS; i++)
		{
			auto& cb = e_type_data.callbacks[i];
			if (getAddress(cb) == address)
			{
				cb = nullptr;
				return;
			}
		}

		DEBUG_ASSERT(true, "removeEventListener called on invalid function");
	}

	void EventHandler::triggerEvent(size_t type, Event& e)
	{
		auto it = listeners.find(type);
		if (it == listeners.end())
			return;

		auto& e_type_data = it->second;
		for (size_t i = 0; i < imp::MAX_LISTENERS; i++)
		{
			auto& cb = e_type_data.callbacks[i];
			if (cb)
				cb(e);
		}
	}
}
