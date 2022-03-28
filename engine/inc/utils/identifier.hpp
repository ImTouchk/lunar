#pragma once
#include "utils/debug.hpp"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <mutex>

// Just a type alias, however it allows for explicit constructors
using Identifier = unsigned;

// Returns a simple identifier generated from a thread-safe counter
// The function guarantees that numbers are obviously given in an ascending order
// which might prove useful
inline Identifier get_unique_number()
{
	static Identifier counter = 0;
	static std::mutex counter_mutex = {};

	std::unique_lock lock(counter_mutex);
	return ++counter;
}

// The idea is that since most element vectors will have their identifiers
// in an ascending order, there's faster solutions available for finding an element
// TODO: Implement some sort of binary search for large vectors
template<typename T>
T* find_by_identifier(std::vector<T>& elements, unsigned identifier)
{
	T* pElements = elements.data();

	for (int i = 0; i < elements.size(); i++)
	{
		T* pElement = pElements + i;
		Identifier* pElementId = reinterpret_cast<Identifier*>(pElement);
		if (*pElementId == identifier)
		{
			return pElement;
		}
	}

	return nullptr;
}

// Wraps raw function find_by_identifier with error handling
template<typename T>
T& find_by_identifier_safe(std::vector<T>& elements, unsigned identifier)
{
	assert(not elements.empty());
	T* element = find_by_identifier(elements, identifier);
	if (element == nullptr)
	{
		throw std::runtime_error("Utils-FindByIdentifier-NullPtrException");
	}
	return *element;
}

template<typename T>
void delete_element_with_identifier(std::vector<T>& elements, unsigned identifier)
{
	T* pElements = elements.data();

	for (int i = 0; i < elements.size(); i++)
	{
		T* pElement = pElements + i;
		Identifier* pElementId = reinterpret_cast<Identifier*>(pElement);
		if (*pElementId == identifier)
		{
			elements.erase(elements.begin() + i);
			return;
		}
	}

	CDebug::Warn("delete_element_with_identifier<T> called on non-existant id.");
}
