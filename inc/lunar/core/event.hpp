#pragma once
#include <lunar/api.hpp>
#include <unordered_map>
#include <functional>
#include <concepts>
#include <vector>

namespace lunar
{
	struct LUNAR_API Event {};

	template<typename T = Event> requires std::derived_from<T, Event>
	using EventListener_T = std::function<void(T&)>;

	using EventListener = EventListener_T<>;

	namespace imp
	{
		constexpr size_t MAX_LISTENERS = 10;

		struct LUNAR_API EventListeners
		{
			EventListener callbacks[MAX_LISTENERS] = {};
		};

	}

	class LUNAR_API EventHandler
	{
	protected:
		void _addEventListener(size_t type, EventListener listener);
		void _removeEventListener(size_t type, EventListener listener);
		void _triggerEvent(size_t type, Event& e);
			
		std::unordered_map<size_t, imp::EventListeners> listeners;
	};
}
