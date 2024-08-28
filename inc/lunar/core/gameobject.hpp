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

	class LUNAR_API GameObject : public Fs::JsonObject, public Identifiable
	{
	public:
		GameObject(nlohmann::json& json);
		GameObject(std::string name);
		GameObject();
		~GameObject();

		GameObject(GameObject&& other);
		GameObject(const GameObject& other);
		GameObject& operator=(GameObject&& other);
		GameObject& operator=(const GameObject& other);

		void fromJson(nlohmann::json& json);

		void update();
        size_t getNameHash() const;
        const std::string& getName() const;
		TransformComponent& getTransform();

		GameObject* getParent();
		Identifiable::NativeType getParentId() const;
		Scene* getParentScene();

		template<typename T> requires std::derived_from<T, Component>
		T& addComponent() 
		{
			return *reinterpret_cast<T*>(
					components.emplace_back(std::make_unique<T>())
						.get()
					);
		}

		Component* getComponent(const char* name)
		{
			size_t name_hash = std::hash<std::string>{}(name);
			for (auto& component : components)
			{
				if (component.get()->getTypeHash() == name_hash)
					return component.get();
			}

			return nullptr;
		}

		template<typename T> requires std::derived_from<T, Component>
		inline T* getComponent()
		{
			if constexpr (std::is_same<T, TransformComponent>::value)
			{
				return &transform;
			}
			else
			{
				auto _default = T();
				return reinterpret_cast<T*>(
					getComponent(
						reinterpret_cast<Component*>(&_default)->getType()
					)
				);
			}

		}

		template<typename T> requires std::derived_from<T, Component>
		inline T& getComponentRef()
		{
			if constexpr (std::is_same<T, TransformComponent>::value)
			{
				return transform;
			}
			else
			{
				auto _default = T();
				return *reinterpret_cast<T*>(
					getComponent(
						reinterpret_cast<Component*>(&_default)->getType()
					)
				);
			}
		}

	private:
		friend class Scene;

		Identifiable::NativeType scene;
		Identifiable::NativeType parent;
		size_t nameHash;
        std::string name;
		TransformComponent transform;
		std::vector<std::unique_ptr<Component>> components;
		
	};
}
