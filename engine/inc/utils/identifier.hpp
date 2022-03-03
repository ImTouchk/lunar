#pragma once
#include "utils/debug.hpp"

#include <algorithm>
#include <vector>
#include <mutex>
#include <cassert>

// Returns a simple identifier generated from a thread-safe counter
inline unsigned get_unique_number()
{
	static unsigned counter = 0;
	static std::mutex counter_mutex = {};

	std::lock_guard lock(counter_mutex);
	return ++counter;
}

template<typename T>
T* find_by_identifier(std::vector<T>& elements, unsigned identifier)
{
	assert(not elements.empty());

	T* pElements = elements.data();

	for (int i = 0; i < elements.size(); i++)
	{
		T* pElement = pElements + i;
		unsigned* pElementId = reinterpret_cast<unsigned*>(pElement);
		if (*pElementId == identifier)
		{
			return pElement;
		}
	}

	return nullptr;
}

template<typename T>
void delete_element_with_identifier(std::vector<T>& elements, unsigned identifier)
{
	T* pElements = elements.data();

	for (int i = 0; i < elements.size(); i++)
	{
		T* pElement = pElements + i;
		unsigned* pElementId = reinterpret_cast<unsigned*>(pElement);
		if (*pElementId == identifier)
		{
			elements.erase(elements.begin() + i);
			return;
		}
	}

	CDebug::Warn("delete_element_with_identifier<T> called on non-existant id.");
}
