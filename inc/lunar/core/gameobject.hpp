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

	class LUNAR_API GameObject : public Fs::JsonObject, public Identifiable
	{
	public:
		GameObject(nlohmann::json& json);
		GameObject(std::string name);
		GameObject();

		void fromJson(nlohmann::json& json);

		void update();
        size_t getNameHash() const;
        const std::string& getName() const;

		template<typename T> requires std::derived_from<T, Component>
		void addComponent(const T& component) { components.push_back(std::make_unique<T>(component)); }

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
			auto _default = T();
			return reinterpret_cast<T*>(
				getComponent(
					reinterpret_cast<Component*>(&_default)->getType()
				)
			);
		}

		template<typename T> requires std::derived_from<T, Component>
		inline T& getComponentRef()
		{
			auto _default = T();
			return *reinterpret_cast<T*>(
				getComponent(
					reinterpret_cast<Component*>(&_default)->getType()
				)
			);
		}

	private:
		friend class Scene;

		size_t nameHash;
        std::string name;
		std::vector<std::unique_ptr<Component>> components;
		
	};
}
