#pragma once
#include <string>
#include <string_view>
#include <memory>

#ifdef WIN32
#	ifdef LUNAR_LIBRARY_EXPORT
#		define LUNAR_API __declspec(dllexport)
#	else
#		define LUNAR_API __declspec(dllimport)
#	endif
#endif

