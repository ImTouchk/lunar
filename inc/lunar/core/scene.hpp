#pragma once
#include <lunar/core/gameobject.hpp>
#include <lunar/file/json_file.hpp>
#include <string>
#include <vector>

namespace Core
{
	class LUNAR_API Scene : public Fs::JsonObject
	{
	public:
		Scene(const Fs::Path& path);
		Scene(const std::string& name);
		Scene();

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        size_t getNameHash() const;
        const std::string& getName() const;
        GameObject& getGameObject(const char* name);
        GameObject& getGameObject(int id);

		void fromJson(nlohmann::json& json) override;

		std::vector<GameObject>& getGameObjects();

	private:
		size_t nameHash;
		std::string name;
		std::vector<GameObject> objects;
	};

    LUNAR_API Scene& getActiveScene();
    LUNAR_API Scene& getScene(const char* name);
    LUNAR_API Scene& getScene(size_t nameHash);
}
