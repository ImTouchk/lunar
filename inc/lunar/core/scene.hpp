#pragma once
#include <lunar/core/gameobject.hpp>
#include <lunar/file/json_file.hpp>
#include <lunar/utils/identifiable.hpp>
#include <string>
#include <vector>

namespace Core
{
	class LUNAR_API Scene : public Fs::JsonObject, public Identifiable
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
        GameObject& getGameObject(Identifiable::NativeType id);

		void fromJson(nlohmann::json& json) override;

		std::vector<GameObject>& getGameObjects();

	private:
		size_t nameHash;
		std::string name;
		std::vector<GameObject> objects;
	};

    LUNAR_API Scene& getActiveScene();
    LUNAR_API Scene& getSceneByName(const char* name);
    LUNAR_API Scene& getSceneById(Identifiable::NativeType id);
}
