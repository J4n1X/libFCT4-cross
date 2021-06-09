#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <vector>

#include "libFCT/libFCT.h"

// TODO: rewrite
int createArchive(int argc, char **argv, bool verbose) {
    if (argc < 5) {
        std::cout << "Not enough arguments entered! Required: 3 or more."
                  << std::endl;
        return errno;
    }

    int retVal = 0;
    FCT::FctArchive archive(argv[3], std::stoi(std::string(argv[2])));
    std::cout << "Create Mode" << std::endl;
    std::cout << "-----------" << std::endl;
    std::vector<FCT::FileParser> fileParsers;
    for (int i = 4; i < argc; i++) {
        std::string argString(argv[i]);

        // Windows does some weird stuff with the trailing backslash,
        // interpreting it as an escape sequence
#if defined _WIN32
        if (argString.back() == '\"') argString.pop_back();
#elif defined __linux__
        if (argString.back() == FCT::PathDelimiter)  // remove trailing slash
            argString.pop_back();
#endif

        std::filesystem::path rootDir(argString);
        rootDir = std::filesystem::absolute(rootDir);

        if (std::filesystem::is_directory(argString)) {
            rootDir = std::filesystem::absolute(rootDir);
            std::vector<std::filesystem::path> files =
                FCT::FS::expandDirectory(argString);

            int temp;
            for (auto &it : files) {
                FCT::FileParser file;
                try {
                    file = FCT::FileParser(it, rootDir, archive.getChunkSize());
                } catch (const char *msg) {
                    std::cerr << msg << std::endl;
                    continue;
                }

                fileParsers.push_back(file);
            }
        } else if (std::filesystem::is_regular_file(argv[i])) {
            rootDir = std::filesystem::absolute(rootDir).parent_path();

            FCT::FileParser file;
            try {
                file =
                    FCT::FileParser(argv[i], rootDir, archive.getChunkSize());
            } catch (const char *msg) {
                std::cout << msg << std::endl;
                continue;
            }

            fileParsers.push_back(file);
        } else {
            return errno;
        }
    }
    for(auto &it : fileParsers){
         if (verbose)
                std::cout << FCT::printFileVerbose(it) << std::endl;
            else
                std::cout << it << std::endl;
        retVal += archive.addFile(it);
    }
    std::cout << "File count: " << archive.getFileIndex().size() << std::endl;
    return 0;
}

int appendArchive(int argc, char **argv, bool verbose) {
    if (argc < 4) {
        std::cout << "Not enough arguments entered! Required: 2 or more."
                  << std::endl;
        return errno;
    }
    std::cout << "Append Mode" << std::endl;
    std::cout << "-----------" << std::endl;

    int retVal = 0;
    FCT::FctArchive archive(argv[2]);
        std::vector<FCT::FileParser> fileParsers;
    for (int i = 3; i < argc; i++) {
        std::string argString(argv[i]);

        // Windows does some weird stuff with the trailing backslash,
        // interpreting it as an escape sequence
#if defined _WIN32
        if (argString.back() == '\"') argString.pop_back();
#elif defined __linux__
        if (argString.back() == FCT::PathDelimiter)  // remove trailing slash
            argString.pop_back();
#endif

        std::filesystem::path rootDir(argString);
        rootDir = std::filesystem::absolute(rootDir);

        if (std::filesystem::is_directory(argString)) {
            rootDir = std::filesystem::absolute(rootDir);
            std::vector<std::filesystem::path> files =
                FCT::FS::expandDirectory(argString);

            int temp;
            for (auto &it : files) {
                FCT::FileParser file;
                try {
                    file = FCT::FileParser(it, rootDir, archive.getChunkSize());
                } catch (const char *msg) {
                    std::cerr << msg << std::endl;
                    continue;
                }

                fileParsers.push_back(file);
            }
        } else if (std::filesystem::is_regular_file(argv[i])) {
            rootDir = std::filesystem::absolute(rootDir).parent_path();

            FCT::FileParser file;
            try {
                file =
                    FCT::FileParser(argv[i], rootDir, archive.getChunkSize());
            } catch (const char *msg) {
                std::cout << msg << std::endl;
                continue;
            }

            fileParsers.push_back(file);
        } else {
            return errno;
        }
    }
    for(auto &it : fileParsers){
         if (verbose)
                std::cout << FCT::printFileVerbose(it) << std::endl;
            else
                std::cout << it << std::endl;
        retVal += archive.addFile(it);
    }
    std::cout << "File count: " << archive.getFileIndex().size() << std::endl;
    std::cout << "Sorting..." << std::endl;
    archive.sortFiles(verbose);

    return retVal;
}

