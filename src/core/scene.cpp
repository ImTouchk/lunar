#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>

namespace Core
{
	Scene::Scene(const Fs::Path& path)
		: name(),
		nameHash(0),
		objects()
	{
		fromFile(path);
	}

	Scene::Scene(const std::string& name)
		: name(name),
		nameHash(std::hash<std::string>{}(name)),
		objects()
	{
	}

	Scene::Scene()
		: name("Default Scene"),
		nameHash(std::hash<std::string>{}("Default Scene")),
		objects()
	{
	}

	void Scene::fromJson(nlohmann::json& json)
	{
		name = json["name"];
		nameHash = std::hash<std::string>{}(name);
		DEBUG_LOG("Loading scene \"{}\" from json object.", name);
		if (json.contains("gameObjects"))
		{
			auto& gameobjects_json = json["gameObjects"];
			for (auto& [key, gameobj_data] : gameobjects_json.items())
			{
				objects.push_back(std::move(GameObject(gameobj_data)));
			}
		}


	}

	std::vector<GameObject>& Scene::getGameObjects()
	{
		return objects;
	}
}
