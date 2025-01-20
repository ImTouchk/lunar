//#pragma once
//#include <lunar/api.hpp>
//#include <lunar/core/event.hpp>
//#include <lunar/utils/identifiable.hpp>
//#include <functional>
//
//namespace Core
//{
//	enum class LUNAR_API SceneEventType
//	{
//		eUnknown = 0,
//		eObjectDeleted = 1,
//		eObjectCreated = 2,
//	};
//
//	class LUNAR_API Scene;
//	class LUNAR_API GameObject;
//
//	struct LUNAR_API SceneEvent : Event 
//	{
//	public:
//		SceneEvent(Scene& sc) : scene(sc) {}
//
//		Scene& scene;
//	};
//
//	namespace Events
//	{
//		// Called right before deleting the GameObject
//		struct LUNAR_API SceneObjectDeleted : public SceneEvent
//		{
//		public:
//			SceneObjectDeleted(Scene& sc, GameObject& obj) : 
//				SceneEvent(sc),
//				gameObject(obj)
//			{}
//
//			GameObject& gameObject;
//		};
//
//		// Called right after a GameObject has been created
//		struct LUNAR_API SceneObjectCreated : public SceneEvent
//		{
//		public:
//			SceneObjectCreated(Scene& sc, GameObject& obj) : 
//				SceneEvent(sc), 
//				gameObject(obj)
//			{}
//
//			GameObject& gameObject;
//		};
//	}
//
//	namespace imp
//	{
//		template<typename T> requires std::derived_from<T, SceneEvent>
//		inline SceneEventType GetSceneEventType()
//		{
//			using std::is_same_v;
//			if constexpr (is_same_v<T, Events::SceneObjectCreated>) return SceneEventType::eObjectCreated;
//			else if constexpr (is_same_v<T, Events::SceneObjectDeleted>) return SceneEventType::eObjectDeleted;
//			else return SceneEventType::eUnknown;
//		}
//	}
//}
