#include <lunar/core/scene.hpp>
#include <lunar/debug/log.hpp>

namespace Core
{
	Scene::Scene(const Fs::Path& path)
		: name(),
		nameHash(0),
		objects(),
		Identifiable()
	{
		fromFile(path);
	}

	Scene::Scene(const std::string& name)
		: name(name),
		nameHash(std::hash<std::string>{}(name)),
		objects(),
		Identifiable()
	{
	}

	Scene::Scene()
		: name("Default Scene"),
		nameHash(std::hash<std::string>{}("Default Scene")),
		objects(),
		Identifiable()
	{
	}

	Scene& getActiveScene()
	{
        static auto active = Scene(Fs::dataDirectory().append("main_scene.json"));
        return active;
	}

	Scene& getSceneByName(const char* name)
	{
		return getActiveScene();
	}

    // TODO:
    Scene& getSceneById(Identifiable::NativeType id)
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

    GameObject& Scene::getGameObject(Identifiable::NativeType id)
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
		auto hash = std::hash<std::string>{}(name);
		for (auto& game_object : objects)
		{
			if (game_object.getNameHash() == hash)
				return game_object;
		}

		DEBUG_ERROR("Called on inexistent game object (name: {})", name);
        throw;
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
				objects.emplace_back(gameobj_data);
			}
		}
	}

	std::vector<GameObject>& Scene::getGameObjects()
	{
		return objects;
	}
}
