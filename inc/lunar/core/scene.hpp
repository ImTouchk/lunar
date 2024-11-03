#pragma once
#include <lunar/core/gameobject.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

namespace Script
{
	class LUNAR_API VirtualMachine;
}

namespace Core
{
	class LUNAR_API Scene : public Identifiable
	{
	public:
		Scene(
			const std::string& name,
			std::shared_ptr<Script::VirtualMachine>& scriptingVm
		);
		Scene() = default;

		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

        size_t getNameHash() const;
        const std::string& getName() const;
        GameObject& getGameObject(const char* name);
        GameObject& getGameObject(Identifiable::NativeType id);

		std::vector<GameObject>& getGameObjects();

	private:
		size_t nameHash = SIZE_MAX;
		std::string name = "Untitled Scene";
		std::vector<GameObject> objects = {};
		std::shared_ptr<Script::VirtualMachine>& scriptingVm;
	};

	struct LUNAR_API SceneBuilder
	{
		using ComponentJsonParser = std::function<Component*(const nlohmann::json&)>;

		SceneBuilder() = default;
		~SceneBuilder() = default;
		
		SceneBuilder& useDefaultComponentParsers();
		SceneBuilder& useComponentParser(
			const std::string& componentName,
			const ComponentJsonParser& parser
		);
		SceneBuilder& fromJsonFile(const Fs::Path& path);

		SceneBuilder& setName(const std::string_view& name);
		SceneBuilder& useScriptingVm(std::shared_ptr<Script::VirtualMachine>& vm);
		std::shared_ptr<Scene> create();

	private:
		void parseGameObject(
			const nlohmann::json& json, 
			Scene* scene,
			GameObject* parent = nullptr
		);

		std::string name = "Untitled Scene";
		std::vector<GameObject> objects = {};
		std::shared_ptr<Script::VirtualMachine> scriptingVm = nullptr;
		std::unordered_map<std::string, ComponentJsonParser> componentParsers = {};
		Fs::Path jsonFile = "";
	};
}
