#pragma once
#include <filesystem>
#include "FileParser.h"

namespace FCT {
	namespace FS {
		int fileOpen(FILE** f, const char* name, const char* mode);
		std::vector<std::filesystem::path> expandDirectory(std::filesystem::path folderPath);
		void createNewDirectory(std::filesystem::path path);
		void createDirectories(std::vector<std::string> fileIndex);
		std::string FormatPath(std::filesystem::path rootDir, std::filesystem::path filePath);
	}
}