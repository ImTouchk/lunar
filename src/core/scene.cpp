#include <lunar/core/scene.hpp>
#include <lunar/core/time.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/debug/log.hpp>

namespace lunar
{
	LUNAR_HANDLE_IMPL(Scene);
	LUNAR_HANDLE_IMPL(GameObject);

	Scene_T::Scene_T(const std::string_view& name) noexcept
		: name(name)
	{
	}

	Scene_T::~Scene_T() noexcept
	{
	}

	GameObject Scene_T::getGameObject(size_t number)
	{
		return make_handle(objects, number);
	}

	GameObject Scene_T::getGameObject(const std::string_view& name)
	{
		for (size_t i = 0; i < objects.size(); i++)
			if (objects[i].getName().compare(name) == 0)
				return make_handle(objects, i);

		return nullptr;
	}

	GameObject Scene_T::createGameObject(const std::string_view& name, GameObject parent)
	{
		objects.emplace_back(this, name, parent);
		return make_handle(objects);
	}
}

//namespace Core
//{
//	SceneBuilder& SceneBuilder::useScriptingVm(std::shared_ptr<Script::VirtualMachine>& vm)
//	{
//		scriptingVm = vm;
//		return *this;
//	}
//
//	SceneBuilder& SceneBuilder::setName(const std::string_view& name)
//	{
//		this->name = name;
//		return *this;
//	}
//
//	SceneBuilder& SceneBuilder::useCoreSerializers()
//	{
//		useCustomClassSerializer("core.scriptComponent", [](const nlohmann::json&) -> std::shared_ptr<Component> {
//			DEBUG_LOG("Hello, world!");
//			return nullptr;
//		});
//
//		useCustomClassSerializer("core.render.camera", [](const nlohmann::json&) -> std::shared_ptr<Component> {
//			return std::make_shared<Render::Camera>();
//		});
//		return *this;
//	}
//
//	SceneBuilder& SceneBuilder::useCustomClassSerializer(const std::string& name, const ComponentJsonParser& parser)
//	{
//		componentParsers[name] = parser;
//		return *this;
//	}
//
//	SceneBuilder& SceneBuilder::fromJsonFile(const Fs::Path& path)
//	{
//		jsonFile = path;
//		return *this;
//	}
//
//	void SceneBuilder::parseGameObject
//	(
//		const nlohmann::json& json,
//		Scene* scene,
//		GameObject* parent
//	)
//	{
//		const std::string name = json.contains("name")
//			? json["name"]
//			: "GameObject";
//
//		auto& game_object = scene->createGameObject(name, parent);
//
//		if (json.contains("transform"))
//		{
//			auto& transform_data = json["transform"];
//			auto& transform      = game_object.getTransform();
//			
//			if (transform_data.contains("position"))
//			{
//				auto& pos_data = transform_data["position"];
//				auto& position = transform.position;
//				position.x = pos_data.value("x", 0.f);
//				position.y = pos_data.value("y", 0.f);
//				position.z = pos_data.value("z", 0.f);
//			}
//
//			if (transform_data.contains("rotation"))
//			{
//				auto& rot_data = transform_data["rotation"];
//				auto& rotation = transform.rotation;
//				rotation.x = rot_data.value("x", 0.f);
//				rotation.y = rot_data.value("y", 0.f);
//				rotation.z = rot_data.value("z", 0.f);
//			}
//
//			if (transform_data.contains("scale"))
//			{
//				auto& scale_data = transform_data["scale"];
//				auto& scale = transform.scale;
//				scale.x = scale_data.value("x", 0.f);
//				scale.y = scale_data.value("y", 0.f);
//				scale.z = scale_data.value("z", 0.f);
//			}
//		}
//
//		if (json.contains("components"))
//		{
//			auto& components = json["components"];
//			for (auto& [key, component] : components.items())
//			{
//				const std::string component_type = component["type"];
//				if (!componentParsers.contains(component_type))
//				{
//					DEBUG_WARN("Found component '{}' inside scene file '{}' but no suitable parser for it. Skipping...", component_type, jsonFile.string());
//					continue;
//				}
//
//				auto parsed_component = componentParsers[component_type](component);
//				if (parsed_component == nullptr)
//				{
//					DEBUG_WARN("Component of type '{}' was parsed but result was NULL. Skipping...", component_type);
//					continue;
//				}
//
//				game_object.addComponent(parsed_component);
//				DEBUG_LOG("Parsed component of type '{}'...", component_type);
//			}
//		}
//
//		if (json.contains("children"))
//		{
//			auto& children = json["children"];
//			for (auto& [key, child] : children.items())
//				parseGameObject(child, scene, &game_object);
//		}
//	}
//
//	std::shared_ptr<Scene> SceneBuilder::create()
//	{
//		auto scene = std::make_shared<Scene>(name, scriptingVm);
//
//		if (!std::filesystem::exists(jsonFile))
//			return scene;
//
//		auto file = Fs::JsonFile(jsonFile);
//		const auto& json = file.content;
//
//		name = json["name"];
//		if (!json.contains("gameObjects"))
//			return scene;
//
//		auto& game_objects = json["gameObjects"];
//		for (auto& [key, game_object] : game_objects.items())
//			parseGameObject(game_object, scene.get());
//
//		return scene;
//	}
//
//	Scene::Scene
//	(
//		const std::string& name,
//		std::shared_ptr<Script::VirtualMachine>& scriptingVm
//	) : name(name),
//		nameHash(std::hash<std::string>{}(name)),
//		scriptingVm(scriptingVm),
//		objects(),
//		Identifiable()
//	{
//		physicsWorld = physicsCommon.createPhysicsWorld();
//	}
//
//	const std::string& Scene::getName() const
//	{
//		return name;
//	}
//
//	size_t Scene::getNameHash() const
//	{
//		return nameHash;
//	}
//
//	void Scene::deleteGameObject(Identifiable::NativeType id)
//	{
//		// TODO: Rethink this function; it somehow messes up the ids of all objects
//		
//		auto& object       = getGameObject(id);
//
//		auto  delete_event = Events::SceneObjectDeleted(*this, object);
//		triggerEvent(SceneEventType::eObjectDeleted, delete_event);
//
//		auto  children = object.getChildren();
//		for (size_t i = 0; i < children.size(); i++)
//			deleteGameObject(children[i]->getId());
//
//		for (size_t i = 0; i < objects.size(); i++)
//		{
//			if (objects[i].getId() != id)
//				continue;
//
//			auto name = objects[i].getName();
//
//			std::iter_swap(objects.begin() + i, objects.end() - 1);
//			objects.pop_back();
//			
//			DEBUG_LOG("Deleted object '{}'", name);
//			return;
//		}
//
//		DEBUG_ASSERT(true, "deleteGameObject called on inexistent id");
//	}
//
//	GameObject& Scene::createGameObject(const std::string_view& name, GameObject* parent)
//	{
//		auto& created_object = objects.emplace_back(name, this, parent);
//		auto  created_event  = Events::SceneObjectCreated(*this, created_object);
//		triggerEvent(SceneEventType::eObjectCreated, created_event);
//		return created_object;
//	}
//
//    GameObject& Scene::getGameObject(Identifiable::NativeType id)
//    {
//        for(auto& game_object : objects)
//        {
//            if(game_object.getId() == id)
//                return game_object;
//        }
//
//        DEBUG_ERROR("Called on inexistent game object (id: {})", id);
//        throw;
//    }
//
//	const GameObject& Scene::getGameObject(Identifiable::NativeType id) const
//	{
//		for (auto& game_object : objects)
//		{
//			if (game_object.getId() == id)
//				return game_object;
//		}
//
//		DEBUG_ERROR("Called on inexistent game object (id: {})", id);
//		throw;
//	}
//
//    GameObject& Scene::getGameObject(const std::string_view& name)
//    {
//		size_t name_hash = Lunar::imp::fnv1a_hash(name);
//		for (auto& game_object : objects)
//		{
//			if (game_object.getNameHash() == name_hash)
//				return game_object;
//		}
//
//		DEBUG_ERROR("Called on inexistent game object (name: {})", name);
//        throw;
//    }
//
//	const GameObject& Scene::getGameObject(const std::string_view& name) const
//	{
//		size_t name_hash = Lunar::imp::fnv1a_hash(name);
//		for (auto& game_object : objects)
//		{
//			if (game_object.getNameHash() == name_hash)
//				return game_object;
//		}
//
//		DEBUG_ERROR("Called on inexistent game object (name: {})", name);
//		throw;
//	}
//
//	std::vector<GameObject>& Scene::getGameObjects()
//	{
//		return objects;
//	}
//
//	void Scene::setMainCamera(Render::Camera& camera)
//	{
//		mainCamera = camera
//			.getGameObject()
//			.getId();
//	}
//
//	Render::Camera* Scene::getMainCamera()
//	{
//		if (mainCamera == -1)
//			return nullptr;
//
//		return getGameObject(mainCamera)
//			.getComponent<Render::Camera>();
//	}
//
//	void Scene::update()
//	{
//		for (auto& object : getGameObjects())
//			object.update();
//
//		physicsWorld->update(Time::DeltaTime());
//	}
//
//	void Scene::renderUpdate(Render::RenderContext& context)
//	{
//		for (auto& object : getGameObjects())
//			object.renderUpdate(context);
//	}
//
//	void Scene::addEventListener(SceneEventType type, EventListener listener)
//	{
//		_addEventListener(static_cast<size_t>(type), listener);
//	}
//
//	void Scene::removeEventListener(SceneEventType type, EventListener listener)
//	{
//		_removeEventListener(static_cast<size_t>(type), listener);
//	}
//
//	void Scene::triggerEvent(SceneEventType type, Event& e)
//	{
//		_triggerEvent(static_cast<size_t>(type), e);
//	}
//}
