#include <lunar/core/gameobject.hpp>
#include <lunar/core/component.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>
#include <functional>
#include <map>

namespace Core
{
	GameObject::GameObject(const std::string_view& name, Scene* scene, GameObject* parent) 
		: name(name),
		nameHash(Lunar::imp::fnv1a_hash(this->name)),
		components(),
		transform(),
		scene(scene),
		parent(parent != nullptr ? parent->id : -1),
		Identifiable()
	{
	}

	GameObject::GameObject()
		: name(),
		nameHash(),
		components(),
		transform(),
		scene(nullptr),
		parent(),
		Identifiable(-1)
	{
	}

	GameObject::~GameObject()
	{
		for (size_t i = 0; i < components.size(); i++)
			delete components[i];
	}

	GameObject* GameObject::getParent()
	{
		if (parent != -1)
			return &scene->getGameObject(parent);

		return nullptr;
	}

	void GameObject::addComponent(Component* constructed)
	{
		constructed->_scene = scene;
		constructed->_gameObject = id;
		components.push_back(constructed);
	}

	Component* GameObject::getComponent(const std::type_info& ty)
	{
		for (Component* component : components)
			if (typeid(*component).hash_code() == ty.hash_code())
				return component;

		return nullptr;
	}

	Identifiable::NativeType GameObject::getParentId() const
	{
		return parent;
	}

	TransformComponent& GameObject::getTransform()
	{
		return transform;
	}

	const TransformComponent& GameObject::getTransform() const
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
		return scene;
	}

	void GameObject::update()
	{
		for (auto& component : components)
		{
			if (component->_getClassFlags() & ComponentClassFlagBits::eUpdateable)
				component->update();
		}
		//for (auto& component_ptr : components)
		//{
		//	Component* component = component_ptr.get();
		//	if (component->isUpdateable())
		//		component->update();
		//}
	}

	//void GameObject::fromJson(nlohmann::json& json)
	//{
	//	using json_obj = nlohmann::json;
	//	using comp_ptr = std::unique_ptr<Component>;

	//	static std::map<std::string, std::function<void(json_obj&, comp_ptr&)>> component_types = {
	//		{
	//			"core.scriptComponent", [](auto& json, auto& res) {
	//				std::string script_name = json["scriptName"];
	//				res = std::make_unique<ScriptComponent>(script_name);
	//			},
	//		}
	//	};

 //       name = json["name"];
	//	nameHash = std::hash<std::string>{}(name);
	//	DEBUG_LOG("Loading GameObject \"{}\" from json object.", name);
	//	if (json.contains("components"))
	//	{
	//		auto& components_json = json["components"];
	//		for (auto& [key, comp_data] : components_json.items())
	//		{
	//			std::string type = comp_data["type"];
	//			if (!component_types.contains(type))
	//			{
	//				DEBUG_ERROR("Component type \"{}\" does not exist.", type);
	//				throw;
	//			}

	//			std::unique_ptr<Component> component_ptr;
	//			component_types.at(comp_data["type"])(comp_data, component_ptr);
	//			if(component_ptr != nullptr)
	//				components.push_back(std::move(component_ptr));
	//		}
	//	}
	//	
	//	if (json.contains("transform"))
	//	{
	//		auto& transform_json = json["transform"];
	//		auto& position = transform_json["position"];
	//		auto& rotation = transform_json["rotation"];
	//		auto& scale = transform_json["scale"];
	//		transform.position = glm::dvec3 { position["x"], position["y"], position["z"] };
	//		transform.rotation = glm::dvec3 { rotation["x"], rotation["y"], rotation["z"] };
	//		transform.scale = glm::dvec3 { scale["x"], scale["y"], scale["z"] };
	//	}
	//}
}
