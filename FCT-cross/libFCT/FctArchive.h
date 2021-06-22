#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "FileParser.h"

namespace FCT
{
#if defined _WIN32
    constexpr char PathDelimiter = '\\';
#elif defined __linux__
    constexpr char PathDelimiter = '/';
#endif
    constexpr int MaxChunkSize = 65535;
    constexpr int ArchiveHeaderSize = 5;

    
    int writeFile(FILE *archivePtr, FILE *filePtr, FileParser file);

    class FctArchive
    {
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

        int readFile(FileParser file);
        void writeFile(FileParser file, FILE *filePtr);
        int writeFile(FileParser file, std::string extractDir);

    public:
        std::filesystem::path ArchivePath;
        FctArchive(std::string archivePath);
        FctArchive(std::string newArchivePath, uint16_t chunkSize);
        ~FctArchive();

        int addFiles(std::vector<FileParser> files, bool verbose);
        int removeFiles(std::vector<uint32_t> indices, bool verbose);

        int extractFiles(std::string outputPath, std::vector<uint32_t> indices,
                         bool verbose);
        int extractAll(std::string outputPath, bool verbose);

        void generateFileIndex();

        // only use outside of class methods as this is rather slow
        std::vector<FileParser> getFileIndex();
        uint16_t getChunkSize();
        FILE *getFileHandle();
    };
} // namespace FCT
