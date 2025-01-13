#include <lunar/utils/stopwatch.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>

namespace Utils
{
	Stopwatch::Stopwatch(bool initialized)
		: state(initialized ? State::eStarted : State::eUndefined),
		beginTime(initialized ? glfwGetTime() : 0),
		endTime(initialized ? glfwGetTime() : 0)
	{
	}

	void Stopwatch::begin()
	{
		DEBUG_ASSERT(state != State::eStarted);
		state = State::eStarted;
		beginTime = glfwGetTime();
		endTime = glfwGetTime();
	}

	void Stopwatch::reset()
	{
		state = State::eStarted;
		beginTime = glfwGetTime();
		endTime = glfwGetTime();
	}

	Stopwatch::ValueType Stopwatch::end()
	{
		DEBUG_ASSERT(state == State::eStarted);
		state = State::eFinished;
		endTime = glfwGetTime();
		return elapsed();
	}

	Stopwatch::ValueType Stopwatch::elapsed() const
	{
		DEBUG_ASSERT(state != State::eUndefined);

		ValueType end = (state == State::eFinished)
			? endTime
			: glfwGetTime();
		
		return end - beginTime;
	}
}
