#include <core/scene.hpp>
#include <debug/log.hpp>

namespace Core
{
	Scene::Scene(const Fs::Path& path)
		: name(),
		nameHash(0)
	{
		fromFile(path);
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
				objects.push_back(GameObject(gameobj_data));
			}
		}


	}

	std::vector<GameObject>& Scene::getGameObjects()
	{
		return objects;
	}
}
