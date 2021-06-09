#include <assert.h>
#include <ctype.h>
#include <filesystem>

#include "FSoperations.h"
#include "FctArchive.h"

namespace FCT {
	namespace FS {
		
		int fileOpen(FILE** f, const char* name, const char* mode) {
#if defined _WIN32
			return fopen_s(f, name, mode);
#elif defined __linux__
			int ret = 0;
			assert(f);
			*f = fopen(name, mode);
			/* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
			if (!*f)
				ret = errno;
			return ret;
#endif
		}

		// create the folders from a list of files
		void createDirectories(std::vector<std::string> fileIndex) {
			for (auto& iter : fileIndex) {
				std::filesystem::path folder = std::filesystem::path(iter).parent_path();
				if (!std::filesystem::exists(folder))
					std::filesystem::create_directories(folder);
			}
		}

		void createNewDirectory(std::filesystem::path path){
			if(!std::filesystem::exists(path.parent_path()))
				std::filesystem::create_directories(path.parent_path());
		}

		// gets files recursively through directories
		std::vector<std::filesystem::path> expandDirectory(std::filesystem::path folderPath)
		{
			std::vector<std::filesystem::path> pathVector;
			for (std::filesystem::recursive_directory_iterator i(folderPath), end; i != end; ++i) {
				std::filesystem::path absPath = std::filesystem::absolute(i->path());
				if (std::filesystem::is_regular_file(absPath)) {
					pathVector.push_back(absPath);
				}
			}
			return pathVector;
		}

		// Remove everything behind the rootdir
		std::string FormatPath(std::filesystem::path rootDir, std::filesystem::path filePath) {
			return std::filesystem::relative(filePath, rootDir).generic_string();
			/*uint64_t wordStartPos = filePath.generic_string().find(rootDir.generic_string());
			return std::filesystem::path(filePath).generic_string().erase(0,wordStartPos);*/
		}
	}
}