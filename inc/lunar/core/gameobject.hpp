#pragma once
#include <lunar/core/component.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/debug.hpp>
#include <lunar/api.hpp>
#include <concepts>
#include <string>
#include <memory>
#include <vector>

namespace Core
{
	// TODO: sort components based on nameHash so binary search can be done
	class LUNAR_API Scene;

	class LUNAR_API GameObject : public Identifiable
	{
	public:
		GameObject(const std::string_view& name, Scene* scene, GameObject* parent = nullptr);
		GameObject();
		~GameObject();

		void update();
        size_t getNameHash() const;
        const std::string& getName() const;
		TransformComponent& getTransform();

		GameObject* getParent();
		Identifiable::NativeType getParentId() const;
		Scene* getParentScene();

		void addComponent(Component* constructed);

		template<typename T> requires IsDerivedComponent<T>
		T* getComponent()
		{
			return static_cast<T*>(getComponent(typeid(T)));
		}

		template <typename T, class... _Valty> requires IsDerivedComponent<T>
		T& addComponent(_Valty&&... ctor_values)
		{	
			DEBUG_ASSERT(getComponent<T>() == nullptr, "There can exist only one component of type <T> on a single gameobject.");
			T* new_component = new T(std::forward<_Valty>(ctor_values)...);
			components.push_back(new_component);
			return *new_component;
		}
		
	private:
		Component* getComponent(const std::type_info& ty);

        std::string name;
		size_t nameHash;

		TransformComponent transform;
		std::vector<Component*> components;
		
		Scene* scene;
		Identifiable::NativeType parent;	
		
		friend class Scene;
	};
}
