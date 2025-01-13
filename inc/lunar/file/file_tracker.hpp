#pragma once
#include <lunar/api.hpp>
#include <lunar/file/filesystem.hpp>
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

namespace Fs
{
	class LUNAR_API FileTracker
	{
	public:
		using EventHandler = std::function<void(Fs::Path)>;

		FileTracker() = default;
		~FileTracker() = default;

		FileTracker& trackPath(const Fs::Path& path);
		FileTracker& trackPath(const Fs::Path& path, const EventHandler& handler);
		FileTracker& update();

		[[nodiscard]] bool hasChanged(const Fs::Path& path);
		[[nodiscard]] std::vector<Fs::Path> getChangedPaths() const;
	private:
		struct PathData
		{
			bool changedFlag = false;
			Fs::Path path;
			size_t hash;
			std::optional<EventHandler> handler;
		};

		size_t getFileHash(const Fs::Path& file);

		std::vector<PathData> list = {};
	};
}
