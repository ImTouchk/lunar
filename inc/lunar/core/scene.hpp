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

		void fromJson(nlohmann::json& json) override;

		std::vector<GameObject>& getGameObjects();

	private:
		size_t nameHash;
		std::string name;
		std::vector<GameObject> objects;
	};
}
