#pragma once
#include <string>
#include <vector>
#include <memory>
#include <concepts>
#include <core/component.hpp>
#include <file/json_file.hpp>

namespace Core
{
	// TODO: sort components based on nameHash so binary search can be done

	class GameObject : public Fs::JsonObject
	{
	public:
		GameObject(nlohmann::json& json);
		GameObject(std::string name);

		void fromJson(nlohmann::json& json);

		void update();

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

		struct ObjectData
		{
			int nameSize;
			const char* name;
		} data;

		size_t nameHash;
		std::vector<std::unique_ptr<Component>> components;
		
	};
}
