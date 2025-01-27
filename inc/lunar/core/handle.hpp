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

	//template<typename T>
	//concept RefCounted = requires(T t) { 
	//	{ t.refCount } -> std::same_as<size_t>; 
	//};

	/*
		A Handle<T> object acts like a normal raw pointer, except it guarantees that it 
		will not be invalidated in the future due to something out of the API caller's 
		control (e.g.: vector resizing).
	*/
	template<typename T>
	class LUNAR_API Handle
	{
	public:
		Handle(std::nullptr_t)                      noexcept : ref(nullptr), idx(0) {}
		Handle(vector<T>& collection, size_t index) noexcept : ref(&collection), idx(index) {}
		Handle(vector<T>& collection, T* object)    noexcept : ref(&collection), idx((size_t)(object - collection.data())) {}
		Handle()                                    noexcept = default;
		~Handle()                                   noexcept = default;

		T* operator->()                           { return &(ref->operator[](idx)); }
		const T* operator->()               const { return &(ref->operator[](idx)); }
		T& get()                                  { return ref->operator[](idx); }
		const T& get()                      const { return ref->operator[](idx); }
		T*       pointer()                        { if (ref == nullptr) return nullptr; return &(get()); }
		bool     operator==(T* pointer)     const { return pointer == (ref->data() + idx); }
		bool     operator==(std::nullptr_t) const { return ref == nullptr; }

		template<typename = typename std::enable_if<HasValidCheck<T>>::type>
		bool     valid()     const { return get().valid(); }

	protected:
		vector<T>* ref = nullptr;
		size_t     idx = 0;
	};

	/*
		A RefHandle<T> acts pretty much like a smart pointer for an object created by the engine.
	*/
	template<typename T>
	class LUNAR_API RefHandle
	{
	public:
		using TyPtr = T*;
		using TyRef = T&;

		RefHandle(std::nullptr_t)                          noexcept : ref(nullptr), idx(0) {}
		RefHandle(vector<TyPtr>& collection, size_t index) noexcept : ref(&collection), idx(index)
		{
			T* element = (*ref)[idx];
			element->refCount++;
		}
		RefHandle(vector<TyPtr>& collection, TyPtr object) noexcept : ref(&collection), idx(0)
		{
			for (size_t i = 0; i < collection.size(); i++)
				if (collection[i] == object)
					idx = i;

			T* element = (*ref)[idx];
			element->refCount++;
		}
		RefHandle()                                    noexcept = default;
		~RefHandle()                                   noexcept
		{
			if (ref != nullptr)
			{
				T* element = (*ref)[idx];
				element->refCount--;

				if (element->refCount <= 0)
					delete element;
			}
		}

		T*       operator->()           { return ref->operator[](idx); }
		const T* operator->() const     { return ref->operator[](idx); }
		T&       get()                  { return *(ref->operator[](idx)); }
		const T& get()       const      { return *(ref->operator[](idx)); }
		bool     operator==(T* pointer) { return pointer == (*ref)[idx]; }
		
		bool     exists()     const     { return ref !=  nullptr;}

		/* Copy & move operators */

		RefHandle(const RefHandle& other) noexcept
			: ref(other.ref),
			idx(other.idx)
		{
			if (this->ref != nullptr)
			{
				T* element = (*ref)[idx];
				element->refCount++;
			}
		}

		RefHandle& operator=(const RefHandle& other) noexcept
		{
			ref = other.ref;
			idx = other.idx;
			
			if (this->ref != nullptr)
			{
				T* element = (*ref)[idx];
				element->refCount++;
			}

			return *this;
		}

		RefHandle(RefHandle&& other) noexcept
			: ref(other.ref),
			idx(other.idx)
		{
			other.ref = nullptr;
			other.idx = 0;
		}

		RefHandle& operator=(RefHandle&& other) noexcept
		{
			ref = other.ref;
			idx = other.idx;

			other.ref = nullptr;
			other.idx = 0;
			return *this;
		}

	protected:
		vector<TyPtr>* ref = nullptr;
		size_t         idx = 0;
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
#define LUNAR_HANDLE_IMPL(Type)     template class LUNAR_API lunar::Handle<Type##_T>

#define LUNAR_REF_HANDLE(Type)      using Type = lunar::RefHandle<Type##_T>
#define LUNAR_REF_HANDLE_IMPL(Type) template class LUNAR_API lunar::RefHandle<Type##_T>

#define LUNAR_SHARED_HANDLE(Type)   using Type = std::shared_ptr<Type##_T>
