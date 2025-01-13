#pragma once
#include <lunar/api.hpp>
#include <atomic>

namespace Time
{
	struct LUNAR_API TimeContext_T
	{
		std::atomic<double>   lastTime    = 0.f;
		std::atomic<double>   currentTime = 0.f;
		std::atomic<double>   deltaTime   = 0.f;
		std::atomic<double>   timer       = 0.f;
		std::atomic<int>      fps_sum     = 0;
		std::atomic<int>      fps_avg     = 0;
		std::atomic<uint64_t> frames      = 0;
	};

	using TimeContext = TimeContext_T*;

	LUNAR_API void        Update();
	LUNAR_API TimeContext GetGlobalContext();
}
