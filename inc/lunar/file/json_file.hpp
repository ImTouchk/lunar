#pragma once
#include <lunar/file/filesystem.hpp>
#include <lunar/api.hpp>
#include <nlohmann/json.hpp>
#include <concepts>

namespace Fs
{
	class LUNAR_API JsonFile : public Resource
	{
	public:
		JsonFile(const Path& path);
		JsonFile() = default;

		bool fromFile(const Path& path) override;
		void toFile(const Path& path) override;

		nlohmann::json content;
	};

	class LUNAR_API JsonObject : public Resource
	{
	public:
		bool fromFile(const Path& path) override;
		void toFile(const Path& path) override;

		virtual void fromJson(nlohmann::json& json);
		virtual nlohmann::json toJson();
	};

}

template<typename T>
concept IsJsonSerializable = requires (T, const T & memoryForm, const nlohmann::json & serializedForm) {

	{ T::Deserialize(serializedForm) } -> std::same_as<T>;
	{ T::Serialize(memoryForm) } -> std::same_as<nlohmann::json>;
};
