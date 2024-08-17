#include <core/gameobject.hpp>
#include <debug/log.hpp>
#include <functional>
#include <map>

namespace Core
{
	GameObject::GameObject(nlohmann::json& json)
		: name(),
		nameHash(),
		components()
	{
		fromJson(json);
	}

	GameObject::GameObject(std::string name)
		: name(name),
		nameHash(std::hash<std::string>{}(name)),
		components()
	{
	}

	void GameObject::update()
	{
		for (auto& component_ptr : components)
		{
			Component* component = component_ptr.get();
			if (component->isUpdateable())
				component->update();
		}
	}

	void GameObject::fromJson(nlohmann::json& json)
	{
		using json_obj = nlohmann::json;
		using comp_ptr = std::unique_ptr<Component>;

		static std::map<std::string, std::function<void(json_obj&, comp_ptr&)>> component_types = {
			{ 
				"core.scriptComponent", [](auto& json, auto& res) {
					std::string script_name = json["scriptName"];
					DEBUG_LOG("Loading script component \"{}\"...", script_name);
					res = std::make_unique<ScriptComponent>(script_name);
				}
			}
		};

		name = json["name"];
		nameHash = std::hash<std::string>{}(name);
		DEBUG_LOG("Loading GameObject \"{}\" from json object.", name);
		if (json.contains("components"))
		{
			auto& components_json = json["components"];
			for (auto& [key, comp_data] : components_json.items())
			{
				std::string type = comp_data["type"];
				if (!component_types.contains(type))
				{
					DEBUG_ERROR("Component type \"{}\" does not exist.", type);
					throw;
				}

				std::unique_ptr<Component> component;
				component_types.at(comp_data["type"])(comp_data, component);
				components.push_back(std::move(component));
			}
		}
	}
}
