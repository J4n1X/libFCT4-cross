// FCT-final-attempt.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.

#include <iostream>
#include <vector>
#include <algorithm>
#include "libFCT.h"

// TODO: rewrite
int createArchive(int argc, char** argv) {
	if (argc < 5) {
		std::cout << "Not enough arguments entered! Required: 3 or more." << std::endl;
		return errno;
	}

	FCT::FctArchive archive(argv[3], std::stoi(std::string(argv[2])));
	std::cout << "Create Mode" << std::endl;
	std::cout << "-----------" << std::endl;
	for (int i = 4; i < argc; i++) {
		std::string argString(argv[i]);
		std::filesystem::path rootDir = std::filesystem::path(argString).filename();
		if (std::filesystem::is_directory(argv[i])) {
			std::vector<std::filesystem::path> files = FCT::FS::expandDirectory(argv[i]);

			int temp;
			for (auto& it : files) {
				FCT::FileParser file;
				try {
					file = FCT::FileParser(it, rootDir, archive.getChunkSize());
				}
				catch (const char* msg) {
					std::cerr << msg << std::endl;
					continue;
				}
				std::cout << std::endl << file;
				std::cout << (!(archive.addFile(file)) ? ": Wrote file successfully\n" : ": Failed to write file\n");

			}
		}
		else if (std::filesystem::is_regular_file(argv[i])) {
			std::cout << std::endl;
			FCT::FileParser file;
			try {
				file = FCT::FileParser(argv[i], rootDir, archive.getChunkSize());
			}
			catch (const char* msg) {
				std::cout << msg << std::endl;
				continue;
			}
			std::cout << file << std::endl;
			archive.addFile(file);
		}
		else {
			return errno;
		}
	}
	std::cout << "File count: " << archive.getFileIndex().size() << std::endl;
	return 0;
}

int appendArchive(int argc, char** argv) {
	if (argc < 4) {
		std::cout << "Not enough arguments entered! Required: 2 or more." << std::endl;
		return errno;
	}
	std::cout << "Append Mode" << std::endl;
	std::cout << "-----------" << std::endl;

	FCT::FctArchive archive(argv[2]);
	std::string rootDir;
	std::vector<std::filesystem::path> files;
	std::vector<int> retVals;

	for (int i = 3; i < argc; i++) {
		if (std::filesystem::exists(argv[i])) {
			if (std::filesystem::is_regular_file(argv[i])) {
				files.push_back(std::filesystem::path(argv[i]));
			}
			else if (std::filesystem::is_directory(argv[i])) {
				std::vector<std::filesystem::path> directoryContent(FCT::FS::expandDirectory(argv[i]));
				files.insert(files.end(), directoryContent.begin(), directoryContent.end());
			}
		}
	}

	for (auto& it : files) {
		FCT::FileParser file;
		// if we're appending files, they are placed into the root for now, so
		// TODO: Better directory handling
		rootDir = std::filesystem::path(it).filename().string();
		try {
			file = FCT::FileParser(it, rootDir, archive.getChunkSize());
		}
		catch (const char* msg) {
			std::cerr << msg << std::endl;
			continue;
		}
		std::cout << file << std::endl;
		retVals.push_back(archive.addFile(file));
	}

	int ret = 0;
	for (auto& it : retVals)
		ret |= it;
	return ret;
}

int listArchive(int argc, char** argv) {
	if (argc < 3) {
		std::cout << "Not enough arguments entered! Required: 1 or more" << std::endl;
		return errno;
	}
	std::cout << "List Mode" << std::endl;
	std::cout << "---------" << std::endl;
	FCT::FctArchive archive(argv[2]);
	if (argc > 3) {
		for (int i = 3; i < argc; i++)
			std::cout << "File: " << argv[i] << std::endl << archive.getFileIndex()[std::stoi(argv[i])] << std::endl;
		std::cout << "Archive file count: " << archive.getFileIndex().size() << std::endl;
	}
	else {
		for (uint32_t i = 0; i < archive.getFileIndex().size(); i++) {
			std::cout << "File Nr. " << i + 1 << std::endl;
			std::cout << archive.getFileIndex()[i] << std::endl;
		}
	}
	return 0;
}

int extractArchive(int argc, char** argv) {
	if (argc < 4) {
		std::cout << "Not enough arguments entered! Required: 2 or more." << std::endl;
		return errno;
	}
	std::cout << "Extract Mode" << std::endl;
	std::cout << "------------" << std::endl;
	FCT::FctArchive archive(argv[2]);
	if (argc == 4) {
		int ret = 0;
		for (auto& iter : archive.extractAll(argv[3])) {
			ret += iter; // we'll return the amount of failed files
			if (iter == 1)
				std::cerr << "File Nr. " << iter << " failed to extract" << std::endl;
		}

		return ret;
	}
	else { // extract single files
		int returnVal = 0; // if an extraction fails, this number will increase by one
		std::vector<uint32_t> indices;
		for (int i = 4; i < argc; i++) {
			uint32_t val = std::stoi(std::string(argv[i]));
			if (val == 0) {
				std::cout << "Error: Please list indices starting from 1" << std::endl;
				return errno;
			}
			indices.push_back(val - 1); // we expect file 0 to be named as 1 and adjust if 0 is given
		}

		return archive.extractFiles(indices);
	}
}

int removeFromArchive(int argc, char** argv) {
	if (argc < 4) {
		std::cout << "Not enough arguments entered! Required: 2 or more." << std::endl;
		return errno;
	}

	std::cout << "Remove Mode" << std::endl;
	std::cout << "-----------" << std::endl;

	FCT::FctArchive archive(argv[2]);

	std::vector<uint32_t> indices;
	for (int i = 3; i < argc; i++) {
		indices.push_back(std::stoi(std::string(argv[i])));
		std::cout << archive.getFileIndex()[indices.back()] << std::endl;
	}


	return archive.removeFiles(indices);
}

int main(int argc, char** argv) {
	std::string programName = std::filesystem::path(argv[0]).filename().string();
	int retVal = 0;
	if (argc > 1) {		// TODO: Better error handling
		switch (argv[1][0]) {
		case 'a':
			retVal = appendArchive(argc, argv);
			break;
		case 'c':
			retVal = createArchive(argc, argv);
			break;
		case 'e':
			retVal = extractArchive(argc, argv); // TODO: Better extraction of specific files
			break;
		case 'l':
			retVal = listArchive(argc, argv);
			break;
		case 'r':
			retVal = removeFromArchive(argc, argv);
			break;
		case 'h':
		case '?':
			std::cout << "FCT File Container is an archival software used to pack files" << std::endl;
			std::cout << "Modes:" << std::endl;
			std::cout << "a - Append to archive. Usage: " << programName << " a <path to archive> <paths to files or directories>" << std::endl;
			std::cout << "c - Create archive. Usage: " << programName << " c <chunk size (max: " << FCT::MaxChunkSize << ")> <path to new archive> <paths to files or directories>" << std::endl;
			std::cout << "e - Extract from archive. Usage: " << programName << " e <path to archive> <output directory> <file indices (if none, all is extracted)>" << std::endl;
			std::cout << "h - Show help. Usage " << programName << " h" << std::endl;
			std::cout << "l - List archive contents Usage: " << programName << " l <path to archive> <file indices (if none, all is shown)>" << std::endl;
			break;
		default:
			std::cout << "Invalid mode selector: " << argv[1][0] << std::endl;
			break;
		}
	}
	else {
		std::cout << "Missing mode selector! Run " << programName << " h to display help." << std::endl;
		std::cout << "Press Enter to exit..." << std::endl;
		std::cin.get();
	}
	return retVal;
}