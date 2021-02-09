#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

#include "FileParser.h"


namespace FCT {
#if defined _WIN32
	constexpr char PathDelimiter = '\\';
#elif defined __linux__
	constexpr char PathDelimiter = '/';
#endif
	constexpr int MaxChunkSize = 65535;
	constexpr int ArchiveHeaderSize = 5;
	class FctArchive {
	private:
		uint16_t ChunkSize;
		std::vector<FileParser> FileIndex;
		bool FileIndexStale;
		FILE* ArchiveFile;

		// only seeks over header 
		void seekFileHeader(FileParser file);
		// only seeks over data
		void seekFileData(FileParser file);
		// seeks both header and data
		void seekFile(FileParser file);

	public:
		std::filesystem::path ArchivePath;
		FctArchive(std::string archivePath);
		FctArchive(std::string newArchivePath, uint16_t chunkSize);
		~FctArchive();

		int addFile(FileParser file);
		int removeFiles(std::vector<uint32_t> indices);

		int extractFiles(std::vector<uint32_t> indices);
		std::vector<int> extractAll(std::string outputPath);

		void generateFileIndex();

		// only use outside of class methods as this is rather slow
		std::vector<FileParser> getFileIndex();
		uint16_t getChunkSize();
	};
}
