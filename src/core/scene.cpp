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

	Scene& getActiveScene()
	{
        static auto active = Scene(Fs::dataDirectory().append("main_scene.json"));
        return active;
	}

    // TODO:
    Scene& getScene(size_t nameHash)
    {
        return getActiveScene();
    }

    const std::string& Scene::getName() const
    {
        return name;
    }

    size_t Scene::getNameHash() const
    {
        return nameHash;
    }

    GameObject& Scene::getGameObject(int id)
    {
        for(auto& game_object : objects)
        {
            if(game_object.getId() == id)
                return game_object;
        }

        DEBUG_ERROR("Called on inexistent game object (id: {})", id);
        throw;
    }

    GameObject& Scene::getGameObject(const char* name)
    {
        size_t hash = std::hash<std::string>{}(name);
        return getGameObject(hash);
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
                auto& game_object = objects.emplace_back();
                game_object.fromJson(gameobj_data);
			}
		}
	}

	std::vector<GameObject>& Scene::getGameObjects()
	{
		return objects;
	}
}
