#pragma once
#include <lunar/api.hpp>
#include <lunar/utils/collections.hpp>
#include <lunar/utils/identifiable.hpp>
#include <concepts>

namespace lunar
{
	template<typename T>
	concept HasValidCheck = requires(T t) {
		{ t.valid() } -> std::same_as<bool>;
		{ t.setValid( bool() ) };
	};

	template<typename T>
	class LUNAR_API Handle
	{
	public:
		Handle(std::nullptr_t)                      noexcept : ref(nullptr), idx(0) {}
		Handle(vector<T>& collection, size_t index) noexcept : ref(&collection), idx(index) {}
		Handle(vector<T>& collection, T* object)    noexcept : ref(&collection), idx((size_t)(object - collection.data())) {}
		Handle()                                    noexcept = default;
		~Handle()                                   noexcept = default;

		T*       operator->()           { return &(ref->operator[](idx)); }
		const T* operator->() const     { return &(ref->operator[](idx)); }
		T&       get()                  { return ref->operator[](idx); }
		const T& get()       const      { return ref->operator[](idx); }
		bool     operator==(T* pointer) { return pointer == (ref->data() + idx); }
		
		template<typename = typename std::enable_if<HasValidCheck<T>>::type>
		bool     valid()     const      { return get().valid(); }


	protected:
		vector<T>* ref = nullptr;
		size_t     idx = 0;
	};

	template<typename T>
	inline Handle<T> make_handle(vector<T>& collection, size_t index)
	{
		return Handle<T>(collection, index);
	}

	template<typename T>
	inline Handle<T> make_handle(vector<T>& collection)
	{
		return Handle<T>(collection, collection.size() - 1);
	}

	template<typename T>
	inline Handle<T> make_handle(vector<T>& collection, T* object)
	{
		return Handle<T>(collection, object);
	}
}

#define LUNAR_HANDLE(Type)          using Type = lunar::Handle<Type##_T>
#define LUNAR_SHARED_HANDLE(Type)   using Type = std::shared_ptr<Type##_T>
#define LUNAR_HANDLE_IMPL(Type)     template class LUNAR_API lunar::Handle<Type##_T>
