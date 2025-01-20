#pragma once
#include <lunar/core/gameobject.hpp>
#include <lunar/core/event.hpp>
#include <lunar/utils/collections.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

#include <reactphysics3d/reactphysics3d.h>

namespace Script { class LUNAR_API VirtualMachine; }
namespace Render { class LUNAR_API Camera; }

namespace lunar
{
	class LUNAR_API Scene_T : public EventHandler
	{
	public:
		Scene_T(const std::string_view& name) noexcept;
		Scene_T()                             noexcept = default;
		~Scene_T()                            noexcept;

		GameObject getGameObject(size_t number);
		GameObject getGameObject(const std::string_view& name);
		GameObject createGameObject
		(
			const std::string_view& name,
			GameObject              parent = nullptr
		);

	private:
		std::string          name         = "Scene";
		vector<GameObject_T> objects      = {};
		vector<Component>    components   = {};
		rp3d::PhysicsWorld*  physicsWorld = nullptr;

		friend class GameObject_T;
	};
}

namespace Core
{
	//class LUNAR_API Scene : public EventHandler
	//{
	//public:
	//	Scene(
	//		const std::string& name,
	//		std::shared_ptr<Script::VirtualMachine>& scriptingVm
	//	);
	//	Scene() = default;
	//	Scene(Scene&&)            = delete;
	//	Scene& operator=(Scene&&) = delete;

	//	void                     update();
	//	void                     renderUpdate(Render::RenderContext& context);
 //       const std::string&       getName() const;
 //       size_t                   getNameHash() const;
 //       GameObject&              getGameObject(const std::string_view& name);
	//	const GameObject&        getGameObject(const std::string_view& name) const;
 //       GameObject&              getGameObject(Identifiable::NativeType id);
	//	const GameObject&        getGameObject(Identifiable::NativeType id) const;
	//	std::vector<GameObject>& getGameObjects();
	//	GameObject&              createGameObject(const std::string_view& name, GameObject* parent = nullptr);
	//	void                     deleteGameObject(Identifiable::NativeType id);
	//	Render::Camera*          getMainCamera();
	//	void                     setMainCamera(Render::Camera& camera);
	//	void                     addEventListener(SceneEventType type, EventListener listener);
	//	void                     removeEventListener(SceneEventType type, EventListener listener);

	//	template<typename T>
	//	inline void              addEventListener(EventListener_T<T> listener)
	//	{
	//		auto type = imp::GetSceneEventType<T>();
	//		addEventListener(type, [listener](Core::Event& e) { listener(static_cast<T&>(e)); });
	//	}

	//private:
	//	void triggerEvent(SceneEventType type, Event& e);

	//	Identifiable::NativeType                 mainCamera    = -1;
	//	size_t                                   nameHash      = SIZE_MAX;
	//	std::string                              name          = "Untitled Scene";
	//	std::vector<GameObject>                  objects       = {};
	//	rp3d::PhysicsCommon                      physicsCommon = {};
	//	rp3d::PhysicsWorld*                      physicsWorld  = nullptr;
	//	std::shared_ptr<Script::VirtualMachine>& scriptingVm;
	//};

	//struct LUNAR_API SceneBuilder
	//{
	//	using ComponentJsonParser = std::function<std::shared_ptr<Component>(const nlohmann::json&)>;

	//	SceneBuilder() = default;
	//	~SceneBuilder() = default;
	//	
	//	SceneBuilder& useCoreSerializers();
	//	SceneBuilder& useCustomClassSerializer(
	//		const std::string& componentName,
	//		const ComponentJsonParser& parser
	//	);

	//	template<typename T> requires IsDerivedComponent<T> && IsJsonSerializable<T>
	//	SceneBuilder& useClassSerializer(const std::string& componentName) 
	//	{
	//		return useCustomClassSerializer(componentName, [](const nlohmann::json& json) -> std::shared_ptr<Component> {
	//			auto component = std::make_shared<Component>(T::Deserialize(json));
	//			return component;
	//		});
	//	}

	//	SceneBuilder& fromJsonFile(const Fs::Path& path);

	//	SceneBuilder& setName(const std::string_view& name);
	//	SceneBuilder& useScriptingVm(std::shared_ptr<Script::VirtualMachine>& vm);
	//	std::shared_ptr<Scene> create();

	//private:
	//	void parseGameObject(
	//		const nlohmann::json& json, 
	//		Scene* scene,
	//		GameObject* parent = nullptr
	//	);

	//	std::string name = "Untitled Scene";
	//	std::shared_ptr<Script::VirtualMachine> scriptingVm = nullptr;
	//	std::unordered_map<std::string, ComponentJsonParser> componentParsers = {};
	//	Fs::Path jsonFile = "";
	//};
}
