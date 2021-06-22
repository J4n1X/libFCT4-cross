#include "FctArchive.h"
#include "FSoperations.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace FCT
{

#pragma region Constructors &Destructor
    // Create a new archive
    FctArchive::FctArchive(std::string newArchivePath, uint16_t chunkSize)
    {
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
    FctArchive::FctArchive(std::string archivePath)
    {
        ArchivePath = std::filesystem::path(archivePath);
        if (FCT::FS::fileOpen(&ArchiveFile, ArchivePath.string().c_str(), "rb+"))
            throw "Failed to open archive";

        fseek(ArchiveFile, 3, SEEK_SET);

        fread(&ChunkSize, 2, 1, ArchiveFile);

        generateFileIndex();
    }

    FctArchive::~FctArchive() { fclose(ArchiveFile); }
#pragma endregion

    void FctArchive::seekFileHeader(FileParser file)
    {
        fseek(ArchiveFile, file.Header.size(), SEEK_CUR);
    }

    void FctArchive::seekFileData(FileParser file)
    {
        // this seek supports 64 bit numbers!
        int remainderChunk = (file.LastChunkContentSize ? 1 : 0);
        int64_t seekOffset = ((uint64_t)file.ChunkCount + remainderChunk) * (uint64_t)ChunkSize;
        if (seekOffset > -1)
        {
#if defined _WIN32
            _fseeki64(ArchiveFile, seekOffset, SEEK_CUR);
#elif defined __linux__
            fseeko64(ArchiveFile, seekOffset, SEEK_CUR);
#endif
        }
        // fall back to save seek which will most likely be able to handle the file
        else
        {
            for (uint64_t i = 0; i < (file.ChunkCount + remainderChunk); i++)
                fseek(ArchiveFile, ChunkSize, SEEK_CUR);
        }
    }

    void FctArchive::seekFile(FileParser file)
    {
        fseek(ArchiveFile, file.Header.size(), SEEK_CUR);
        seekFileData(file);
    }

    int FctArchive::readFile(FileParser file)
    {
        if (file.FileSize < 0)
        {
            return 1;
        }

        FILE *inFile;
        if (FCT::FS::fileOpen(&inFile, file.RawFilePath.string().c_str(), "rb"))
            return 1;

        // write header first
        fseek(ArchiveFile, 0, SEEK_END);
        fwrite(file.Header.data(), 1, file.Header.size(), ArchiveFile);

        // now write over file
        char *buffer = new char[ChunkSize];

        // This can be done faster by writing all at once and then filling up
        while (true)
        {
            memset(buffer, 0, ChunkSize);
            if (fread(buffer, 1, ChunkSize, inFile) == 0)
                break;
            fwrite(buffer, 1, ChunkSize, ArchiveFile);
        }

        fclose(inFile);
        delete[] buffer;
        FileIndexStale = true;
        return 0;
    }

    void FctArchive::writeFile(FileParser file, FILE *filePtr)
    {
        char *buffer = new char[ChunkSize];

        seekFileHeader(file);
        if (file.ChunkCount > 0)
        {
            for (uint32_t i = 0; i < file.ChunkCount; i++)
            {
                fread(buffer, 1, ChunkSize, ArchiveFile);
                fwrite(buffer, 1, ChunkSize, filePtr);
            }
            fread(buffer, 1, ChunkSize, ArchiveFile);
            fwrite(buffer, 1, file.LastChunkContentSize, filePtr);
        }
        delete[] buffer;
    }

    int FctArchive::writeFile(FileParser file, std::string outputPath)
    {
        FILE *filePtr;
        std::filesystem::path extractPath = outputPath;
        extractPath.append(file.FormattedFilePath);
        FCT::FS::createNewDirectory(extractPath); // creates directory if it doesn't exist
        if (FCT::FS::fileOpen(&filePtr, extractPath.string().c_str(), "wb+"))
            return 1;
        
        writeFile(file, filePtr);
        fclose(filePtr);
        return 0;
    }

    int FctArchive::addFiles(std::vector<FileParser> files, bool verbose)
    {
        int retVal = 0;
        for (auto &file : files)
        {
            if (verbose)
                std::cout << FCT::printFileVerbose(file) << std::endl;
            else
                std::cout << file << std::endl;
            retVal += readFile(file);
        }
        return retVal;
    }

    // basically like extractFiles but everything except the indices is written to a
    // new file
    int FctArchive::removeFiles(std::vector<uint32_t> indices, bool verbose)
    {
        FILE *tmpFile;
        char *buffer = new char[ChunkSize];
        std::filesystem::path tmpPath =
            std::filesystem::path(ArchivePath.generic_string() + ".tmp");
        FCT::FS::fileOpen(&tmpFile, tmpPath.string().c_str(), "wb+");
        if (tmpFile == NULL)
            return 1;

        if (indices.size() > 1)
            std::sort(indices.begin(), indices.end(),
                      [](int a, int b)
                      { return a > b; });

        rewind(ArchiveFile);

        char *headerBuffer = new char[5];
        fread(headerBuffer, 1, ArchiveHeaderSize, ArchiveFile);
        fwrite(headerBuffer, 1, ArchiveHeaderSize, tmpFile);
        delete[] headerBuffer;

        FileParser file;
        uint32_t indexSize = FileIndex.size();
        for (uint32_t i = 0; i < indexSize; i++)
        {
            file = FileIndex[i];

            if (!indices.empty() && i == indices.back())
            {
                if (verbose)
                    std::cout << FCT::printFileVerbose(file) << std::endl;
                else
                    std::cout << file << std::endl;
                seekFile(file);
                indices.pop_back();
            }
            else
            {
                fwrite(file.Header.data(), 1, file.Header.size(), tmpFile);
                // illegal trick to save some time. Since the index goes stale, this won't even be a problem.
                file.LastChunkContentSize = ChunkSize;
                writeFile(file, tmpFile);
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
        if (ArchiveFile == NULL)
            return 1;

        return 0;
    }

    int FctArchive::extractFiles(std::string outputPath,
                                 std::vector<uint32_t> indices, bool verbose)
    {
        fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);

        if (indices.size() > 1)
            std::sort(indices.begin(), indices.end(),
                      [](int a, int b)
                      { return a > b; });

        int retVal = 0;
        uint32_t curIndex = 0;

        if (FileIndexStale)
            generateFileIndex();
        for (uint32_t i = 0; i < FileIndex.size(); i++)
        {
            auto &file = FileIndex[i];
            if (!indices.empty() && i == indices.back())
            {
                if (verbose)
                    std::cout << FCT::printFileVerbose(file) << std::endl;
                else
                    std::cout << file << std::endl;

                retVal += writeFile(file, outputPath);

                indices.pop_back();
            }
            else
            {
                seekFile(file);
            }
        }
        return retVal;
    }

    int FctArchive::extractAll(std::string outputPath, bool verbose)
    {
        fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);
        int returnVal;

        // TODO: Rewrite to use FileIndex
        if (FileIndexStale)
            generateFileIndex();
        for (auto &file : FileIndex)
        {
            if (verbose)
                std::cout << FCT::printFileVerbose(file) << std::endl;
            else
                std::cout << file << std::endl;

            writeFile(file, outputPath);
        }
        return returnVal;
    }

    // rebuild indexed File info
    void FctArchive::generateFileIndex()
    {
        FileIndex.clear();
        int counter = 1;
        fseek(ArchiveFile, ArchiveHeaderSize, SEEK_SET);
        while (true)
        {
            int c = fgetc(ArchiveFile);
            if (feof(ArchiveFile))
                break;
            else
                ungetc(c, ArchiveFile);

            FileParser currentFile;
            try
            {
                currentFile = FileParser(ArchiveFile, ChunkSize);
            }
            catch (const char *msg)
            {
                std::cerr << msg << std::endl;
            }
            FileIndex.push_back(currentFile);
            seekFileData(currentFile);
        }
        FileIndexStale = false;
    }

    std::vector<FileParser> FctArchive::getFileIndex()
    {
        if (FileIndexStale == true)
        {
            generateFileIndex();
            FileIndexStale = false;
        }
        return FileIndex;
    }

    uint16_t FctArchive::getChunkSize() { return ChunkSize; }

    FILE *FctArchive::getFileHandle() { return this->ArchiveFile; }
} // namespace FCT