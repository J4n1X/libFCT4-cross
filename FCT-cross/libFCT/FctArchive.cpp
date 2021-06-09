#include "FctArchive.h"

#include <algorithm>
#include <cstring>
#include <fstream>

#include "FSoperations.h"

namespace FCT {
#pragma region Constructors &Destructor
// Create a new archive
FctArchive::FctArchive(std::string newArchivePath, uint16_t chunkSize) {
    ArchivePath = std::filesystem::path(newArchivePath);
    ChunkSize = chunkSize;
    FileIndexStale = false;
    if (FCT::FS::fileOpen(&ArchiveFile, ArchivePath.string().c_str(), "wb+"))
        throw "Failed to open archive";
    std::vector<char> temp;
    uint8_t fctHeader[ArchiveHeaderSize] = {'F', 'C', 'T', (uint8_t)ChunkSize,
                                            (uint8_t)(ChunkSize >> 8)};
    fwrite(fctHeader, 1, ArchiveHeaderSize, ArchiveFile);
}

// Open existing archive
FctArchive::FctArchive(std::string archivePath) {
    ArchivePath = std::filesystem::path(archivePath);
    if (FCT::FS::fileOpen(&ArchiveFile, ArchivePath.string().c_str(), "rb+"))
        throw "Failed to open archive";

    fseek(ArchiveFile, 3, SEEK_SET);

    fread(&ChunkSize, 2, 1, ArchiveFile);

    generateFileIndex();
}

FctArchive::~FctArchive() { fclose(ArchiveFile); }
#pragma endregion

void FctArchive::seekFileHeader(FileParser file) {
    fseek(ArchiveFile, file.Header.size(), SEEK_CUR);
}

void FctArchive::seekFileData(FileParser file) {
    // this seek supports 64 bit numbers!
    int64_t seekOffset = (uint64_t)file.ChunkCount * (uint64_t)ChunkSize;
    if (seekOffset > -1)
#if defined _WIN32
        _fseeki64(ArchiveFile, seekOffset, SEEK_CUR);
#elif defined __linux__
        fseeko64(ArchiveFile, seekOffset, SEEK_CUR);
#endif
    else  // fall back to save seek which will most likely be able to handle the
          // file
        for (uint32_t i = 0; i < file.ChunkCount; i++)
            fseek(ArchiveFile, ChunkSize, SEEK_CUR);
}

void FctArchive::seekFile(FileParser file) {
    fseek(ArchiveFile, file.Header.size(), SEEK_CUR);
    seekFileData(file);
}

int FctArchive::addFile(FileParser file) {
    // check if the file is valid first
    if (file.FileSize < 0) {
        return errno;
    }

    FILE *inFile;
    if (FCT::FS::fileOpen(&inFile, file.RawFilePath.string().c_str(), "rb"))
        return errno;

    // write header first
    std::vector<uint8_t> header = file.Header;
    fseek(ArchiveFile, 0, SEEK_END);
    fwrite(header.data(), 1, header.size(), ArchiveFile);

    // now write over file
    char *buffer = new char[ChunkSize];

    // This can be done faster by writing all at once and then filling up
    while (true) {
        memset(buffer, 0, ChunkSize);
        if (fread(buffer, 1, ChunkSize, inFile) == 0) break;
        fwrite(buffer, 1, ChunkSize, ArchiveFile);
    }

    fclose(inFile);
    delete[] buffer;
    FileIndexStale = true;
    return 0;
}

// basically like extractFiles but everything except the indices is written to a
// new file
int FctArchive::removeFiles(std::vector<uint32_t> indices, bool verbose) {
    FILE *tmpFile;
    char *buffer = new char[ChunkSize];
    std::filesystem::path tmpPath =
        std::filesystem::path(ArchivePath.generic_string() + ".tmp");
    FCT::FS::fileOpen(&tmpFile, tmpPath.string().c_str(), "wb+");
    if (tmpFile == NULL) return errno;

    if (indices.size() > 1)
        std::sort(indices.begin(), indices.end(),
                  [](int a, int b) { return a > b; });

    rewind(ArchiveFile);

    char *headerBuffer = new char[5];
    fread(headerBuffer, 1, ArchiveHeaderSize, ArchiveFile);
    fwrite(headerBuffer, 1, ArchiveHeaderSize, tmpFile);
    delete[] headerBuffer;

    FileParser file;
    uint32_t indexSize = FileIndex.size();
    for (uint32_t i = 0; i < indexSize; i++) {
        file = FileIndex[i];

        if (!indices.empty() && i == indices.back()) {
            if (verbose)
                std::cout << FCT::printFileVerbose(file) << std::endl;
            else
                std::cout << file << std::endl;
            seekFile(file);
            indices.pop_back();
        } else {
            fwrite(file.Header.data(), 1, file.Header.size(), tmpFile);
            for (uint32_t j = 0; j < file.ChunkCount; j++) {
                fread(buffer, 1, ChunkSize, ArchiveFile);
                fwrite(buffer, 1, ChunkSize, tmpFile);
            }
        }
    }
    delete[] buffer;
    FileIndexStale = true;
    fclose(tmpFile);
    fclose(ArchiveFile);

    std::filesystem::remove(ArchivePath);
    std::filesystem::rename(std::filesystem::path(tmpPath),
                            std::filesystem::path(ArchivePath));
    FCT::FS::fileOpen(&ArchiveFile, ArchivePath.string().c_str(), "rb+");
    if (ArchiveFile == NULL) return errno;

    return 0;
}

int FctArchive::extractFiles(std::string outputPath,
                             std::vector<uint32_t> indices, bool verbose) {
    fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);

