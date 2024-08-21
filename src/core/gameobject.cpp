#include <lunar/core/gameobject.hpp>
#include <lunar/core/component.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>
#include <functional>
#include <map>

namespace Core
{
	GameObject::GameObject(const GameObject& other)
		: nameHash(other.nameHash),
		name(other.name),
		components(),
		transform(other.transform),
		parent(other.parent),
		scene(other.scene),
		Identifiable(other)
	{
		//for (size_t i = 0; i < components.size(); i++)
		//{
		//	components.push_back(std::move(other.components[i]));
		//}
	}

	GameObject::GameObject(GameObject&& other)
		: nameHash(other.nameHash),
		name(other.name),
		components(),
		transform(other.transform),
		parent(other.parent),
		scene(other.scene),
		Identifiable(other)
	{
		for (size_t i = 0; i < components.size(); i++)
		{
			components.push_back(std::move(other.components[i]));
		}
	}

	GameObject::GameObject(nlohmann::json& json)
		: nameHash(),
        name(),
		components(),
		parent(-1),
		transform(),
        Identifiable()
	{
		fromJson(json);
	}

	GameObject::GameObject(std::string name)
		: nameHash(std::hash<std::string>{}(name)),
        name(name),
		components(),
		parent(-1),
		scene(-1),
		transform(),
        Identifiable()
	{
	}

	GameObject::GameObject()
		: nameHash(std::hash<std::string>{}("Default GameObject")),
		components(),
        name("Default GameObject"),
		parent(-1),
		scene(-1),
        Identifiable()
	{
	}

	GameObject::~GameObject()
	{
		// TODO: unload script components
	}

	GameObject& GameObject::operator=(GameObject&& other)
	{
		name = other.name;
		nameHash = other.nameHash;
		id = other.id;
		components = std::vector<std::unique_ptr<Component>>();
		transform = other.transform;
		// TODO:
		return *this;
	}

	GameObject& GameObject::operator=(const GameObject& other)
	{
		name = other.name;
		nameHash = other.nameHash;
		id = other.id;
		transform = other.transform;
		components = std::vector<std::unique_ptr<Component>>();


		// TODO:
		return *this;
	}

	GameObject* GameObject::getParent()
	{
		if (parent == -1)
			return nullptr;
		else return &getParentScene()
						->getGameObject(parent);
	}

	Identifiable::NativeType GameObject::getParentId() const
	{
		return parent;
	}

	TransformComponent& GameObject::getTransform()
	{
		return transform;
	}

    size_t GameObject::getNameHash() const
    {
        return nameHash;
    }

    const std::string &GameObject::getName() const
    {
        return name;
    }

	Scene* GameObject::getParentScene()
	{
		if (scene == -1)
			return &getActiveScene();
		else 
			return &getSceneById(scene);
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
					res = std::make_unique<ScriptComponent>(script_name);
				},
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

				std::unique_ptr<Component> component_ptr;
				component_types.at(comp_data["type"])(comp_data, component_ptr);
				if(component_ptr != nullptr)
					components.push_back(std::move(component_ptr));
			}
		}
		
		if (json.contains("transform"))
		{
			auto& transform_json = json["transform"];
			auto& position = transform_json["position"];
			auto& rotation = transform_json["rotation"];
			auto& scale = transform_json["scale"];
			transform.position = glm::dvec3 { position["x"], position["y"], position["z"] };
			transform.rotation = glm::dvec3 { rotation["x"], rotation["y"], rotation["z"] };
			transform.scale = glm::dvec3 { scale["x"], scale["y"], scale["z"] };
		}
	}
}
