#pragma once
#include <lunar/core/component.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
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
		Component* getComponent(const char* name);

		template <typename T, class... Val> 
			requires std::derived_from<T, Component>
		T& addComponent(Val&&... ctor_values)
		{
			T* new_component = new T(std::forward<Val>(ctor_values)...);
			components.push_back(new_component);
			return *new_component;
		}
		

	private:
		friend class Scene;

        std::string name;
		size_t nameHash;

		TransformComponent transform;
		std::vector<Component*> components;
		
		Scene* scene;
		Identifiable::NativeType parent;	
	};
}