    if (indices.size() > 1)
        std::sort(indices.begin(), indices.end(),
                  [](int a, int b) { return a > b; });

    uint32_t curIndex = 0;
    char *buffer = new char[ChunkSize];
    FileParser file;
    FILE *curFile;

    for (uint32_t i = 0; i < FileIndex.size(); i++) {
        file = FileIndex[i];
        if (!indices.empty() && i == indices.back()) {
            if (verbose)
                std::cout << FCT::printFileVerbose(file) << std::endl;
            else
                std::cout << file << std::endl;

            std::filesystem::path extractPath = outputPath;
            extractPath.append(file.FormattedFilePath);
            FCT::FS::createNewDirectory(extractPath);
            FCT::FS::fileOpen(&curFile, extractPath.generic_string().c_str(),
                              "wb");
            if (curFile == NULL) return errno;

            seekFileHeader(file);
            for (uint32_t j = 0; j < (file.ChunkCount - 1); j++) {
                fread(buffer, 1, ChunkSize, ArchiveFile);
                fwrite(buffer, 1, ChunkSize, curFile);
            }
            // I believe this is faster than a conditional declaration
            int lastWriteSize = file.LastChunkContentSize == 0
                                    ? ChunkSize
                                    : file.LastChunkContentSize;
            fread(buffer, 1, ChunkSize, ArchiveFile);
            fwrite(buffer, 1, lastWriteSize, curFile);

            fclose(curFile);
            indices.pop_back();
        } else {
            seekFile(file);
        }
    }

    delete[] buffer;
    return 0;
}

std::vector<int> FctArchive::extractAll(std::string outputPath, bool verbose) {
    fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);
    std::vector<int> returnVals;
    FILE *outFile;
    char *buffer = new char[ChunkSize];

    // TODO: Rewrite to use FileIndex
    if (FileIndexStale) generateFileIndex();
    for (auto &file : FileIndex) {
        if (verbose)
            std::cout << FCT::printFileVerbose(file) << std::endl;
        else
            std::cout << file << std::endl;
        std::filesystem::path extractPath = outputPath;
        extractPath.append(file.FormattedFilePath);
        FCT::FS::createNewDirectory(
            extractPath);  // creates directory if it doesn't exist

        FCT::FS::fileOpen(&outFile, extractPath.string().c_str(), "wb");

        if (outFile == NULL) {
            returnVals.push_back(1);
            continue;
        }

        seekFileHeader(file);
        if (file.ChunkCount > 0) {
            for (uint32_t i = 0; i < (file.ChunkCount - 1); i++) {
                fread(buffer, 1, ChunkSize, ArchiveFile);
                fwrite(buffer, 1, ChunkSize, outFile);
            }
            fread(buffer, 1, ChunkSize, ArchiveFile);
            fwrite(buffer, 1, file.LastChunkContentSize, outFile);
        }

        fclose(outFile);
        returnVals.push_back(0);
    }
    delete[] buffer;

    return returnVals;
}

// rebuild indexed File info
void FctArchive::generateFileIndex() {
    FileIndex.clear();
    int counter = 1;
    fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);
    while (true) {
        int c = fgetc(ArchiveFile);
        if (feof(ArchiveFile))
            break;
        else
            ungetc(c, ArchiveFile);

        FileParser currentFile;
        try {
            currentFile = FileParser(ArchiveFile, ChunkSize);
        } catch (const char *msg) {
            std::cerr << msg << std::endl;
        }
        FileIndex.push_back(currentFile);
        seekFileData(currentFile);
    }
    FileIndexStale = false;
}

std::vector<FileParser> FctArchive::getFileIndex() {
    if (FileIndexStale == true) {
        generateFileIndex();
        FileIndexStale = false;
    }
    return FileIndex;
}

uint16_t FctArchive::getChunkSize() { return ChunkSize; }

FILE *FctArchive::getFileHandle() { return this->ArchiveFile; }
}  // namespace FCT