int listArchive(int argc, char **argv, bool verbose) {
    if (argc < 3) {
        std::cout << "Not enough arguments entered! Required: 1 or more"
                  << std::endl;
        return errno;
    }
    std::cout << "List Mode" << std::endl;
    std::cout << "---------" << std::endl;
    FCT::FctArchive archive(argv[2]);
    if (argc > 3) {
        for (int i = 3; i < argc; i++)
            std::cout << "File: " << argv[i] << std::endl
                      << archive.getFileIndex()[std::stoi(argv[i])]
                      << std::endl;
        std::cout << "Archive file count: " << archive.getFileIndex().size()
                  << std::endl;
    } else {
        for (uint32_t i = 0; i < archive.getFileIndex().size(); i++) {
            if (verbose)
                std::cout << i + 1 << ": "
                          << FCT::printFileVerbose(archive.getFileIndex()[i])
                          << std::endl;
            else
                std::cout << i + 1 << ": " << archive.getFileIndex()[i]
                          << std::endl;
        }
    }
    return 0;
}

int extractArchive(int argc, char **argv, bool verbose) {
    if (argc < 4) {
        std::cout << "Not enough arguments entered! Required: 2 or more."
                  << std::endl;
        return errno;
    }
    std::cout << "Extract Mode" << std::endl;
    std::cout << "------------" << std::endl;
    FCT::FctArchive archive(argv[2]);
    if (argc == 4) {
        int ret = 0;
        for (auto &iter : archive.extractAll(argv[3], verbose)) {
            ret += iter;  // we'll return the amount of failed files
            if (iter == 1)
                std::cerr << "File Nr. " << iter << " failed to extract"
                          << std::endl;
        }

        return ret;
    } else {  // extract single files
        // if an extraction fails, this number will increase by one
        int returnVal = 0;
        std::vector<uint32_t> indices;
        for (int i = 4; i < argc; i++) {
            uint32_t val = std::stoi(std::string(argv[i]));
            if (val == 0) {
                std::cout << "Error: Please list indices starting from 1"
                          << std::endl;
                return errno;
            }
            indices.push_back(val - 1);  // we expect file 0 to be named as 1
                                         // and adjust if 0 is given
        }

        return archive.extractFiles(argv[3], indices, verbose);
    }
}

int removeFromArchive(int argc, char **argv, bool verbose) {
    if (argc < 4) {
        std::cout << "Not enough arguments entered! Required: 2 or more."
                  << std::endl;
        return errno;
    }

    std::cout << "Remove Mode" << std::endl;
    std::cout << "-----------" << std::endl;

    FCT::FctArchive archive(argv[2]);

    std::vector<uint32_t> indices;
    for (int i = 3; i < argc; i++)
        indices.push_back(std::stoi(std::string(argv[i])));

    return archive.removeFiles(indices, verbose);
}

int sortArchive(int argc, char **argv, bool verbose) {
    if (argc < 3) {
        std::cout << "Not enough arguments entered! Required: 1" << std::endl;
        return errno;
    }
    std::cout << "Sort Mode" << std::endl;
    std::cout << "---------" << std::endl;
    FCT::FctArchive archive(argv[2]);

    return archive.sortFiles(verbose, true);
}

int main(int argc, char **argv) {
    std::string programName =
        std::filesystem::path(argv[0]).filename().string();
    int retVal = 0;

    if (argc > 1) {  // TODO: Better error handling
        bool verbose = (argv[1][1] == 'v') ? true : false;
        switch (argv[1][0]) {
            case 'a':
                retVal = appendArchive(argc, argv, verbose);
                break;
            case 'c':
                retVal = createArchive(argc, argv, verbose);
                break;
            case 'e':
                retVal = extractArchive(
                    argc, argv,
                    verbose);  // TODO: Better extraction of specific files
                break;
            case 'l':
                retVal = listArchive(argc, argv, verbose);
                break;
            case 'r':
                retVal = removeFromArchive(argc, argv, verbose);
                break;
            case 's':
                retVal = sortArchive(argc, argv, verbose);
                break;
            case 'h':
            case '?':
                std::cout << "FCT File Container is an archival software used "
                             "to pack files"
                          << std::endl;
                std::cout << "Modes:" << std::endl;
                std::cout << "a - Append to archive. Usage: " << programName
                          << " a(v)(s) <path to archive> <paths to files or "
                             "directories>"
                          << std::endl;
                std::cout << "c - Create archive. Usage: " << programName
                          << " c <chunk size (max: " << FCT::MaxChunkSize
                          << ")> <path to new archive> <paths to files or "
                             "directories>"
                          << std::endl;
                std::cout << "e - Extract from archive. Usage: " << programName
                          << " e <path to archive> <output directory> <file "
                             "indices (if none, all is extracted)>"
                          << std::endl;
                std::cout << "h - Show help. Usage: " << programName << " h"
                          << std::endl;
                std::cout << "l - List archive contents Usage: " << programName
                          << " l <path to archive> <file indices (if none, all "
                             "is shown)>"
                          << "s - Sort the archive files. Usage: "
                          << programName << "s <path to archive>" << std::endl;
                std::cout
                    << "v - Can be added to all file modes for verbose output"
                    << std::endl;
                break;
            default:
                std::cout << "Invalid mode selector: " << argv[1][0]
                          << std::endl;
                break;
        }
    } else {
        std::cout << "Missing mode selector! Run " << programName
                  << " h to display help." << std::endl;
// I'll just boldly assume that Windows users sometimes are not clever enough to
// notice that this is a console program
#if defined _WIN32
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
#endif
    }
    return retVal;
}
