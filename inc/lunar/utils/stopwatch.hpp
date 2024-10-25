#pragma once
#include <lunar/api.hpp>
#include <unordered_map>
#include <string_view>

namespace Utils
{
	class LUNAR_API Stopwatch
	{
	public:
		using ValueType = double;
		
		Stopwatch(bool initialized = false);
		~Stopwatch() = default;

		void begin();
		void reset();
		ValueType end();
		[[nodiscard]] ValueType elapsed() const;

	private:
		enum class State
		{
			eUndefined = 0,
			eStarted,
			eFinished
		};
		
		State state;
		ValueType beginTime;
		ValueType endTime;
	};

	class LUNAR_API FrameScheduler
	{
	public:
		FrameScheduler() = default;
		~FrameScheduler() = default;



	private:
		std::unordered_map<std::string_view, Stopwatch> sections;
	};
}
