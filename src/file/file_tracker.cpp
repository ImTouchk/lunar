#include <lunar/file/file_tracker.hpp>
#include <lunar/debug.hpp>

namespace Fs
{
	FileTracker& FileTracker::trackPath(const Fs::Path& path)
	{
		list.push_back(PathData{
			.path = path,
			.hash = getFileHash(path)
		});

		return *this;
	}

	FileTracker& FileTracker::trackPath(const Fs::Path& path, const EventHandler& handler)
	{
		list.push_back(PathData {
			.path    = path,
			.hash    = getFileHash(path),
			.handler = handler,
		});

		return *this;
	}

	FileTracker& FileTracker::update()
	{
		for (auto& file : list)
		{
			size_t old = file.hash;

			file.changedFlag = false;
			file.hash = getFileHash(file.path);
			if (file.hash == old)
				continue;

			file.changedFlag = true;
			if (file.handler.has_value())
				file.handler.value()(file.path);
		}

		return *this;
	}

	std::vector<Fs::Path> FileTracker::getChangedPaths() const
	{
		std::vector<Fs::Path> changed_files = {};

		for (auto& file : list)
			if (file.changedFlag)
				changed_files.push_back(file.path);

		return changed_files;
	}
	 
	bool FileTracker::hasChanged(const Fs::Path& path)
	{
		for (auto& file : list)
		{
			if (file.path == path)
				return file.changedFlag;
		}

		throw;
		return false;
	}

	size_t FileTracker::getFileHash(const Fs::Path& path)
	{
		using namespace std::filesystem;
		return last_write_time(path)
				.time_since_epoch()
				.count();
	}
}
