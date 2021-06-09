#pragma once
#include <vector>
#include <string>

namespace FCT {
	constexpr uint16_t MaxPathLength = 65526;

	class FileParser
	{
	private:
		void pushUint32(std::vector<uint8_t>* arrayOfByte, uint32_t paramInt) {
			arrayOfByte->push_back(paramInt);
			arrayOfByte->push_back(paramInt >> 8);
			arrayOfByte->push_back(paramInt >> 16);
			arrayOfByte->push_back(paramInt >> 24);
		}

		void pushUint16(std::vector<uint8_t>* arrayOfByte, uint16_t paramInt) {
			arrayOfByte->push_back((uint8_t)paramInt);
			arrayOfByte->push_back(paramInt >> 8);
		}
	public:
		int64_t FileSize;
		uint32_t ChunkCount;
		uint16_t LastChunkContentSize;
		std::filesystem::path RawFilePath;
		std::string FormattedFilePath;
		std::vector<uint8_t> Header;

		FileParser();
		FileParser(std::filesystem::path filePath, std::filesystem::path rootDir, uint16_t ChunkSize);
		FileParser(FILE* archiveFile, uint16_t chunkSize);
		~FileParser();

		
		bool operator<(const FileParser &file);
		bool operator>(const FileParser &file);
		bool operator==(const FileParser &file);
		bool operator!=(const FileParser &file);
		
		friend std::ostream& operator<<(std::ostream& os, const FileParser &file);
	};
	std::string printFileVerbose(FileParser file);
}