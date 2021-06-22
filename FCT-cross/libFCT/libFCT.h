// This is the explanation for the FCT file container system
// Instead of a true Folder system, there will be a pseudo folder system based on filepaths.
// Important variables are:
// int64_t FileSize (regular filesize, not found in header)
// uin16_t ChunkSize (size of chunk in bytes)
// uint32_t ChunkCount (amount of Chunks in file)
// uint16_t LastChunkContentSize (size of content in last, not fully filled chunk)
// file size can be calculated by doing ChunkCount * ChunkSize - (ChunkSize - LastChunkContentSize)
// file size can be calculated by doing ChunkCount * ChunkSize + LastChunkContentSize

// FORMAT EXPLANATION: ARCHIVE HEADER
// The archive header is incredibly simple:
// "FCT" (3 bytes)
// ChunkSize (2 bytes)

// FORMAT EXPLANATION: FILE HEADER
// The Header will have a format that allows for dynamic chunk size. This is the only exception to the fixed chunk rule.
// Maximum header size in bytes is 65535 bytes
// NOTE: This format aims for simplicity of programming, not for efficiency
// Format will be as follows:
// ChunkCount (4 bytes)
// LastChunkContentSize (2 bytes)
// FilePathSize (2 bytes)
// FormattedFilePath (up to 65526 bytes)
//
// If a chunk does not fit inside chunks without a remainder, then that incomplete chunk is not added to the ChunkCount total and will instead be identified by a non-zero remainder

// BIG TODO: Error handling with nice exceptions that don't cause problems all the time.

#pragma once

#include "FSoperations.h"
#include "FctArchive.h"
#include "FileParser.h"