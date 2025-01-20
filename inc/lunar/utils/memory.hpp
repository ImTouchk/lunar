#pragma once
#include <memory>

namespace lunar
{
	template<typename T> using shared_ptr = std::shared_ptr<T>;
	template<typename T> using unique_ptr = std::unique_ptr<T>;

	using std::make_unique;
	using std::make_shared;
}
