#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "FileParser.h"

namespace FCT {
#if defined _WIN32
constexpr char PathDelimiter = '\\';
#elif defined __linux__
constexpr char PathDelimiter = '/';
#endif
constexpr int MaxChunkSize = 65535;
constexpr int ArchiveHeaderSize = 5;

const auto FileSort = [](FCT::FileParser &lhs, FCT::FileParser &rhs) {
            size_t lhsSlash = lhs.FormattedFilePath.find_last_of('/');
            size_t rhsSlash = rhs.FormattedFilePath.find_last_of('/');
            lhsSlash = lhsSlash == -1 ? 0 : lhsSlash;
            rhsSlash = rhsSlash == -1 ? 0 : rhsSlash;

            std::string lhsDir = lhs.FormattedFilePath.substr(0, lhsSlash);
            std::string rhsDir = rhs.FormattedFilePath.substr(0, rhsSlash);
            std::string lhsFile = lhs.FormattedFilePath.substr(lhsSlash);
            std::string rhsFile = rhs.FormattedFilePath.substr(rhsSlash);
            if (lhsDir == rhsDir) {
                return lhsFile < rhsFile;
            }
            return lhsDir < rhsDir;
        };

class FctArchive {
   private:
    uint16_t ChunkSize;
    std::vector<FileParser> FileIndex;
    bool FileIndexStale;
    FILE *ArchiveFile;

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
    int removeFiles(std::vector<uint32_t> indices, bool verbose);
    int sortFiles(bool verbose, bool output = false);

    int extractFiles(std::string outputPath, std::vector<uint32_t> indices,
                     bool verbose);
    std::vector<int> extractAll(std::string outputPath, bool verbose);

    void generateFileIndex();

    // only use outside of class methods as this is rather slow
    std::vector<FileParser> getFileIndex();
    uint16_t getChunkSize();
    FILE *getFileHandle();
};
}  // namespace FCT
