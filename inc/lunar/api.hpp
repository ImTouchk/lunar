#pragma once
#include <string>
#include <string_view>
#include <memory>

#ifndef APP_NAME
#	define APP_NAME "UntitledLunarApp"
#endif

#ifndef APP_VER_MAJOR
#	define APP_VER_MAJOR 0
#endif

#ifndef APP_VER_MINOR
#	define APP_VER_MINOR 0
#endif

#ifndef APP_VER_PATCH
#	define APP_VER_PATCH 1
#endif

#define LUNAR_VER_MAJOR 0
#define LUNAR_VER_MINOR 1
#define LUNAR_VER_PATCH 0

#ifdef WIN32
#	ifdef LUNAR_LIBRARY_EXPORT
#		define LUNAR_API __declspec(dllexport)
#	else
#		define LUNAR_API __declspec(dllimport)
#	endif
#endif

#ifndef NDEBUG
#	define LUNAR_DEBUG_BUILD 1
#else
#	define LUNAR_DEBUG_BUILD 0
#endif
