#include <lunar/core/gameobject.hpp>
#include <lunar/core/component.hpp>
#include <lunar/debug/log.hpp>
#include <functional>
#include <map>

namespace Core
{
	GameObject::GameObject(nlohmann::json& json)
		: nameHash(),
        name(),
		components(),
        Identifiable()
	{
		fromJson(json);
	}

	GameObject::GameObject(std::string name)
		: nameHash(std::hash<std::string>{}(name)),
        name(name),
		components(),
        Identifiable()
	{
	}

	GameObject::GameObject()
		: nameHash(std::hash<std::string>{}("Default GameObject")),
		components(),
        name("Default GameObject"),
        Identifiable()
	{
	}

    size_t GameObject::getNameHash() const
    {
        return nameHash;
    }

    const std::string &GameObject::getName() const
    {
        return name;
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

		static std::map<std::string, std::function<comp_ptr(json_obj&)>> component_types = {
			{
				"core.scriptComponent", [](auto& json) -> auto {
					std::string script_name = json["scriptName"];
					DEBUG_LOG("Loading script component \"{}\"...", script_name);
					return std::make_unique<ScriptComponent>(script_name);
				},
			},
			{
				"core.transformComponent", [](auto& json) -> auto {
					// TODO: handle error cases
					DEBUG_LOG("Loading transform component...");
					glm::vec3 pos = { json["position"]["x"], json["position"]["y"], json["position"]["z"] };
					glm::vec3 rot = { json["rotation"]["x"], json["rotation"]["y"], json["rotation"]["z"] };
					glm::vec3 scale = { json["scale"]["x"], json["scale"]["y"], json["scale"]["z"] };
					return std::make_unique<TransformComponent>();

					//auto& _tr = *((TransformComponent*)res.get());
					//_tr.position = pos;
					//_tr.rotation = rot;
					//_tr.scale = scale;
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

				std::unique_ptr<Component> component = component_types.at(comp_data["type"])(comp_data);
				components.push_back(std::move(component));
			}
		}
	}
}
