#include <filesystem>
#include <iostream>
#include <limits.h>
#include <sstream>

#if defined __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#endif

#include "FileParser.h"
#include "FSoperations.h"

namespace FCT {
#pragma region Constructors & Destructor
	FileParser::FileParser() {
		FileSize = 0;
		ChunkCount = 0;
		LastChunkContentSize = 0;
		RawFilePath = "";
		FormattedFilePath = "";
	}

	// parse from file
	FileParser::FileParser(std::filesystem::path filePath, std::filesystem::path rootDir, uint16_t chunkSize) {
		// TODO: FILE NAME FORMATTING, WE JUST INSERT THE PATH FOR NOW
		RawFilePath = filePath;
		FormattedFilePath = FCT::FS::FormatPath(rootDir, filePath);

		// get file size (TODO: multi-platform)
#if defined _WIN32
		struct __stat64 stat_buf;
		if (_stat64(filePath.string().c_str(), &stat_buf) != 0)
			throw "Failed to stat file";
#elif defined __linux__
		struct stat64 stat_buf;
		if (stat64(filePath.string().c_str(), &stat_buf) != 0)
			throw "Failed to stat file";
#endif
		FileSize = stat_buf.st_size;
		// get chunk count
		if (FileSize > 0) {
			//ChunkCount = (uint32_t)(FileSize / chunkSize); // old way
			ChunkCount = (uint32_t)(floor((float)(FileSize / chunkSize)));

			// add one more chunk if we have a remainder
			LastChunkContentSize = FileSize % chunkSize;
			ChunkCount += (LastChunkContentSize != 0 ? 1 : 0);
			if (ChunkCount >= UINT32_MAX)
				throw "Chunk count too high. Try increasing chunk size";
		}
		else {
			ChunkCount = 0;
			LastChunkContentSize = 0;
		}

		// now that we're done, we can generate the header
		uint16_t filePathLength = static_cast<uint16_t>(FormattedFilePath.length());
		pushUint32(&Header, ChunkCount);
		pushUint16(&Header, LastChunkContentSize);
		pushUint16(&Header, filePathLength);
		Header.insert(Header.end(), (uint8_t*)FormattedFilePath.c_str(), (uint8_t*)(FormattedFilePath.c_str() + FormattedFilePath.length()));
		/*for (int i = 0; i < filePathLength; i++)
			Header.push_back(fileNameCharArray[i]);*/
	}

	// parse from header data in an archive
	FileParser::FileParser(FILE* archiveFile, uint16_t chunkSize) {
		uint16_t filePathLength = 0;
		RawFilePath = "";

		fread(&ChunkCount, 4, 1, archiveFile);
		fread(&LastChunkContentSize, 2, 1, archiveFile);
		fread(&filePathLength, 2, 1, archiveFile);

		if (ChunkCount < 0 || ChunkCount >= UINT32_MAX ||
			LastChunkContentSize < 0 || LastChunkContentSize > chunkSize ||
			filePathLength < 0 || filePathLength > MaxPathLength) {
			throw "Failed to parse from header data";
		}

		FileSize = ((uint64_t)ChunkCount) * ((uint64_t)chunkSize) + ((uint64_t)LastChunkContentSize);

		char* filePath = new char[(uint64_t)filePathLength + 1];
		filePath[filePathLength] = 0;
		fread(filePath, 1, filePathLength, archiveFile);
		FormattedFilePath = std::string(filePath);
		delete[] filePath;

		Header.insert(Header.end(), (uint8_t*)&ChunkCount, (uint8_t*)&ChunkCount + 4);
		Header.insert(Header.end(), (uint8_t*)&LastChunkContentSize, (uint8_t*)&LastChunkContentSize + 2);
		Header.insert(Header.end(), (uint8_t*)&filePathLength, (uint8_t*)&filePathLength + 2);
		Header.insert(Header.end(), (uint8_t*)FormattedFilePath.c_str(), (uint8_t*)FormattedFilePath.c_str() + FormattedFilePath.length());
	}

	FileParser::~FileParser() {
	}
#pragma endregion 

	std::ostream& operator<<(std::ostream& os, const FileParser &file){
		os << "Filename: " << file.FormattedFilePath << std::endl;
		os << "Filesize: " << file.FileSize << std::endl;
		os << "Chunk count: " << file.ChunkCount << std::endl;
		os << "Last Chunk remainder: " << file.LastChunkContentSize << std::endl;
		return os;
	} 
}